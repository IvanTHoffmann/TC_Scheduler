#include "Util.h"

int GetScheduleSegment(int slot)
{
    return 32 + slot * 2; // 8:00 is segment 32 (8*4), 30mins increments
}

Rml::String FormatTimeSlot(int slot)
{
    int total_minutes = 8 * 60 + slot * 30;
    int hour = total_minutes / 60;
    int minute = total_minutes % 60;
    char buf[8];
    sprintf(buf, "%02d:%02d", hour, minute);
    return buf;
}

int TimeValueToSlot(int time_value)
{
    // Convert a HHMM integer (e.g. 1330) into a grid slot index.
    int hour = time_value / 100;
    int minute = time_value % 100;
    int total_minutes = hour * 60 + minute;
    int slot = (total_minutes - 8 * 60) / 30;
    if (slot < 0)
        slot = 0;
    if (slot > 22)
        slot = 22;
    return slot;
}

int SlotToTimeValue(int slot)
{
    // Convert a schedule grid slot back into an HHMM integer for JSON storage.
    int total_minutes = 8 * 60 + slot * 30;
    int hour = total_minutes / 60;
    int minute = total_minutes % 60;
    return hour * 100 + minute;
}

void LoadTutorScheduleFromShifts(Tutor &tutor, const Json &tutor_json)
{
    // Read each saved shift object and map it to the schedule's slot matrix.
    for (const Json &shift_json : tutor_json["shifts"].array_items())
    {
        int start = shift_json["start"].int_value();
        int end = shift_json["end"].int_value();
        int start_slot = TimeValueToSlot(start);
        int end_slot = TimeValueToSlot(end);
        if (end_slot <= start_slot)
            continue;

        for (const Json &day_json : shift_json["days"].array_items())
        {
            int day = day_json.int_value();
            if (day < 0 || day >= 7)
                continue;
            for (int slot = start_slot; slot < end_slot && slot < 22; ++slot)
            {
                int segment = GetScheduleSegment(slot);
                tutor.schedule.days[day].segments[segment] = 1;
            }
        }
    }
}

Json::array SerializeTutorShifts(const Tutor &tutor)
{
    // Build JSON shift objects from contiguous selected slots in the tutor schedule.
    Json::array shifts_json;
    for (int day = 0; day < 7; ++day)
    {
        int slot = 0;
        while (slot < 22)
        {
            if (!tutor.schedule.days[day].segments[GetScheduleSegment(slot)])
            {
                slot++;
                continue;
            }

            int start_slot = slot;
            while (slot < 22 && tutor.schedule.days[day].segments[GetScheduleSegment(slot)])
            {
                slot++;
            }

            int end_slot = slot;
            Json::object shift_obj;
            Json::array days_json;
            days_json.push_back(day);
            shift_obj["days"] = days_json;
            shift_obj["service_type"] = 0;
            shift_obj["start"] = SlotToTimeValue(start_slot);
            shift_obj["end"] = SlotToTimeValue(end_slot);
            shifts_json.push_back(shift_obj);
        }
    }
    return shifts_json;
}

std::string FormatTutorHours(int slot_count)
{
    int whole_hours = slot_count / 2;
    bool half = (slot_count % 2) != 0;
    char buf[16];
    if (half)
        sprintf(buf, "%d.5", whole_hours);
    else
        sprintf(buf, "%d", whole_hours);
    return std::string(buf);
}

Rml::String GetSlotElementId(int day, int slot)
{
    char buf[32];
    sprintf(buf, "slot_%d_%d", day, slot);
    return buf;
}

string FormatList(const vector<string>& inStrings)
{
    string listString = "";
    bool needsCommas = (inStrings.size() > 2);
	for (int i = 0; i < inStrings.size(); i++)
	{
		bool isFirst = (i == 0);
		bool isLast = (i == inStrings.size() - 1);
		if (!isFirst)
		{
			listString += needsCommas ? ", " : " ";
			if (isLast)
			{
				listString += "and ";
			}
		}
        listString += inStrings[i];
	}
    return listString;
}

bool ReadUntil(istream& inStream, ostream& outStream, char delim)
{
    char c;
    while (inStream.get(c)){
        if (c == delim){
            return true;
        }
        outStream << c;
    }
    return false;
}
