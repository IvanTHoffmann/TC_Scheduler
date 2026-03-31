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

	bool load();
	bool save();
	
	bool Initialize(const Rml::String& title, Rml::Context* context);
	void Shutdown();

	void Update();

	void ProcessEvent(Rml::Event& event) override;

	Rml::ElementDocument* GetDocument();

	void SubmitForm(Rml::String in_submit_message);

	// Schedule interaction
	void OnTutorChanged();
	void OnScheduleCellMouseDown(int day, int slot);
	void OnScheduleCellMouseMove(int day, int slot);
	void OnScheduleCellMouseUp(int day, int slot);
	void UpdateScheduleGrid();
	void BuildScheduleGrid();
	Rml::String GetSlotElementId(int day, int slot) const;
	uint8_t GetScheduleValue(int tutor_index, int day, int slot);
	void SetScheduleValue(int tutor_index, int day, int slot, uint8_t value);

	void AddCourses(Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments);
	void RemoveCourses(Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments);
	void AddTutor(Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments);
	void RemoveTutor(Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments);

private:
	Rml::ElementDocument* document = nullptr;
	Rml::ElementDocument* iframe = nullptr;

	Rml::DataModelHandle dataModelHandle;
	Json jsonDocument;

	bool schedule_drag_active = false;
	bool schedule_grid_built = false;
	bool schedule_drag_target_state = true;
};
