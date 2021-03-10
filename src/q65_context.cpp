//
// Author: Alexander Sholohov <ra9yer@yahoo.com>
//
// License: MIT
//

#include "q65_context.h"

#include <sstream>

//-----------------------------------------------------
Q65Context::Q65Context()
    : Context()
{
    spot_reporter_wsjt_mode = "q65";
}


//-----------------------------------------------------
std::string Q65Context::asString() const
{
    std::ostringstream info;
    info << Context::asString();
    info << "Q65 submode: " << this->q65_submode << "(" << static_cast<char>('A' + this->q65_submode) << ")" << std::endl;
    info << "Q65 depth: " << this->q65_depth << std::endl;
    if (this->immediate_read) {
        info << "Immediate read mode activated." << std::endl;
    }
    else 
    {
        info << "JT65 T/R interval: " << this->tr_interval_in_seconds << " seconds" << std::endl;
    }

    return info.str();
}
