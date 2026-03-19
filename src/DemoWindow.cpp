#include "DemoWindow.h"
#include "json11.hpp"

#include "RmlUi/Core/StreamMemory.h"
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Elements/ElementFormControl.h>
#include <RmlUi/Core/FileInterface.h>
#include <RmlUi/Core/StyleSheetContainer.h>
#include <RmlUi_Backend.h>
#include <RmlUi/Core.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

using namespace json11;
using namespace std;

typedef uint8_t ServiceIndex_t;


template <class _Ty>
class vectorPtr {
	private:
	vector<_Ty> *v;

	public:
	using value_type = _Ty;

	void Update(vector<_Ty> *new_v){
		v = new_v;
	}

	vector<_Ty>::iterator begin(){
		return v->begin();
	}

	size_t size(){
		return v->size();
	}
};


struct Service{
    Rml::String name;
	int min_hours;
	int max_hours;
};

struct ClassList{
    uint8_t department_id;
    bool all_except;
    vector<int> courses;
};

struct DaySchedule{
    array<ServiceIndex_t, 96> segments;
};

struct WeekSchedule {
    array<DaySchedule, 7> days;
};

struct Tutor {
    Rml::String first_name, last_name;

    uint16_t total_hours;
    uint16_t min_hours;
    uint16_t max_hours;

    WeekSchedule schedule;
    vector<ClassList> classes;
};

struct AppData {
	string window_title;
    vector<Tutor> tutors;
	vectorPtr<ClassList> selectedTutorClasses;

	int selected_tutor;
	int selected_department;

	// CONFIG
	vector<array<int, 2>> resolutionOptions;
    int resolution[2];
    float fontSize;
    int schedule_id;

	// INDIVIDUAL
	vector<Rml::String> departments;
	vector<Service> services;
    
	// SUMMARY
	vector<Rml::String> filters;
} appData;


bool DemoWindow::load() {
	ifstream fin;
	string buf, err;

	fin.open(APPDATA_FILENAME);
	if (fin.is_open()) {
		string line;
		while (std::getline(fin, line)) {
			buf += line + "\n";
		}
		fin.close();
	}
	else {
		// create a default appdata.json file
	}

	jsonDocument = Json::parse(buf, err);

	//*
	if (!err.empty()) {
		cout << "failed to load " << APPDATA_FILENAME << ": " << err << endl;
		return false;
	}
	cout << "loaded " << APPDATA_FILENAME << ": " << jsonDocument.dump().c_str() << endl;

	//*/

	// Read document settings into appdata
	Json settings = jsonDocument["settings"];

	appData.window_title = settings["window_title"].string_value();
	appData.schedule_id = settings["startup_schedule"].int_value();

	Json::array resolution_options = settings["resolution_options"].array_items();
	array<int, 2> resolutionOption;
	for (const Json res : resolution_options){
		resolutionOption[0] = res["w"].int_value();
		resolutionOption[1] = res["h"].int_value();
		appData.resolutionOptions.push_back(resolutionOption);
	}

	int resolution_id = settings["resolution"].int_value();
	appData.resolution[0] = appData.resolutionOptions[resolution_id][0];
	appData.resolution[1] = appData.resolutionOptions[resolution_id][1];

	
	Json::array schedules = jsonDocument["schedules"].array_items();
	Json schedule = schedules[0];
	
	Json::array departments = schedule["departments"].array_items();
	for (const Json deptName : departments){
		appData.departments.push_back(deptName.string_value());
	}
	
	Json::array services = schedule["services"].array_items();
	Service service;
	for (const Json service_json : services){
		service.name = service_json["name"].string_value();
		service.min_hours = service_json["min_hours"].int_value();
		service.max_hours = service_json["max_hours"].int_value();
		appData.services.push_back(service);
	}
	
	Json::array tutors = schedule["tutors"].array_items();
	for (const Json tutor_json : tutors){
		Tutor tutor;
		tutor.total_hours = 0;
		tutor.min_hours = tutor_json["min_hours"].int_value();
		tutor.max_hours = tutor_json["max_hours"].int_value();
		tutor.first_name = tutor_json["first_name"].string_value();
		tutor.last_name = tutor_json["last_name"].string_value();

		for (int d = 0; d < 7; ++d)
			for (int seg = 0; seg < 96; ++seg)
				tutor.schedule.days[d].segments[seg] = 0;

		for (const Json class_json : tutor_json["classes"].array_items()){
			ClassList classList;
			classList.department_id = class_json["department_id"].int_value();
			classList.all_except = class_json["all_except"].bool_value();
			for (const Json course_json : class_json["courses"].array_items()){
				classList.courses.push_back(course_json.int_value());
			}
			tutor.classes.push_back(classList);
		}

		appData.tutors.push_back(tutor);
	}



    return true;
}

