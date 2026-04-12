#include "Util.h"

// Accepts a vector<string> and returns a string with the format "A, B, C, ..., and N".
string FormatList(const vector<string> &inStrings)
{
    string listString = "";
    bool needsCommas = (inStrings.size() > 2);
    for (int i = 0; i < inStrings.size(); i++)
    {
        bool isFirst = (i == 0);
        bool isLast = (i == inStrings.size() - 1);
        if (!isFirst)
        {
            listString += needsCommas ? ", " : " ";
            if (isLast)
            {
                listString += "and ";
            }
        }
        listString += inStrings[i];
    }
    return listString;
}

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
