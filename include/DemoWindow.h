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
	float CalculateTotalHours(const WeekSchedule& schedule);
	void UpdateTotalHours();

	void OnClick_SummaryTab(CALLBACK_PARAMS);
	void OnClick_SchedulesTab(CALLBACK_PARAMS);
	void OnClick_ClassesTab(CALLBACK_PARAMS);

	void SetSelectedServices(CALLBACK_PARAMS);
	void SetSelectedDepartments(CALLBACK_PARAMS);
	void SetSelectedTutors(CALLBACK_PARAMS);

	void AddStartTime(CALLBACK_PARAMS);
	void AddEndTime(CALLBACK_PARAMS);
	
	void OnTutorChanged(CALLBACK_PARAMS);
	void EnableEditTutor(CALLBACK_PARAMS);
	void ConfirmEditTutor(CALLBACK_PARAMS);
	void ScheduleLimitsChanged(CALLBACK_PARAMS);

	void AddTutor(CALLBACK_PARAMS);
	void RemoveTutor(CALLBACK_PARAMS);
	void AddService(CALLBACK_PARAMS);
	void RemoveService(CALLBACK_PARAMS);
	void AddDepartment(CALLBACK_PARAMS);
	void RemoveDepartment(CALLBACK_PARAMS);

	void ExportTutorPage(CALLBACK_PARAMS);
	void ExportSubjectPage(CALLBACK_PARAMS);
	void ExportTimetable(CALLBACK_PARAMS);
	void ExportRolodex(CALLBACK_PARAMS);
	void ExportAll(CALLBACK_PARAMS);
	void OnSlotMouseDown(CALLBACK_PARAMS);
	void OnSlotMouseOver(CALLBACK_PARAMS);
	void OnTimetableMouseUp(CALLBACK_PARAMS);
	void OnTimetableMouseOut(CALLBACK_PARAMS);

private:
	void ColorSlot(int, int);

	Rml::ElementDocument* document = nullptr;
	Rml::ElementDocument* iframe = nullptr;

	Rml::DataModelHandle dataModelHandle;
	Json jsonDocument;

	int schedule_drag_button = -1;
	ServiceIndex_t schedule_service_brush;

	AppData appData;
	Exporter exporter;
};
