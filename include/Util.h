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
template <typename _ContainerT>
string FormatList(const _ContainerT &inStrings)
{
    string listString = "";
    bool needsCommas = (inStrings.size() > 2);
    int i = 0;
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

bool ReadUntil(istream &inStream, ostream &outStream, char delim);
string FormatTime(int timeValue);

const array<int, 20> contrastingColors = {
    0xe6194b, // Red        #e6194b
    0x3cb44b, // Green      #3cb44b
    0xffe119, // Yellow     #ffe119
    0x4363d8, // Blue       #4363d8
    0xf58231, // Orange     #f58231
    0x911eb4, // Purple     #911eb4
    0x42d4f4, // Cyan       #42d4f4
    0xf032e6, // Magenta    #f032e6
    0xbfef45, // Lime       #bfef45
    0xfabed4, // Pink       #fabed4
    0x469990, // Teal       #469990
    0xdcbeff, // Lavender   #dcbeff
    0x9a6324, // Brown      #9a6324
    0xfffac8, // Beige      #fffac8
    0x800000, // Maroon     #800000
    0xaaffc3, // Mint       #aaffc3
    0x808000, // Olive      #808000
    0xffd8b1, // Apricot    #ffd8b1
    0x000075, // Navy       #000075
    0xa9a9a9  // Grey       #a9a9a9
};

#endif