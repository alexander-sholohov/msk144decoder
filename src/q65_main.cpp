//
// Author: Alexander Sholohov <ra9yer@yahoo.com>
//
// License: MIT
//

#include "wavfile.h"
#include "q65_context.h"

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

    size_t fortran_working_area[16];
};


extern "C" {
  // --- Fortran routines ---

    void __q65_decode_MOD_decode(
        void* me,
        void* callback,
        const short int iwave[], // Raw data, i*2
        int* nutc, // UTC for time-tagging the decode
        int* ntrperiod, // T/R sequence length (s)
        int* nsubmode, // Tone-spacing indicator, 0-4 for A-E
        int* nfqso, // Target signal frequency (Hz)
        int* ntol, // Search range around nfqso (Hz)
        int* ndepth, // Optional decoding level
        int* nfa0, // 200?
        int* nfb0, // 4000?
        int* lclearave, // bool. Flag to clear the message-averaging arrays
        int* single_decode, // bool.
        int* lagain, // bool.
        int* lnewdat0, // bool.
        int* emedelay, // Sync search extended to cover EME delays
        char* mycall,
        char* hiscall,
        char* hisgrid,
        int* nQSOprogress, // Auto-sequencing state for the present QSO
        int* ncontest, // Supported contest type
        int* lapcqonly, // bool. Flag to use AP only for CQ calls
        int* navg0, // output?
        fortran_charlen_t, fortran_charlen_t, fortran_charlen_t);

}

extern "C" void q65_decode_callback(
    GFortranContext * me,
    int* nutc,
    float* snr1,
    int* nsnr,
    float* dt,
    float* freq,
    char* decoded, // size of 37
    int* idec,
    int* nused,
    int* ntrperiod)
{
    std::ostringstream info;

    info << "nutc=" << *nutc
        << " snr1=" << *snr1
        << " nsnr=" << *nsnr
        << " dt=" << *dt
        << " freq=" << *freq
        << " decod=" << decoded
        << " idec=" << *idec
        << " nused=" << *nused
        << " ntrperiod=" << *ntrperiod
        << std::endl;

    if (*idec > -1)
    {
        DecodeResult dr;
        dr.initFromParams(*snr1, *dt, *freq, *nutc, rtrim_copy(decoded), info.str());
        me->decode_results.emplace_back(dr);
    }

    std::cout << "*** " << info.str() << std::endl;
}


