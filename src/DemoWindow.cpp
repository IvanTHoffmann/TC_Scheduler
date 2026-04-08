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

#include "DataModelTypes.h"
#include "Util.h"

using namespace json11;
using namespace std;

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
	}

	jsonDocument = Json::parse(buf, err);

	//*
	if (!err.empty())
	{
		cout << "failed to load " << APPDATA_FILENAME << ": " << err << endl;
		return false;
	}
	cout << "loaded " << APPDATA_FILENAME << ": " << jsonDocument.dump().c_str() << endl;

	//*/

	// Read document settings into appdata
	Json settings = jsonDocument["settings"];

	appData.window_title = settings["window_title"].string_value();
	appData.export_dir = settings["export_directory"].string_value();
	appData.schedule_id = settings["startup_schedule"].int_value();
	appData.resolution[0] = settings["resolution"]["w"].int_value();
	appData.resolution[1] = settings["resolution"]["h"].int_value();

	Json::array schedules = jsonDocument["schedules"].array_items();
	Json schedule = schedules[appData.schedule_id];

	Json::array departments = schedule["departments"].array_items();
	Department department;
	for (const Json deptName : departments)
	{
		department.name = deptName.string_value();
		department.edit_subtractive = false;
		appData.departments.push_back(department);
	}

	Json::array services = schedule["services"].array_items();
	Service service;
	for (const Json service_json : services)
	{
		service.name = service_json["name"].string_value();
		service.min_hours = service_json["min_hours"].int_value();
		service.max_hours = service_json["max_hours"].int_value();
		appData.services.push_back(service);
	}

	Json::array tutors = schedule["tutors"].array_items();
	for (const Json tutor_json : tutors)
	{
		Tutor tutor;
		tutor.total_hours = 0;
		tutor.min_hours = tutor_json["min_hours"].int_value();
		tutor.max_hours = tutor_json["max_hours"].int_value();
		tutor.first_name = tutor_json["first_name"].string_value();
		tutor.last_name = tutor_json["last_name"].string_value();
		tutor.email = tutor_json["email"].string_value();

		for (int d = 0; d < 7; ++d)
			for (int seg = 0; seg < 96; ++seg)
				tutor.schedule.days[d].segments[seg] = 0;

		// Populate the tutor schedule from saved JSON shift definitions.
		LoadTutorScheduleFromShifts(tutor, tutor_json);

		tutor.classes.clear();
		for (const Json class_json : tutor_json["classes"].array_items())
		{
			ClassList classList;
			classList.department_name = appData.departments[class_json["department_id"].int_value()].name;
			classList.subtractive = class_json["subtractive"].bool_value();

			classList.courses.clear();
			for (const Json course_json : class_json["courses"].array_items())
			{
				classList.courses.push_back(course_json.int_value());
			}
			tutor.classes.push_back(classList);
		}

		appData.tutors.push_back(tutor);
	}

	return true;
}

bool DemoWindow::Save()
{
	// Persist any schedule changes back into the JSON document.
	if (!jsonDocument.is_object())
		return false;

	Json::object root_obj = jsonDocument.object_items();
	Json::array schedules = root_obj["schedules"].array_items();
	if (schedules.empty())
		return false;

	Json::object schedule_obj = schedules[0].object_items();
	Json::array tutors_json;
	Json::array original_tutors = schedule_obj["tutors"].array_items();

	for (size_t tutor_index = 0; tutor_index < appData.tutors.size(); ++tutor_index)
	{
		Json::object tutor_obj;
		if (tutor_index < original_tutors.size())
		{
			tutor_obj = original_tutors[tutor_index].object_items();
		}
		tutor_obj["shifts"] = SerializeTutorShifts(appData.tutors[tutor_index]);
		tutors_json.push_back(tutor_obj);
	}

	schedule_obj["tutors"] = tutors_json;
	schedules[0] = schedule_obj;
	root_obj["schedules"] = schedules;

	Json new_root(root_obj);
	string output;
	new_root.dump(output);

	ofstream fout(APPDATA_FILENAME);
	if (!fout.is_open())
		return false;

	fout << output;
	return fout.good();
}

