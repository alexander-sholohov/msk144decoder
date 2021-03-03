//
// Author: Alexander Sholohov <ra9yer@yahoo.com>
//
// License: MIT
//

#include "wrk_thread.h"
#include "context.h"
#include "wavfile.h"
#include "utils.h"
#include "http_reporter.h"


#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>

//---------------------------------------------------------------------------------------------
static void file_process(const Context& ctx, std::vector<short> const& stream, DecodeResult const& decode_result, size_t seq_no)
{
    std::time_t utc_time = std::time(nullptr);
    std::tm utc = *std::gmtime(&utc_time);

    std::ostringstream dirname;
    dirname << ctx.file_log_workdir << "/";
    dirname << std::setfill('0') << std::setw(4) << (1900 + utc.tm_year);
    dirname << std::setfill('0') << std::setw(2) << (1 + utc.tm_mon);

    create_dir_if_not_exists(dirname.str());

    std::ostringstream filename;
    filename << dirname.str() << "/";
    filename << std::setfill('0') << std::setw(2) << (utc.tm_mday);
    filename << std::setfill('0') << std::setw(2) << (utc.tm_hour);
    filename << std::setfill('0') << std::setw(2) << (utc.tm_min);
    filename << "_";
    filename << seq_no;

    // write text line
    {
        std::ofstream myfile;
        myfile.open(filename.str() + ".txt");
        myfile << decode_result.original_line << std::endl;
        myfile.close();
    }

    // write wav file
    WavFile::writeToFile(filename.str() + ".wav", stream);

}

//---------------------------------------------------------------------------------------------
void wrk_thread(const Context& ctx, std::vector<short> stream, DecodeResult decode_result, size_t seq_no)
{
    try
    {
        // std::cout << "wrk thread. work_dir=" << ctx.file_log_workdir << " " << decode_result.asString() << "  seq_no=" << seq_no << std::endl;

        if (ctx.file_log_enabled) {
            file_process(ctx, stream, decode_result, seq_no);
        }

        if (ctx.sport_reporter_enabled) {
            http_reporter(ctx, decode_result);
        }
    }
    catch (std::exception& ex)
    {
        std::cerr << "Exception in wrk_thread: " << ex.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "Unknown exception in wrk_thread." << std::endl;
    }

}
