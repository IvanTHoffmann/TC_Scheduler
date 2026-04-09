#include "Exporter.h"
#include "Util.h"

#include <filesystem>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <map>

using namespace std;

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
    if (!ReadUntil(istr, text, '{'))
    {
        return false;
    }
    if (!ReadUntil(istr, symbol, '}'))
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
        text << "{" << symbol.str() << "}";

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
        return invoke(result->second, *this, istr, ostr);
    }
    cout << "invalid symbol: " << symbol << endl;
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
    symbols["if_in_person"] = Exporter::If_InPerson;
    symbols["foreach_weekday"] = Exporter::Foreach_Weekday;
    symbols["weekday_name"] = Exporter::GetWeekdayName;
    symbols["foreach_in_person_shift"] = Exporter::Foreach_InPersonShift;
    symbols["shift_duration"] = Exporter::GetShiftDuration;
    symbols["if_by_appointment"] = Exporter::If_ByAppointment;
    symbols["selected_department_services"] = Exporter::GetSelectedDepartmentServices;
    symbols["foreach_tutor"] = Exporter::Foreach_Tutor;
    symbols["tutor_firstname"] = Exporter::GetTutorFirstName;
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
    cout << "Export Timetable" << endl;
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

    ostr << setw(4) << left << iter_classList->department_name;
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

    if (iter_classList->subtractive) {
        ostr << "All courses";
        if (iter_classList->courses.size()){
            ostr << " except ";
        }
    }
    ostr << FormatList(course_list);
    return true;
}

bool Exporter::If_InPerson(istream &istr, ostream &ostr)
{
    if (appData->selected_tutor.accessor.size() == 0)
    {
        // Warning: No tutor selected
        return false;
    }

    Tutor &tutor = *appData->selected_tutor.accessor.begin();
    stringstream buffer;

    // Read the loop body into buffer
    if (!ReadToSymbol(istr, buffer, "endif_in_person"))
    {
        return false;
    }

    // perform replacements in loop body
    Process(buffer, ostr);
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

bool Exporter::Foreach_InPersonShift(istream &istr, ostream &ostr)
{
    if (appData->selected_tutor.accessor.size() == 0)
    {
        // Warning: No tutor selected
        return false;
    }

    Tutor &tutor = *appData->selected_tutor.accessor.begin();
    stringstream buffer;

    // Read the loop body into buffer
    if (!ReadToSymbol(istr, buffer, "endloop_in_person_shift"))
    {
        return false;
    }

    // perform replacements in loop body
    Process(buffer, ostr);
    return true;
}

bool Exporter::GetShiftDuration(istream &istr, ostream &ostr)
{
    return true;
}

bool Exporter::If_ByAppointment(istream &istr, ostream &ostr)
{
    if (appData->selected_tutor.accessor.size() == 0)
    {
        // Warning: No tutor selected
        return false;
    }

    Tutor &tutor = *appData->selected_tutor.accessor.begin();
    stringstream buffer;

    // Read the loop body into buffer
    if (!ReadToSymbol(istr, buffer, "endif_by_appointment"))
    {
        return false;
    }

    // perform replacements in loop body
    Process(buffer, ostr);
    return true;
}

bool Exporter::GetSelectedDepartmentServices(istream &istr, ostream &ostr) {

    return true;
}

bool Exporter::Foreach_Tutor(istream &istr, ostream &ostr) {    
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
    for (Tutor& tutor : appData->tutors){
        for (const ClassList& classList : tutor.classes) {
            if (classList.department_name == department.name){
                iter_tutor = &tutor;
                stringstream link_buffer;
                Process(buffer, link_buffer);
                tutor_links.push_back(link_buffer.str());
            }
        }
    }

    ostr << (tutor_links.size() > 1 ? "tutors are " : "tutor is");
    ostr << FormatList(tutor_links);
    return true;
}

bool Exporter::GetTutorFirstName(istream &istr, ostream &ostr) {
    if (!iter_tutor){
        return false;
    }
    ostr << iter_tutor->first_name;
    return true;
}
