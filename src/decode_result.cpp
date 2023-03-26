//
// Author: Alexander Sholohov <ra9yer@yahoo.com>
//
// License: MIT
//

#include "decode_result.h"
#include "utils.h"

#include <vector>
#include <iostream>
#include <sstream>
#include <iomanip>

//--------------------------------------------------------------------
void DecodeResult::initFromMSK144Result(std::string const& s)
{
    // example: "145618   0  0.6 1500 &  HELLO       "
    // from mskrtd.f90:
    // 1021 format(i6.6,i4,f5.1,i5,a4,a37,a1)

    this->original_line = s;

    if (s.empty())
        return;

    std::vector<std::string> patterns = { " &  ", " ^  " };
    size_t pos = std::string::npos;
    for (auto elm : patterns)
    {
        pos = s.find(elm);
        if (pos != std::string::npos)
            break;
    }

    // no markers found
    if (pos == std::string::npos)
        return;

    try 
    {
        this->utc_time = s.substr(0, 6);

        this->db_ratio = std::stoi(s.substr(6, 4));
        this->dt_shift = std::stof(s.substr(10, 15));
        this->frequency = std::stoi(s.substr(15, 20));
        this->message = rtrim_copy(s.substr(pos + 4));

        this->_result_valid = true;
    } 
    catch (std::exception& ex)
    {
        std::cerr << ex.what();
    }

}

//--------------------------------------------------------------------
void DecodeResult::initFromParams(int db_ratio, float dt_shift, float frequency, int utc_time, std::string const& message, std::string const& original_line)
{
    this->db_ratio = db_ratio;
    this->dt_shift = dt_shift;
    this->frequency = frequency;

    std::ostringstream buf_time;
    buf_time << std::setfill('0') << std::setw(4) << utc_time;
    this->utc_time = buf_time.str();

    this->message = message;
    this->original_line = original_line;

    this->_result_valid = true;
}

//--------------------------------------------------------------------
std::string DecodeResult::asString() const
{
    std::ostringstream buf;

    if (_result_valid) {
        buf << "DecodeResult: utc_time=" << utc_time
            << " db_ratio=" << db_ratio
            << " dt_shift=" << dt_shift
            << " frequency=" << frequency
            << " message='" << message
            << "'";
    }
    else {
        buf << "DecodeResult: no-valid-result";
    }

    return buf.str();
}
