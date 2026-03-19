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

using namespace json11;
using namespace std;

typedef uint8_t ServiceIndex_t;


template <class _Ty>
class vectorInterface {
	private:
	vector<_Ty> *v;

	public:
	using value_type = _Ty;

	void update(vector<_Ty> *new_v){
		v = new_v;
	}

	vector<_Ty>::iterator begin(){
		return v->begin();
	}

	vector<_Ty>::iterator end(){
		return v->end();
	}

	size_t size(){
		return (v != nullptr) ? v->size() : 0;
	}
};


struct Service {
    Rml::String name;
	int min_hours;
	int max_hours;
};

struct ClassList{
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
	string window_title;
    vector<Tutor> tutors;
	vectorInterface<ClassList> selectedTutorClasses;

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
	Tutor tutor;
	ClassList classList;
	int courseID;
	for (const Json tutor_json : tutors){
		tutor.first_name = tutor_json["first_name"].string_value();
		tutor.last_name = tutor_json["last_name"].string_value();
		tutor.min_hours = tutor_json["min_hours"].int_value();
		tutor.max_hours = tutor_json["max_hours"].int_value();

		tutor.classes.clear();
		for (const Json class_json : tutor_json["classes"].array_items()){
			cout << "found class" << endl;
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


void DemoWindow::SetTutor(Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments)
{
	if (arguments.size() != 1){
		return;
	}

	const int index = arguments[0].Get<int>();
	if (index == -1){
		appData.selectedTutorClasses.update(nullptr);
	} else {
		appData.selectedTutorClasses.update(&appData.tutors[index].classes);
	}
	
	dataModelHandle.DirtyVariable("selected_tutor_classes");
}

const vector<ClassList>& DemoWindow::GetSelectedTutorClasses(Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments)
{
	if (arguments.size() != 0){
		return;
	}
	
	return appData.tutors[appData.selected_tutor].classes;
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
		constructor.RegisterArray<vectorInterface<ClassList>>();

		constructor.Bind("departments", &appData.departments);
		constructor.Bind("services", &appData.services);
		constructor.Bind("tutors", &appData.tutors);
		constructor.Bind("selected_department", &appData.selected_department);
		constructor.Bind("selected_tutor", &appData.selected_tutor);
		constructor.Bind("selected_tutor_classes", &appData.selectedTutorClasses);

		constructor.BindEventCallback("SetTutor", &DemoWindow::SetTutor, this);
		//constructor.BindEventCallback("GetSelectedTutorClasses", &DemoWindow::GetSelectedTutorClasses, this);

		dataModelHandle = constructor.GetModelHandle();
    }

	using namespace Rml;

	document = context->LoadDocument("assets/demo.rml");
	if (!document)
		return false;

	document->GetElementById("title")->SetInnerRML(title);

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