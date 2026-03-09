#include "DemoWindow.h"
#include "RmlUi/Core/StreamMemory.h"
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Elements/ElementFormControl.h>
#include <RmlUi/Core/FileInterface.h>
#include <RmlUi/Core/StyleSheetContainer.h>
#include <RmlUi_Backend.h>
#include <RmlUi/Core.h>

#include <vector>
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
    vector<Tutor> tutors;

	// CONFIG
    int resolution[2];
    float fontSize, fontSpacing;
    Rml::String scheduleName;

	// INDIVIDUAL
	vector<Rml::String> categories;
	vector<BrushType> brushTypes;
    
	// SUMMARY
	vector<Rml::String> filters;
	WeekSchedule schedule;

} appData;


bool DemoWindow::Initialize(const Rml::String& title, Rml::Context* context)
{
	// Load schedule

	// Create data model
	if (Rml::DataModelConstructor constructor = context->CreateDataModel("app_data"))
    {
		if (auto tutor_handle = constructor.RegisterStruct<Tutor>())
		{
			tutor_handle.RegisterMember("first_name", &Tutor::firstName);
			tutor_handle.RegisterMember("last_name", &Tutor::lastName);
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