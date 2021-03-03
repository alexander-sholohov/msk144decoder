#pragma once
//
// Author: Alexander Sholohov <ra9yer@yahoo.com>
//
// License: MIT
//

struct Context;
struct DecodeResult;

void http_reporter(Context const& ctx, DecodeResult const& decode_result);
