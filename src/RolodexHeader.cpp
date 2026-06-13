#include "RolodexHeader.h"

#include <iostream>

using namespace std;


RolodexHeader::RolodexHeader() : description()
{
}

void RolodexHeader::Save(Json::object &outElement) const
{
    // string description
    outElement["description"] = description;
	cout << "description: " << description << endl; 

	// vector<ClassList> classes;
	Json::array classes_json;
	for (const ClassList &classList : classes)
	{
		cout << "\tclasslist dept: " << classList.department_name << endl; 
		Json::object class_json;
		classList.Save(class_json);
		classes_json.push_back(class_json);
	}
	outElement["classes"] = classes_json;
}

void RolodexHeader::Load(const Json::object &inElement)
{
    // string description
    description = inElement.at("description").string_value();

	// ClassList classes
    classes.clear();
	for (const Json& inJson : inElement.at("classes").array_items()) {
		classes.push_back({});
		classes.back().Load(inJson.object_items());
	}
}
