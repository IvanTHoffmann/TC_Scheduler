#ifndef UTIL_H
#define UTIL_H

#include <iostream>
#include <sstream>

#include <RmlUi/Core.h>
#include "json11.hpp"
#include "DataModelTypes.h"

using namespace json11;
using namespace std;

string FormatList(const vector<string>& inStrings);
bool ReadUntil(istream& inStream, ostream& outStream, char delim);
string FormatTime(int timeValue);


#endif