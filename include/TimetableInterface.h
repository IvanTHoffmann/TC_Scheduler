#ifndef TIMETABLEINTERFACE_H
#define TIMETABLEINTERFACE_H

#include <RmlUi/Core.h>
#include "json11.hpp"
#include <vector>
#include <array>
#include <string>

using namespace std;
using namespace json11;

#include "VectorInterface.h"
#include "TimetableRow.h"
#include "Schedule.h"

class TimetableInterface
{
	public:
	vector<string> labels = {"12a", "1a", "2a", "3a", "4a", "5a", "6a", "7a", "8a", "9a", "10a", "11a", 
						     "12p", "1p", "2p", "3p", "4p", "5p", "6p", "7p", "8p", "9p", "10p", "11p", "12a"};

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
	void Clear(); // sets all slots to UNSCHEDULED_SLOT
};

#endif // TIMETABLEINTERFACE_H