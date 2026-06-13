#include "Exporter.h"
#include "Util.h"

#include <filesystem>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <map>

#define SYMBOL_START '['
#define SYMBOL_END ']'

using namespace std;

string Exporter::GetExportPath(string filename)
{
    filesystem::create_directory(appData->export_dir);
    return appData->export_dir + "/" + filename;
}

string Exporter::GetExportPath(string subDirectory, string filename)
{
    cout << filename << endl;
    filesystem::create_directory(appData->export_dir);
    string path = appData->export_dir + "/" + subDirectory + "/";
    filesystem::create_directory(path);
    return path + filename;
}

bool Exporter::ReadToNextSymbol(istream &istr, ostream &text, ostream &symbol)
{
    if (!ReadUntil(istr, text, SYMBOL_START))
    {
        return false;
    }
    if (!ReadUntil(istr, symbol, SYMBOL_END))
    {
        return false;
    }
    return true;
}

bool Exporter::ReadToSymbol(istream &istr, ostream &text, const string &targetSymbol)
{
    stringstream symbol;
    while (ReadToNextSymbol(istr, text, symbol))
    {
        if (symbol.str() == targetSymbol)
        {
            return true;
        }
        text << SYMBOL_START << symbol.str() << SYMBOL_END;

        symbol.str(std::string());
        symbol.clear();
    }
    return false;
}

bool Exporter::Process(istream &istr, ostream &ostr)
{
    istr.clear();
    istr.seekg(0, istr.beg);
    stringstream symbol;
    while (ReadToNextSymbol(istr, ostr, symbol))
    {
        if (!Invoke(istr, ostr, symbol.str()))
        {
            cout << "ERROR: symbol \"" << symbol.str() << "\" returned false" << endl;
            return false;
        }

        symbol.str(std::string());
        symbol.clear();
    }
    return true;
}

bool Exporter::Invoke(istream &istr, ostream &ostr, const string &symbol)
{
    auto result = symbols.find(symbol);
    if (result != symbols.end())
    {
        cout << "invoke: " << symbol << endl;
        return invoke(result->second, *this, istr, ostr);
    }
    cout << "invalid symbol: " << symbol << endl;
    return false;
}

bool Exporter::ServiceIsSelected(string inServiceName)
{
    return find(service_names.begin(), service_names.end(), inServiceName) != service_names.end();
}

Exporter::Exporter(AppData *_appData) : appData(_appData)
{
    // DEFINE SYMBOLS
    symbols["selected_department_name"] = &Exporter::GetSelectedDepartmentName;
    symbols["selected_tutor_firstname"] = &Exporter::GetSelectedTutorFirstName;
    symbols["selected_tutor_email"] = &Exporter::GetSelectedTutorEmail;
    symbols["foreach_classList"] = &Exporter::Foreach_Classlist;
    symbols["classList_deptname"] = &Exporter::GetClassListDeptName;
    symbols["classList_courses"] = &Exporter::GetClassListCourses;
    symbols["set_service"] = &Exporter::SetService;
    symbols["if_service"] = &Exporter::If_Service;
    symbols["if_email"] = &Exporter::If_Email;
    symbols["if_dept_services"] = &Exporter::If_Dept_Services;
    symbols["if_n_dept_tutors"] = &Exporter::If_N_Dept_Tutors;
    symbols["foreach_weekday"] = &Exporter::Foreach_Weekday;
    symbols["weekday_name"] = &Exporter::GetWeekdayName;
    symbols["foreach_service_shift"] = &Exporter::Foreach_ServiceShift;
    symbols["shift_duration"] = &Exporter::GetShiftDuration;
    symbols["selected_department_services"] = &Exporter::GetSelectedDepartmentServices;
    symbols["foreach_dept_tutor"] = &Exporter::Foreach_Department_Tutor;
    symbols["tutor_firstname"] = &Exporter::GetTutorFirstName;
    symbols["term_season"] = &Exporter::GetTermSeason;
    symbols["term_year"] = &Exporter::GetTermYear;

    symbols["foreach_timetable_timeslot"] = &Exporter::Foreach_Timetable_Timeslot;
    symbols["timeslot_start"] = &Exporter::GetTimeslotStart;
    symbols["timeslot_end"] = &Exporter::GetTimeslotEnd;
    symbols["foreach_weekday_ns"] = &Exporter::Foreach_WeekdayNS;
    symbols["timetable_tutor_list"] = &Exporter::GetTimetableTutorList;
    symbols["timetable_slot_class"] = &Exporter::GetTimetableSlotClass;

    symbols["foreach_rolodex_header"] = &Exporter::Foreach_RolodexHeader;
    symbols["rolodex_description"] = &Exporter::GetRolodexDescription;
    symbols["rolodex_tutor_list"] = &Exporter::GetRolodexTutorList;
    symbols["foreach_rolodex_shift"] = &Exporter::Foreach_RolodexShift;
    symbols["rolodex_shift_start"] = &Exporter::GetRolodexShiftStartTime;
    symbols["rolodex_shift_end"] = &Exporter::GetRolodexShiftEndTime;

    symbols["foreach_service"] = &Exporter::Foreach_Service;
    symbols["if_rolodex_service"] = &Exporter::If_Rolodex_Service;
    symbols["service_description"] = &Exporter::GetServiceDescription;
    symbols["if_rolodex_day"] = &Exporter::If_Rolodex_Day;
}

