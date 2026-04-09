#ifndef EXPORTER_H
#define EXPORTER_H

#include <string>
#include <map>

#include "DataModelTypes.h"

using namespace std;


class Exporter {
    private:
    ClassList* iter_classList;
    int* iter_courseId;
    int iter_weekday;
    Tutor* iter_tutor;

    AppData* appData;
    map<string, bool (Exporter::*)(istream&, ostream&)> symbols, loopSymbols;
	string GetExportPath(string subDirectory, string filename);
    bool ReadToNextSymbol(istream& istr, ostream& text, ostream& symbol);
    bool ReadToSymbol(istream& istr, ostream& text, const string& targetSymbol);
    bool Process(istream& istr, ostream& ostr);
    bool Invoke(istream& istr, ostream& ostr, const string& symbol);

    // Looping Symbols
    bool Foreach_Classlist(istream& istr, ostream& ostr);
    bool Foreach_Weekday(istream& istr, ostream& ostr);
    bool Foreach_InPersonShift(istream& istr, ostream& ostr);
    bool Foreach_Tutor(istream& istr, ostream& ostr);

    // Conditional Symbols
    bool If_InPerson(istream& istr, ostream& ostr);
    bool If_ByAppointment(istream& istr, ostream& ostr);

    // Getter Symbols
    bool GetSelectedDepartmentName(istream& istr, ostream& ostr);
    bool GetSelectedTutorFirstName(istream& istr, ostream& ostr);
    bool GetSelectedTutorEmail(istream& istr, ostream& ostr);
    bool GetClassListDeptName(istream& istr, ostream& ostr);
    bool GetClassListCourses(istream& istr, ostream& ostr);
    bool GetWeekdayName(istream& istr, ostream& ostr);
    bool GetShiftDuration(istream& istr, ostream& ostr);
    bool GetCourseList(istream& istr, ostream& ostr);
    bool GetSelectedDepartmentServices(istream& istr, ostream& ostr);
    bool GetTutorFirstName(istream& istr, ostream& ostr);
    
    public:

    Exporter(AppData* _appData);
	void ExportTutorPage();
	void ExportSubjectPage();
	void ExportTimetable();
	void ExportRolodex();
	void ExportAll();

};

#endif