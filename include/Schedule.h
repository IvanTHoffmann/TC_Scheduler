#ifndef SCHEDULE_H
#define SCHEDULE_H

#include <RmlUi/Core.h>
#include "json11.hpp"
#include <vector>
#include <unordered_set>
#include <array>

using namespace json11;
using namespace std;

typedef int ServiceIndex_t;
const ServiceIndex_t UNSCHEDULED_SLOT = -1;

struct ShiftSchedule{
	int start, end;
	string service_name;

	// SAVE/LOAD
	void Save(Json::object &outElement) const;
	void Load(const Json::object &inElement);
};

struct DaySchedule
{
	vector<ShiftSchedule> shifts;

	// SAVE/LOAD
	void Save(Json::array &outElement) const;
	void Load(const Json::array &inElement);
};

struct WeekSchedule
{
	array<DaySchedule, 7> days;

	// SAVE/LOAD
	void Save(Json::array &outElement) const;
	void Load(const Json::array &inElement);

	unordered_set<string> GetServices() const;
	void Clear();
};

#endif // SCHEDULE_H