#pragma once

#include <RmlUi/Core.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/EventListener.h>

#include <json11.hpp>

using namespace json11;

#define APPDATA_FILENAME "data/appdata.json"


class DemoWindow : public Rml::EventListener {
public:
	bool loadData();
	
	bool Initialize(const Rml::String& title, Rml::Context* context);
	void Shutdown();

	void Update();

	void ProcessEvent(Rml::Event& event) override;

	Rml::ElementDocument* GetDocument();

	void SubmitForm(Rml::String in_submit_message);

private:
	Rml::ElementDocument* document = nullptr;
	Rml::ElementDocument* iframe = nullptr;

	Rml::DataModelHandle dataModelHandle;
	Json json;
};
