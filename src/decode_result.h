#pragma once
//
// Author: Alexander Sholohov <ra9yer@yahoo.com>
//
// License: MIT
//

#include <string>

struct DecodeResult
{
    void initFromMSK144Result(std::string const& s); // init from msk144 string
    void initFromParams(int db_ratio, float dt_shift, float frequency, int utc_time, std::string const& message, std::string const& original_line);

    std::string asString() const;
    bool isValid() const { return _result_valid; }

    bool _result_valid = false;
    std::string original_line;
    std::string utc_time;
    int db_ratio = 0.0;
    float dt_shift = 0.0;
    int frequency = 0.0;
    std::string message;

};