void Exporter::Export(const string templateFilename, const string outFilename)
{
    ifstream fin;
    ofstream fout;

    string templateFullPath = "assets/ExportTemplates/" + templateFilename + ".html";
    fin.open(templateFullPath);
    if (!fin.is_open())
    {
        cout << "Failed to open template: " << templateFullPath << endl;
        return;
    }

    fout.open(outFilename);
    if (!fout.is_open())
    {
        cout << "Failed to open output file: " << outFilename << endl;
        fin.close();
        return;
    }

    // file is ready to be written to
    Process(fin, fout);

    fout.close();
    fin.close();
}

void Exporter::ExportTutorPage(Tutor &tutor)
{
    iter_tutor = &tutor;
    Export("tutorPage", GetExportPath("ByTutor", tutor.last_name + "_" + tutor.first_name + ".html"));
}

void Exporter::ExportSubjectPage(Department &department)
{
    curDepartment = &department;
    Export("subjectPage", GetExportPath("BySubject", department.name + ".html"));
}

void Exporter::ExportTimetable()
{
    Export("timetable", GetExportPath("timetable.html"));
}

void Exporter::ExportRolodex()
{
    Export("rolodex", GetExportPath("rolodex.html"));
}

// SYMBOL FUNCS
bool Exporter::GetSelectedDepartmentName(istream &istr, ostream &ostr)
{
    if (curDepartment)
    {
        ostr << curDepartment->name;
    }
    return curDepartment;
}

bool Exporter::GetSelectedTutorFirstName(istream &istr, ostream &ostr)
{
    if (iter_tutor)
    {
        ostr << iter_tutor->first_name;
    }
    return iter_tutor;
}

bool Exporter::GetSelectedTutorEmail(istream &istr, ostream &ostr)
{
    if (iter_tutor)
    {
        ostr << iter_tutor->email;
    }
    return iter_tutor;
}

bool Exporter::Foreach_Classlist(istream &istr, ostream &ostr)
{
    if (!iter_tutor)
    {
        // Warning: No tutor selected
        return false;
    }

    stringstream buffer;

    // Read the loop body into buffer
    if (!ReadToSymbol(istr, buffer, "endloop_classList"))
    {
        cout << "Expected Symbol: endloop_classlist" << endl;
        return false;
    }

    // perform replacements in loop body
    for (auto &classList : iter_tutor->classes)
    {
        iter_classList = &classList;
        Process(buffer, ostr);
    }
    iter_classList = nullptr;
    return true;
}

bool Exporter::GetClassListDeptName(istream &istr, ostream &ostr)
{
    if (!iter_classList)
    {
        return false;
    }

    ostr << iter_classList->department_name;
    return true;
}

bool Exporter::GetClassListCourses(istream &istr, ostream &ostr)
{
    if (!iter_classList)
    {
        return false;
    }

    vector<string> course_list;
    for (int &courseId : iter_classList->courses)
    {
        stringstream course_stream;
        course_stream << courseId;
        course_list.push_back(course_stream.str());
    }

    if (iter_classList->subtractive)
    {
        ostr << "All courses";
        if (iter_classList->courses.size())
        {
            ostr << " except ";
        }
    }
    ostr << FormatList(course_list);
    return true;
}

