#include "DataModelTypes.h"
#include "Util.h"

void FormattedIntVector::read(const Rml::Variant &variant)
{
	// Sanitize input and place
	buffer.clear();

	char last = ',';
	for (char c : variant.Get<Rml::String>())
	{
		c = isspace(c) ? ',' : c;

		bool numBegin = (last == ',') && std::isdigit(c) && (c != '0');		 // numbers can only begin with non-zero digits
		bool numContinue = (last != ',') && (std::isdigit(c) || (c == ',')); // digits can only be followed by digits and commas

		if (numBegin || numContinue)
		{
			buffer += c;
			last = c;
		}
	}

	_target->clear();

	int value = 0;
	for (char c : buffer)
	{
		if (c == ',')
		{
			_target->push_back(value);
			value = 0;
		}
		else
		{
			value = (value * 10) + (c - '0');
		}
	}
	if (value)
	{
		_target->push_back(value);
	}
}

void Get_IntVectorScalar(const FormattedIntVector &int_vector_scalar, Rml::Variant &variant)
{
	variant = int_vector_scalar.buffer;
}

void Set_IntVectorScalar(FormattedIntVector &int_vector_scalar, const Rml::Variant &variant)
{
	int_vector_scalar.read(variant);
}

void FormattedIntVector::syncBuffer()
{
	buffer.clear();
	for (int i : *_target)
	{
		buffer += to_string(i) + ',';
	}
}

void FormattedIntVector::setTarget(vector<int> *newTarget)
{
	_target = newTarget;
}

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
	return row.slots[globalSlotIndex];
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
			ServiceIndex_t slotService = ((end_slot == row.slots.size()) ? -1 : row.slots.at(end_slot));
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
		for (ServiceIndex_t& slot : row.slots){
			slot = UNSCHEDULED_SLOT;
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
				row.slots.at(slot) = shift.service_type;
			}
		}
	}
}

Department::Department()
{
	edit_formatted_courses.setTarget(&edit_courses);
}

Department::Department(const Department &src)
{
	name = src.name;
	edit_subtractive = src.edit_subtractive;
	edit_courses = src.edit_courses;
	edit_formatted_courses = src.edit_formatted_courses;
	edit_formatted_courses.setTarget(&edit_courses);
}

void Service::Save(Json::object &outElement) const
{
	// Rml::String name;
	outElement["name"] = name;

	// int min_hours;
	outElement["min_hours"] = min_hours;

	// int max_hours;
	outElement["max_hours"] = max_hours;

	// color
	Json::array color_json;
	for(int i=0; i<3; i++){
		color_json.push_back(color[i]);
	}
	outElement["color"] = color_json;
}

void Service::Load(const Json::object &inElement)
{
	// Rml::String name;
	name = inElement.at("name").string_value();

	// int min_hours;
	min_hours = inElement.at("min_hours").int_value();

	// int max_hours;
	max_hours = inElement.at("max_hours").int_value();

	// color
	Json::array color_json = inElement.at("color").array_items();
	for(int i=0; i<3; i++){
		color[i] = color_json.at(i).int_value();
	}

}

void ClassList::Save(Json::object &outElement) const
{
	// Rml::String department_name;
	outElement["department_name"] = department_name;

	// bool subtractive;
	outElement["subtractive"] = subtractive;

	// vector<int> courses;
	Json::array array_json;
	for (int courseNum : courses)
	{
		array_json.push_back(courseNum);
	}
	outElement["courses"] = array_json;
}

void ClassList::Load(const Json::object &inElement)
{
	// Rml::String department_name;
	department_name = inElement.at("department_name").string_value();

	// bool subtractive;
	subtractive = inElement.at("subtractive").bool_value();

	// vector<int> courses;
	courses.clear();
	for (const Json& classNum : inElement.at("courses").array_items()){
		courses.push_back(classNum.int_value());
	}
}

void ShiftSchedule::Save(Json::object &outElement) const
{
	// int start, end;
	outElement["start"] = start;
	outElement["end"] = end;
	// ServiceIndex_t service_type;
	outElement["service_type"] = service_type;
}

void ShiftSchedule::Load(const Json::object &inElement)
{
	start = inElement.at("start").int_value();
	end = inElement.at("end").int_value();
	service_type = inElement.at("service_type").int_value();
}

void DaySchedule::Save(Json::array &outElement) const
{
	for (const ShiftSchedule &shift : shifts)
	{
		Json::object element_json;
		shift.Save(element_json);
		outElement.push_back(element_json);
	}
}

void DaySchedule::Load(const Json::array &inElement)
{
	for (const Json& inJson : inElement) {
		shifts.push_back({});
		shifts.back().Load(inJson.object_items());
	}
}

void WeekSchedule::Save(Json::array &outElement) const
{
	for (const DaySchedule &day : days)
	{
		Json::array element_json;
		day.Save(element_json);
		outElement.push_back(element_json);
	}
}

void WeekSchedule::Load(const Json::array &inElement)
{
	int dayIndex = 0;
	for (const Json& inJson : inElement) {
		days[dayIndex].Load(inJson.array_items());
		dayIndex++;
	}
}

