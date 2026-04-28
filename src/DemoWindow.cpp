#include "DemoWindow.h"
#include "json11.hpp"

#include "RmlUi/Core/StreamMemory.h"
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/FileInterface.h>
#include <RmlUi_Backend.h>
#include <RmlUi/Core.h>

#include <filesystem>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <sstream>

#include "DataModelTypes.h"
#include "Util.h"

using namespace json11;
using namespace std;

const float MINUTES_PER_HOUR = 60.0f;

bool DemoWindow::Load()
{
	ifstream fin;
	string buf, err;

	fin.open(APPDATA_FILENAME);
	if (fin.is_open())
	{
		string line;
		while (std::getline(fin, line))
		{
			buf += line + "\n";
		}
		fin.close();
	}
	else
	{
		// create a default appdata.json file
		buf = "{\"schedules\": [{\"budgets\": [],\"departments\": [],\"name\": \"Unnamed Schedule\",\"services\": [],\"tutors\": []}]}";
	}

	jsonDocument = Json::parse(buf, err);

	//*
	if (!err.empty())
	{
		cout << "failed to load " << APPDATA_FILENAME << ": " << err << endl;
		return false;
	}
	// cout << "loaded " << APPDATA_FILENAME << ": " << jsonDocument.dump().c_str() << endl;

	// Load document settings into appData
	Json settings_json = jsonDocument.object_items().at("settings");
	appData.LoadSettings(settings_json.object_items());

	Json::array schedules_json = jsonDocument.object_items().at("schedules").array_items();

	appData.schedule_names.clear();
	for (const Json &schedule : schedules_json)
	{
		const string &schedule_name = schedule.object_items().at("name").string_value();
		appData.schedule_names.push_back(schedule_name);
	}

	Json schedule_json = schedules_json.at(appData.schedule_id);
	appData.LoadSchedule(schedule_json.object_items());

	return true;
}

bool DemoWindow::Save()
{
	cout << "Save document" << endl;

	Json::object newDocument = jsonDocument.object_items(); // Make a copy of loaded document

	Json::object settings_json;
	appData.SaveSettings(settings_json);	 // Save settings to a new json object
	newDocument["settings"] = settings_json; // Write settings to the new copy of the document

	Json::array schedules_json = newDocument.at("schedules").array_items(); // Make a copy of the schedules array
	Json::object schedule_json;
	schedule_json["name"] = appData.schedule_names[appData.schedule_id];
	appData.SaveSchedule(schedule_json);					// Save the current schedule to a new json object
	schedules_json.at(appData.schedule_id) = schedule_json; // Write the current schedule to the new copy of the schedules array
	newDocument["schedules"] = schedules_json;				// Write new schedule array to the new copy of the document

	jsonDocument = newDocument; // Overwrite the previous document

	ofstream fout(APPDATA_FILENAME);
	if (!fout.is_open())
	{
		return false;
	}

	fout << jsonDocument.dump().c_str();
	return fout.good();
}

DemoWindow::DemoWindow() : appData(), exporter(&appData)
{
}

DemoWindow::~DemoWindow()
{
	Save();
}

const string &DemoWindow::GetWindowTitle()
{
	return appData.window_title;
}

int DemoWindow::GetWidth()
{
	return appData.resolution[0];
}

int DemoWindow::GetHeight()
{
	return appData.resolution[1];
}

