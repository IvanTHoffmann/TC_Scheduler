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
	switch(appData.timetableDisplayMode){
		case AppData::TimetableDisplayMode::SUMMARY:
			// summarize data based on active filters
			for (const Tutor& tutor : appData.tutors){
				if (!tutor.selected){
					continue;
				}
				
				// search for shifts that match the search filter
				bool matches = false;
				for (const Department& dept : appData.departments){
					if (!dept.selected){
						continue;
					}

					for (const ClassList& classList : tutor.classes){
						if (classList.department_name == dept.name){
							matches = true;
							break;
						}
					}
					if (matches){
						break;
					}
				}
				if (!matches){
					continue;
				}

				int dayId = 0;
				for (const DaySchedule& day : tutor.schedule.days){
					for (const ShiftSchedule& shift : day.shifts){
						if (!appData.services.at(shift.service_type).selected){
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
			dataModelHandle.DirtyAllVariables();
			break;
	}
}

void DemoWindow::UpdateScheduleSummary()
{
	/*
	if (!document)
	{
		return;
	}
	
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
	*/
}

void DemoWindow::ScheduleLimitsChanged(CALLBACK_PARAMS)
{
	// Only refresh the summary state when min/max sliders change.
	UpdateScheduleSummary();
}

// EVENT CALLBACKS

void DemoWindow::OnTutorChanged(CALLBACK_PARAMS)
{
	UpdateTimetable();
}

void DemoWindow::OnClick_SummaryTab(CALLBACK_PARAMS){
	appData.timetable.editable = false;
	appData.timetable.colorMode = TimetableInterface::ColorModeEnum::HEATMAP;
	appData.timetableDisplayMode = AppData::TimetableDisplayMode::SUMMARY;
	UpdateTimetable();
}

void DemoWindow::OnClick_SchedulesTab(CALLBACK_PARAMS)
{
	appData.timetable.editable = true;
	appData.timetable.colorMode = TimetableInterface::ColorModeEnum::SERVICE;
	appData.timetableDisplayMode = AppData::TimetableDisplayMode::INDIVIDUAL;
	UpdateTimetable();
}

void DemoWindow::OnClick_ClassesTab(CALLBACK_PARAMS)
{
	appData.edit_tutor = false;
	dataModelHandle.DirtyAllVariables();
}

void DemoWindow::SetSelectedServices(CALLBACK_PARAMS)
{
	if (arguments.size() == 0){
		return;
	}

	bool selected = arguments.at(0).Get<bool>();
	for(Service& service : appData.services){
		service.selected = selected;
	}
	dataModelHandle.DirtyAllVariables();
}

void DemoWindow::SetSelectedDepartments(CALLBACK_PARAMS)
{
	if (arguments.size() == 0){
		return;
	}

	bool selected = arguments.at(0).Get<bool>();
	for(Department& department : appData.departments){
		department.selected = selected;
	}
	dataModelHandle.DirtyAllVariables();
}

void DemoWindow::SetSelectedTutors(CALLBACK_PARAMS)
{
	if (arguments.size() == 0){
		return;
	}

	bool selected = arguments.at(0).Get<bool>();
	for(Tutor& tutor : appData.tutors){
		tutor.selected = selected;
	}
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

void DemoWindow::RemoveTutor(CALLBACK_PARAMS)
{
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
	if (!appData.timetable.editable){
		return;
	}

	Tutor *tutor = appData.selected_tutor.accessor.ptr();
	if (!tutor)
	{
		return;
	}

	int dayIndex = arguments.at(0).Get<int>();
	int slotIndex = arguments.at(1).Get<int>();

	ServiceIndex_t &serviceIndex = appData.timetable.GetServiceIndex(dayIndex, slotIndex);
	serviceIndex = appData.selected_service.index;

	schedule_drag_active = true;

	dataModelHandle.DirtyAllVariables();
}

void DemoWindow::OnSlotMouseOver(CALLBACK_PARAMS)
{
	if (schedule_drag_active)
	{
		OnSlotMouseDown(model, ev, arguments);
	}
}

void DemoWindow::OnSlotMouseUp(CALLBACK_PARAMS)
{
	if (!schedule_drag_active){
		return;
	}

	Tutor *tutor = appData.selected_tutor.accessor.ptr();
	if (!tutor)
	{
		return;
	}

	int dayIndex = arguments.at(0).Get<int>();
	int slotIndex = arguments.at(1).Get<int>();

	schedule_drag_active = false;

	cout << "Save Tutor: " << tutor->first_name << endl;
	appData.timetable.Save(tutor->schedule);

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
		constructor.Bind("edit_tutor", &appData.edit_tutor);
		constructor.Bind("dev_enable", &appData.dev_enable);
		constructor.Bind("timetable", &appData.timetable);

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
		constructor.BindEventCallback("CopyTutorHtml", &DemoWindow::ExportTutorPage, this);
		constructor.BindEventCallback("CopySubjectHtml", &DemoWindow::ExportSubjectPage, this);
		constructor.BindEventCallback("ExportTimetable", &DemoWindow::ExportTimetable, this);
		constructor.BindEventCallback("ExportRolodex", &DemoWindow::ExportRolodex, this);
		constructor.BindEventCallback("ExportAll", &DemoWindow::ExportAll, this);
		constructor.BindEventCallback("OnTutorChanged", &DemoWindow::OnTutorChanged, this);
		constructor.BindEventCallback("OnSlotMouseDown", &DemoWindow::OnSlotMouseDown, this);
		constructor.BindEventCallback("OnSlotMouseOver", &DemoWindow::OnSlotMouseOver, this);
		constructor.BindEventCallback("OnSlotMouseUp", &DemoWindow::OnSlotMouseUp, this);

		// Register transform functions
		AppData &app_data_var = appData;
		Rml::DataTransformFunc GetSlotColor = [app_data_var](const Rml::VariantList &arguments) -> Rml::Variant
		{
			if (arguments.empty())
			{
				return {};
			}

			const ServiceIndex_t &service_type = arguments[0].Get<ServiceIndex_t>();

			Rml::Colourb slotColor;
			if (0 <= service_type && service_type < app_data_var.services.size())
			{
				slotColor = app_data_var.services.at(service_type).color;
			}
			else
			{
				slotColor = Rml::Colourb(255, 255, 255);
			}
			return Rml::Variant(ToString(slotColor));
		};
		constructor.RegisterTransformFunc("GetSlotColor", GetSlotColor);

		Rml::DataTransformFunc GetSelectedColor = [app_data_var](const Rml::VariantList &arguments) -> Rml::Variant
		{
			if (arguments.empty())
			{
				return {};
			}

			const bool &selected = arguments[0].Get<bool>();
			return Rml::Variant(selected ? "#AFA" : "#DDD");
		};
		constructor.RegisterTransformFunc("GetSelectedColor", GetSelectedColor);

		dataModelHandle = constructor.GetModelHandle();

		appData.selected_tutor.setTarget(&appData.tutors);
		appData.selected_department.setTarget(&appData.departments);
		appData.selected_service.setTarget(&appData.services);
		appData.selected_tutor.size = 1;	  // only one tutor can be selected at a time
		appData.selected_department.size = 1; // only one department can be selected at a time
		appData.selected_service.size = 1; // only one service can be selected at a time

		appData.timetable.rows[0].SetLabel("Mon");
		appData.timetable.rows[1].SetLabel("Tue");
		appData.timetable.rows[2].SetLabel("Wed");
		appData.timetable.rows[3].SetLabel("Thu");
		appData.timetable.rows[4].SetLabel("Fri");
		appData.timetable.rows[5].SetLabel("Sat");
		appData.timetable.rows[6].SetLabel("Sun");

		appData.timetable.SetStartHour(5);
		appData.timetable.SetEndHour(20);
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