//-----------------------------------------------------------------------------------
static void call_q65_decoder(std::vector<short> stream, Q65Context const& ctx, int utc_hhmm)
{
    int ntrperiod = ctx.tr_interval_in_seconds;
    int nsubmode = ctx.q65_submode;
    int nfqso = 200; // 
    int ntol = 0;
    int ndepth = ctx.q65_depth;

    int nfa0 = 200;
    int nfb0 = 4000;

    int lclearave = MY_FALSE; //
    int single_decode = MY_FALSE;
    int lagain = MY_FALSE;
    int lnewdat0 = MY_FALSE;
    int emedelay = 0;

    char mycall[12];
    char hiscall[12];
    char hisgrid[6];

    int nQSOprogress = 0;
    int ncontest = 0;
    int lapcqonly = MY_FALSE;
    int navg0 = 0; // output

    memset(mycall, 0, sizeof(mycall));
    memset(hiscall, 0, sizeof(hiscall));
    memset(hisgrid, 0, sizeof(hisgrid));

    GFortranContext fortranContext;

    void* callback = (void*)&q65_decode_callback;

    __q65_decode_MOD_decode(
        &fortranContext,
        callback,
        &stream[0], // Raw data, i*2
        &utc_hhmm, // UTC for time-tagging the decode
        &ntrperiod, // T/R sequence length (s)
        &nsubmode, // Tone-spacing indicator, 0-4 for A-E
        &nfqso, // Target signal frequency (Hz)
        &ntol, // Search range around nfqso (Hz)
        &ndepth, // Optional decoding level
        &nfa0, // 
        &nfb0, // 
        &lclearave, // Flag to clear the message-averaging arrays
        &single_decode, // 
        &lagain,
        &lnewdat0,
        &emedelay, // Sync search extended to cover EME delays
        mycall,
        hiscall,
        hisgrid,
        &nQSOprogress, // Auto-sequencing state for the present QSO
        &ncontest, // Supported contest type
        &lapcqonly, // Flag to use AP only for CQ calls
        &navg0,
        12/* mycall */, 12 /* hiscall */, 6 /* hisgrid */);

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
int utc_as_wsjt_int_by_tr_interval(int tr_interval)
{
    return (tr_interval >= 60) ? utc_as_wsjt_int_hhmm() : utc_as_wsjt_int_hhmmss();
}

//-------------------------------------------------------------------
static void usage(void)
{
    std::ostringstream buf;
    buf << "\nq65decoder - q65 digital mode stream decoder. Accepts audio stream - 16 bits signed, 12000 samples per second, mono.\n\n";
    buf << "Usage:\t[--spot-reporter-url= Address of spot reporter service. (default: http://192.168.1.200:9000/spotter/default/populate_spot ]\n";
    buf << "\t[--spot-reporter-enable= Enable or not http report. true or 1 to enable (default: false)]\n";
    buf << "\t[--file-log-enable= Enable or not file logging. true or 1 to enable (default: false)]\n";
    buf << "\t[--file-log-workdir= Directory name where put file logs into. (default: \".\")]\n";
    buf << "\t[--q65-submode= Submode 0,1,2,3,4 means A,B,C,D,E (default: 0)]\n";
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
    Q65Context ctx;

    static struct option long_options[] = {
            {"help", no_argument, 0, 0}, // 0
            {"sport-reporter-src", required_argument, 0, 0}, // 1
            {"spot-reporter-url", required_argument, 0, 0}, // 2
            {"spot-reporter-magic-key", required_argument, 0, 0}, // 3
            {"spot-reporter-enable", required_argument, 0, 0}, // 4
            {"file-log-enable", required_argument, 0, 0}, // 5
            {"file_log_workdir", required_argument, 0, 0}, // 6
            {"q65-submode", required_argument, 0, 0}, // 7
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
                ctx.q65_submode = atoi(optarg);
                break;
            case 8:
                ctx.tr_interval_in_seconds = atoi(optarg);
                break;
            case 9:
                ctx.q65_depth = atoi(optarg);
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

    std::vector<int> valid_tr_interval = { 15, 30, 60, 120, 300 };
    if (std::find(valid_tr_interval.begin(), valid_tr_interval.end(), ctx.tr_interval_in_seconds) == valid_tr_interval.end())
    {
        std::cerr << "Wrong T/R interval." << std::endl;
        return -1;
    }

    const size_t IDLE_READ_SIZE = 1024;
    std::vector<short> idle_buffer(IDLE_READ_SIZE);

    const size_t COLLECTING_READ_SIZE = 1024;
    const size_t sample_rate = 12000;
    const size_t target_num_samples = sample_rate * ctx.tr_interval_in_seconds;
    std::vector<short> collecting_buffer(target_num_samples);

    size_t collecting_pos = 0;

    std::cout << "Q65 decoder started." << std::endl;

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
            
            // fill rest by zeroes
            std::fill(collecting_buffer.begin() + collecting_pos, collecting_buffer.end(), 0);

            int utc = utc_as_wsjt_int_by_tr_interval(ctx.tr_interval_in_seconds); //
            std::thread t(call_q65_decoder, collecting_buffer, ctx, utc);
            t.join();
        }

        std::cout << "Q65 decoder immediate read normal exit." << std::endl;
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
                limit_num_samples_extra = (ctx.tr_interval_in_seconds - passed - 1) * sample_rate;
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

                // fill rest by zeroes
                std::fill(collecting_buffer.begin() + collecting_pos, collecting_buffer.end(), 0);

                int utc = utc_as_wsjt_int_by_tr_interval(ctx.tr_interval_in_seconds);
                std::thread t(call_q65_decoder, collecting_buffer, ctx, utc);
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
    std::cout << "Q65 decoder exit." << std::endl;

    return 0;
}
