#ifndef CLASSLIST_H
#define CLASSLIST_H

#include <RmlUi/Core.h>
#include "json11.hpp"
#include <vector>

using namespace json11;
using namespace std;

struct ClassList
{
	string department_name;
	bool subtractive;
	vector<int> courses;

	// SAVE/LOAD
	void Save(Json::object &outElement) const;
	void Load(const Json::object &inElement);
};

#endif // CLASSLIST_H