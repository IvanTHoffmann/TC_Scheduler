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
    cout << filename << endl;
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

bool Exporter::ServiceIsSelected(int service_type)
{
    for (int service_id : service_ids)
    {
        if (service_type == service_id)
        {
            return true;
        }
    }
    return false;
}

Exporter::Exporter(AppData *_appData) : appData(_appData)
{
    // DEFINE SYMBOLS
    symbols["selected_department_name"] = Exporter::GetSelectedDepartmentName;
    symbols["selected_tutor_firstname"] = Exporter::GetSelectedTutorFirstName;
    symbols["selected_tutor_email"] = Exporter::GetSelectedTutorEmail;
    symbols["foreach_classList"] = Exporter::Foreach_Classlist;
    symbols["classList_deptname"] = Exporter::GetClassListDeptName;
    symbols["classList_courses"] = Exporter::GetClassListCourses;
    symbols["set_service"] = Exporter::SetService;
    symbols["if_service"] = Exporter::If_Service;
    symbols["if_email"] = Exporter::If_Email;
    symbols["foreach_weekday"] = Exporter::Foreach_Weekday;
    symbols["weekday_name"] = Exporter::GetWeekdayName;
    symbols["foreach_service_shift"] = Exporter::Foreach_ServiceShift;
    symbols["shift_duration"] = Exporter::GetShiftDuration;
    symbols["selected_department_services"] = Exporter::GetSelectedDepartmentServices;
    symbols["foreach_dept_tutor"] = Exporter::Foreach_Department_Tutor;
    symbols["tutor_firstname"] = Exporter::GetTutorFirstName;
    symbols["term_season"] = Exporter::GetTermSeason;
    symbols["term_year"] = Exporter::GetTermYear;

    symbols["foreach_timetable_timeslot"] = Exporter::Foreach_Timetable_Timeslot;
    symbols["timeslot_start"] = Exporter::GetTimeslotStart;
    symbols["timeslot_end"] = Exporter::GetTimeslotEnd;
    symbols["foreach_weekday_ns"] = Exporter::Foreach_WeekdayNS;
    symbols["timetable_tutor_list"] = Exporter::GetTimetableTutorList;
    symbols["timetable_slot_class"] = Exporter::GetTimetableSlotClass;
}

void Exporter::ExportTutorPage()
{
    ifstream fin;
    ofstream fout;

    fin.open("assets/ExportTemplates/tutorPage.html");
    if (!fin.is_open())
    {
        cout << "failed to open template" << endl;
        return;
    }

    if (appData->selected_tutor.accessor.size() == 0)
    {
        cout << "no tutor selected" << endl;
        return;
    }

    const Tutor &tutor = *appData->selected_tutor.accessor.begin();

    fout.open(GetExportPath("ByTutor", tutor.last_name + "_" + tutor.first_name + ".html"));
    if (!fout.is_open())
    {
        fin.close();
        return;
    }

    // file is ready to be written to
    Process(fin, fout);

    fout.close();
    fin.close();
}

void Exporter::ExportSubjectPage()
{
    ifstream fin;
    ofstream fout;

    fin.open("assets/ExportTemplates/subjectPage.html");

    if (!fin.is_open())
    {
        return;
    }
    const Department &department = *appData->selected_department.accessor.begin();

    fout.open(GetExportPath("BySubject", department.name + ".html"));
    if (!fout.is_open())
    {
        fin.close();
        return;
    }

    // file is ready to be written to
    Process(fin, fout);

    fout.close();
    fin.close();
}

void Exporter::ExportTimetable()
{
    ifstream fin;
    ofstream fout;

    fin.open("assets/ExportTemplates/timetable.html");

    if (!fin.is_open())
    {
        return;
    }

    fout.open(GetExportPath("timetable.html"));
    if (!fout.is_open())
    {
        fin.close();
        return;
    }

    // file is ready to be written to
    Process(fin, fout);

    fout.close();
    fin.close();
}

void Exporter::ExportRolodex()
{
    cout << "Export Rolodex" << endl;
}

void Exporter::ExportAll()
{
    cout << "Export All" << endl;
}

// SYMBOL FUNCS
bool Exporter::GetSelectedDepartmentName(istream &istr, ostream &ostr)
{
    ostr << appData->selected_department.accessor.begin()->name;
    return true;
}

bool Exporter::GetSelectedTutorFirstName(istream &istr, ostream &ostr)
{
    ostr << appData->selected_tutor.accessor.begin()->first_name;
    return true;
}

bool Exporter::GetSelectedTutorEmail(istream &istr, ostream &ostr)
{
    ostr << appData->selected_tutor.accessor.begin()->email;
    return true;
}

