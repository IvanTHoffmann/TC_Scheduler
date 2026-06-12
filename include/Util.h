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

/*
#800000
#e6194b
#fabed4
#9a6324
#f58231
#ffd8b1
#808000
#ffe119
#fffac8
#bfef45
#3cb44b
#aaffc3
#469990
#42d4f4
#000075
#4363d8
#911eb4
#dcbeff
#f032e6
#a9a9a9
*/

const array<int, 20> contrastingColors = {
    0x800000,
    0xe6194b,
    0xfabed4,
    0x9a6324,
    0xf58231,
    0xffd8b1,
    0x808000,
    0xffe119,
    0xfffac8,
    0xbfef45,
    0x3cb44b,
    0xaaffc3,
    0x469990,
    0x42d4f4,
    0x000075,
    0x4363d8,
    0x911eb4,
    0xdcbeff,
    0xf032e6,
    0xa9a9a9};

#endif