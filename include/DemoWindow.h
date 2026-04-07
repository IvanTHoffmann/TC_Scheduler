#pragma once

#include <RmlUi/Core.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/EventListener.h>

#include "DataModelTypes.h"

#include <string>
#include <json11.hpp>

using namespace json11;
using namespace std;


#define APPDATA_FILENAME "data/appdata.json"

#define CALLBACK_PARAMS Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments


class DemoWindow : public Rml::EventListener {
public:
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

<<<<<<< Updated upstream
	// Schedule interaction
	void OnTutorChanged();
	void OnScheduleCellMouseDown(int day, int slot);
	void OnScheduleCellMouseMove(int day, int slot);
	void OnScheduleCellMouseUp(int day, int slot);
	void UpdateScheduleGrid();
	void UpdateScheduleSummary();
	void BuildScheduleGrid();
	Rml::String GetSlotElementId(int day, int slot) const;
	uint8_t GetScheduleValue(int tutor_index, int day, int slot);
	void SetScheduleValue(int tutor_index, int day, int slot, uint8_t value);

	void ChangedTab(Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments);
	void EnableEditTutor(Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments);
	void ConfirmEditTutor(Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments);
	void ResetSchedule(Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments);
	void ScheduleLimitsChanged(Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments);
=======
	void ChangedTab(CALLBACK_PARAMS);
	void EnableEditTutor(CALLBACK_PARAMS);
	void ConfirmEditTutor(CALLBACK_PARAMS);
>>>>>>> Stashed changes

	void AddTutor(CALLBACK_PARAMS);
	void RemoveTutor(CALLBACK_PARAMS);

	string GetExportPath(string subDirectory, string filename);

	void ExportTutorPage(CALLBACK_PARAMS);
	void ExportSubjectPage(CALLBACK_PARAMS);
	void ExportTimetable(CALLBACK_PARAMS);
	void ExportRolodex(CALLBACK_PARAMS);
	void ExportAll(CALLBACK_PARAMS);

private:
	Rml::ElementDocument* document = nullptr;
	Rml::ElementDocument* iframe = nullptr;

	Rml::DataModelHandle dataModelHandle;
	Json jsonDocument;

<<<<<<< Updated upstream
	bool schedule_drag_active = false;
	bool schedule_grid_built = false;
	bool schedule_drag_target_state = true;
=======
	AppData appData;
>>>>>>> Stashed changes
};
