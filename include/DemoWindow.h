#pragma once

#include <RmlUi/Core.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/EventListener.h>

#include <string>
#include <json11.hpp>

using namespace json11;
using namespace std;


#define APPDATA_FILENAME "data/appdata.json"


class DemoWindow : public Rml::EventListener {
public:
	const string& getWindowTitle();
	int getWidth();
	int getHeight();

	bool Load();
	bool Save();
	
	bool Initialize(const Rml::String& title, Rml::Context* context);
	void Shutdown();

	void Update();

	void ProcessEvent(Rml::Event& event) override;

	Rml::ElementDocument* GetDocument();

	void SubmitForm(Rml::String in_submit_message);

	void ChangedTab(Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments);
	void EnableEditTutor(Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments);
	void ConfirmEditTutor(Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments);

	void AddTutor(Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments);
	void RemoveTutor(Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments);

private:
	Rml::ElementDocument* document = nullptr;
	Rml::ElementDocument* iframe = nullptr;

	Rml::DataModelHandle dataModelHandle;
	Json jsonDocument;
};
