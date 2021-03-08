#pragma once
//
// Author: Alexander Sholohov <ra9yer@yahoo.com>
//
// License: MIT
//

#include "context.h"
#include <string>

struct JT65Context : public Context
{
    JT65Context();
    // jt65
    int jt65_submode = 0;
    int jt65_depth = 3;
    int tr_interval_in_seconds = 60;
    bool immediate_read = false; // useful for read one file as stream

    // helper methods
    std::string asString() const;
};
