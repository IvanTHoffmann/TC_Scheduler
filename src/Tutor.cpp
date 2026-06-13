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
	for (const Json &inJson : inElement.at("classes").array_items())
	{
		classes.push_back({});
		classes.back().Load(inJson.object_items());
	}
}

bool Tutor::tutorsClasses(const vector<ClassList> &requireClasses) const
{
	for (const ClassList &filterClassList : requireClasses)
	{
		bool foundDepartment = false;
		for (const ClassList &tutorClassList : classes)
		{
			if (tutorClassList.department_name == filterClassList.department_name)
			{
				foundDepartment = true;

				if (filterClassList.subtractive)
				{
					if (tutorClassList.subtractive)
					{
						// Ensure that all non-tutored classes are non-required classes
						for (int nonTutoredCourse : tutorClassList.courses)
						{
							auto result = find(filterClassList.courses.begin(), filterClassList.courses.end(), nonTutoredCourse);
							bool required = (result == filterClassList.courses.end());

							if (required) // not tutored, but required
							{
								return false;
							}
						}
					}
					else
					{
						// A potentially infinite number of classes are required, but only a finite number of classes are tutored
						return false;
					}
				}
				else // Requirements are additive
				{
					if (tutorClassList.subtractive)
					{
						// Ensure that no non-tutored classes are required classes
						for (int nonTutoredCourse : tutorClassList.courses)
						{
							auto result = find(filterClassList.courses.begin(), filterClassList.courses.end(), nonTutoredCourse);
							bool required = (result != filterClassList.courses.end());

							if (required) // not tutored, but required
							{
								return false;
							}
						}
					}
					else
					{
						// Ensure that all required classes are tutored
						for (int requiredCourse : filterClassList.courses)
						{
							auto result = find(tutorClassList.courses.begin(), tutorClassList.courses.end(), requiredCourse);
							bool notFound = (result == tutorClassList.courses.end());

							if (notFound) // required, but not tutored
							{
								return false;
							}
						}
					}
				}

				break; // There is no need to search this tutor's other classLists
			}
		}
		if (!foundDepartment)
		{
			return false;
		}
	}
	return true;
}

bool Tutor::usesService(const string &serviceName) const
{
	for (const DaySchedule &day : schedule.days)
	{	
		for (const ShiftSchedule& shift : day.shifts){
			if (shift.service_name == serviceName){
				return true;
			}
		}
	}
	return false;
}
