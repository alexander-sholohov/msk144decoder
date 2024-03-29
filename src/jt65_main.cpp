//
// Author: Alexander Sholohov <ra9yer@yahoo.com>
//
// License: MIT
//

#include "wavfile.h"
#include "jt65_context.h"

#include "report_tasks.h"
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
#include <algorithm>

#define MY_FALSE (0)
#define MY_TRUE (1)

enum class WORKING_MODE
{
    IDLE,
    COLLECTING,
};

// GFortran interop trick.
struct GFortranContext
{
    void* data0 = &fortran_working_area[0];
    void* data1 = 0;
    std::vector<DecodeResult> decode_results;
    int utc_time = 0;

    size_t fortran_working_area[16];
};


extern "C" {
  // --- Fortran routines ---

    void __jt65_decode_MOD_decode(
        void* me,
        void* callback,
        const float dd0[], // 60*12000 samples max
        int* npts,
        int* newdat, // bool
        int* nutc,
        int* nf1,
        int* nf2,
        int* nfqso,
        int* ntol,
        int* nsubmode,
        int* minsync,
        int* nagain, // bool
        int* n2pass,
        int* nrobust, // bool
        int* ntrials,
        int* naggressive,
        int* ndepth,
        float* emedelay,
        int* clearave, // bool
        char* mycall, // 12
        char* hiscall, // 12
        char* hisgrid, // 6
        int* nexp_decode,
        int* nQSOProgress,
        int* ljt65apon, // bool
        fortran_charlen_t, fortran_charlen_t, fortran_charlen_t);

}

extern "C" void jt65_decode_callback(GFortranContext *  me,
    float* sync,
    int* snr,
    float* dt,
    int* freq,
    int* drift,
    int* nflip,
    float* width,
    char* decoded, // size of 22
    int* ft,
    int* qual,
    int* nsmo,
    int* nsum,
    int* minsync)
{
    std::ostringstream info;
    info << "sync=" << *sync
        << " snr=" << *snr
        << " dt=" << *dt
        << " freq=" << *freq
        << " drift=" << *drift
        << " nflip=" << *nflip
        << " width=" << *width
        << " decod=" << decoded
        << " ft=" << *ft
        << " qual=" << *qual
        << " nsmo=" << *nsmo
        << " nsum=" << *nsum
        << " minsync=" << *minsync;

    DecodeResult dr;
    dr.initFromParams(*snr, *dt, *freq, me->utc_time, rtrim_copy(decoded), info.str());
    me->decode_results.emplace_back(dr);

    std::cout << "*** " << info.str() << std::endl;
}

//-----------------------------------------------------------------------------------
static void call_jt65_decoder(std::vector<short> stream, int num_samples, JT65Context const& ctx, int utc_hhmm)
{
    const int JT65_EXPECTING_NUM_SAMPLES = 60 * 12000;
    std::vector<float> buffer(JT65_EXPECTING_NUM_SAMPLES);

    // get result length
    int len = std::min(static_cast<int>(stream.size()), num_samples);
    // convert samples to float buffer
    for (size_t idx = 0; idx < len && idx < buffer.size(); idx++)
    { 
        buffer[idx] = static_cast<float>( stream[idx] ); 
    }

    int npts = len;
    int newdat = MY_FALSE; // bool
    int nutc = utc_hhmm;
    int nf1 = 200;
    int nf2 = 4000;
    int nfqso = 0; // valid for special conditions
    int ntol = 1000; // ???
    int nsubmode = ctx.jt65_submode;// 0-2  = A-C
    int minsync = 0;
    int nagain = MY_FALSE; // bool
    int n2pass = 0;
    int nrobust = MY_FALSE; // bool
    int ntrials = 1;
    int naggressive = 0;
    int ndepth = ctx.jt65_depth;
    float emedelay = 0;
    int clearave = MY_TRUE; // bool
    char mycall[12]; // 12
    char hiscall[12]; // 12
    char hisgrid[6]; // 6
    int nexp_decode = 0;
    int nQSOProgress = 0;
    int ljt65apon = MY_FALSE; // bool

    memset(mycall, 0, sizeof(mycall));
    memset(hiscall, 0, sizeof(hiscall));
    memset(hisgrid, 0, sizeof(hisgrid));

    GFortranContext fortranContext;
    fortranContext.utc_time = utc_hhmm;

    void* callback = (void*)&jt65_decode_callback;

    __jt65_decode_MOD_decode(
        &fortranContext,
        callback,
        &buffer[0], // exact 60*12000 samples
        &npts, // ~51*12000 samples max
        &newdat, // bool
        &nutc,
        &nf1,
        &nf2,
        &nfqso,
        &ntol,
        &nsubmode,
        &minsync,
        &nagain, // bool
        &n2pass,
        &nrobust, // bool
        &ntrials,
        &naggressive,
        &ndepth,
        &emedelay,
        &clearave, // bool
        mycall, // 12
        hiscall, // 12
        hisgrid, // 6
        &nexp_decode,
        &nQSOProgress,
        &ljt65apon, // bool
        12, 12, 6);

    size_t seq_no = 0;
    for (DecodeResult const& dr : fortranContext.decode_results)
    {
        if (dr.isValid())
        {
            // call as normal function, not a thread
            report_tasks(ctx, std::move(stream), dr, seq_no);
            seq_no++;
        }
    }

}

