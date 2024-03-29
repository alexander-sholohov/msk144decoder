#pragma once
//
// Author: Alexander Sholohov <ra9yer@yahoo.com>
//
// License: MIT
//

#include <string>
#include <ctime>

int utc_as_wsjt_int_hhmmss();

int utc_as_wsjt_int_hhmm();

int seconds_since_midnight();

int seconds_since_launch(int tr_interval);

int seconds_to_next_launch(int tr_interval);

void create_dir_if_not_exists(std::string const& dirname);

void swap_endians_if_need(short *buf, size_t length);

std::tm utcnow();

std::string rtrim_copy(std::string s);
