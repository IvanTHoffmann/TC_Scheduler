#include "ClassList.h"

// ClassList implementation
void ClassList::Save(Json::object &outElement) const
{
	// Rml::String department_name;
	outElement["department_name"] = department_name;

	// bool subtractive;
	outElement["subtractive"] = subtractive;

	// vector<int> courses;
	Json::array array_json;
	for (int courseNum : courses)
	{
		array_json.push_back(courseNum);
	}
	outElement["courses"] = array_json;
}

void ClassList::Load(const Json::object &inElement)
{
	// Rml::String department_name;
	department_name = inElement.at("department_name").string_value();

	// bool subtractive;
	subtractive = inElement.at("subtractive").bool_value();

	// vector<int> courses;
	courses.clear();
	for (const Json& classNum : inElement.at("courses").array_items()){
		courses.push_back(classNum.int_value());
	}
}