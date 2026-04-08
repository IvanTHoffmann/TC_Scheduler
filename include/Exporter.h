#ifndef EXPORTER_H
#define EXPORTER_H

#include <string>
#include <map>

#include "DataModelTypes.h"

using namespace std;


class Exporter {
    private:
    AppData* appData;
    map<string, bool (Exporter::*)(istream&, ostream&)> symbols, loopSymbols;
	string GetExportPath(string subDirectory, string filename);

    // Symbol functions
    bool GetSelectedDepartmentName(istream& istr, ostream& ostr);
    bool GetSelectedTutorFirstName(istream& istr, ostream& ostr);
    bool GetSelectedTutorEmail(istream& istr, ostream& ostr);
    bool Foreach_Classlist(istream& istr, ostream& ostr);
    bool GetClassListDeptName(istream& istr, ostream& ostr);
    bool Foreach_Course(istream& istr, ostream& ostr);
    bool GetCourseId(istream& istr, ostream& ostr);
    bool EndLoop_Course(istream& istr, ostream& ostr);
    bool Endloop_ClassList(istream& istr, ostream& ostr);
    bool If_InPerson(istream& istr, ostream& ostr);
    bool Foreach_Weekday(istream& istr, ostream& ostr);
    bool GetWeekdayName(istream& istr, ostream& ostr);
    bool Foreach_InPersonShift(istream& istr, ostream& ostr);
    bool GetShiftDuration(istream& istr, ostream& ostr);
    bool EndLoop_InPersonShift(istream& istr, ostream& ostr);
    bool Endloop_Weekday(istream& istr, ostream& ostr);
    bool Endif_InPerson(istream& istr, ostream& ostr);
    bool If_ByAppointment(istream& istr, ostream& ostr);
    bool Endif_ByAppointment(istream& istr, ostream& ostr);
    bool Foreach_Service(istream& istr, ostream& ostr);
    bool GetServiceName(istream& istr, ostream& ostr);
    bool EndLoop_Service(istream& istr, ostream& ostr);
    bool Foreach_Tutor(istream& istr, ostream& ostr);
    bool GetTutorFirstName(istream& istr, ostream& ostr);
    bool EndLoop_Tutor(istream& istr, ostream& ostr);
    
    public:

    Exporter(AppData* _appData);
	void ExportTutorPage();
	void ExportSubjectPage();
	void ExportTimetable();
	void ExportRolodex();
	void ExportAll();

};

#endif