#ifndef UTIL_H
#define UTIL_H

#include <RmlUi/Core.h>
#include "json11.hpp"
#include "DataModelTypes.h"

using namespace json11;
using namespace std;

int GetScheduleSegment(int slot);
Rml::String FormatTimeSlot(int slot);
int TimeValueToSlot(int time_value);
int SlotToTimeValue(int slot);
void LoadTutorScheduleFromShifts(Tutor &tutor, const Json &tutor_json);
Json::array SerializeTutorShifts(const Tutor &tutor);
std::string FormatTutorHours(int slot_count);
Rml::String GetSlotElementId(int day, int slot);

#endif