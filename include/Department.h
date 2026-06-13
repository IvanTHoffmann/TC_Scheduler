#ifndef DEPARTMENT_H
#define DEPARTMENT_H

#include "FormattedIntVector.h"
#include "json11.hpp"
#include <RmlUi/Core.h>
#include <vector>

using namespace json11;
using namespace std;

struct Department
{
	Rml::String name;
	bool selected; // used for summary page

	// used for editing tutor class lists
	bool edit_selected;
	bool edit_subtractive;
	vector<int> edit_courses;
	FormattedIntVector edit_formatted_courses;

	Department();
	Department(const Department &src);

	// SAVE/LOAD
	void Save(Json::object &outElement) const;
	void Load(const Json::object &inElement);
};

#endif // DEPARTMENT_H