bool Exporter::If_Service(istream &istr, ostream &ostr)
{
    if (!iter_tutor || service_names.empty())
    {
        // Warning: No tutor selected
        return false;
    }

    stringstream buffer;

    // Read the loop body into buffer
    if (!ReadToSymbol(istr, buffer, "endif_service"))
    {
        return false;
    }

    bool foundInPersonShift = false;
    for (const DaySchedule &daySchedule : iter_tutor->schedule.days)
    {
        for (const ShiftSchedule &shift : daySchedule.shifts)
        {
            if (ServiceIsSelected(shift.service_name))
            {
                // perform replacements in loop body
                Process(buffer, ostr);
                return true;
            }
        }
    }
    return true;
}

bool Exporter::If_Email(istream &istr, ostream &ostr)
{
    if (!iter_tutor)
    {
        // Warning: No tutor selected
        return false;
    }

    stringstream buffer;

    // Read the loop body into buffer
    if (!ReadToSymbol(istr, buffer, "endif_email"))
    {
        return false;
    }

    if (iter_tutor->email.size())
    {
        // perform replacements in loop body
        Process(buffer, ostr);
    }
    return true;
}

bool Exporter::If_Dept_Services(istream &istr, ostream &ostr)
{
    if (!curDepartment)
    {
        // Warning: No department selected
        return false;
    }

    stringstream buffer;
    // Read the loop body into buffer
    if (!ReadToSymbol(istr, buffer, "endif_dept_services"))
    {
        return false;
    }

    vector<ClassList> classLists(1);
    classLists[0].department_name = curDepartment->name;

    service_names.clear();
    for (const Service &service : appData->services)
    {
        for (Tutor &tutor : appData->tutors)
        {
            if (tutor.tutorsClasses(classLists) && tutor.usesService(service.name))
            {
                service_names.emplace(service.name);
                break;
            }
        }
    }

    if (!service_names.empty())
    {
        Process(buffer, ostr); // found service. Process conditional body
        return true;
    }
    // failed to find service

    return true;
}

bool Exporter::If_N_Dept_Tutors(istream &istr, ostream &ostr)
{
    if (!curDepartment)
    {
        // Warning: No department selected
        return false;
    }

    stringstream buffer;
    // Read the loop body into buffer
    if (!ReadToSymbol(istr, buffer, "endif_n_dept_tutors"))
    {
        return false;
    }

    for (Tutor &tutor : appData->tutors)
    {
        for (const ClassList &classList : tutor.classes)
        {
            if (classList.department_name == curDepartment->name)
            {
                return true;
            }
        }
    }
    // failed to find tutor

    Process(buffer, ostr); // Process conditional body
    return true;
}

bool Exporter::If_Rolodex_Day(istream &istr, ostream &ostr)
{
        if (curService < appData->services.begin() || curService >= appData->services.end())
    {
        return false;
    }

    if (iter_weekday < 0 || 7 <= iter_weekday)
    {
        cout << "Error: If_Rolodex_Day(): No Valid Weekday Selected" << endl;
        return false;
    }

    // Read the loop body into buffer
    stringstream buffer;
    if (!ReadToSymbol(istr, buffer, "endif_rolodex_day"))
    {
        return false;
    }

    // Find a shift that matches the day and service
    for (const Tutor* tutor : cur_rolodex_tutors){
        for (const ShiftSchedule& shift : tutor->schedule.days.at(iter_weekday).shifts){
            if (shift.service_name == curService->name){
                Process(buffer, ostr);
                return true;
            }
        }
    }

    return false;
}

bool Exporter::If_Rolodex_Service(istream &istr, ostream &ostr)
{
    if (curService < appData->services.begin() || curService >= appData->services.end())
    {
        cout << "Error: If_Rolodex_Service(): No Valid Service Selected" << endl;
        return false;
    }

    // Read the loop body into buffer
    stringstream buffer;
    if (!ReadToSymbol(istr, buffer, "endif_rolodex_service"))
    {
        cout << "ERROR: If_Rolodex_Service() failed to find terminating symbol in buffer \"" << buffer.str() << "\"" << endl;
        return false;
    }

    // Find a shift that matches the service
    for (const Tutor* tutor : cur_rolodex_tutors){
        for (const DaySchedule& day : tutor->schedule.days){
            for (const ShiftSchedule& shift : day.shifts){
                if (shift.service_name == curService->name){
                    Process(buffer, ostr);
                    return true;
                }
            }
        }
    }

    cout << "Error: If_Rolodex_Service(): No Tutor is scheduled for the specified service" << endl;
    return false;
}