void DemoWindow::UpdateTimetable()
{
	WeekSchedule filteredSchedule;
	switch (appData.timetableDisplayMode)
	{
	case AppData::TimetableDisplayMode::SUMMARY:
		// summarize data based on active filters
		for (const Tutor &tutor : appData.tutors)
		{
			if (!tutor.selected)
			{
				continue;
			}

			// search for shifts that match the search filter
			bool matchesFilter = false;
			for (const Department &dept : appData.departments)
			{
				if (!dept.selected)
				{
					continue;
				}

				for (const ClassList &classList : tutor.classes)
				{
					if (classList.department_name == dept.name)
					{
						matchesFilter = true;
						break;
					}
				}
				if (matchesFilter)
				{
					break;
				}
			}
			if (!matchesFilter)
			{
				continue;
			}

			int dayId = 0;
			for (const DaySchedule &day : tutor.schedule.days)
			{
				for (const ShiftSchedule &shift : day.shifts)
				{
					if (!appData.services.at(shift.service_type).selected)
					{
						// this shift's service type is not selected
						continue;
					}
					// This shift is of a selected service type
					filteredSchedule.days.at(dayId).shifts.push_back(shift);
				}
				dayId++;
			}
		}

		// load summarized schedule
		appData.timetable.Load(filteredSchedule);
		dataModelHandle.DirtyAllVariables();
		break;

	case AppData::TimetableDisplayMode::INDIVIDUAL:
		Tutor *tutor = appData.selected_tutor.accessor.ptr();
		if (!tutor)
		{
			return;
		}

		appData.timetable.Load(tutor->schedule);
		UpdateTotalHours();
		dataModelHandle.DirtyAllVariables();
		break;
	}
}

void DemoWindow::ScheduleLimitsChanged(CALLBACK_PARAMS)
{
	// Only refresh the summary state when min/max sliders change.
	UpdateTotalHours();
}

float DemoWindow::CalculateTotalHours(const WeekSchedule& schedule)
{
	float totalMinutes = 0.0f;
	for (const DaySchedule& day : schedule.days)
	{
		for (const ShiftSchedule& shift : day.shifts)
		{
			float startMinutes = (shift.start / 100) * MINUTES_PER_HOUR + (shift.start % 100);
			float endMinutes = (shift.end / 100) * MINUTES_PER_HOUR + (shift.end % 100);
			totalMinutes += (endMinutes - startMinutes);
		}
	}
	return totalMinutes / MINUTES_PER_HOUR; // Convert to hours
}

void DemoWindow::UpdateTotalHours()
{
	Tutor* tutor = appData.selected_tutor.accessor.ptr();
	if (tutor)
	{
		appData.total_hours = CalculateTotalHours(tutor->schedule);
		
		// Check if within valid range
        appData.schedule_valid = (appData.total_hours >= tutor->min_hours && appData.total_hours <= tutor->max_hours);

		// Format as hours and minutes
		int totalMinutes = (appData.total_hours * MINUTES_PER_HOUR);
		int hours = totalMinutes / static_cast<int>(MINUTES_PER_HOUR);
		int minutes = totalMinutes % static_cast<int>(MINUTES_PER_HOUR);
		
		std::stringstream ss;
		ss << hours << "h " << minutes << "m";
		appData.formatted_total_hours = ss.str();
	}
	else
	{
		appData.total_hours = 0.0f;
		appData.formatted_total_hours = "0h 0m";
		appData.schedule_valid = true;
	}
	dataModelHandle.DirtyVariable("total_hours");
	dataModelHandle.DirtyVariable("formatted_total_hours");
	dataModelHandle.DirtyVariable("schedule_valid");
}

// EVENT CALLBACKS

void DemoWindow::OnTutorChanged(CALLBACK_PARAMS)
{
	UpdateTimetable();
}

void DemoWindow::OnClick_SummaryTab(CALLBACK_PARAMS)
{
	appData.timetable.editable = false;
	appData.timetable.colorMode = TimetableInterface::ColorModeEnum::HEATMAP;
	appData.timetableDisplayMode = AppData::TimetableDisplayMode::SUMMARY;
	UpdateTimetable();
}

void DemoWindow::OnClick_SchedulesTab(CALLBACK_PARAMS)
{
	appData.edit_tutor = false;
	appData.timetable.editable = true;
	appData.timetable.colorMode = TimetableInterface::ColorModeEnum::SERVICE;
	appData.timetableDisplayMode = AppData::TimetableDisplayMode::INDIVIDUAL;
	UpdateTimetable();
}

void DemoWindow::OnClick_ClassesTab(CALLBACK_PARAMS)
{
	// TODO: remove
}

void DemoWindow::SetSelectedServices(CALLBACK_PARAMS)
{
	if (arguments.size() == 0)
	{
		return;
	}

	bool selected = arguments.at(0).Get<bool>();
	for (Service &service : appData.services)
	{
		service.selected = selected;
	}
	dataModelHandle.DirtyAllVariables();
}

