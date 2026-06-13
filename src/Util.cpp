#include "Util.h"

#include <iomanip>


// Reads from inStream into outStream until delim is read.
// delim is not read into outStream
bool ReadUntil(istream &inStream, ostream &outStream, char delim)
{
    char c;
    while (inStream.get(c))
    {
        if (c == delim)
        {
            return true;
        }
        outStream << c;
    }
    return false;
}

string FormatTime(int timeValue)
{
    stringstream ss;
    int hours = timeValue / 100;
    int minutes = timeValue % 100;
    int hourValue = (hours - 1) % 12 + 1;
    ss << hourValue << ":" << setw(2) << setfill('0') << minutes << (hours < 12 ? "am" : "pm");
    return ss.str();
}
