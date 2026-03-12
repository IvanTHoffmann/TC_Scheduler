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

typedef uint8_t BrushIndex_t;


struct BrushType{
    Rml::String label;
};

struct ClassList{
    uint8_t categoryID;
    bool excludeList;
    vector<uint16_t> classes;
};

struct DaySchedule{
    array<BrushIndex_t, 96> segments;
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
    array<ClassList, 12> categories;
};

struct AppData {
	string window_title;
    vector<Tutor> tutors;

	// CONFIG
    int resolution[2];
    float fontSize;
    int schedule_id;

	// INDIVIDUAL
	vector<Rml::String> categories;
	vector<BrushType> brushTypes;
    
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

	json = Json::parse(buf, err);

	//*
	if (err.empty()) {
		cout << "loaded " << APPDATA_FILENAME << ": " << json.dump().c_str() << endl;
	}
	else {
		cout << "failed to load " << APPDATA_FILENAME << ": " << err << endl;
	}
	//*/

	Json settings = json["settings"];

	appData.window_title = settings["window_title"].string_value();
	appData.schedule_id = settings["startup_schedule"].int_value();

	Json::array resolution_options = settings["resolution_options"].array_items();
	int resolution_id = settings["resolution"].int_value();
	Json::array resolution = resolution_options[resolution_id].array_items();
	appData.resolution[0] = resolution[0].int_value();
	appData.resolution[1] = resolution[1].int_value();

	//cout << "settings: " << settings.dump().c_str() << endl;
	//cout << "resolution: (" << appData.resolution[0] << ", " << appData.resolution[1] << ")" << endl;

    return err.empty();
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


bool DemoWindow::Initialize(const Rml::String& title, Rml::Context* context)
{
	// Create data model
	if (Rml::DataModelConstructor constructor = context->CreateDataModel("app_data"))
    {
		if (auto tutor_handle = constructor.RegisterStruct<Tutor>())
		{
			tutor_handle.RegisterMember("first_name", &Tutor::first_name);
			tutor_handle.RegisterMember("last_name", &Tutor::last_name);
		}

		constructor.RegisterArray<vector<Rml::String>>();
		constructor.RegisterArray<vector<Tutor>>();

		constructor.Bind("categories", &appData.categories);
		constructor.Bind("tutors", &appData.tutors);

		//dataModelHandle = constructor.GetModelHandle();
		//dataModelHandle.DirtyVariable("names");
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