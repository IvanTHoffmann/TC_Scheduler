#include "Schedule.h"
#include "AppData.h"

// ShiftSchedule implementation
void ShiftSchedule::Save(Json::object &outElement) const
{
	// int start, end;
	outElement["start"] = start;
	outElement["end"] = end;
	// ServiceIndex_t service_type;
	outElement["service_name"] = service_name;
}

void ShiftSchedule::Load(const Json::object &inElement)
{
	start = inElement.at("start").int_value();
	end = inElement.at("end").int_value();
	service_name = inElement.at("service_name").string_value();
}

// DaySchedule implementation
void DaySchedule::Save(Json::array &outElement) const
{
	for (const ShiftSchedule &shift : shifts)
	{
		Json::object element_json;
		shift.Save(element_json);
		outElement.push_back(element_json);
	}
}

void DaySchedule::Load(const Json::array &inElement)
{
	for (const Json& inJson : inElement) {
		shifts.push_back({});
		shifts.back().Load(inJson.object_items());
	}
}

// WeekSchedule implementation
void WeekSchedule::Save(Json::array &outElement) const
{
	for (const DaySchedule &day : days)
	{
		Json::array element_json;
		day.Save(element_json);
		outElement.push_back(element_json);
	}
}

void WeekSchedule::Load(const Json::array &inElement)
{
	int dayIndex = 0;
	for (const Json& inJson : inElement) {
		days[dayIndex].Load(inJson.array_items());
		dayIndex++;
	}
}

unordered_set<string> WeekSchedule::GetServices() const
{
    unordered_set<string> services;
	for (const DaySchedule& day : days){
		for (const ShiftSchedule& shift : day.shifts){
			services.emplace(shift.service_name);
		}
	}
	return move(services);
}

void WeekSchedule::Clear()
{
	for (DaySchedule& day : days){
		day.shifts.clear();
	}
}