//-------------------------------------------------------------------
static void usage(void)
{
    std::ostringstream buf;
    buf << "\njt65decoder - jt65 digital mode stream decoder. Accepts audio stream - 16 bits signed, 12000 samples per second, mono.\n\n";
    buf << "Usage:\t[--spot-reporter-url= Address of spot reporter service. (default: http://192.168.1.200:9000/spotter/default/populate_spot ]\n";
    buf << "\t[--spot-reporter-enable= Enable or not http report. true or 1 to enable (default: false)]\n";
    buf << "\t[--file-log-enable= Enable or not file logging. true or 1 to enable (default: false)]\n";
    buf << "\t[--file-log-workdir= Directory name where put file logs into. (default: \".\")]\n";
    buf << "\t[--jt65-submode= Submode 0,1,2 means A,B,C (default: 0)]\n";
    buf << "\t[--tr-interval= T/R interval in seconds (default: 60)]\n";
    buf << "\t[--depth= Deep of analysis. (default: 3)]\n";
    buf << "\t[--immediate-read  Read stream until buffer full, run decoder and exit.]\n";
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
    JT65Context ctx;

    static struct option long_options[] = {
            {"help", no_argument, 0, 0}, // 0
            {"sport-reporter-src", required_argument, 0, 0}, // 1
            {"spot-reporter-url", required_argument, 0, 0}, // 2
            {"spot-reporter-magic-key", required_argument, 0, 0}, // 3
            {"spot-reporter-enable", required_argument, 0, 0}, // 4
            {"file-log-enable", required_argument, 0, 0}, // 5
            {"file_log_workdir", required_argument, 0, 0}, // 6
            {"jt65-submode", required_argument, 0, 0}, // 7
            {"tr-interval", required_argument, 0, 0}, // 8
            {"depth", required_argument, 0, 0}, // 9
            {"immediate-read", no_argument, 0, 0}, // 10
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
                ctx.jt65_submode = atoi(optarg);
                break;
            case 8:
                ctx.tr_interval_in_seconds = atoi(optarg);
                break;
            case 9:
                ctx.jt65_depth = atoi(optarg);
                break;
            case 10:
                ctx.immediate_read = true;
                break;
            default:
                usage();
            }
        }
    }

    // print Context info to stdout
    std::cout << ctx.asString() << std::endl;

    const size_t IDLE_READ_SIZE = 1024;
    std::vector<short> idle_buffer(IDLE_READ_SIZE);

    const size_t COLLECTING_READ_SIZE = 1024;
    const size_t max_record_duration_in_seconds = 50; // 50 seconds max for jt65
    const size_t sample_rate = 12000;
    const size_t max_num_samples = sample_rate * max_record_duration_in_seconds;
    std::vector<short> collecting_buffer(max_num_samples);

    size_t collecting_pos = 0;

    std::cout << "JT65 decoder started." << std::endl;

    // ----- Short Stream Mode --------
    if (ctx.immediate_read)
    {
        while (true)
        {
            if (feof(stdin)) 
            {
                std::cerr << "Got EOF" << std::endl;
                break;
            }

            if (collecting_pos + COLLECTING_READ_SIZE > collecting_buffer.size())
            {
                std::cout << "Read done" << std::endl;
                break;
            }
            else
            {
                // consume samples to buffer
                int rc = fread(&collecting_buffer[collecting_pos], sizeof(short), COLLECTING_READ_SIZE, stdin);
                if (rc != IDLE_READ_SIZE) { std::cout << "Incomplete read. rc=" << rc << std::endl; break; }
                collecting_pos += COLLECTING_READ_SIZE;
            }
        }

        if (collecting_pos)
        {
            std::cout << "Start decoding ..." << std::endl;
            int utc = utc_as_wsjt_int_hhmm(); //
            swap_endians_if_need(&collecting_buffer[0], collecting_pos);
            std::thread t(call_jt65_decoder, collecting_buffer, collecting_pos, ctx, utc);
            t.join();
        }

        std::cout << "JT65 decoder immediate read normal exit." << std::endl;
        return 0;
    }


    WORKING_MODE working_mode = WORKING_MODE::IDLE;
    int prev_seconds_to_launch = ctx.tr_interval_in_seconds;
    int limit_num_samples_extra = 0; // 0 - means no extra limit

    // --- Normal Stream Mode ---
    while(true)
    {
        if (feof(stdin)) 
        {
            std::cerr << "Got EOF" << std::endl;
            break;
        }

        if (working_mode == WORKING_MODE::IDLE) 
        {
            // wait moment to start

            int s = seconds_to_next_launch(ctx.tr_interval_in_seconds);

            bool need_launch = false;

            // basic condition to start
            if (s == 0) 
            {
                need_launch = true;
                limit_num_samples_extra = 0;
            }

            // in case we missed launch moment somehow
            if (!need_launch && s > prev_seconds_to_launch) 
            {
                int passed = seconds_since_launch(ctx.tr_interval_in_seconds);
                limit_num_samples_extra = (max_record_duration_in_seconds - passed) * sample_rate;
                if (limit_num_samples_extra > 0)
                {
                    need_launch = true;
                }
            }

            prev_seconds_to_launch = s;

            if (need_launch) 
            {
                working_mode = WORKING_MODE::COLLECTING;
                collecting_pos = 0;
            }
        }

        if (working_mode == WORKING_MODE::COLLECTING)
        {
            if (collecting_pos + COLLECTING_READ_SIZE > collecting_buffer.size()
                || ((limit_num_samples_extra != 0) && collecting_pos + COLLECTING_READ_SIZE > limit_num_samples_extra))
            {
                working_mode = WORKING_MODE::IDLE;
                prev_seconds_to_launch = ctx.tr_interval_in_seconds;

                int utc = utc_as_wsjt_int_hhmm(); // 100*hh + mm;
                swap_endians_if_need(&collecting_buffer[0], collecting_pos);
                std::thread t(call_jt65_decoder, collecting_buffer, collecting_pos, ctx, utc);
                t.detach();
            }
            else
            {
                // consume samples to buffer
                int rc = fread(&collecting_buffer[collecting_pos], sizeof(short), COLLECTING_READ_SIZE, stdin);
                if (rc != IDLE_READ_SIZE) { std::cerr << "Incomplete read error. rc=" <<  rc << std::endl; break; }
                collecting_pos += COLLECTING_READ_SIZE;
            }
        }
        else
        {
            // reading idle samples for nothing
            int rc = fread(&idle_buffer[0], sizeof(short), IDLE_READ_SIZE, stdin);
            if (rc != IDLE_READ_SIZE) { std::cerr << "Incomplete read error. rc=" << rc << std::endl; break; }
        }

        sched_yield(); // Force give time to OS. Probably not very necessary.
    }

    ::usleep(1000 * 1000); // just 1 second delay before exit. 
    std::cout << "JT65 decoder exit." << std::endl;

    return 0;
}