void DemoWindow::SetSelectedDepartments(CALLBACK_PARAMS)
{
	if (arguments.size() == 0)
	{
		return;
	}

	bool selected = arguments.at(0).Get<bool>();
	for (Department &department : appData.departments)
	{
		department.selected = selected;
	}
	dataModelHandle.DirtyAllVariables();
}

void DemoWindow::SetSelectedTutors(CALLBACK_PARAMS)
{
	if (arguments.size() == 0)
	{
		return;
	}

	bool selected = arguments.at(0).Get<bool>();
	for (Tutor &tutor : appData.tutors)
	{
		tutor.selected = selected;
	}
	dataModelHandle.DirtyAllVariables();
}

void DemoWindow::AddStartTime(CALLBACK_PARAMS)
{
	if (arguments.size() == 0)
	{
		return;
	}

	int adjustment = arguments[0].Get<int>();
	int prevStartHour = appData.timetable.GetStartHour();
	appData.timetable.SetStartHour(prevStartHour + adjustment);
	dataModelHandle.DirtyAllVariables();
}

void DemoWindow::AddEndTime(CALLBACK_PARAMS)
{
	if (arguments.size() == 0)
	{
		return;
	}
	
	int adjustment = arguments[0].Get<int>();
	int prevEndHour = appData.timetable.GetEndHour();
	appData.timetable.SetEndHour(prevEndHour + adjustment);
	dataModelHandle.DirtyAllVariables();
}

void DemoWindow::EnableEditTutor(CALLBACK_PARAMS)
{
	Tutor *tutor = appData.selected_tutor.accessor.ptr();
	if (tutor == nullptr)
	{
		return;
	}

	for (Department &dept : appData.departments)
	{
		dept.edit_subtractive = false;
		dept.edit_courses.clear();
		dept.edit_formatted_courses.syncBuffer();
	}

	for (const ClassList &classList : tutor->classes)
	{
		for (Department &dept : appData.departments)
		{
			if (dept.name == classList.department_name)
			{
				dept.edit_subtractive = classList.subtractive;
				copy(classList.courses.begin(), classList.courses.end(), back_inserter(dept.edit_courses));
				dept.edit_formatted_courses.syncBuffer();
				break;
			}
		}
	}

	appData.edit_tutor = true;
	dataModelHandle.DirtyAllVariables();
}

void DemoWindow::ConfirmEditTutor(CALLBACK_PARAMS)
{
	Tutor *tutor = appData.selected_tutor.accessor.ptr();
	if (tutor == nullptr)
	{
		return;
	}

	tutor->classes.clear();
	for (Department &dept : appData.departments)
	{
		if (dept.edit_subtractive || !dept.edit_courses.empty())
		{
			ClassList classList;
			classList.department_name = dept.name;
			classList.subtractive = dept.edit_subtractive;
			copy(dept.edit_courses.begin(), dept.edit_courses.end(), back_inserter(classList.courses));
			sort(classList.courses.begin(), classList.courses.end());
			tutor->classes.push_back(classList);
		}
	}

	appData.edit_tutor = false;
	dataModelHandle.DirtyAllVariables();
}

void DemoWindow::AddTutor(CALLBACK_PARAMS)
{
	if (appData.mod_tutor_first_name.size() == 0 || appData.mod_tutor_last_name.size() == 0)
	{
		return;
	}

	appData.tutors.push_back({});

	appData.tutors.back().first_name = appData.mod_tutor_first_name;
	appData.tutors.back().last_name = appData.mod_tutor_last_name;

	sort(appData.tutors.begin(), appData.tutors.end(), 
	[](const Tutor& a, const Tutor& b) { 
		return (a.first_name < b.first_name) || ((a.first_name == b.first_name) && (a.last_name == b.last_name)); });

	appData.mod_tutor_first_name = "";
	appData.mod_tutor_last_name = "";

	dataModelHandle.DirtyAllVariables();
}

