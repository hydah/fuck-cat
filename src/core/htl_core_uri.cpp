/*
The MIT License (MIT)

Copyright (c) 2013-2015 winlin

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <htl_stdinc.hpp>

#include <stdlib.h>

#include <string>
#include <sstream>
using namespace std;

#include <htl_core_log.hpp>
#include <htl_core_error.hpp>

#include <htl_core_uri.hpp>
#include <htl_utility.hpp>

Uri::Uri(){
}

Uri::~Uri(){
}

ProtocolUrl::ProtocolUrl(){
}

ProtocolUrl::~ProtocolUrl(){
}

int ProtocolUrl::Initialize(std::string http_url){
    int ret = ERROR_SUCCESS;

    url = http_url;
    const char* purl = url.c_str();

    if((ret = http_parser_parse_url(purl, url.length(), 0, &hp_u)) != 0){
        int code = ret;
        ret = ERROR_HP_PARSE_URL;

        Error("parse url %s failed, code=%d, ret=%d", purl, code, ret);
        return ret;
    }

    if(Get(UF_SCHEMA) != ""){
        schema = Get(UF_SCHEMA);
    }

    host = Get(UF_HOST);

    if(Get(UF_PORT) != ""){
        port = atoi(Get(UF_PORT).c_str());
    }

    path = Get(UF_PATH);

    if (Get(UF_QUERY) != "") {
        query = Get(UF_QUERY);
    }

    return ret;
}

const char* ProtocolUrl::GetUrl(){
    return url.c_str();
}

const char* ProtocolUrl::GetSchema(){
    return schema.c_str();
}

const char* ProtocolUrl::GetHost(){
    return host.c_str();
}

int ProtocolUrl::GetPort(){
    return port;
}

string ProtocolUrl::Get(http_parser_url_fields field){
    return HttpUrl::GetUriField(url, &hp_u, field);
}

string ProtocolUrl::GetUriField(string uri, http_parser_url* hp_u, http_parser_url_fields field){
    if((hp_u->field_set & (1 << field)) == 0){
        return "";
    }

    Verbose("uri field matched, off=%d, len=%d, value=%.*s",
        hp_u->field_data[field].off, hp_u->field_data[field].len, hp_u->field_data[field].len,
        uri.c_str() + hp_u->field_data[field].off);

    return uri.substr(hp_u->field_data[field].off, hp_u->field_data[field].len);
}

HttpUrl::HttpUrl(){
    port = 80;
}

HttpUrl::~HttpUrl(){
}

#define PROTOCOL_HTTP "http://"
#define PROTOCOL_HTTPS "https://"
string HttpUrl::Resolve(string origin_url){
    string copy = origin_url;

    size_t pos = string::npos;
    string key = "./";
    if((pos = origin_url.find(key)) == 0){
        copy = origin_url.substr(key.length());
    }

    // uri
    if(copy.find(PROTOCOL_HTTP) == 0 || copy.find(PROTOCOL_HTTPS) == 0){
        return copy;
    }

    // abs or relative url
    stringstream ss;
    ss << schema << "://" << host;

    if(port != 80){
        ss << ":" << port;
    }

    // relative path
    if(copy.find("/") != 0){
        string dir = path;

        if((pos = dir.rfind("/")) != string::npos){
            dir = dir.substr(0, pos);
        }

        ss << dir << "/";
    }

    ss << copy;

    return ss.str();
}

HttpUrl* HttpUrl::Copy(){
    HttpUrl* copy = new HttpUrl();

    copy->Initialize(url);

    return copy;
}

const char* HttpUrl::GetPath(){
    path_query = path;
    if (!query.empty()) {
        path_query += "?" + query;
    }
    return path_query.c_str();
}

RtmpUrl::RtmpUrl(){
    port = 1935;
}

RtmpUrl::~RtmpUrl(){
}

int RtmpUrl::Initialize(std::string http_url){
    int ret = ERROR_SUCCESS;

    size_t pos = std::string::npos;
    url = http_url;

    if ((pos = url.find("://")) != std::string::npos) {
        schema = url.substr(0, pos);
        url = url.substr(schema.length() + 3);
    }


    if ((pos = url.find("/")) != std::string::npos) {
        host = url.substr(0, pos);
        path = url.substr(pos);
    }

    string tmp_host = host;
    string tmp_port = "1935";
    // do not check the validation
    int length = tmp_host.length();
    // 127.0.0.1(:80) [::1](:80)
    pos = tmp_host.rfind(":");   // Look for ":" from the end, to work with IPv6.
    if (pos != std::string::npos) {
        //127.0.0.1:80 [::1]:80 [::1]
        if ((pos >= 1) && (tmp_host[0] == '[') && (tmp_host[pos - 1] == ']')) {
            // Handle IPv6 in RFC 2732 format, e.g. [3ffe:dead:beef::1]:1935
            host = tmp_host.substr(1, pos - 2);
            tmp_port = tmp_host.substr(pos + 1);
        } else if (tmp_host[0] == '[' && tmp_host[length-1] == ']'){
            // Handle IPv6 address
            host = tmp_host.substr(1, length-2);
        } else {
            host = tmp_host.substr(0, pos);
            tmp_port = tmp_host.substr(pos + 1);
        }
    }
    port = atoi(tmp_port.c_str());

    vhost = host;
    app = path.c_str() + 1;

    if((pos = app.find("/")) == string::npos){
        ret = ERROR_RTMP_URL;
        Error("invalid rtmp url, no stream found, ret=%d", ret);
        return ret;
    }

    stream = app.substr(pos + 1);
    app = app.substr(0, pos);

    if ((pos = stream.find("?")) != string::npos) {
        query = stream.substr(pos+1);
        stream = stream.substr(0, pos);
    }

    string tmp_param;
    if ((pos = app.find("?")) != std::string::npos) {
        tmp_param = app.substr(pos+1);
    }
    if (!tmp_param.empty()) {
        query = query.empty() ? tmp_param : "&" + tmp_param;
    }
    if (!query.empty()) {
        stream += "?" + query;
    }
    // filter tcUrl
    app = srs_string_replace(app, ",", "?");
    app = srs_string_replace(app, "...", "?");
    app = srs_string_replace(app, "&&", "?");
    app = srs_string_replace(app, "=", "?");

    if ((pos = app.find("?")) != std::string::npos) {
        std::string query = app.substr(pos + 1);
        app = app.substr(0, pos);

        if ((pos = query.find("vhost?")) != std::string::npos) {
            query = query.substr(pos + 6);
            if (!query.empty()) {
                vhost = query;
            }
            if ((pos = vhost.find("?")) != std::string::npos) {
                vhost = vhost.substr(0, pos);
            }
            if ((pos = vhost.find("&")) != std::string::npos) {
                vhost = vhost.substr(0, pos);
            }
        }
    }

    stringstream ss;
    ss << schema << "://" << vhost << ":" << port << "/" << app;
    tcUrl = ss.str();

    return ret;
}

const char* RtmpUrl::GetTcUrl(){
    return tcUrl.c_str();
}

const char* RtmpUrl::GetVhost(){
    return vhost.c_str();
}

const char* RtmpUrl::GetApp(){
    return app.c_str();
}

const char* RtmpUrl::GetStream(){
    return stream.c_str();
}