DemoWindow::DemoWindow() : appData(), exporter(&appData) {}

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

int DemoWindow::CountTutorScheduleSlots(int tutor_index)
{
	if (tutor_index < 0 || tutor_index >= (int)appData.tutors.size())
		return 0;

	int count = 0;
	for (int day = 0; day < 7; ++day)
	{
		for (int slot = 0; slot < 22; ++slot)
		{
			int segment = GetScheduleSegment(slot);
			count += appData.tutors[tutor_index].schedule.days[day].segments[segment] ? 1 : 0;
		}
	}
	return count;
}

void DemoWindow::BuildScheduleGrid()
{
	if (!document)
		return;

	Rml::Element *container = document->GetElementById("schedule_grid_container");
	if (!container)
		return;

	std::string html = "<div class='schedule-grid'>";

	// Header row: time slots
	html += "<div class='schedule-row schedule-header'><div class='slot day-label'></div>";
	for (int slot = 0; slot < 22; ++slot)
	{
		html += "<div class='slot'>";
		html += FormatTimeSlot(slot);
		html += "</div>";
	}
	html += "</div>";

	static const char *days[7] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
	for (int day = 0; day < 7; ++day)
	{
		html += "<div class='schedule-row'>";
		html += "<div class='slot day-label'>";
		html += days[day];
		html += "</div>";
		for (int slot = 0; slot < 22; ++slot)
		{
			Rml::String id = GetSlotElementId(day, slot);
			char buf[256];
			sprintf(buf, "<div id='%s' class='slot' data-day='%d' data-slot='%d' onmousedown='schedule_slot' onmouseover='schedule_slot' onmouseup='schedule_slot'></div>", id.c_str(), day, slot);
			html += buf;
		}
		html += "</div>";
	}

	html += "</div>";
	container->SetInnerRML(html);
	schedule_grid_built = true;
}

uint8_t DemoWindow::GetScheduleValue(int tutor_index, int day, int slot)
{
	if (tutor_index < 0 || tutor_index >= (int)appData.tutors.size() || day < 0 || day >= 7 || slot < 0 || slot >= 22)
	{
		return 0;
	}
	int segment = GetScheduleSegment(slot);
	return appData.tutors[tutor_index].schedule.days[day].segments[segment];
}

void DemoWindow::SetScheduleValue(int tutor_index, int day, int slot, uint8_t value)
{
	if (tutor_index < 0 || tutor_index >= (int)appData.tutors.size() || day < 0 || day >= 7 || slot < 0 || slot >= 22)
		return;
	int segment = GetScheduleSegment(slot);
	appData.tutors[tutor_index].schedule.days[day].segments[segment] = value ? 1 : 0;
}

void DemoWindow::UpdateScheduleGrid()
{
	if (!document)
		return;

	if (!schedule_grid_built)
		BuildScheduleGrid();

	UpdateScheduleSummary();

	int selected_tutor = appData.selected_tutor.index;

	for (int day = 0; day < 7; ++day)
	{
		for (int slot = 0; slot < 22; ++slot)
		{
			bool selected = selected_tutor >= 0 && selected_tutor < (int)appData.tutors.size() && GetScheduleValue(selected_tutor, day, slot);
			Rml::Element *slot_el = document->GetElementById(GetSlotElementId(day, slot));
			if (!slot_el)
				continue;
			slot_el->SetClass("selected", selected);
		}
	}
}

void DemoWindow::UpdateScheduleSummary()
{
	if (!document)
		return;

	int selected_tutor = appData.selected_tutor.index;
	int scheduled_slots = CountTutorScheduleSlots(selected_tutor);
	std::string scheduled_hours = FormatTutorHours(scheduled_slots);
	std::string summary_text;
	bool invalid_summary = false;
	if (selected_tutor >= 0 && selected_tutor < (int)appData.tutors.size())
	{
		summary_text = "Total: " + scheduled_hours + " hr";
		int min_slots = appData.tutors[selected_tutor].min_hours * 2;
		int max_slots = appData.tutors[selected_tutor].max_hours * 2;
		invalid_summary = scheduled_slots < min_slots || scheduled_slots > max_slots;
	}
	else
	{
		summary_text = "Total: 0 hr";
	}

	if (auto summary_el = document->GetElementById("schedule_hours_summary"))
	{
		summary_el->SetInnerRML(summary_text);
		summary_el->SetClass("invalid", invalid_summary);
	}
}