void DemoWindow::RemoveTutor(CALLBACK_PARAMS)
{
	for (auto tutor = appData.tutors.begin(); tutor != appData.tutors.end(); tutor++)
	{
		if ((tutor->first_name == appData.mod_tutor_first_name) && (tutor->last_name == appData.mod_tutor_last_name))
		{
			appData.tutors.erase(tutor);
			appData.mod_tutor_first_name = "";
			appData.mod_tutor_last_name = "";
			dataModelHandle.DirtyAllVariables();
			break;
		}
	}
}

void DemoWindow::AddService(CALLBACK_PARAMS)
{
	if (appData.mod_service_name.size() == 0)
	{
		return;
	}
	appData.services.push_back({});
	appData.services.back().name = appData.mod_service_name;
	
	appData.mod_service_name = "";
	dataModelHandle.DirtyAllVariables();
}

void DemoWindow::RemoveService(CALLBACK_PARAMS)
{
	for (auto service = appData.services.begin(); service != appData.services.end(); service++)
	{
		if (service->name == appData.mod_service_name)
		{
			appData.services.erase(service);
			dataModelHandle.DirtyAllVariables();
			break;
		}
	}
}

void DemoWindow::AddDepartment(CALLBACK_PARAMS)
{
	if (appData.mod_department_name.size() == 0)
	{
		return;
	}
	appData.departments.push_back({});
	appData.departments.back().name = appData.mod_department_name;
	
	sort(appData.departments.begin(), appData.departments.end(), 
	[](const Department& a, const Department& b) { return a.name < b.name; });
	
	appData.mod_department_name = "";
	dataModelHandle.DirtyAllVariables();
}

void DemoWindow::RemoveDepartment(CALLBACK_PARAMS)
{
	for (auto department = appData.departments.begin(); department != appData.departments.end(); department++)
	{
		if (department->name == appData.mod_department_name)
		{
			appData.departments.erase(department);
			dataModelHandle.DirtyAllVariables();
			break;
		}
	}
}

void DemoWindow::ExportTutorPage(CALLBACK_PARAMS)
{
	exporter.ExportTutorPage();
}

void DemoWindow::ExportSubjectPage(CALLBACK_PARAMS)
{
	exporter.ExportSubjectPage();
}

void DemoWindow::ExportTimetable(CALLBACK_PARAMS)
{
	exporter.ExportTimetable();
}

void DemoWindow::ExportRolodex(CALLBACK_PARAMS)
{
	exporter.ExportRolodex();
}

void DemoWindow::ExportAll(CALLBACK_PARAMS)
{
	exporter.ExportAll();
}

void DemoWindow::OnSlotMouseDown(CALLBACK_PARAMS)
{
	schedule_drag_button = ev.GetParameter("button", -1);

	int dayIndex = arguments.at(0).Get<int>();
	int slotIndex = arguments.at(1).Get<int>();
	ColorSlot(dayIndex, slotIndex);
}

void DemoWindow::OnSlotMouseOver(CALLBACK_PARAMS)
{
	int dayIndex = arguments.at(0).Get<int>();
	int slotIndex = arguments.at(1).Get<int>();
	ColorSlot(dayIndex, slotIndex);
}

void DemoWindow::OnTimetableMouseOut(CALLBACK_PARAMS)
{
	if (ev.GetPhase() != Rml::EventPhase::Target)
	{
		return;
	}
	OnTimetableMouseUp(model, ev, arguments);
}

void DemoWindow::OnTimetableMouseUp(CALLBACK_PARAMS)
{
	if (schedule_drag_button == -1)
	{
		return;
	}

	if (appData.timetableDisplayMode != AppData::TimetableDisplayMode::INDIVIDUAL)
	{
		return;
	}

	Tutor *tutor = appData.selected_tutor.accessor.ptr();
	if (!tutor)
	{
		return;
	}

	schedule_drag_button = -1;

	appData.timetable.Save(tutor->schedule);

	UpdateTotalHours();

	dataModelHandle.DirtyAllVariables();
}