bool Exporter::SetService(istream &istr, ostream &ostr)
{
    stringstream buffer;
    // Read the set body into buffer
    if (!ReadToSymbol(istr, buffer, "end_set_service"))
    {
        return false;
    }

    stringstream tmpOutStream;
    stringstream service_str;
    service_names.clear();
    while (ReadToNextSymbol(buffer, tmpOutStream, service_str))
    {
        bool foundService;
        for (const Service &service : appData->services)
        {
            if (service.name == service_str.str())
            {
                service_names.emplace(service.name);
                foundService = true;
                break;
            }
        }

        if (!foundService)
        {
            cout << "ERROR: invalid service: " << service_str.str() << endl;
            return false;
        }

        service_str.str("");
        service_str.clear();
    }
    return true;
}

bool Exporter::Foreach_Weekday(istream &istr, ostream &ostr)
{
    // Read the loop body into buffer
    stringstream buffer;
    if (!ReadToSymbol(istr, buffer, "endloop_weekday"))
    {
        return false;
    }

    for (iter_weekday = 0; iter_weekday < 7; iter_weekday++)
    {
        Process(buffer, ostr);
    }
    iter_weekday = -1;
    return true;
}

bool Exporter::GetWeekdayName(istream &istr, ostream &ostr)
{
    switch (iter_weekday)
    {
    case 0:
        ostr << "Monday";
        break;
    case 1:
        ostr << "Tuesday";
        break;
    case 2:
        ostr << "Wednesday";
        break;
    case 3:
        ostr << "Thursday";
        break;
    case 4:
        ostr << "Friday";
        break;
    case 5:
        ostr << "Saturday";
        break;
    case 6:
        ostr << "Sunday";
        break;
    default:
        return false;
    }

    return true;
}

bool Exporter::Foreach_ServiceShift(istream &istr, ostream &ostr)
{
    if (!iter_tutor || service_names.empty())
    {
        // Warning: No tutor selected
        return false;
    }

    stringstream buffer;

    // Read the loop body into buffer
    if (!ReadToSymbol(istr, buffer, "endloop_service_shift"))
    {
        return false;
    }

    // perform replacements in loop body
    DaySchedule &daySchedule = iter_tutor->schedule.days.at(iter_weekday);

    iter_found_shift = false;
    for (const ShiftSchedule &shift : daySchedule.shifts)
    {
        if (ServiceIsSelected(shift.service_name))
        {
            if (iter_found_shift)
            {
                if (iter_shift.end == shift.start)
                {
                    // merge shifts into one
                    iter_shift.end = shift.end;
                }
                else
                {
                    // perform replacements in loop body
                    Process(buffer, ostr);
                    iter_shift = shift;
                }
            }
            else
            {
                iter_shift = shift;
                iter_found_shift = true;
            }
        }
    }

    Process(buffer, ostr); // Writes the final shift or "No Availability" if iter_found_shift is false

    return true;
}

bool Exporter::GetShiftDuration(istream &istr, ostream &ostr)
{
    if (iter_found_shift)
    {
        ostr << FormatTime(iter_shift.start) << " - " << FormatTime(iter_shift.end);
    }
    else
    {
        ostr << "<span style=\"color:#717171;font-size:10pt;\">No Availability</span>";
    }
    return true;
}

bool Exporter::GetSelectedDepartmentServices(istream &istr, ostream &ostr)
{
    if (!curDepartment)
    {
        return false;
    }

    unordered_set<string> services;
    for (const Tutor &tutor : appData->tutors)
    {
        for (const ClassList &classList : tutor.classes)
        {
            if (classList.department_name == curDepartment->name)
            {
                // This tutor tutors this subject. Find the services that this tutor is scheduled for.
                services.merge(tutor.schedule.GetServices());
            }
        }
    }

    vector<string> service_descriptions;
    for (const string &serviceName : services)
    {
        const Service *service = appData->GetService(serviceName);
        if (service->on_canvas)
        {
            service_descriptions.push_back(service->description);
        }
    }
    sort(service_descriptions.begin(), service_descriptions.end());

    ostr << FormatList(service_descriptions);

    return true;
}