bool DemoWindow::save(){
	return false;
}

const string& DemoWindow::getWindowTitle(){
	return appData.window_title;
}

int DemoWindow::getWidth(){
	return appData.resolution[0];
}

int DemoWindow::getHeight(){
	return appData.resolution[1];
}

static int GetScheduleSegment(int slot) {
	return 32 + slot * 2; // 8:00 is segment 32 (8*4), 30mins increments
}

static Rml::String FormatTimeSlot(int slot) {
	int total_minutes = 8 * 60 + slot * 30;
	int hour = total_minutes / 60;
	int minute = total_minutes % 60;
	char buf[8];
	sprintf(buf, "%02d:%02d", hour, minute);
	return buf;
}

uint8_t DemoWindow::GetScheduleValue(int tutor_index, int day, int slot) {
	if (tutor_index < 0 || tutor_index >= (int)appData.tutors.size() || day < 0 || day >= 7 || slot < 0 || slot >= 22) {
		return 0;
	}
	int segment = GetScheduleSegment(slot);
	return appData.tutors[tutor_index].schedule.days[day].segments[segment];
}

void DemoWindow::SetScheduleValue(int tutor_index, int day, int slot, uint8_t value) {
	if (tutor_index < 0 || tutor_index >= (int)appData.tutors.size() || day < 0 || day >= 7 || slot < 0 || slot >= 22)
		return;
	int segment = GetScheduleSegment(slot);
	appData.tutors[tutor_index].schedule.days[day].segments[segment] = value ? 1 : 0;
}

