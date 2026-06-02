#ifndef APPDATA_H
#define APPDATA_H

#include <RmlUi/Core.h>
#include "json11.hpp"
#include <vector>
#include <array>

using namespace json11;
using namespace std;

#include "VectorInterface.h"
#include "Department.h"
#include "Service.h"
#include "Tutor.h"
#include "TimetableInterface.h"
#include "RolodexHeader.h"

struct AppData
{
	Rml::String current_tab;
	Rml::String window_title;
	Rml::String export_dir;

	Rml::String mod_tutor_first_name, mod_tutor_last_name, mod_service_name, mod_department_name;
	
	enum TimetableDisplayMode {
		SUMMARY,
		INDIVIDUAL
	} timetableDisplayMode;
	TimetableInterface timetable;

	// rolodex headers
	vector<RolodexHeader> rolodex_headers;
	
	// PREFERENCES
	vector<Rml::String> schedule_names;
	Rml::String term_season;
	int term_year;
	int schedule_id;
	bool dev_enable;
	int resolution[2];
	float fontSize;
	
	// INDIVIDUAL
	float total_hours;
	Rml::String formatted_total_hours;
	bool schedule_valid;
	bool edit_tutor;
	vector<Rml::String> budgets;
	vector<Department> departments;
	vector<Service> services;
	vector<Tutor> tutors;
	SelectedRangeInterface<Department> selected_department;
	SelectedRangeInterface<Service> selected_service;
	SelectedRangeInterface<Tutor> selected_tutor;

	// SAVE/LOAD
	void SaveSchedule(Json::object &outElement) const;
	void LoadSchedule(const Json::object &inElement);

	void SaveSettings(Json::object &outElement) const;
	void LoadSettings(const Json::object &inElement);
};

#endif // APPDATA_H