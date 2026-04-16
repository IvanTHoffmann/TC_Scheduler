#ifndef DATAMODELTYPES_H
#define DATAMODELTYPES_H

#include <RmlUi/Core.h>
#include "json11.hpp"

#include <vector>
#include <array>
#include <map>

using namespace std;
using namespace json11;

typedef int DepartmentID_t;
typedef int ServiceIndex_t;
const ServiceIndex_t UNSCHEDULED_SLOT = -1;

template <class _Ty>
class SelectedRangeInterface;

template <class _Ty>
class VectorInterface
{
private:
	vector<_Ty> *_target;
	SelectedRangeInterface<_Ty> *_interface;

public:
	using value_type = _Ty;

	VectorInterface(SelectedRangeInterface<_Ty> *interface) : _interface(interface) {}

	void setTarget(vector<_Ty> *newTarget)
	{
		_target = newTarget;
	}

	vector<_Ty>::iterator begin()
	{
		return _target->begin() + _interface->index;
	}

	vector<_Ty>::iterator end()
	{
		return _target->begin() + _interface->index + size();
	}

	_Ty *ptr();

	size_t size()
	{
		if (_target == nullptr)
		{
			return 0;
		}
		if (_interface->index < 0)
		{
			return 0;
		}
		if (_interface->index + _interface->size > _target->size())
		{
			return 0;
		}
		return _interface->size;
	}
};

template <class _Ty>
_Ty *VectorInterface<_Ty>::ptr()
{
	return size() ? &(_target->at(_interface->index)) : nullptr;
}

template <class _Ty>
class SelectedRangeInterface
{
public:
	VectorInterface<_Ty> accessor;
	int index;
	int size;

	SelectedRangeInterface() : accessor(this), index(-1) {}
	void setTarget(vector<_Ty> *newTarget)
	{
		accessor.setTarget(newTarget);
	}
};

class FormattedIntVector
{
private:
	vector<int> *_target;

public:
	Rml::String buffer;

	void syncBuffer();
	void setTarget(vector<int> *newTarget);
	void read(const Rml::Variant &variant);
};

void Get_IntVectorScalar(const FormattedIntVector &int_vector_scalar, Rml::Variant &variant);
void Set_IntVectorScalar(FormattedIntVector &int_vector_scalar, const Rml::Variant &variant);

struct Department
{
	Rml::String name;
	bool selected; // used for summary page

	// used for editing tutor class lists
	bool edit_subtractive;
	vector<int> edit_courses;
	FormattedIntVector edit_formatted_courses;

	Department();
	Department(const Department &src);

	// SAVE/LOAD
	void Save(Json::object &outElement) const;
	void Load(const Json::object &inElement);
};

struct Service
{
	Rml::String name;
	bool selected; // used for summary page

	int min_hours;
	int max_hours;
	Rml::Colourb color{255, 0, 0};

	Rml::String GetColor() {
		return Rml::ToString(color);
	}

	// SAVE/LOAD
	void Save(Json::object &outElement) const;
	void Load(const Json::object &inElement);
};

struct ClassList
{
	Rml::String department_name;
	bool subtractive;
	vector<int> courses;

	// SAVE/LOAD
	void Save(Json::object &outElement) const;
	void Load(const Json::object &inElement);
};

struct ShiftSchedule{
	int start, end;
	ServiceIndex_t service_type;

	// SAVE/LOAD
	void Save(Json::object &outElement) const;
	void Load(const Json::object &inElement);
};

struct DaySchedule
{
	vector<ShiftSchedule> shifts;

	// SAVE/LOAD
	void Save(Json::array &outElement) const;
	void Load(const Json::array &inElement);
};

struct WeekSchedule
{
	array<DaySchedule, 7> days;

	// SAVE/LOAD
	void Save(Json::array &outElement) const;
	void Load(const Json::array &inElement);
};

struct Tutor
{
	Rml::String first_name, last_name, email;
	bool selected; // used for summary page

	int total_hours;
	int min_hours;
	int max_hours;

	WeekSchedule schedule;
	vector<ClassList> classes;

	// SAVE/LOAD
	void Save(Json::object &outElement) const;
	void Load(const Json::object &inElement);
};

struct TimetableSlot {
	int value; // Can be ServiceIndex or a number of occurances
};

struct TimetableRow {
	Rml::String label;
	vector<TimetableSlot> slots;
	array<SelectedRangeInterface<TimetableSlot>,1> view;

	TimetableRow();
	void SetLabel(string newLabel);
};

class TimetableInterface
{
	public:
	vector<string> labels = {"12:00am", "1:00am", "2:00am", "3:00am", "4:00am", "5:00am", "6:00am", "7:00am", "8:00am", "9:00am", "10:00am", "11:00am", 
						     "12:00pm", "1:00pm", "2:00pm", "3:00pm", "4:00pm", "5:00pm", "6:00pm", "7:00pm", "8:00pm", "9:00pm", "10:00pm", "11:00pm", "12:00am"};

	// View is stored in an array of size 1 because rmlui doesn't allow directly nesting structs.
	// I think this is a bug. I might look into it later.
	array<SelectedRangeInterface<string>, 1> view;

	array<TimetableRow, 7> rows;
	int slotsPerHour = 2;
	int minHours = 8;
	
	bool editable = false;
	enum ColorModeEnum {
		SERVICE,
		HEATMAP
	} colorMode;

	TimetableInterface();
	void SetSlotsPerHour(int newSlotsPerHour);
	void SetStartHour(int newStartHour);
	void SetEndHour(int newEndHour);
	int SlotToTime(int slot);
	int TimeToSlot(int time);
	int GetStartHour();
	int GetEndHour();
	void SyncRowViews();

	ServiceIndex_t& GetServiceIndex(int dayIndex, int slotIndex);

	void Load(const WeekSchedule& schedule);
	void Save(WeekSchedule& schedule);
};

struct AppData
{
	Rml::String current_tab;
	Rml::String window_title;
	Rml::String export_dir;
	
	enum TimetableDisplayMode {
		SUMMARY,
		INDIVIDUAL
	} timetableDisplayMode;
	TimetableInterface timetable;
	// SummaryFilter summary_filter; // flag each item instead
	
	// PREFERENCES
	vector<Rml::String> schedule_names;
	Rml::String term_season;
	int term_year;
	int schedule_id;
	bool dev_enable;
	int resolution[2];
	float fontSize;
	
	// INDIVIDUAL
	int total_hours; // TODO: implement
	bool edit_tutor;
	bool filter_all_services;
	bool filter_all_departments;
	bool filter_all_tutors;
	vector<Rml::String> budgets;
	vector<Department> departments;
	vector<Service> services;
	vector<Tutor> tutors;
	SelectedRangeInterface<Department> selected_department;
	SelectedRangeInterface<Service> selected_service;
	SelectedRangeInterface<Tutor> selected_tutor;

	// SAVE/LOAD
	void SaveSchedule(Json::object &outElement) const;
	void LoadSchedule(const Json::object &inElement);

	void SaveSettings(Json::object &outElement) const;
	void LoadSettings(const Json::object &inElement);
};

#endif