#include "Tutor.h"

// Tutor implementation
void Tutor::Save(Json::object &outElement) const
{
	// Rml::String first_name;
	outElement["first_name"] = first_name;

	// Rml::String last_name;
	outElement["last_name"] = last_name;

	// Rml::String email;
	outElement["email"] = email;

	// int min_hours;
	outElement["min_hours"] = min_hours;

	// int max_hours;
	outElement["max_hours"] = max_hours;

	// WeekSchedule schedule;
	Json::array schedule_json;
	schedule.Save(schedule_json);
	outElement["schedule"] = schedule_json;

	// vector<ClassList> classes;
	Json::array classes_json;
	for (const ClassList &classList : classes)
	{
		Json::object class_json;
		classList.Save(class_json);
		classes_json.push_back(class_json);
	}
	outElement["classes"] = classes_json;
}

void Tutor::Load(const Json::object &inElement)
{
	// Rml::String first_name;
	first_name = inElement.at("first_name").string_value();

	// Rml::String last_name;
	last_name = inElement.at("last_name").string_value();

	// Rml::String email;
	email = inElement.at("email").string_value();

	// int min_hours;
	min_hours = inElement.at("min_hours").int_value();

	// int max_hours;
	max_hours = inElement.at("max_hours").int_value();

	// WeekSchedule schedule;
	schedule.Load(inElement.at("schedule").array_items());

	// vector<ClassList> classes;
	classes.clear();
	for (const Json& inJson : inElement.at("classes").array_items()) {
		classes.push_back({});
		classes.back().Load(inJson.object_items());
	}
}