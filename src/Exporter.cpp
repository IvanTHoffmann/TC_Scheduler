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



Exporter::Exporter(AppData* _appData) : appData(_appData)
{
    // DEFINE SYMBOLS
    symbols["selected_department_name"] = Exporter::GetSelectedDepartmentName;
    symbols["selected_tutor_firstname"] = Exporter::GetSelectedTutorFirstName;
    symbols["tutor_firstname"] = Exporter::GetSelectedTutorFirstName;
    symbols["selected_tutor_email"] = Exporter::GetSelectedTutorEmail;
    symbols["foreach_classList"] = Exporter::Foreach_Classlist;
    symbols["classList_deptname"] = Exporter::GetClassListDeptName;
    symbols["foreach_course"] = Exporter::Foreach_Course;
    symbols["course_id"] = Exporter::GetCourseId;
    symbols["endloop_course"] = Exporter::EndLoop_Course;
    symbols["endloop_classlist"] = Exporter::Endloop_ClassList;
    symbols["if_in_person"] = Exporter::If_InPerson;
    symbols["foreach_weekday"] = Exporter::Foreach_Weekday;
    symbols["weekday_name"] = Exporter::GetWeekdayName;
    symbols["foreach_in_person_shift"] = Exporter::Foreach_InPersonShift;
    symbols["shift_duration"] = Exporter::GetShiftDuration;
    symbols["endloop_in_person_shift"] = Exporter::EndLoop_InPersonShift;
    symbols["endloop_weekday"] = Exporter::Endloop_Weekday;
    symbols["endif_in_person"] = Exporter::Endif_InPerson;
    symbols["if_by_appointment"] = Exporter::If_ByAppointment;
    symbols["endif_by_appointment"] = Exporter::Endif_ByAppointment;
    symbols["foreach_service"] = Exporter::Foreach_Service;
    symbols["service_name"] = Exporter::GetServiceName;
    symbols["endloop_service"] = Exporter::EndLoop_Service;
    symbols["foreach_tutor"] = Exporter::Foreach_Tutor;
    symbols["tutor_firstname"] = Exporter::GetTutorFirstName;
    symbols["endloop_tutor"] = Exporter::EndLoop_Tutor;
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

    cout << appData->tutors.size() << endl;
    
    if (appData->selected_tutor.accessor.size() == 0){
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
    while (ReadUntil(fin, fout, '{'))
    {
        stringstream symbol;
        if (ReadUntil(fin, symbol, '}')){
            auto result = symbols.find(symbol.str());
            if (result != symbols.end())
            {
                invoke(result->second, *this, fin, fout);
            }
            else {
                cout << "invalid symbol: " << symbol.str() << endl;
            }
        }
        else {
            break;
        }
    }
        
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

    while (ReadUntil(fin, fout, '{'))
    {
        stringstream symbol;
        if (ReadUntil(fin, symbol, '}')){
            auto result = symbols.find(symbol.str());
            if (result != symbols.end())
            {
                invoke(result->second, *this, fin, fout);
            }
            else {
                cout << "invalid symbol: " << symbol.str() << endl;
            }
        }
        else {
            break;
        }
    }

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
bool Exporter::GetSelectedDepartmentName(std::istream& istream, std::ostream& ostream){
    ostream << appData->selected_department.accessor.begin()->name;
    return true;
}


bool Exporter::GetSelectedTutorFirstName(std::istream& istream, std::ostream& ostream){
    ostream << appData->selected_tutor.accessor.begin()->first_name;
    return true;
}


bool Exporter::GetSelectedTutorEmail(std::istream& istream, std::ostream& ostream){
    ostream << appData->selected_tutor.accessor.begin()->email;
    return true;
}


bool Exporter::Foreach_Classlist(std::istream& istream, std::ostream& ostream){
    return true;
}

bool Exporter::GetClassListDeptName(istream &istr, ostream &ostr)
{
    return false;
}

bool Exporter::Foreach_Course(istream &istr, ostream &ostr)
{
    return false;
}

bool Exporter::GetCourseId(istream &istr, ostream &ostr)
{
    return false;
}

bool Exporter::EndLoop_Course(istream &istr, ostream &ostr)
{
    return false;
}

bool Exporter::Endloop_ClassList(istream &istr, ostream &ostr)
{
    return false;
}

bool Exporter::If_InPerson(istream &istr, ostream &ostr)
{
    return false;
}

bool Exporter::Foreach_Weekday(istream &istr, ostream &ostr)
{
    return false;
}

bool Exporter::GetWeekdayName(istream &istr, ostream &ostr)
{
    return false;
}

bool Exporter::Foreach_InPersonShift(istream &istr, ostream &ostr)
{
    return false;
}

bool Exporter::GetShiftDuration(istream &istr, ostream &ostr)
{
    return false;
}

bool Exporter::EndLoop_InPersonShift(istream &istr, ostream &ostr)
{
    return false;
}

bool Exporter::Endloop_Weekday(istream &istr, ostream &ostr)
{
    return false;
}

bool Exporter::Endif_InPerson(istream &istr, ostream &ostr)
{
    return false;
}

bool Exporter::If_ByAppointment(istream &istr, ostream &ostr)
{
    return false;
}

bool Exporter::Endif_ByAppointment(istream &istr, ostream &ostr)
{
    return false;
}

bool Exporter::Foreach_Service(istream &istr, ostream &ostr)
{
    return false;
}

bool Exporter::GetServiceName(istream &istr, ostream &ostr)
{
    return false;
}

bool Exporter::EndLoop_Service(istream &istr, ostream &ostr)
{
    return false;
}

bool Exporter::Foreach_Tutor(istream &istr, ostream &ostr)
{
    return false;
}

bool Exporter::GetTutorFirstName(istream &istr, ostream &ostr)
{
    return false;
}

bool Exporter::EndLoop_Tutor(istream &istr, ostream &ostr)
{
    return false;
}
