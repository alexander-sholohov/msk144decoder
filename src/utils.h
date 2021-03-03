#pragma once
//
// Author: Alexander Sholohov <ra9yer@yahoo.com>
//
// License: MIT
//

#include <string>
#include <ctime>

int utc_as_wsjt_int();

void create_dir_if_not_exists(std::string const& dirname);

std::tm utcnow();

std::string rtrim_copy(std::string s);
