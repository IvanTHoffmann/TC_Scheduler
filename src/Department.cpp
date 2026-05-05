#include "Department.h"

// Department implementation
Department::Department()
{
	edit_formatted_courses.setTarget(&edit_courses);
}

Department::Department(const Department &src)
{
	name = src.name;
	edit_subtractive = src.edit_subtractive;
	edit_courses = src.edit_courses;
	edit_formatted_courses = src.edit_formatted_courses;
	edit_formatted_courses.setTarget(&edit_courses);
}

void Department::Save(Json::object &outElement) const
{
	// Rml::String name;
	outElement["name"] = name;

	// bool selected;
	outElement["selected"] = selected;

	// bool edit_subtractive;
	outElement["edit_subtractive"] = edit_subtractive;

	// vector<int> edit_courses;
	Json::array array_json;
	for (int courseNum : edit_courses)
	{
		array_json.push_back(courseNum);
	}
	outElement["edit_courses"] = array_json;
}

void Department::Load(const Json::object &inElement)
{
	// Rml::String name;
	name = inElement.at("name").string_value();

	// bool selected;
	selected = inElement.at("selected").bool_value();

	// bool edit_subtractive;
	edit_subtractive = inElement.at("edit_subtractive").bool_value();

	// vector<int> edit_courses;
	edit_courses.clear();
	for (const Json& classNum : inElement.at("edit_courses").array_items()){
		edit_courses.push_back(classNum.int_value());
	}
}