bool Exporter::Foreach_Classlist(istream &istr, ostream &ostr)
{
    if (appData->selected_tutor.accessor.size() == 0)
    {
        // Warning: No tutor selected
        return false;
    }

    Tutor &tutor = *appData->selected_tutor.accessor.begin();
    stringstream buffer;

    // Read the loop body into buffer
    if (!ReadToSymbol(istr, buffer, "endloop_classList"))
    {
        cout << "Expected Symbol: endloop_classlist" << endl;
        return false;
    }

    // perform replacements in loop body
    for (auto &classList : tutor.classes)
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
    if ((appData->selected_tutor.accessor.size() == 0) || service_ids.empty())
    {
        // Warning: No tutor selected
        return false;
    }

    Tutor &tutor = *appData->selected_tutor.accessor.begin();
    stringstream buffer;

    // Read the loop body into buffer
    if (!ReadToSymbol(istr, buffer, "endif_service"))
    {
        return false;
    }

    bool foundInPersonShift = false;
    for (const DaySchedule &daySchedule : tutor.schedule.days)
    {
        for (const ShiftSchedule &shift : daySchedule.shifts)
        {
            if (ServiceIsSelected(shift.service_type))
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
    if (appData->selected_tutor.accessor.size() == 0)
    {
        // Warning: No tutor selected
        return false;
    }

    Tutor &tutor = *appData->selected_tutor.accessor.begin();
    stringstream buffer;

    // Read the loop body into buffer
    if (!ReadToSymbol(istr, buffer, "endif_email"))
    {
        return false;
    }

    if (tutor.email.size())
    {
        // perform replacements in loop body
        Process(buffer, ostr);
    }
    return true;
}

bool Exporter::If_Dept_Tutors(istream &istr, ostream &ostr)
{
    Department *department = appData->selected_department.accessor.ptr();
    if (department)
    {
        // Warning: No tutor selected
        return false;
    }

    stringstream buffer;
    // Read the loop body into buffer
    if (!ReadToSymbol(istr, buffer, "endif_dept_tutors"))
    {
        return false;
    }

    return true;
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
    service_ids.clear();
    while (ReadToNextSymbol(buffer, tmpOutStream, service_str))
    {
        int service_id = 0;
        for (const Service &service : appData->services)
        {
            if (service.name == service_str.str())
            {
                break;
            }
            service_id++;
        }

        if (service_id == appData->services.size())
        {
            cout << "invalid service: " << service_str.str() << endl;
            return false;
        }

        service_ids.push_back(service_id);
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
    if ((appData->selected_tutor.accessor.size() == 0) || service_ids.empty())
    {
        // Warning: No tutor selected
        return false;
    }

    Tutor &tutor = *appData->selected_tutor.accessor.begin();
    stringstream buffer;

    // Read the loop body into buffer
    if (!ReadToSymbol(istr, buffer, "endloop_service_shift"))
    {
        return false;
    }

    // perform replacements in loop body
    DaySchedule &daySchedule = tutor.schedule.days.at(iter_weekday);

    iter_found_shift = false;
    for (const ShiftSchedule &shift : daySchedule.shifts)
    {
        if (ServiceIsSelected(shift.service_type))
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

    return true;
}

bool Exporter::Foreach_Department_Tutor(istream &istr, ostream &ostr)
{
    if (appData->selected_department.accessor.size() == 0)
    {
        // Warning: No tutor selected
        return false;
    }

    Department &department = *appData->selected_department.accessor.begin();
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
            if (classList.department_name == department.name)
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

    int slotDuration = 30;
    for (size_t slotHour = 0; slotHour < 24; slotHour++)
    {
        for (size_t slotMin = 0; slotMin < 60; slotMin += slotDuration)
        {
            iter_slotStart = slotHour * 100 + slotMin;

            int endSlotHour = slotHour;
            int endSlotMin = slotMin + slotDuration;
            if (endSlotMin >= 60)
            {
                endSlotHour += endSlotMin / 60;
                endSlotMin %= 60;
            }
            int slotEnd = endSlotHour * 100 + endSlotMin;

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

    bool foundTutor = false;
    selected_tutors.str("");
    selected_tutors.clear();
    for (const Tutor &tutor : appData->tutors)
    {
        const DaySchedule &day = tutor.schedule.days.at(iter_weekday);
        for (const ShiftSchedule &shift : day.shifts)
        {
            if (shift.start <= iter_slotStart && iter_slotStart < shift.end)
            {
                if (ServiceIsSelected(shift.service_type))
                {
                    // The tutor is scheduled for a selected service
                    if (foundTutor)
                    {
                        selected_tutors << ", ";
                    }
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
    ostr << "NoClass";
    return true;
}