bool Exporter::Foreach_Department_Tutor(istream &istr, ostream &ostr)
{
    if (!curDepartment)
    {
        // Warning: No tutor selected
        return false;
    }

    stringstream buffer;

    // Read the loop body into buffer
    if (!ReadToSymbol(istr, buffer, "endloop_tutor"))
    {
        return false;
    }

    vector<string> tutor_links;
    // perform replacements in loop body
    for (Tutor &tutor : appData->tutors)
    {
        for (const ClassList &classList : tutor.classes)
        {
            if (classList.department_name == curDepartment->name)
            {
                iter_tutor = &tutor;
                stringstream link_buffer;
                Process(buffer, link_buffer);
                tutor_links.push_back(link_buffer.str());
            }
        }
    }

    ostr << (tutor_links.size() > 1 ? "tutors are " : "tutor is ");
    ostr << FormatList(tutor_links);
    return true;
}

bool Exporter::Foreach_Timetable_Timeslot(istream &istr, ostream &ostr)
{
    // Read the loop body into buffer
    stringstream buffer;
    if (!ReadToSymbol(istr, buffer, "endloop_timetable_timeslot"))
    {
        return false;
    }

    int weekStartTime = 2400;
    int weekEndTime = 0;

    for (int dayId = 0; dayId < 7; dayId++)
    {
        int &startTime = timeRanges.at(dayId).first;
        int &endTime = timeRanges.at(dayId).second;
        startTime = 2400;
        endTime = 0;
        for (const Tutor &tutor : appData->tutors)
        {
            const DaySchedule &day = tutor.schedule.days.at(dayId);
            for (const ShiftSchedule &shift : day.shifts)
            {
                if (ServiceIsSelected(shift.service_name))
                {
                    startTime = shift.start < startTime ? shift.start : startTime;
                    endTime = endTime < shift.end ? shift.end : endTime;
                }
            }
        }
        weekStartTime = weekStartTime < startTime ? weekStartTime : startTime;
        weekEndTime = weekEndTime > endTime ? weekEndTime : endTime;
    }

    int slotDuration = 30;
    for (size_t slotHour = 0; slotHour < 24; slotHour++)
    {
        for (size_t slotMin = 0; slotMin < 60; slotMin += slotDuration)
        {
            iter_slotStart = slotHour * 100 + slotMin;
            if (iter_slotStart < weekStartTime)
            {
                continue;
            }

            int endSlotHour = slotHour;
            int endSlotMin = slotMin + slotDuration;
            if (endSlotMin >= 60)
            {
                endSlotHour += endSlotMin / 60;
                endSlotMin %= 60;
            }
            int slotEnd = endSlotHour * 100 + endSlotMin;

            if (weekEndTime < slotEnd)
            {
                break;
            }

            timeslot_start = FormatTime(iter_slotStart);
            timeslot_end = FormatTime(slotEnd);

            Process(buffer, ostr);
        }
    }
    return false;
}

bool Exporter::Foreach_WeekdayNS(istream &istr, ostream &ostr)
{
    // Read the loop body into buffer
    stringstream buffer;
    if (!ReadToSymbol(istr, buffer, "endloop_weekday_ns"))
    {
        return false;
    }

    for (iter_weekday = 0; iter_weekday < 6; iter_weekday++)
    {
        Process(buffer, ostr);
    }
    iter_weekday = -1;
    return true;
}

bool Exporter::Foreach_Service(istream &istr, ostream &ostr)
{
    // Read the loop body into buffer
    stringstream buffer;
    if (!ReadToSymbol(istr, buffer, "endloop_service"))
    {
        return false;
    }

    for (curService = appData->services.begin(); curService != appData->services.end(); ++curService)
    {
        if (curService->on_canvas){
            Process(buffer, ostr);
        }
    }
    return true;
}

bool Exporter::GetTutorFirstName(istream &istr, ostream &ostr)
{
    if (!iter_tutor)
    {
        return false;
    }
    ostr << iter_tutor->first_name;
    return true;
}

bool Exporter::GetTermSeason(istream &istr, ostream &ostr)
{
    ostr << appData->term_season;
    return true;
}

bool Exporter::GetTermYear(istream &istr, ostream &ostr)
{
    ostr << appData->term_year;
    return true;
}

bool Exporter::GetTimeslotStart(istream &istr, ostream &ostr)
{
    ostr << timeslot_start;
    return true;
}

bool Exporter::GetTimeslotEnd(istream &istr, ostream &ostr)
{
    ostr << timeslot_end;
    return true;
}

