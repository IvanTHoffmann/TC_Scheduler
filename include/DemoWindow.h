#pragma once

#include <RmlUi/Core.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/EventListener.h>

#include "DataModelTypes.h"
#include "Exporter.h"

#include <string>
#include <json11.hpp>

using namespace json11;
using namespace std;


#define APPDATA_FILENAME "data/appdata.json"

#define CALLBACK_PARAMS Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments


class DemoWindow : public Rml::EventListener {
public:
	DemoWindow();
	~DemoWindow();

	const string& GetWindowTitle();
	int GetWidth();
	int GetHeight();

	bool Load();
	bool Save();
	
	bool Initialize(const Rml::String& title, Rml::Context* context);
	void Shutdown();
	void Update();

	void ProcessEvent(Rml::Event& event) override;

	Rml::ElementDocument* GetDocument();

	void SubmitForm(Rml::String in_submit_message);

	// Schedule interaction
	void UpdateTimetable();
	void UpdateScheduleSummary();
	int CountTutorScheduleSlots(int tutor_index);

	void OnClick_SummaryTab(CALLBACK_PARAMS);
	void OnClick_SchedulesTab(CALLBACK_PARAMS);
	void OnClick_ClassesTab(CALLBACK_PARAMS);

	void ToggleAllServices(CALLBACK_PARAMS);
	void ToggleAllDepartments(CALLBACK_PARAMS);
	void ToggleAllTutors(CALLBACK_PARAMS);
	
	void OnTutorChanged(CALLBACK_PARAMS);
	void EnableEditTutor(CALLBACK_PARAMS);
	void ConfirmEditTutor(CALLBACK_PARAMS);
	void ScheduleLimitsChanged(CALLBACK_PARAMS);

	void AddTutor(CALLBACK_PARAMS);
	void RemoveTutor(CALLBACK_PARAMS);

	void ExportTutorPage(CALLBACK_PARAMS);
	void ExportSubjectPage(CALLBACK_PARAMS);
	void ExportTimetable(CALLBACK_PARAMS);
	void ExportRolodex(CALLBACK_PARAMS);
	void ExportAll(CALLBACK_PARAMS);
	void OnSlotMouseDown(CALLBACK_PARAMS);
	void OnSlotMouseOver(CALLBACK_PARAMS);
	void OnSlotMouseUp(CALLBACK_PARAMS);

private:
	Rml::ElementDocument* document = nullptr;
	Rml::ElementDocument* iframe = nullptr;

	Rml::DataModelHandle dataModelHandle;
	Json jsonDocument;

	bool schedule_drag_active = false;
	ServiceIndex_t schedule_service_brush;

	AppData appData;
	Exporter exporter;
};
