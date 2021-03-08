#pragma once
//
// Author: Alexander Sholohov <ra9yer@yahoo.com>
//
// License: MIT
//

#include "context.h"
#include <string>

struct MSK144Context : public Context
{
    MSK144Context();
    // msk144
    int mskrtd_nrxfreq = 1500;
    int mskrtd_ndepth = 3;

    // helper methods
    std::string asString() const;
};
