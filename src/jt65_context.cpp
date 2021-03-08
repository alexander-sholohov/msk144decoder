//
// Author: Alexander Sholohov <ra9yer@yahoo.com>
//
// License: MIT
//

#include "jt65_context.h"

#include <sstream>

//-----------------------------------------------------
JT65Context::JT65Context()
    : Context()
{
    spot_reporter_wsjt_mode = "jt65";
}


//-----------------------------------------------------
std::string JT65Context::asString() const
{
    std::ostringstream info;
    info << Context::asString();
    info << "JT65 submode: " << this->jt65_submode << "(" << static_cast<char>('A' + this->jt65_submode) << ")" << std::endl;
    info << "JT65 depth: " << this->jt65_depth << std::endl;
    if (this->immediate_read) {
        info << "Immediate read mode activated." << std::endl;
    }
    else 
    {
        info << "JT65 T/R interval: " << this->tr_interval_in_seconds << " seconds" << std::endl;
    }

    return info.str();
}
