#pragma once
//
// Author: Alexander Sholohov <ra9yer@yahoo.com>
//
// License: MIT
//

#include <string>

struct Context
{
    std::string wsjt_workdir = ".";
    //
    bool file_log_enabled = false;
    std::string file_log_workdir = ".";
    //
    bool sport_reporter_enabled = false;
    std::string spot_reporter_wsjt_mode = "none"; // "msk144" . Will be set in the nested constructors.
    std::string spot_reporter_magic_key = "";
    std::string sport_reporter_src = "";
    std::string spot_reporter_url = "http://192.168.1.200:9000/spotter/default/populate_spot";

    // helper methods
    std::string asString() const;
};
