#include "Service.h"

// Service implementation
void Service::Save(Json::object &outElement) const
{
	// Rml::String name;
	outElement["name"] = name;
	outElement["description"] = description;

	// int min_hours;
	outElement["min_hours"] = min_hours;

	// int max_hours;
	outElement["max_hours"] = max_hours;

	// bool on_canvas;
	outElement["on_canvas"] = on_canvas;

	// color
	Json::array color_json;
	for(int i=0; i<3; i++){
		color_json.push_back(color[i]);
	}
	outElement["color"] = color_json;
}

void Service::Load(const Json::object &inElement)
{
	// Rml::String name;
	name = inElement.at("name").string_value();
	description = inElement.at("description").string_value();

	// int min_hours;
	min_hours = inElement.at("min_hours").int_value();

	// int max_hours;
	max_hours = inElement.at("max_hours").int_value();

	// bool on_canvas;
	on_canvas = inElement.at("on_canvas").bool_value();

	// color
	Json::array color_json = inElement.at("color").array_items();
	for(int i=0; i<3; i++){
		color[i] = color_json.at(i).int_value();
	}
}