void DemoWindow::ColorSlot(int dayIndex, int slotIndex)
{

	if (!appData.timetable.editable)
	{
		return;
	}

	// cout << "color: " << schedule_drag_button << endl;

	ServiceIndex_t &serviceIndex = appData.timetable.GetServiceIndex(dayIndex, slotIndex);
	switch (schedule_drag_button)
	{
	case 0: // left
		serviceIndex = appData.selected_service.index;
		break;
	case 1: // right
		serviceIndex = -1;
		break;
	default:
		return;
	}

	dataModelHandle.DirtyAllVariables();
}

// INIT

template <typename _Ty>
void RegisterSelectedRangeInterface(Rml::DataModelConstructor &constructor)
{
	constructor.RegisterArray<VectorInterface<_Ty>>();
	if (auto handle = constructor.RegisterStruct<SelectedRangeInterface<_Ty>>())
	{
		handle.RegisterMember("index", &SelectedRangeInterface<_Ty>::index);
		handle.RegisterMember("size", &SelectedRangeInterface<_Ty>::size);
		handle.RegisterMember("accessor", &SelectedRangeInterface<_Ty>::accessor);
	}
}

bool DemoWindow::Initialize(const Rml::String &title, Rml::Context *context)
{
	// Create data model
	if (Rml::DataModelConstructor constructor = context->CreateDataModel("app_data"))
	{

		constructor.RegisterArray<vector<Rml::String>>();

		// Register IntVectorEditable

		constructor.RegisterArray<vector<int>>();
		constructor.RegisterScalar<FormattedIntVector>(Get_IntVectorScalar, Set_IntVectorScalar);

		// Register vector<Department>
		if (auto handle = constructor.RegisterStruct<Department>())
		{
			handle.RegisterMember("name", &Department::name);
			handle.RegisterMember("selected", &Department::selected);
			handle.RegisterMember("edit_subtractive", &Department::edit_subtractive);
			handle.RegisterMember("edit_courses", &Department::edit_courses);
			handle.RegisterMember("edit_formatted_courses", &Department::edit_formatted_courses);
		}

		constructor.RegisterArray<vector<Department>>();

		RegisterSelectedRangeInterface<Department>(constructor);

		// Register vector<ClassList>
		if (auto handle = constructor.RegisterStruct<ClassList>())
		{
			handle.RegisterMember("department_name", &ClassList::department_name);
			handle.RegisterMember("subtractive", &ClassList::subtractive);
			handle.RegisterMember("courses", &ClassList::courses);
		}
		constructor.RegisterArray<vector<ClassList>>();

		// Register vector<Tutor>
		if (auto handle = constructor.RegisterStruct<Tutor>())
		{
			handle.RegisterMember("first_name", &Tutor::first_name);
			handle.RegisterMember("last_name", &Tutor::last_name);
			handle.RegisterMember("email", &Tutor::email);
			handle.RegisterMember("selected", &Tutor::selected);
			handle.RegisterMember("min_hours", &Tutor::min_hours);
			handle.RegisterMember("max_hours", &Tutor::max_hours);
			handle.RegisterMember("classes", &Tutor::classes);
		}
		constructor.RegisterArray<vector<Tutor>>();
		RegisterSelectedRangeInterface<Tutor>(constructor);
		// constructor.RegisterArray<VectorInterface<ClassList>>();

		// Register vector<Service>
		if (auto handle = constructor.RegisterStruct<Service>())
		{
			handle.RegisterMember("name", &Service::name);
			handle.RegisterMember("selected", &Service::selected);
			handle.RegisterMember("min_hours", &Service::min_hours);
			handle.RegisterMember("max_hours", &Service::max_hours);
			handle.RegisterMember("color", &Service::GetColor);
		}
		constructor.RegisterArray<vector<Service>>();
		RegisterSelectedRangeInterface<Service>(constructor);

		// Register TimetableInterface
		if (auto handle = constructor.RegisterStruct<TimetableSlot>())
		{
			handle.RegisterMember("value", &TimetableSlot::value);
		}
		constructor.RegisterArray<vector<TimetableSlot>>();
		RegisterSelectedRangeInterface<TimetableSlot>(constructor);
		constructor.RegisterArray<array<SelectedRangeInterface<TimetableSlot>, 1>>();

		if (auto handle = constructor.RegisterStruct<TimetableRow>())
		{
			handle.RegisterMember("label", &TimetableRow::label);
			handle.RegisterMember("slots", &TimetableRow::slots);
			handle.RegisterMember("view", &TimetableRow::view);
		}
		constructor.RegisterArray<array<TimetableRow, 7>>();

		RegisterSelectedRangeInterface<string>(constructor);
		constructor.RegisterArray<array<SelectedRangeInterface<string>, 1>>();
		if (auto handle = constructor.RegisterStruct<TimetableInterface>())
		{
			handle.RegisterMember("view", &TimetableInterface::view);
			handle.RegisterMember("rows", &TimetableInterface::rows);
			handle.RegisterMember("slotsPerHour", &TimetableInterface::slotsPerHour);
		}

		// Bind appData members
		constructor.Bind("export_directory", &appData.export_dir);
		constructor.Bind("departments", &appData.departments);
		constructor.Bind("services", &appData.services);
		constructor.Bind("tutors", &appData.tutors);
		constructor.Bind("selected_tutor", &appData.selected_tutor);
		constructor.Bind("selected_department", &appData.selected_department);
		constructor.Bind("selected_service", &appData.selected_service);
		constructor.Bind("total_hours", &appData.total_hours);
		constructor.Bind("formatted_total_hours", &appData.formatted_total_hours);
		constructor.Bind("edit_tutor", &appData.edit_tutor);
		constructor.Bind("dev_enable", &appData.dev_enable);
		constructor.Bind("timetable", &appData.timetable);
		constructor.Bind("mod_tutor_first_name", &appData.mod_tutor_first_name);
		constructor.Bind("mod_tutor_last_name", &appData.mod_tutor_last_name);
		constructor.Bind("mod_service_name", &appData.mod_service_name);
		constructor.Bind("mod_department_name", &appData.mod_department_name);
		constructor.Bind("schedule_valid", &appData.schedule_valid);

		// Bind Event Callbacks
		constructor.BindEventCallback("OnClick_SummaryTab", &DemoWindow::OnClick_SummaryTab, this);
		constructor.BindEventCallback("OnClick_SchedulesTab", &DemoWindow::OnClick_SchedulesTab, this);
		constructor.BindEventCallback("OnClick_ClassesTab", &DemoWindow::OnClick_ClassesTab, this);
		constructor.BindEventCallback("SetSelectedServices", &DemoWindow::SetSelectedServices, this);
		constructor.BindEventCallback("SetSelectedDepartments", &DemoWindow::SetSelectedDepartments, this);
		constructor.BindEventCallback("SetSelectedTutors", &DemoWindow::SetSelectedTutors, this);
		constructor.BindEventCallback("EnableEditTutor", &DemoWindow::EnableEditTutor, this);
		constructor.BindEventCallback("ConfirmEditTutor", &DemoWindow::ConfirmEditTutor, this);
		constructor.BindEventCallback("ScheduleLimitsChanged", &DemoWindow::ScheduleLimitsChanged, this);
		constructor.BindEventCallback("AddTutor", &DemoWindow::AddTutor, this);
		constructor.BindEventCallback("RemoveTutor", &DemoWindow::RemoveTutor, this);
		constructor.BindEventCallback("AddService", &DemoWindow::AddService, this);
		constructor.BindEventCallback("RemoveService", &DemoWindow::RemoveService, this);
		constructor.BindEventCallback("AddDepartment", &DemoWindow::AddDepartment, this);
		constructor.BindEventCallback("RemoveDepartment", &DemoWindow::RemoveDepartment, this);
		constructor.BindEventCallback("CopyTutorHtml", &DemoWindow::ExportTutorPage, this);
		constructor.BindEventCallback("CopySubjectHtml", &DemoWindow::ExportSubjectPage, this);
		constructor.BindEventCallback("ExportTimetable", &DemoWindow::ExportTimetable, this);
		constructor.BindEventCallback("ExportRolodex", &DemoWindow::ExportRolodex, this);
		constructor.BindEventCallback("ExportAll", &DemoWindow::ExportAll, this);
		constructor.BindEventCallback("OnTutorChanged", &DemoWindow::OnTutorChanged, this);
		constructor.BindEventCallback("OnSlotMouseDown", &DemoWindow::OnSlotMouseDown, this);
		constructor.BindEventCallback("OnSlotMouseOver", &DemoWindow::OnSlotMouseOver, this);
		constructor.BindEventCallback("OnTimetableMouseUp", &DemoWindow::OnTimetableMouseUp, this);
		constructor.BindEventCallback("OnTimetableMouseOut", &DemoWindow::OnTimetableMouseOut, this);
		constructor.BindEventCallback("AddStartTime", &DemoWindow::AddStartTime, this);
		constructor.BindEventCallback("AddEndTime", &DemoWindow::AddEndTime, this);

		// Register transform functions
		AppData *appData_ptr = &appData;
		Rml::DataTransformFunc GetSlotColor = [appData_ptr](const Rml::VariantList &arguments) -> Rml::Variant
		{
			if (arguments.empty())
			{
				return {};
			}

			const int &slot_value = arguments[0].Get<int>();

			int lerpValue;
			Rml::Colourb slotColor(255, 255, 255);
			switch (appData_ptr->timetable.colorMode)
			{
			case TimetableInterface::ColorModeEnum::SERVICE:
				if (0 <= slot_value && slot_value < appData_ptr->services.size())
				{
					slotColor = appData_ptr->services.at(slot_value).color;
				}
				break;
			case TimetableInterface::ColorModeEnum::HEATMAP:
				if (slot_value != -1)
				{
					lerpValue = min(255, 80 * slot_value);
					slotColor.red = 255 - lerpValue;
					slotColor.blue = 0;
					slotColor.green = lerpValue;
				}
				break;
			}
			return Rml::Variant(ToString(slotColor));
		};
		constructor.RegisterTransformFunc("GetSlotColor", GetSlotColor);

		dataModelHandle = constructor.GetModelHandle();

		appData.selected_tutor.setTarget(&appData.tutors);
		appData.selected_department.setTarget(&appData.departments);
		appData.selected_service.setTarget(&appData.services);
		appData.selected_tutor.size = 1;	  // only one tutor can be selected at a time
		appData.selected_department.size = 1; // only one department can be selected at a time
		appData.selected_service.size = 1;	  // only one service can be selected at a time

		appData.timetable.rows[0].SetLabel("Mon");
		appData.timetable.rows[1].SetLabel("Tue");
		appData.timetable.rows[2].SetLabel("Wed");
		appData.timetable.rows[3].SetLabel("Thu");
		appData.timetable.rows[4].SetLabel("Fri");
		appData.timetable.rows[5].SetLabel("Sat");
		appData.timetable.rows[6].SetLabel("Sun");

		appData.timetable.SetStartHour(5);
		appData.timetable.SetEndHour(20);

		// Prepare the application to disp;lay the summary tab
		appData.timetable.editable = false;
		appData.timetable.colorMode = TimetableInterface::ColorModeEnum::HEATMAP;
		appData.timetableDisplayMode = AppData::TimetableDisplayMode::SUMMARY;
		UpdateTimetable();
	}

	using namespace Rml;

	document = context->LoadDocument("assets/demo.rml");
	if (!document)
	{
		return false;
	}

	// document->GetElementById("title")->SetInnerRML(title);

	document->Show();

	return true;
}

void DemoWindow::Shutdown()
{
	if (document)
	{
		document->Close();
		document = nullptr;
	}
}

void DemoWindow::Update()
{
	if (iframe)
	{
		iframe->UpdateDocument();
	}

	// Always keep the total hours summary in sync with slider changes.
	UpdateTotalHours();
}

void DemoWindow::ProcessEvent(Rml::Event &event)
{
	using namespace Rml;

	switch (event.GetId())
	{
	case EventId::Keydown:
	{
		Rml::Input::KeyIdentifier key_identifier = (Rml::Input::KeyIdentifier)event.GetParameter<int>("key_identifier", 0);

		if (key_identifier == Rml::Input::KI_ESCAPE)
		{
			Backend::RequestExit();
		}
	}
	break;

	default:
	{
		break;
	}
	}
}

Rml::ElementDocument *DemoWindow::GetDocument()
{
	return document;
}