//
// Author: Alexander Sholohov <ra9yer@yahoo.com>
//
// License: MIT
//

#include "wavfile.h"
#include "msk144_context.h"

#include "wrk_thread.h"
#include "utils.h"
#include "http_reporter.h"
#include "decode_result.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sched.h>
#include <unistd.h>
#include <getopt.h>

#if __GNUC__ > 7
#include <cstddef>
  typedef size_t fortran_charlen_t;
#else
  typedef int fortran_charlen_t;
#endif

#include <string>
#include <vector>
#include <iostream>
#include <thread>
#include <sstream>


extern "C" {
  // --- Fortran routines ---

  void mskrtd_(const short int id2[], int* nutc0, float* tsec, int* ntol, int* nrxfreq, int* ndepth, 
                char mycall[], char hiscall[], bool* bshmsg, bool* btrain,
                double const pcoeffs[], bool* bswl, char datadir[], char line[],
                fortran_charlen_t, fortran_charlen_t, fortran_charlen_t, fortran_charlen_t);

}

//-----------------------------------------------------------------------------------
static std::string call_mskrtd(const short int* buffer, MSK144Context const& ctx, int time_as_int, float shift_in_seconds)
{
    int nutc0 = time_as_int; // 10000*4 + 100*3 + (cnt * HALF_UNIT_SIZE / 12000);
    float tsec = shift_in_seconds; // cnt * HALF_UNIT_SIZE / 12000.0;
    int ntol = 100; // +-100Hz from peak
    int nrxfreq = ctx.mskrtd_nrxfreq;  // 1500
    int ndepth = ctx.mskrtd_ndepth;  // 3

    char mycall[12];
    char hiscall[12];
    char datadir[512];
    char line[80];


    mycall[0] = 0;
    hiscall[0] = 0;
    strcpy(datadir, ctx.wsjt_workdir.c_str()); // provide current dir for wsjt's software internal usage.
    memset(line, 0, sizeof(line));

    bool bshmsg = false;
    bool btrain = false;
    double pcoeffs[] = {0.0, 0.0, 0.0, 0.0, 0.0};
    bool bswl = false;

    mskrtd_(buffer, &nutc0, &tsec, &ntol, &nrxfreq, &ndepth, 
        mycall, hiscall, &bshmsg, &btrain,
        pcoeffs, &bswl, datadir, line,
        12, 12, 512, 80);

    return std::string(line);
}

//-------------------------------------------------------------------
static void usage(void)
{
    std::ostringstream buf;
    buf << "\nmsk144decoder - msk144 mode stream decoder. Accepts audio stream - 16 bits signed, 12000 samples per second, mono.\n\n";
    buf << "Usage:\t[--spot_reporter_url= Address of sport reporter service. (default: http://192.168.1.200:9000/spotter/default/populate_spot ]\n";
    buf << "\t[--spot_reporter_enable= Enable or not http report. true or 1 to enable (default: false)]\n";
    buf << "\t[--file_log_enable= Enable or not file logging. true or 1 to enable (default: false)]\n";
    buf << "\t[--file_log_workdir= Directory name where put file logs into. (default: \".\")]\n";
    buf << "\t[--rxfreq= Center of rx frequency (default: 1500)]\n";
    buf << "\t[--depth= How deep frames average. (default: 3)]\n";
    buf << "\t[--help this text]\n";

    std::cout << buf.str() << std::endl;

    exit(1);
}

//------------------------------------------------------------
static bool str2bool(const char* str)
{
    if (strcmp(str, "true") == 0)
        return true;

    if (strcmp(str, "1") == 0)
        return true;

    return false;
}

//------------------------------------------------------------
int main(int argc, char** argv)
{
    MSK144Context ctx;

    static struct option long_options[] = {
            {"help", no_argument, 0, 0}, // 0
            {"sport_reporter_src", required_argument, 0, 0}, // 1
            {"spot_reporter_url", required_argument, 0, 0}, // 2
            {"spot_reporter_magic_key", required_argument, 0, 0}, // 3
            {"spot_reporter_enable", required_argument, 0, 0}, // 4
            {"file_log_enable", required_argument, 0, 0}, // 5
            {"file_log_workdir", required_argument, 0, 0}, // 6
            {"rxfreq", required_argument, 0, 0}, // 7
            {"depth", required_argument, 0, 0}, // 8
            {0, 0, 0, 0}
    };

    while (true)
    {
        int option_index = 0;

        int c = getopt_long(argc, argv, "", long_options, &option_index);

        if (c == -1)
            break;

        if (c == 0)
        {
            switch (option_index)
            {
            case 0:
                usage();
                break;
            case 1:
                ctx.sport_reporter_src = optarg;
                break;
            case 2:
                ctx.spot_reporter_url = optarg;
                break;
            case 3:
                ctx.spot_reporter_magic_key = optarg;
                break;
            case 4:
                ctx.sport_reporter_enabled = str2bool(optarg);
                break;
            case 5:
                ctx.file_log_enabled = str2bool(optarg);
                break;
            case 6:
                ctx.file_log_workdir = optarg;
                break;
            case 7:
                ctx.mskrtd_nrxfreq = atoi(optarg);
                break;
            case 8:
                ctx.mskrtd_ndepth = atoi(optarg);
                break;
            default:
                usage();
            }
        }
    }

    // print Context info to stdout
    std::cout << ctx.asString() << std::endl;


    // Reserve space for the buffer.
    const size_t HALF_UNIT_SIZE = 7 * 512; // 
    std::vector<short> buffer(2 * HALF_UNIT_SIZE);

    std::cout << "MSK144decoder started." << std::endl;
 
    size_t success_seq_no = 0;

    while(true)
    {
        if (feof(stdin)) {
            std::cerr << "Got EOF" << std::endl;
            break;
        }

        std::copy(buffer.begin() + HALF_UNIT_SIZE, buffer.end(), buffer.begin());

        int rc = fread(&buffer[HALF_UNIT_SIZE], sizeof(short), HALF_UNIT_SIZE, stdin);
        if(rc != HALF_UNIT_SIZE) { std::cerr << "Incomplete read error. rc=" << rc; break; }

        int nutc0 = utc_as_wsjt_int(); // 10000*hh + 100*mm + ss;
        std::string line = call_mskrtd(&buffer[0], ctx, nutc0, 0.0);

        // Print RAW string from decoder to console with *** prefix at begin of line.
        if (!line.empty()) {
            std::cout << "*** " << line << std::endl;
        }

        // Run file and http reporter tasks in separate thread.
        DecodeResult decode_result(line);
        if(decode_result.isValid())
        {
            success_seq_no++;
            std::thread t(wrk_thread, ctx, buffer, decode_result, success_seq_no);
            t.detach();
        }

        sched_yield(); // Force give time to OS. Probably not very necessary.

    }

    ::usleep(1000 * 1000); // just 1 second delay before exit. 
    std::cout << "MSK144decoder exit." << std::endl;

    return 0;
}
