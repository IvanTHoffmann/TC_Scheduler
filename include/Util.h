#ifndef UTIL_H
#define UTIL_H

#include <iostream>
#include <sstream>

#include <RmlUi/Core.h>
#include "json11.hpp"
#include "DataModelTypes.h"

using namespace json11;
using namespace std;

// Accepts a container of strings and returns a string with the format "A, B, C, ..., and N".
template<typename _ContainerT>
string FormatList(const _ContainerT &inStrings)
{
    string listString = "";
    bool needsCommas = (inStrings.size() > 2);
    int i=0;
    for (const string &s : inStrings)
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
        listString += s;
        ++i;
    }
    return listString;
}

bool ReadUntil(istream& inStream, ostream& outStream, char delim);
string FormatTime(int timeValue);


#endif