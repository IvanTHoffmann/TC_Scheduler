#include "DemoWindow.h"
#include "json11.hpp"

#include "RmlUi/Core/StreamMemory.h"
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/FileInterface.h>
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
class SelectedItemInterface;


template <class _Ty>
class vectorInterface {
	private:
	vector<_Ty> *_target;
	SelectedItemInterface<_Ty> *_interface;

	public:
	using value_type = _Ty;

	vectorInterface(SelectedItemInterface<_Ty> *interface) : _interface(interface) {}

	void setTarget(vector<_Ty> *newTarget){
		_target = newTarget;
	}

	vector<_Ty>::iterator begin(){
		return _target->begin() + _interface->index;
	}

	vector<_Ty>::iterator end(){
		return _target->begin() + _interface->index + size();
	}

	size_t size(){
		if (_target == nullptr) { return 0; }
		if (_interface->index < 0) { return 0; }
		if (_interface->index >= _target->size()) { return 0; }
		return 1;
	}
};

template <class _Ty>
class SelectedItemInterface {
	public:
	vectorInterface<_Ty> accessor;
	int index;

	SelectedItemInterface() : accessor(this), index(-1) {}

	void setTarget(vector<_Ty> *newTarget){
		accessor.setTarget(newTarget);
	}
};

struct IntVectorEditable;

class IntVectorScalar {
	private:
	IntVectorEditable* _target;
	vector<int> tmp;

	public:
	Rml::String buffer;

	IntVectorScalar(IntVectorEditable* target) : _target(target), buffer() {}

	void read(const Rml::Variant& variant);
};

struct IntVectorEditable{
	vector<int> data;
	IntVectorScalar scalar;
	IntVectorEditable() : scalar(this) {}
};

void IntVectorScalar::read(const Rml::Variant& variant) {
	buffer.clear();
	tmp.clear();

	int value = -1;
	bool isValid = true;
	for (const char& c : variant.Get<Rml::String>()) {
		if (std::isdigit(c)){
			if ((value != -1) || (c != '0')){
				if (value == -1){
					value = 0;
				}
				value *= 10;
				value += c - '0';
				buffer += c;
			}
		}
		else if ((c == ',' || isspace(c)) && (value != -1)){
			tmp.push_back(value);
			value = -1;
			buffer += ',';
		}
		else {
			isValid = false;
		}
	}
	if (value > 0){
		tmp.push_back(value);
	}

	if (isValid){	
		_target->data.clear();
		for (const int& i : tmp){
			_target->data.push_back(i);
		}
	}
}


struct Service {
    Rml::String name;
	int min_hours;
	int max_hours;
};

struct ClassList {
    int department_id;
    bool subtractive;
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

    int total_hours;
    int min_hours;
    int max_hours;

    WeekSchedule schedule;
    vector<ClassList> classes;
};

struct AppData {
	IntVectorEditable testIntVectorEditable;

	string window_title;
    vector<Tutor> tutors;
	SelectedItemInterface<Tutor> selected_tutor;

	//int selected_tutor;
	int selected_department;
	Rml::String courses_entry;

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

