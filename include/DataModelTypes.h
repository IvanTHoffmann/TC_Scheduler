#ifndef DATAMODELTYPES_H
#define DATAMODELTYPES_H

#include <RmlUi/Core.h>

#include <vector>
#include <array>
#include <map>

using namespace std;


typedef uint8_t ServiceIndex_t;
typedef int DepartmentID_t;


template <class _Ty>
class SelectedItemInterface;


template <class _Ty>
class VectorInterface {
	private:
	vector<_Ty> *_target;
	SelectedItemInterface<_Ty> *_interface;

	public:
	using value_type = _Ty;

	VectorInterface(SelectedItemInterface<_Ty> *interface) : _interface(interface) {}

	void setTarget(vector<_Ty> *newTarget) {
		_target = newTarget;
	}

	vector<_Ty>::iterator begin(){
		return _target->begin() + _interface->index;
	}

	vector<_Ty>::iterator end(){
		return _target->begin() + _interface->index + size();
	}

	_Ty* ptr();
	
	size_t size(){
		if (_target == nullptr) { return 0; }
		if (_interface->index < 0) { return 0; }
		if (_interface->index >= _target->size()) { return 0; }
		return 1;
	}
};

template <class _Ty>
_Ty* VectorInterface<_Ty>::ptr(){
	return size() ? &(_target->at(_interface->index)) : nullptr;
}

template <class _Ty>
class SelectedItemInterface {
	public:
	VectorInterface<_Ty> accessor;
	int index;

	SelectedItemInterface() : accessor(this), index(-1) {}
	void setTarget(vector<_Ty> *newTarget) {
		accessor.setTarget(newTarget);
	}
};

class FormattedIntVector {
	private:
	vector<int> *_target;

	public:
	Rml::String buffer;

	void syncBuffer();
	void setTarget(vector<int> *newTarget);
	void read(const Rml::Variant& variant);
};

void Get_IntVectorScalar(const FormattedIntVector& int_vector_scalar, Rml::Variant& variant);
void Set_IntVectorScalar(FormattedIntVector& int_vector_scalar, const Rml::Variant& variant);

struct Department {
	Rml::String name;

    // used for editing tutor class lists
    bool edit_subtractive;
	vector<int> edit_courses;
	FormattedIntVector edit_formatted_courses;

	Department();
	Department(const Department& src);
};

struct Service {
    Rml::String name;
	int min_hours;
	int max_hours;
};

struct ClassList {
    Rml::String department_name;
    bool subtractive;
    vector<int> courses;
};

struct DaySchedule {
    array<ServiceIndex_t, 96> segments;
};

struct WeekSchedule {
    array<DaySchedule, 7> days;
};

struct Tutor {
	Rml::String first_name, last_name, email;

    int total_hours;
    int min_hours;
    int max_hours;

    WeekSchedule schedule;
    vector<ClassList> classes;
};

struct AppData {
	Rml::String window_title;
	Rml::String export_dir;
    vector<Tutor> tutors;
	SelectedItemInterface<Tutor> selected_tutor;
	SelectedItemInterface<Department> selected_department;

	Rml::String term_season;
	int term_year;

	Rml::String courses_entry;

	// PREFERENCES
	bool dev_enable;
	vector<array<int, 2>> resolutionOptions;
    int resolution[2];
    float fontSize;
    int schedule_id;

	// INDIVIDUAL
	bool edit_tutor;
	vector<Department> departments;
	vector<Service> services;
    
	// SUMMARY
	vector<Rml::String> filters;
};



#endif