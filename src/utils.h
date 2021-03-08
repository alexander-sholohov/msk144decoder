#pragma once
//
// Author: Alexander Sholohov <ra9yer@yahoo.com>
//
// License: MIT
//

#include <string>
#include <ctime>

int utc_as_wsjt_int();

int seconds_since_launch(int tr_interval);

int seconds_to_next_launch(int tr_interval);

void create_dir_if_not_exists(std::string const& dirname);

std::tm utcnow();

std::string rtrim_copy(std::string s);