void DemoWindow::ResetSchedule(Rml::DataModelHandle model, Rml::Event &ev, const Rml::VariantList &arguments)
{
	// 1. Safety check: ensure no extra arguments were passed from the UI
	if (arguments.size() != 0)
		return;

	// 2. Use the new Pointer helper to get the currently selected tutor
	Tutor *tutor = appData.selected_tutor.accessor.ptr();

	// 3. If no tutor is selected, exit early to prevent a crash
	if (!tutor)
	{
		cout << "ResetSchedule failed: No tutor selected." << endl;
		return;
	}

	// 4. Loop through every day and every 30-minute slot
	for (int day = 0; day < 7; ++day)
	{
		for (int slot = 0; slot < 22; ++slot)
		{
			// We use the tutor pointer to reach the internal bit-grid directly
			int segment = GetScheduleSegment(slot);
			tutor->schedule.days[day].segments[segment] = 0;
		}
	}

	// 5. Trigger the visual refresh so the grid boxes turn un-highlighted
	UpdateScheduleGrid();

	// 6. Tell RmlUi that data has changed so other UI elements (like total hours) update
	dataModelHandle.DirtyAllVariables();

	// 7. Auto-save the now-empty schedule to the JSON file
	if (!Save())
	{
		cout << "Warning: ResetSchedule could not auto-save to JSON." << endl;
	}
}

void DemoWindow::ScheduleLimitsChanged(Rml::DataModelHandle model, Rml::Event &ev, const Rml::VariantList &arguments)
{
	// Only refresh the summary state when min/max sliders change.
	UpdateScheduleSummary();
}

void DemoWindow::OnTutorChanged()
{
	UpdateScheduleGrid();
}

void DemoWindow::OnScheduleCellMouseDown(int day, int slot)
{
	int selected_tutor = appData.selected_tutor.index;
	if (selected_tutor < 0 || selected_tutor >= (int)appData.tutors.size())
		return;

	uint8_t old_value = GetScheduleValue(selected_tutor, day, slot);
	uint8_t new_value = old_value ? 0 : 1;
	SetScheduleValue(selected_tutor, day, slot, new_value);

	schedule_drag_active = true;
	schedule_drag_target_state = new_value;

	UpdateScheduleGrid();
}

void DemoWindow::OnScheduleCellMouseMove(int day, int slot)
{
	if (!schedule_drag_active)
		return;

	if (day < 0 || day >= 7 || slot < 0 || slot >= 22)
		return;

	int selected_tutor = appData.selected_tutor.index;
	if (selected_tutor < 0 || selected_tutor >= (int)appData.tutors.size())
		return;

	SetScheduleValue(selected_tutor, day, slot, schedule_drag_target_state);
	UpdateScheduleGrid();
}

void DemoWindow::OnScheduleCellMouseUp(int day, int slot)
{
	schedule_drag_active = false;
	UpdateScheduleGrid();
	if (!Save())
	{
		cout << "Failed to save schedule shifts" << endl;
	}
}

// EVENT CALLBACKS

void DemoWindow::ChangedTab(CALLBACK_PARAMS)
{
	appData.edit_tutor = false;
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
			tutor->classes.push_back(classList);
		}
	}

	appData.edit_tutor = false;
	dataModelHandle.DirtyAllVariables();
}

void DemoWindow::AddTutor(CALLBACK_PARAMS)
{
}

void DemoWindow::RemoveTutor(CALLBACK_PARAMS) {}

