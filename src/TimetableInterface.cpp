#include "TimetableInterface.h"

// TimetableInterface implementation
TimetableInterface::TimetableInterface()
{
	view[0].setTarget(&labels);
}

void TimetableInterface::SetSlotsPerHour(int newSlotsPerHour)
{
	switch (newSlotsPerHour) {
		case 1:
		case 2:
		case 4:
			slotsPerHour = newSlotsPerHour;
	};
}


void TimetableInterface::SyncRowViews(){
	// adjust view[0].index and size to prevent hiding scheduled shifts
	for (TimetableRow& row : rows){
		row.view[0].index = view[0].index * slotsPerHour;
		row.view[0].size = view[0].size * slotsPerHour;
	}
}

ServiceIndex_t &TimetableInterface::GetServiceIndex(int dayIndex, int slotIndex)
{
	auto& row = rows.at(dayIndex);
	int globalSlotIndex = row.view[0].index + slotIndex;
	return row.slots[globalSlotIndex].value;
}

void TimetableInterface::SetStartHour(int newStartHour)
{
	newStartHour = max(0, min(24, newStartHour));
	view[0].size = GetEndHour() - newStartHour;
	view[0].index = newStartHour;
	if (view[0].size < minHours){
		// enforce minimum view size
		SetEndHour(newStartHour + minHours);
	}
	SyncRowViews();
}

void TimetableInterface::SetEndHour(int newEndHour)
{
	newEndHour = max(0, min(24, newEndHour));
	view[0].size = newEndHour - GetStartHour();
	if (view[0].size < minHours){
		// enforce minimum view size
		SetStartHour(newEndHour - minHours);
	}
	SyncRowViews();
}

int TimetableInterface::SlotToTime(int slot)
{
	// Convert a schedule grid slot back into an HHMM integer for JSON storage.
    int total_minutes = slot * (int)(60 / slotsPerHour);
    int hour = total_minutes / 60;
    int minute = total_minutes % 60;
    return hour * 100 + minute;
    return 0;
}

int TimetableInterface::TimeToSlot(int time_value)
{
	// Convert a HHMM integer (e.g. 1330) into a grid slot index.
    time_value = max(0, min(2400, time_value));
    int hour = time_value / 100;
    int minute = min(60, time_value % 100);
    int total_slots = hour * slotsPerHour + (int)(minute * slotsPerHour / 60);
    return total_slots;
}

int TimetableInterface::GetStartHour()
{
    return view[0].index;
}

int TimetableInterface::GetEndHour()
{
    return view[0].index + view[0].size;
}

void TimetableInterface::Save(WeekSchedule &schedule)
{
	for (int dayIndex = 0; dayIndex < 7; ++dayIndex){
		TimetableRow& row = rows.at(dayIndex);
		DaySchedule& daySchedule = schedule.days.at(dayIndex);
		daySchedule.shifts.clear();

		ServiceIndex_t shiftService = -1;
		int start_slot = 0;
		for (int end_slot = 0; end_slot <= row.slots.size(); end_slot++)
		{
			ServiceIndex_t slotService = ((end_slot == row.slots.size()) ? -1 : row.slots.at(end_slot).value);
			if (slotService == shiftService)
			{
				// slot matches current shift type
				continue;
			}

			// service changed
			if (shiftService != -1)
			{
				// found end of shift
				daySchedule.shifts.push_back({});
				ShiftSchedule &shift = daySchedule.shifts.back();
				shift.service_type = shiftService;
				shift.start = SlotToTime(start_slot);
				shift.end = SlotToTime(end_slot);
			}

			// start another shift
			start_slot = end_slot;
			shiftService = slotService;
		}
	}
}

void TimetableInterface::Clear()
{
    for (int dayIndex = 0; dayIndex < 7; ++dayIndex)
    {
        TimetableRow& row = rows.at(dayIndex);

        // 1. Ensure the vector is allocated for the full 24 hours
        row.slots.resize(24 * slotsPerHour);

        // 2. Wipe the data
        for (TimetableSlot& slot : row.slots){
            slot.value = UNSCHEDULED_SLOT;
        }
    }
}

void TimetableInterface::Load(const WeekSchedule &schedule)
{
	// Set start and end hours to invalid placeholders
	// The timetable will adjust its view to contain the schedule
	int startHour = 24;
	int endHour = 0;
	for (int dayIndex = 0; dayIndex < 7; ++dayIndex){
		TimetableRow& row = rows.at(dayIndex);
		const DaySchedule& daySchedule = schedule.days[dayIndex];

		// reset slots
		row.slots.resize(24 * slotsPerHour);
		for (TimetableSlot& slot : row.slots){
			slot.value = UNSCHEDULED_SLOT;
		}
		
		// load shifts
		for (const ShiftSchedule& shift : daySchedule.shifts)
		{
			int start_slot = TimeToSlot(shift.start);
			int end_slot = TimeToSlot(shift.end);
			if (end_slot <= start_slot){
				continue;
			}
			for (int slot = start_slot; slot < end_slot; ++slot)
			{
				switch(colorMode){
				case ColorModeEnum::SERVICE:
					row.slots.at(slot).value = shift.service_type;
					break;
				case ColorModeEnum::HEATMAP:
					row.slots.at(slot).value += 1;
					break;
				}
			}
		}
	}
}