#ifndef TIMETABLEROW_H
#define TIMETABLEROW_H

#include <RmlUi/Core.h>
#include "json11.hpp"
#include <vector>
#include <array>

using namespace std;
using namespace json11;

#include "VectorInterface.h"

struct TimetableSlot {
	string service_name;
	int count;
};

struct TimetableRow {
	Rml::String label;
	vector<TimetableSlot> slots;
	array<SelectedRangeInterface<TimetableSlot>,1> view;

	TimetableRow();
	void SetLabel(string newLabel);
};

#endif // TIMETABLEROW_H