void DemoWindow::ExportTutorPage(CALLBACK_PARAMS){
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

// INIT

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
			handle.RegisterMember("edit_subtractive", &Department::edit_subtractive);
			handle.RegisterMember("edit_courses", &Department::edit_courses);
			handle.RegisterMember("edit_formatted_courses", &Department::edit_formatted_courses);
		}

		constructor.RegisterArray<vector<Department>>();

		constructor.RegisterArray<VectorInterface<Department>>();
		if (auto handle = constructor.RegisterStruct<SelectedItemInterface<Department>>())
		{
			handle.RegisterMember("index", &SelectedItemInterface<Department>::index);
			handle.RegisterMember("accessor", &SelectedItemInterface<Department>::accessor);
		}

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
			handle.RegisterMember("min_hours", &Tutor::min_hours);
			handle.RegisterMember("max_hours", &Tutor::max_hours);
			handle.RegisterMember("classes", &Tutor::classes);
		}
		constructor.RegisterArray<vector<Tutor>>();

		// Register vector<Service>
		if (auto handle = constructor.RegisterStruct<Service>())
		{
			handle.RegisterMember("name", &Service::name);
			handle.RegisterMember("min_hours", &Service::min_hours);
			handle.RegisterMember("max_hours", &Service::max_hours);
		}
		constructor.RegisterArray<vector<Service>>();

		// Register VectorInterfaces
		constructor.RegisterArray<VectorInterface<ClassList>>();
		constructor.RegisterArray<VectorInterface<Tutor>>();

		if (auto handle = constructor.RegisterStruct<SelectedItemInterface<Tutor>>())
		{
			handle.RegisterMember("index", &SelectedItemInterface<Tutor>::index);
			handle.RegisterMember("accessor", &SelectedItemInterface<Tutor>::accessor);
		}

		// Bind appData members
		constructor.Bind("export_directory", &appData.export_dir);
		constructor.Bind("departments", &appData.departments);
		constructor.Bind("services", &appData.services);
		constructor.Bind("tutors", &appData.tutors);
		constructor.Bind("selected_department", &appData.selected_department);
		constructor.Bind("selected_tutor", &appData.selected_tutor);
		constructor.Bind("edit_tutor", &appData.edit_tutor);
		constructor.Bind("dev_enable", &appData.dev_enable);

		// Bind Event Callbacks
		constructor.BindEventCallback("ChangedTab", &DemoWindow::ChangedTab, this);
		constructor.BindEventCallback("EnableEditTutor", &DemoWindow::EnableEditTutor, this);
		constructor.BindEventCallback("ConfirmEditTutor", &DemoWindow::ConfirmEditTutor, this);
		constructor.BindEventCallback("ResetSchedule", &DemoWindow::ResetSchedule, this);
		constructor.BindEventCallback("ScheduleLimitsChanged", &DemoWindow::ScheduleLimitsChanged, this);
		constructor.BindEventCallback("AddTutor", &DemoWindow::AddTutor, this);
		constructor.BindEventCallback("RemoveTutor", &DemoWindow::RemoveTutor, this);
		constructor.BindEventCallback("CopyTutorHtml", &DemoWindow::ExportTutorPage, this);
		constructor.BindEventCallback("CopySubjectHtml", &DemoWindow::ExportSubjectPage, this);
		constructor.BindEventCallback("ExportTimetable", &DemoWindow::ExportTimetable, this);
		constructor.BindEventCallback("ExportRolodex", &DemoWindow::ExportRolodex, this);
		constructor.BindEventCallback("ExportAll", &DemoWindow::ExportAll, this);

		dataModelHandle = constructor.GetModelHandle();

		appData.selected_tutor.setTarget(&appData.tutors);
		appData.selected_department.setTarget(&appData.departments);
	}

	using namespace Rml;

	document = context->LoadDocument("assets/demo.rml");
	if (!document)
	{
		return false;
	}

	// document->GetElementById("title")->SetInnerRML(title);

	UpdateScheduleGrid();
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
	UpdateScheduleSummary();
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
			Save();
			Backend::RequestExit();
		}
	}
	break;

	case EventId::Mouseup:
		schedule_drag_active = false;

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