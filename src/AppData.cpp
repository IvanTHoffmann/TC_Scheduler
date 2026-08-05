#include "AppData.h"

#include <iostream>

using namespace std;

// AppData implementation
void AppData::SaveSchedule(Json::object &outElement) const
{
	// Rml::String term_season;
	outElement["term_season"] = term_season;

	// int term_year;
	outElement["term_year"] = term_year;

	// vector<RolodexHeader> rolodex_headers
	Json::array roloHeaders_json;
	for (const RolodexHeader& rolodex_header : rolodex_headers) {
		Json::object roloHeader_json;
		rolodex_header.Save(roloHeader_json);
		roloHeaders_json.push_back(roloHeader_json);
	}
	outElement["rolodex_headers"] = roloHeaders_json;

	// vector<Rml::String> budgets;
	Json::array budgets_json;
	for (const Rml::String &budget_name : budgets)
	{
		Json::object budget_json;
		budget_json["name"] = budget_name;
		budgets_json.push_back(budget_json);
	}
	outElement["budgets"] = budgets_json;

	// vector<Tutor> tutors;
	Json::array tutors_json;
	for (const Tutor &tutor : tutors)
	{
		Json::object tutor_json;
		tutor.Save(tutor_json);
		tutors_json.push_back(tutor_json);
	}
	outElement["tutors"] = tutors_json;

	// vector<Department> departments;
	Json::array departments_json;
	for (const Department &department : departments)
	{
		departments_json.push_back(department.name);
	}
	outElement["departments"] = departments_json;

	// vector<Service> services;
	Json::array services_json;
	for (const Service &service : services)
	{
		Json::object service_json;
		service.Save(service_json);
		services_json.push_back(service_json);
	}
	outElement["services"] = services_json;
}

void AppData::LoadSchedule(const Json::object &inElement)
{
	// TODO: detect which file seasons need to be loaded from

	#ifndef PATCH_LOAD
	// Rml::String term_season;
	term_season = inElement.at("term_season").string_value();

	// int term_year;
	term_year = inElement.at("term_year").int_value();
	#endif

	// vector<RolodexHeader> rolodex_header
	rolodex_headers.clear();
	for (const Json& inJson : inElement.at("rolodex_headers").array_items()) {
		rolodex_headers.push_back({});
		rolodex_headers.back().Load(inJson.object_items());
	}

	// vector<RolodexHeader> rolodex_header
	rolodex_headers.clear();
	for (const Json& inJson : inElement.at("rolodex_headers").array_items()) {
		rolodex_headers.push_back({});
		rolodex_headers.back().Load(inJson.object_items());
	}
	
	// vector<Rml::String> budgets
	budgets.clear();
	for (const Json& inJson : inElement.at("budgets").array_items()) {
		budgets.push_back(inJson.object_items().at("name").string_value());
	}

	// vector<Tutor> tutors;
	tutors.clear();
	for (const Json& inJson : inElement.at("tutors").array_items()) {
		tutors.push_back({});
		tutors.back().Load(inJson.object_items());
	}

	// vector<Department> departments;
	departments.clear();
	for (const Json& inJson : inElement.at("departments").array_items()) {
		departments.push_back({});
		Department &dept = departments.back();
		dept.name = inJson.string_value();
		dept.selected = false;
	}

	// vector<Service> services;
	services.clear();
	for (const Json& inJson : inElement.at("services").array_items()) {
		services.push_back({});
		services.back().Load(inJson.object_items());
	}
}

void AppData::SaveSettings(Json::object &outElement) const
{
	// Rml::String window_title;
	outElement["window_title"] = window_title;

	// Rml::String export_dir;
	outElement["export_directory"] = export_dir;
	outElement["schedules_dir"] = schedules_dir; 
	outElement["startup_schedule"] = schedule_name;

	// int resolution[2];
	Json::object resolution_json;
	resolution_json["w"] = resolution[0];
	resolution_json["h"] = resolution[1];
	outElement["resolution"] = resolution_json;
}

void AppData::LoadSettings(const Json::object &inElement)
{
	#ifdef PATCH_LOAD
	// Rml::String term_season;
	term_season = inElement.at("term_season").string_value();

	// int term_year;
	term_year = inElement.at("term_year").int_value();
	#endif

	// Rml::String window_title;
	window_title = inElement.at("window_title").string_value();

	// Rml::String export_dir;
	export_dir = inElement.at("export_directory").string_value();
	schedules_dir = inElement.at("schedules_dir").string_value();
	schedule_name = inElement.at("startup_schedule").string_value();

	// int resolution[2];
	const Json &resolution_json = inElement.at("resolution");
	resolution[0] = resolution_json.object_items().at("w").int_value();
	resolution[1] = resolution_json.object_items().at("h").int_value();
}

bool AppData::GetService(Service &outService, string serviceName)
{
    for (Service& service : services){
		if (service.name == serviceName){
			outService = service;
			return true;
		}
	}
	return false;
}

Service* AppData::GetService(string serviceName)
{
    for (Service& service : services){
		if (service.name == serviceName){	
			return &service;
		}
	}
	return nullptr;
}