void DemoWindow::UpdateScheduleGrid() {
	if (!document)
		return;

	Rml::Element* container = document->GetElementById("schedule_grid_container");
	if (!container)
		return;

	int selected_tutor = appData.selected_tutor;

	std::string html;
	html += "<div class='schedule-grid'>";

	// Header row: time slots
	html += "<div class='schedule-row schedule-header'><div class='slot day-label'></div>";
	for (int slot = 0; slot < 22; ++slot) {
		html += "<div class='slot'>";
		html += FormatTimeSlot(slot);
		html += "</div>";
	}
	html += "</div>";

	static const char* days[7] = { "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };
	for (int day = 0; day < 7; ++day) {
		html += "<div class='schedule-row'>";
		html += "<div class='slot day-label'>";
		html += days[day];
		html += "</div>";
		for (int slot = 0; slot < 22; ++slot) {
			bool selected = selected_tutor >= 0 && selected_tutor < (int)appData.tutors.size() && GetScheduleValue(selected_tutor, day, slot);
			char buf[256];
			sprintf(buf, "<div class='slot %s' data-day='%d' data-slot='%d' onmousedown='schedule_slot' onmousemove='schedule_slot' onmouseup='schedule_slot'> </div>", selected ? "selected" : "", day, slot);
			html += buf;
		}
		html += "</div>";
	}

	html += "</div>";
	container->SetInnerRML(html);
}

void DemoWindow::OnTutorChanged() {
	UpdateScheduleGrid();
}

void DemoWindow::OnScheduleCellMouseDown(int day, int slot) {
	if (appData.selected_tutor < 0 || appData.selected_tutor >= (int)appData.tutors.size())
		return;

	uint8_t old_value = GetScheduleValue(appData.selected_tutor, day, slot);
	uint8_t new_value = old_value ? 0 : 1;
	SetScheduleValue(appData.selected_tutor, day, slot, new_value);

	schedule_drag_active = true;
	schedule_drag_start_day = day;
	schedule_drag_start_slot = slot;
	schedule_drag_current_day = day;
	schedule_drag_current_slot = slot;
	schedule_drag_target_state = new_value;

	UpdateScheduleGrid();
}

void DemoWindow::OnScheduleCellMouseMove(int day, int slot) {
	if (!schedule_drag_active)
		return;

	if (day < 0 || day >= 7 || slot < 0 || slot >= 22)
		return;

	schedule_drag_current_day = day;
	schedule_drag_current_slot = slot;

	int day0 = std::min(schedule_drag_start_day, schedule_drag_current_day);
	int day1 = std::max(schedule_drag_start_day, schedule_drag_current_day);
	int slot0 = std::min(schedule_drag_start_slot, schedule_drag_current_slot);
	int slot1 = std::max(schedule_drag_start_slot, schedule_drag_current_slot);

	for (int d = day0; d <= day1; ++d) {
		for (int s = slot0; s <= slot1; ++s) {
			SetScheduleValue(appData.selected_tutor, d, s, schedule_drag_target_state);
		}
	}

	UpdateScheduleGrid();
}

void DemoWindow::OnScheduleCellMouseUp(int day, int slot) {
	if (!schedule_drag_active)
		return;

	schedule_drag_active = false;
	UpdateScheduleGrid();
}

bool DemoWindow::Initialize(const Rml::String& title, Rml::Context* context)
{
	// Create data model
	if (Rml::DataModelConstructor constructor = context->CreateDataModel("app_data"))
    {
		
		constructor.RegisterArray<vector<Rml::String>>();
		constructor.RegisterArray<vector<int>>();

		if (auto handle = constructor.RegisterStruct<ClassList>())
		{
			handle.RegisterMember("department_id", &ClassList::department_id);
			handle.RegisterMember("all_except", &ClassList::all_except);
			handle.RegisterMember("courses", &ClassList::courses);
		}

		constructor.RegisterArray<vector<ClassList>>();

		if (auto handle = constructor.RegisterStruct<Tutor>())
		{
			handle.RegisterMember("first_name", &Tutor::first_name);
			handle.RegisterMember("last_name", &Tutor::last_name);
			handle.RegisterMember("min_hours", &Tutor::min_hours);
			handle.RegisterMember("max_hours", &Tutor::max_hours);
			handle.RegisterMember("classes", &Tutor::classes);
		}

		constructor.RegisterArray<vector<Tutor>>();

		if (auto handle = constructor.RegisterStruct<Service>())
		{
			handle.RegisterMember("name", &Service::name);
			handle.RegisterMember("min_hours", &Service::min_hours);
			handle.RegisterMember("max_hours", &Service::max_hours);
		}

		constructor.RegisterArray<vector<Service>>();
		constructor.RegisterArray<vectorPtr<ClassList>>();

		constructor.Bind("departments", &appData.departments);
		constructor.Bind("services", &appData.services);
		constructor.Bind("tutors", &appData.tutors);
		constructor.Bind("selected_department", &appData.selected_department);
		constructor.Bind("selected_tutor", &appData.selected_tutor);
		//constructor.Bind("selected_tutor_classes", &appData.selectedTutorClasses);

		//dataModelHandle = constructor.GetModelHandle();
		//dataModelHandle.DirtyVariable("names");
    }

	using namespace Rml;

	document = context->LoadDocument("assets/demo.rml");
	if (!document)
		return false;

	document->GetElementById("title")->SetInnerRML(title);

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
	if (iframe){
		iframe->UpdateDocument();
	}
}

void DemoWindow::ProcessEvent(Rml::Event& event)
{
	using namespace Rml;

	switch (event.GetId())
	{
	case EventId::Keydown:
	{
		Rml::Input::KeyIdentifier key_identifier = (Rml::Input::KeyIdentifier)event.GetParameter<int>("key_identifier", 0);

		if (key_identifier == Rml::Input::KI_ESCAPE)
			Backend::RequestExit();
	}
	break;

	case EventId::Mouseup:
		schedule_drag_active = false;
		break;

	case EventId::Mouseout:
		schedule_drag_active = false;
		break;

	default: break;
	}
}

Rml::ElementDocument* DemoWindow::GetDocument()
{
	return document;
}

void DemoWindow::SubmitForm(Rml::String in_submit_message)
{
	if (auto el_output = document->GetElementById("form_output"))
		el_output->SetInnerRML("");
	if (auto el_progress = document->GetElementById("submit_progress"))
		el_progress->SetProperty("display", "block");
}