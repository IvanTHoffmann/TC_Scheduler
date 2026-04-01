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
#include <map>

#include "DataModelTypes.h"


using namespace json11;
using namespace std;


AppData appData;


bool DemoWindow::Load() {
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
	Department department;
	for (const Json deptName : departments) {
		department.name = deptName.string_value();
		department.edit_subtractive = false;
		appData.departments.push_back(department);
	}
	
	Json::array services = schedule["services"].array_items();
	Service service;
	for (const Json service_json : services) {
		service.name = service_json["name"].string_value();
		service.min_hours = service_json["min_hours"].int_value();
		service.max_hours = service_json["max_hours"].int_value();
		appData.services.push_back(service);
	}
	
	Json::array tutors = schedule["tutors"].array_items();
	Tutor tutor;
	ClassList classList;
	int courseID;
	for (const Json tutor_json : tutors) {
		tutor.first_name = tutor_json["first_name"].string_value();
		tutor.last_name = tutor_json["last_name"].string_value();
		tutor.min_hours = tutor_json["min_hours"].int_value();
		tutor.max_hours = tutor_json["max_hours"].int_value();

		tutor.classes.clear();
		for (const Json class_json : tutor_json["classes"].array_items()) {
			classList.department_name = appData.departments[class_json["department_id"].int_value()].name;
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

bool DemoWindow::Save(){
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


// EVENT CALLBACKS

void DemoWindow::ChangedTab(Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
	appData.edit_tutor = false;
	dataModelHandle.DirtyAllVariables();
}


void DemoWindow::EnableEditTutor(Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
	Tutor* tutor = appData.selected_tutor.accessor.ptr();

	if (tutor == nullptr){
		return;
	}

	for (Department& dept: appData.departments){
		dept.edit_subtractive = false;
		dept.edit_courses.clear();
		dept.edit_formatted_courses.syncBuffer();
	}

	for (const ClassList& classList : tutor->classes){
		for (Department& dept : appData.departments){
			if (dept.name == classList.department_name){
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

void DemoWindow::ConfirmEditTutor(Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
	Tutor* tutor = appData.selected_tutor.accessor.ptr();

	if (tutor == nullptr){
		return;
	}

	tutor->classes.clear();
	for (Department& dept: appData.departments){
		if (dept.edit_subtractive || !dept.edit_courses.empty()){
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


void DemoWindow::AddTutor(Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
	
}

void DemoWindow::RemoveTutor(Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
	
}


// INIT

bool DemoWindow::Initialize(const Rml::String& title, Rml::Context* context)
{

// Create data model
	if (Rml::DataModelConstructor constructor = context->CreateDataModel("app_data")) {

		constructor.RegisterArray<vector<Rml::String>>();

		// Register IntVectorEditable
		
		constructor.RegisterArray<vector<int>>();
		constructor.RegisterScalar<FormattedIntVector>(Get_IntVectorScalar, Set_IntVectorScalar);
			
		// Register vector<Department>
		if (auto handle = constructor.RegisterStruct<Department>()) {
			handle.RegisterMember("name", &Department::name);
			handle.RegisterMember("edit_subtractive", &Department::edit_subtractive);
			handle.RegisterMember("edit_courses", &Department::edit_courses);
			handle.RegisterMember("edit_formatted_courses", &Department::edit_formatted_courses);
		}
		constructor.RegisterArray<vector<Department>>();
			
		// Register vector<ClassList>
		if (auto handle = constructor.RegisterStruct<ClassList>()) {
			handle.RegisterMember("department_name", &ClassList::department_name);
			handle.RegisterMember("subtractive", &ClassList::subtractive);
			handle.RegisterMember("courses", &ClassList::courses);
		}
		constructor.RegisterArray<vector<ClassList>>();
		
		// Register vector<Tutor>
		if (auto handle = constructor.RegisterStruct<Tutor>()) {
			handle.RegisterMember("first_name", &Tutor::first_name);
			handle.RegisterMember("last_name", &Tutor::last_name);
			handle.RegisterMember("min_hours", &Tutor::min_hours);
			handle.RegisterMember("max_hours", &Tutor::max_hours);
			handle.RegisterMember("classes", &Tutor::classes);
		}
		constructor.RegisterArray<vector<Tutor>>();
		
		// Register vector<Service>
		if (auto handle = constructor.RegisterStruct<Service>()) {
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
			
		// Bind AppData members
		constructor.Bind("departments", &appData.departments);
		constructor.Bind("services", &appData.services);
		constructor.Bind("tutors", &appData.tutors);
		constructor.Bind("selected_department", &appData.selected_department);
		constructor.Bind("selected_tutor", &appData.selected_tutor);
		constructor.Bind("edit_tutor", &appData.edit_tutor);

		constructor.BindEventCallback("ChangedTab", &DemoWindow::ChangedTab, this);
		constructor.BindEventCallback("EnableEditTutor", &DemoWindow::EnableEditTutor, this);
		constructor.BindEventCallback("ConfirmEditTutor", &DemoWindow::ConfirmEditTutor, this);
		constructor.BindEventCallback("AddTutor", &DemoWindow::AddTutor, this);
		constructor.BindEventCallback("RemoveTutor", &DemoWindow::RemoveTutor, this);

		dataModelHandle = constructor.GetModelHandle();

		appData.selected_tutor.setTarget(&appData.tutors);
    }

	using namespace Rml;

	document = context->LoadDocument("assets/demo.rml");
	if (!document){
		return false;
	}

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

	switch (event.GetId()) {
		case EventId::Keydown:
		{
			Rml::Input::KeyIdentifier key_identifier = (Rml::Input::KeyIdentifier)event.GetParameter<int>("key_identifier", 0);

			if (key_identifier == Rml::Input::KI_ESCAPE){
				Save();
				Backend::RequestExit();
			}
			break;
		}
		default:
		{
			break;
		}
	}
}

Rml::ElementDocument* DemoWindow::GetDocument()
{
	return document;
}