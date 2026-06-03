#ifndef TUTOR_H
#define TUTOR_H

#include <RmlUi/Core.h>
#include "json11.hpp"
#include <vector>

using namespace json11;
using namespace std;

#include "ClassList.h"
#include "Schedule.h"

class Tutor
{
	public:
	Rml::String first_name, last_name, email;
	bool selected; // used for summary page

	int total_hours;
	int min_hours;
	int max_hours;

	WeekSchedule schedule;
	vector<ClassList> classes;

	// SAVE/LOAD
	void Save(Json::object &outElement) const;
	void Load(const Json::object &inElement);

	bool tutorsClasses(const vector<ClassList>& requireClasses) const;
};

#endif // TUTOR_H