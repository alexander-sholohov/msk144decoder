//
// Author: Alexander Sholohov <ra9yer@yahoo.com>
//
// License: MIT
//

#include "msk144_context.h"

#include <sstream>

//-----------------------------------------------------
MSK144Context::MSK144Context()
    : Context()
{
    spot_reporter_wsjt_mode = "msk144";
}

//-----------------------------------------------------
std::string MSK144Context::asString() const
{
    std::ostringstream info;
    info << Context::asString();
    info << "MSK144 center frequency: " << this->mskrtd_nrxfreq << " Hz" << std::endl;
    info << "MSK144 depth: " << this->mskrtd_ndepth << std::endl;

    return info.str();
}
