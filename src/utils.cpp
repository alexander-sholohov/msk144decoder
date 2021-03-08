//
// Author: Alexander Sholohov <ra9yer@yahoo.com>
//
// License: MIT
//

#include "utils.h"

#include <sys/types.h>
#include <sys/stat.h>

#include <iomanip>
#include <mutex>
#include <sstream>
#include <algorithm> 

//---------------------------------------------------------------------------------------------
std::tm utcnow()
{
    std::time_t utc_time = std::time(nullptr);
    std::tm utc = *std::gmtime(&utc_time);
    return utc;
}

//---------------------------------------------------------------------------------------------
int utc_as_wsjt_int()
{
    std::tm utc = utcnow();
    return 10000 * utc.tm_hour + 100 * utc.tm_min + utc.tm_sec;
}

//---------------------------------------------------------------------------------------------
int seconds_since_launch(int tr_interval)
{
    int s = utc_as_wsjt_int();
    int m = s % tr_interval;
    return m;
}

//---------------------------------------------------------------------------------------------
int seconds_to_next_launch(int tr_interval)
{
    int m = seconds_since_launch(tr_interval);

    if (m == 0) {
        return 0;
    }

    return tr_interval - m;
}

//---------------------------------------------------------------------------------------------
void create_dir_if_not_exists(std::string const& dirname)
{
    static std::mutex gmtx;
    std::lock_guard<std::mutex> guard(gmtx); // it is possible to call this code in parallel

    struct stat info;
    if (stat(dirname.c_str(), &info) == 0) {
        //
        if (info.st_mode & S_IFDIR) {
            return; // all ok, dir exists
        }
        else {
            std::ostringstream m;
            m << "File '" << dirname << "'' exists but it is not a dir";
            throw std::logic_error(m.str());
        }
    }

    // try to create dir
    if (mkdir(dirname.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0) {
        std::ostringstream m;
        m << "Unable to create dir '" << dirname << "'";
        throw std::logic_error(m.str());
    }

}

// from https://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring

// trim from end (in place)
static inline void rtrim(std::string& s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
        }).base(), s.end());
}

// trim from end (copying)
std::string rtrim_copy(std::string s) {
    rtrim(s);
    return s;
}