		tutor.classes.clear();
		for (const Json class_json : tutor_json["classes"].array_items()){
			ClassList classList;
			classList.department_id = class_json["department_id"].int_value();
			classList.subtractive = class_json["subtractive"].bool_value();
			
			classList.courses.clear();
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

static int CountTutorScheduleSlots(int tutor_index) {
	if (tutor_index < 0 || tutor_index >= (int)appData.tutors.size())
		return 0;

	int count = 0;
	for (int day = 0; day < 7; ++day) {
		for (int slot = 0; slot < 22; ++slot) {
			int segment = GetScheduleSegment(slot);
			count += appData.tutors[tutor_index].schedule.days[day].segments[segment] ? 1 : 0;
		}
	}
	return count;
}

static std::string FormatTutorHours(int slot_count) {
	int whole_hours = slot_count / 2;
	bool half = (slot_count % 2) != 0;
	char buf[16];
	if (half)
		sprintf(buf, "%d.5", whole_hours);
	else
		sprintf(buf, "%d", whole_hours);
	return std::string(buf);
}

Rml::String DemoWindow::GetSlotElementId(int day, int slot) const {
	char buf[32];
	sprintf(buf, "slot_%d_%d", day, slot);
	return buf;
}

void DemoWindow::BuildScheduleGrid() {
	if (!document)
		return;

	Rml::Element* container = document->GetElementById("schedule_grid_container");
	if (!container)
		return;

	std::string html = "<div class='schedule-grid'>";

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

	if (!schedule_grid_built)
		BuildScheduleGrid();

	int selected_tutor = appData.selected_tutor.index;
	int scheduled_slots = CountTutorScheduleSlots(selected_tutor);
	std::string scheduled_hours = FormatTutorHours(scheduled_slots);
	std::string summary_text;
	if (selected_tutor >= 0 && selected_tutor < (int)appData.tutors.size()) {
		summary_text = "Total: " + scheduled_hours + " hr";
	} else {
		summary_text = "Total: 0 hr";
	}

	if (auto summary_el = document->GetElementById("schedule_hours_summary")) {
		summary_el->SetInnerRML(summary_text);
	}

	for (int day = 0; day < 7; ++day) {
		for (int slot = 0; slot < 22; ++slot) {
			bool selected = selected_tutor >= 0 && selected_tutor < (int)appData.tutors.size() && GetScheduleValue(selected_tutor, day, slot);
			Rml::Element* slot_el = document->GetElementById(GetSlotElementId(day, slot));
			if (!slot_el)
				continue;
			slot_el->SetClass("selected", selected);
		}
	}
}

void DemoWindow::OnTutorChanged() {
	UpdateScheduleGrid();
}

void DemoWindow::OnScheduleCellMouseDown(int day, int slot) {
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

void DemoWindow::OnScheduleCellMouseMove(int day, int slot) {
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

void DemoWindow::OnScheduleCellMouseUp(int day, int slot) {
	schedule_drag_active = false;
	UpdateScheduleGrid();
}
void DemoWindow::AddCourses(Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
	if (arguments.size() != 0){
		return;
	}

	vector<int> courses;


	for (Tutor& tutor : appData.selected_tutor.accessor){
		cout << "Tutor name: " << tutor.first_name << endl;

		for (ClassList& classList : tutor.classes) {
			if (classList.department_id == appData.selected_department){
				cout << "Add courses: ";
				if (appData.courses_entry.empty()) {
					cout << "All";
				} else {
					cout << appData.courses_entry;
				}
				cout << endl;

				dataModelHandle.DirtyAllVariables();

				return;
			}
		}

		ClassList newClassList;
		newClassList.department_id = appData.selected_department;
		newClassList.subtractive = appData.courses_entry.empty();

		tutor.classes.push_back(newClassList);

		dataModelHandle.DirtyAllVariables();
	}
}

void DemoWindow::RemoveCourses(Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
	if (arguments.size() != 0){
		return;
	}

	if (appData.selected_tutor.accessor.size()){
		cout << "Tutor name: " << appData.selected_tutor.accessor.begin()[0].first_name << endl;
		if (appData.courses_entry.empty()){
			cout << "Remove courses: " << "All" << endl;
		}
		else {
			cout << "Remove courses: " << appData.courses_entry << endl;
		}
	}
}

void DemoWindow::ResetSchedule(Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
	if (arguments.size() != 0)
		return;

	int selected_tutor = appData.selected_tutor.index;
	if (selected_tutor < 0 || selected_tutor >= (int)appData.tutors.size())
		return;

	for (int day = 0; day < 7; ++day) {
		for (int slot = 0; slot < 22; ++slot) {
			SetScheduleValue(selected_tutor, day, slot, 0);
		}
	}

	UpdateScheduleGrid();
}

void DemoWindow::AddTutor(Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
	
}

void DemoWindow::RemoveTutor(Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
	
}

bool DemoWindow::Initialize(const Rml::String& title, Rml::Context* context)
{
	// Create data model
	if (Rml::DataModelConstructor constructor = context->CreateDataModel("app_data"))
    {
		
		constructor.RegisterArray<vector<Rml::String>>();
		constructor.RegisterArray<vector<int>>();

		constructor.RegisterScalar<IntVectorScalar>(
			[](const IntVectorScalar& int_vector_scalar, Rml::Variant& variant) { 
				variant = int_vector_scalar.buffer; 
			},
    		// This setter will not set if the input is invalid
    		[](IntVectorScalar& int_vector_scalar, const Rml::Variant& variant) {
				int_vector_scalar.read(variant);
			}
		);

		if (auto handle = constructor.RegisterStruct<IntVectorEditable>())
		{
			handle.RegisterMember("data", &IntVectorEditable::data);
			handle.RegisterMember("scalar", &IntVectorEditable::scalar);
		}

		if (auto handle = constructor.RegisterStruct<ClassList>())
		{
			handle.RegisterMember("department_id", &ClassList::department_id);
			handle.RegisterMember("subtractive", &ClassList::subtractive);
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

		//*
		constructor.RegisterTransformFunc("tutor_min_hours", [](const Rml::VariantList& arguments) -> Rml::Variant {
			const Rml::Variant variant = arguments[0];
			return variant;
			//return Rml::Variant(appData.tutors[index].min_hours);
		});
		//*/

		constructor.RegisterArray<vectorInterface<ClassList>>();

		constructor.RegisterArray<vectorInterface<Tutor>>();
		if (auto handle = constructor.RegisterStruct<SelectedItemInterface<Tutor>>())
		{
			handle.RegisterMember("index", &SelectedItemInterface<Tutor>::index);
			handle.RegisterMember("accessor", &SelectedItemInterface<Tutor>::accessor);
		}

		constructor.Bind("departments", &appData.departments);
		constructor.Bind("services", &appData.services);
		constructor.Bind("tutors", &appData.tutors);
		constructor.Bind("selected_department", &appData.selected_department);
		constructor.Bind("courses_entry", &appData.courses_entry);
		constructor.Bind("selected_tutor", &appData.selected_tutor);

		constructor.BindEventCallback("AddCourses", &DemoWindow::AddCourses, this);
		constructor.BindEventCallback("RemoveCourses", &DemoWindow::RemoveCourses, this);
		constructor.BindEventCallback("ResetSchedule", &DemoWindow::ResetSchedule, this);
		constructor.BindEventCallback("AddTutor", &DemoWindow::AddTutor, this);
		constructor.BindEventCallback("RemoveTutor", &DemoWindow::RemoveTutor, this);

		dataModelHandle = constructor.GetModelHandle();


		constructor.Bind("test_int_vector_editable", &appData.testIntVectorEditable);

		appData.selected_tutor.setTarget(&appData.tutors);
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

		if (key_identifier == Rml::Input::KI_ESCAPE){
			Backend::RequestExit();
		}
	}
	break;

	case EventId::Mouseup:
		schedule_drag_active = false;
		break;

	default: break;
	}
}

Rml::ElementDocument* DemoWindow::GetDocument()
{
	return document;
}