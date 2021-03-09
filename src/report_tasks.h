#pragma once
//
// Author: Alexander Sholohov <ra9yer@yahoo.com>
//
// License: MIT
//

#include "decode_result.h"
#include <vector>
#include <string>

struct Context;

void report_tasks(const Context& ctx, std::vector<short> stream, DecodeResult decode_result, size_t seq_no);
