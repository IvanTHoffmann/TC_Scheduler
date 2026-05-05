#include "AppData.h"

// AppData implementation
void AppData::SaveSchedule(Json::object &outElement) const
{
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
		departments.back().name = inJson.string_value();
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

	// Rml::String term_season;
	outElement["term_season"] = term_season;

	// int term_year;
	outElement["term_year"] = term_year;

	// int resolution[2];
	Json::object resolution_json;
	resolution_json["w"] = resolution[0];
	resolution_json["h"] = resolution[1];
	outElement["resolution"] = resolution_json;

	// int schedule_id;
	outElement["startup_schedule"] = schedule_id;
}

void AppData::LoadSettings(const Json::object &inElement)
{
	// Rml::String window_title;
	window_title = inElement.at("window_title").string_value();

	// Rml::String export_dir;
	export_dir = inElement.at("export_directory").string_value();

	// Rml::String term_season;
	term_season = inElement.at("term_season").string_value();

	// int term_year;
	term_year = inElement.at("term_year").int_value();

	// int resolution[2];
	const Json &resolution_json = inElement.at("resolution");
	resolution[0] = resolution_json.object_items().at("w").int_value();
	resolution[1] = resolution_json.object_items().at("h").int_value();

	// int schedule_id;
	schedule_id = inElement.at("startup_schedule").int_value();
}