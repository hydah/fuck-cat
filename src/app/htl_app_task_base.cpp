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

#include <inttypes.h>
#include <stdlib.h>

#include <string>
#include <sstream>
using namespace std;

#include <htl_core_error.hpp>
#include <htl_core_log.hpp>
#include <htl_app_http_client.hpp>

#include <htl_app_task_base.hpp>

StBaseTask::StBaseTask(){
}

StBaseTask::~StBaseTask(){
}

int StBaseTask::InitializeBase(std::string http_url, double startup, double delay, double error, int count){
    int ret = ERROR_SUCCESS;
    
    if((ret = GetUri()->Initialize(http_url)) != ERROR_SUCCESS){
        return ret;
    }
    
    Info("task url(%s) parsed, startup=%.2f, delay=%.2f, error=%.2f, count=%d", http_url.c_str(), startup, delay, error, count);
    
    this->delay_seconds = delay;
    this->startup_seconds = startup;
    this->error_seconds = error;
    this->count = count;
    
    return ret;
}

int StBaseTask::Process(){
    int ret = ERROR_SUCCESS;
    
    if(startup_seconds > 0){
        int sleep_ms = StUtility::BuildRandomMTime(startup_seconds);
        Trace("start random sleep %dms", sleep_ms);
        st_usleep(sleep_ms * 1000);
    }
    
    if((ret = ProcessTask()) != ERROR_SUCCESS){
        return ret;
    }
    
    return ret;
}

