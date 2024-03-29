//
// Author: Alexander Sholohov <ra9yer@yahoo.com>
//
// License: MIT
//

#include "context.h"

#include <sstream>

//-----------------------------------------------------
std::string Context::asString() const
{
    std::ostringstream info;
    info << "Spot reporter service: ";
    if (this->sport_reporter_enabled) {
        info << "enabled. "
            << " url=" << this->spot_reporter_url;

        if (!this->sport_reporter_src.empty()) {
            info << " src=" << this->sport_reporter_src;
        }

        if (!this->spot_reporter_magic_key.empty()) {
            info << " magic_key=" << this->spot_reporter_magic_key;
        }

        info << std::endl;

    }
    else {
        info << "disabled." << std::endl;
    }

    info << "File logging: ";
    if (this->file_log_enabled) {
        info << "enabled. Workdir=" << this->file_log_workdir << std::endl;
    }
    else {
        info << "disabled." << std::endl;
    }

    return info.str();
}
