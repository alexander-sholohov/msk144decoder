//
// Author: Alexander Sholohov <ra9yer@yahoo.com>
//
// License: MIT
//

#include "http_reporter.h"
#include "context.h"
#include "decode_result.h"

#include <stdio.h>
#include <curl/curl.h>

#include <stdexcept>
#include <iostream>
#include <sstream>

/*
* This code post received data to service http://sdr.22dx.ru/spot  
*/

//---------------------------------------------------------------------------------------------
void http_reporter(Context const& ctx, DecodeResult const& dr)
{
    static bool g_inited = false;
    if (!g_inited) {
        g_inited = true;
        curl_global_init(CURL_GLOBAL_ALL);
    }

    CURL* curl = curl_easy_init();
    if (!curl) {
        throw std::logic_error("curl_easy_init() error");
    }

    curl_easy_setopt(curl, CURLOPT_URL, ctx.spot_reporter_url.c_str());
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);

    std::ostringstream post;
    post << "mode=" << ctx.spot_reporter_wsjt_mode;
    post << "&utc_time=" << dr.utc_hhmm;
    post << "&db_ratio=" << dr.db_ratio;
    post << "&dt_shift=" << dr.dt_shift;
    post << "&freq=" << dr.frequency;
    
    if (!ctx.sport_reporter_src.empty()) {
        post << "&src=" << ctx.sport_reporter_src;
    }
    if (!ctx.spot_reporter_magic_key.empty()) {
        post << "&magic_key=" << ctx.spot_reporter_magic_key;
    }

    {
        char* output = curl_easy_escape(curl, dr.message.c_str(), dr.message.length());
        if (output) {
            post << "&message=" << output;
            curl_free(output);
        }
    }

    // std::cout << "Url=" << ctx.spot_reporter_url << " request_string=" << post.str() << std::endl;

    std::string s1 = post.str();
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, s1.c_str());

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "curl_easy_perform() failed with code=" << res
            << " : " << curl_easy_strerror(res) 
            << ". url=" << ctx.spot_reporter_url
            << std::endl;
    }

    curl_easy_cleanup(curl);
}