void Tutor::Save(Json::object &outElement) const
{
	// Rml::String first_name;
	outElement["first_name"] = first_name;

	// Rml::String last_name;
	outElement["last_name"] = last_name;

	// Rml::String email;
	outElement["email"] = email;

	// int min_hours;
	outElement["min_hours"] = min_hours;

	// int max_hours;
	outElement["max_hours"] = max_hours;

	// WeekSchedule schedule;
	Json::array schedule_json;
	schedule.Save(schedule_json);
	outElement["schedule"] = schedule_json;

	// vector<ClassList> classes;
	Json::array classes_json;
	for (const ClassList &classList : classes)
	{
		Json::object class_json;
		classList.Save(class_json);
		classes_json.push_back(class_json);
	}
	outElement["classes"] = classes_json;
}

void Tutor::Load(const Json::object &inElement)
{
	// Rml::String first_name;
	first_name = inElement.at("first_name").string_value();

	// Rml::String last_name;
	last_name = inElement.at("last_name").string_value();

	// Rml::String email;
	email = inElement.at("email").string_value();

	// int min_hours;
	min_hours = inElement.at("min_hours").int_value();

	// int max_hours;
	max_hours = inElement.at("max_hours").int_value();

	// WeekSchedule schedule;
	schedule.Load(inElement.at("schedule").array_items());

	// vector<ClassList> classes;
	classes.clear();
	for (const Json& inJson : inElement.at("classes").array_items()) {
		classes.push_back({});
		classes.back().Load(inJson.object_items());
	}


}

void AppData::SaveSchedule(Json::object &outElement) const
{
	// vector<Rml::String> budgets;
	Json::array budgets_json;
	for (const Rml::String &budget_name : budgets)
	{
		Json::object budget_json;
		budget_json["name"] = budget_name;
		budgets_json.push_back(budget_json);
	}
	outElement["budgets"] = budgets_json;

	// vector<Tutor> tutors;
	Json::array tutors_json;
	for (const Tutor &tutor : tutors)
	{
		Json::object tutor_json;
		tutor.Save(tutor_json);
		tutors_json.push_back(tutor_json);
	}
	outElement["tutors"] = tutors_json;

	// vector<Department> departments;
	Json::array departments_json;
	for (const Department &department : departments)
	{
		departments_json.push_back(department.name);
	}
	outElement["departments"] = departments_json;

	// vector<Service> services;
	Json::array services_json;
	for (const Service &service : services)
	{
		Json::object service_json;
		service.Save(service_json);
		services_json.push_back(service_json);
	}
	outElement["services"] = services_json;

}

void AppData::LoadSchedule(const Json::object &inElement)
{
	// vector<Rml::String> budgets
	budgets.clear();
	for (const Json& inJson : inElement.at("budgets").array_items()) {
		budgets.push_back(inJson.object_items().at("name").string_value());
	}

	// vector<Tutor> tutors;
	tutors.clear();
	for (const Json& inJson : inElement.at("tutors").array_items()) {
		tutors.push_back({});
		tutors.back().Load(inJson.object_items());
	}

	// vector<Department> departments;
	departments.clear();
	for (const Json& inJson : inElement.at("departments").array_items()) {
		departments.push_back({});
		departments.back().name = inJson.string_value();
	}

	// vector<Service> services;
	services.clear();
	for (const Json& inJson : inElement.at("services").array_items()) {
		services.push_back({});
		services.back().Load(inJson.object_items());
	}
}

void AppData::SaveSettings(Json::object &outElement) const
{
	// Rml::String window_title;
	outElement["window_title"] = window_title;

	// Rml::String export_dir;
	outElement["export_directory"] = export_dir;

	// Rml::String term_season;
	outElement["term_season"] = term_season;

	// int term_year;
	outElement["term_year"] = term_year;

	// int resolution[2];
	Json::object resolution_json;
	resolution_json["w"] = resolution[0];
	resolution_json["h"] = resolution[1];
	outElement["resolution"] = resolution_json;

	// int schedule_id;
	outElement["startup_schedule"] = schedule_id;
}

void AppData::LoadSettings(const Json::object &inElement)
{
	// Rml::String window_title;
	window_title = inElement.at("window_title").string_value();

	// Rml::String export_dir;
	export_dir = inElement.at("export_directory").string_value();

	// Rml::String term_season;
	term_season = inElement.at("term_season").string_value();

	// int term_year;
	term_year = inElement.at("term_year").int_value();

	// int resolution[2];
	const Json &resolution_json = inElement.at("resolution");
	resolution[0] = resolution_json.object_items().at("w").int_value();
	resolution[1] = resolution_json.object_items().at("h").int_value();

	// int schedule_id;
	schedule_id = inElement.at("startup_schedule").int_value();
}

TimetableRow::TimetableRow()
{
	view[0].setTarget(&slots);
}

void TimetableRow::SetLabel(string newLabel)
{
	label = newLabel;
}