bool Exporter::GetTimetableTutorList(istream &istr, ostream &ostr)
{
    if (iter_weekday < 0 || 7 <= iter_weekday)
    {
        // warning: invalid weekday
        return false;
    }

    iter_timeslotEmpty = true;
    selected_tutors.str("");
    selected_tutors.clear();
    for (const Tutor &tutor : appData->tutors)
    {
        const DaySchedule &day = tutor.schedule.days.at(iter_weekday);
        for (const ShiftSchedule &shift : day.shifts)
        {
            if (shift.start <= iter_slotStart && iter_slotStart < shift.end)
            {
                if (ServiceIsSelected(shift.service_name))
                {
                    // The tutor is scheduled for a selected service
                    if (!iter_timeslotEmpty)
                    {
                        selected_tutors << ", ";
                    }
                    iter_timeslotEmpty = false;
                    selected_tutors << tutor.first_name;
                }
                break;
            }
        }
    }
    ostr << selected_tutors.str();
    return true;
}

bool Exporter::GetTimetableSlotClass(istream &istr, ostream &ostr)
{
    int &startTime = timeRanges.at(iter_weekday).first;
    int &endTime = timeRanges.at(iter_weekday).second;
    if (startTime <= iter_slotStart && iter_slotStart < endTime)
    {
        ostr << "no_shade";
    }
    else
    {
        ostr << "shade";
    }
    return true;
}

bool Exporter::Foreach_RolodexHeader(istream &istr, ostream &ostr)
{
    // Read the loop body into buffer
    stringstream buffer;
    if (!ReadToSymbol(istr, buffer, "endloop_rolodex_header"))
    {
        return false;
    }

    for (cur_rolodex_header = appData->rolodex_headers.begin(); cur_rolodex_header != appData->rolodex_headers.end(); ++cur_rolodex_header)
    {
        cur_rolodex_schedule.Clear();
        cur_rolodex_tutors.clear();
        for (const Tutor &tutor : appData->tutors)
        {
            if (tutor.tutorsClasses(cur_rolodex_header->classes))
            {
                cur_rolodex_tutors.push_back(&tutor);
            }
        }

        Process(buffer, ostr);
    }
    return true;
}

bool Exporter::GetRolodexDescription(istream &istr, ostream &ostr)
{
    const string &description = cur_rolodex_header->description;
    if (description.size())
    {
        ostr << description;
    }
    else
    {
        ostr << "ERROR | TODO: Generate rolodex description if unspecified";
    }
    return true;
}

bool Exporter::GetRolodexTutorList(istream &istr, ostream &ostr)
{
    bool first = true;
    for (const Tutor *tutor : cur_rolodex_tutors)
    {
        if (!first)
        {
            ostr << ", ";
        }
        ostr << tutor->first_name;
        first = false;
    }
    return true;
}

bool Exporter::Foreach_RolodexShift(istream &istr, ostream &ostr)
{
    if (curService < appData->services.begin() || curService >= appData->services.end())
    {
        return false;
    }

    if (iter_weekday == -1)
    {
        return false;
    }

    // Read the loop body into buffer
    stringstream buffer;
    if (!ReadToSymbol(istr, buffer, "endloop_rolodex_shift"))
    {
        return false;
    }

    // Add every shift that matches the selected day and service 
    cur_rolodex_schedule.Clear();
    for (const Tutor* tutor : cur_rolodex_tutors){
        for (const ShiftSchedule& shift : tutor->schedule.days.at(iter_weekday).shifts){
            if (shift.service_name == curService->name){
                cur_rolodex_schedule.days.at(iter_weekday).shifts.push_back(shift);
            }
        }
    }

    // Use the timetable interface to combine overlapping/joined shifts. This should be a function of WeekSchedule.
    TimetableInterface timetable;
    timetable.Load(cur_rolodex_schedule);
    timetable.Save(*appData, cur_rolodex_schedule);

    // Process each shift
    vector<ShiftSchedule> &shifts = cur_rolodex_schedule.days[iter_weekday].shifts;
    for (curShift = shifts.begin(); curShift != shifts.end(); ++curShift){
        Process(buffer, ostr);
    }
    return true;
}

bool Exporter::GetRolodexShiftStartTime(istream &istr, ostream &ostr)
{
    ostr << FormatTime(curShift->start);
    return true;
}

bool Exporter::GetRolodexShiftEndTime(istream &istr, ostream &ostr)
{
    ostr << FormatTime(curShift->end);
    return true;
}

bool Exporter::GetServiceDescription(istream &istr, ostream &ostr)
{
    if (curService < appData->services.begin() || curService >= appData->services.end())
    {
        return false;
    }

    ostr << curService->description;
    return true;
}
