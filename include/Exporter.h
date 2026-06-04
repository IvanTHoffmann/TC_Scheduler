#ifndef EXPORTER_H
#define EXPORTER_H

#include <sstream>
#include <string>
#include <map>

#include "DataModelTypes.h"

using namespace std;


class Exporter {
    private:
    vector<int> service_ids;

    Department* curDepartment;

    bool iter_found_shift;
    ShiftSchedule iter_shift;
    ClassList* iter_classList;
    int* iter_courseId;
    int iter_weekday;
    vector<RolodexHeader>::iterator cur_rolodex_header;
    vector<const Tutor*> cur_rolodex_tutors;
    Tutor* iter_tutor;
    stringstream selected_tutors;
    int iter_slotStart;
    array<pair<int, int>, 7> timeRanges;
    bool iter_timeslotEmpty;
    string timeslot_start, timeslot_end;

    AppData* appData;
    map<string, bool (Exporter::*)(istream&, ostream&)> symbols, loopSymbols;
	string GetExportPath(string filename);
	string GetExportPath(string subDirectory, string filename);
    bool ReadToNextSymbol(istream& istr, ostream& text, ostream& symbol);
    bool ReadToSymbol(istream& istr, ostream& text, const string& targetSymbol);
    bool Process(istream& istr, ostream& ostr);
    bool Invoke(istream& istr, ostream& ostr, const string& symbol);
    bool ServiceIsSelected(int service_type);

    // Looping Symbols
    bool Foreach_Classlist(istream& istr, ostream& ostr);
    bool Foreach_Weekday(istream& istr, ostream& ostr);
    bool Foreach_ServiceShift(istream& istr, ostream& ostr);
    bool Foreach_Department_Tutor(istream& istr, ostream& ostr);
    bool Foreach_Timetable_Timeslot(istream& istr, ostream& ostr);
    bool Foreach_WeekdayNS(istream& istr, ostream& ostr);

    // Conditional Symbols
    bool If_Service(istream& istr, ostream& ostr);
    bool If_Email(istream& istr, ostream& ostr);
    bool If_Dept_Tutors(istream& istr, ostream& ostr);

    // Setter symbols
    bool SetService(istream& istr, ostream& ostr);

    // Getter Symbols
    bool GetSelectedDepartmentName(istream& istr, ostream& ostr);
    bool GetSelectedTutorFirstName(istream& istr, ostream& ostr);
    bool GetSelectedTutorEmail(istream& istr, ostream& ostr);
    bool GetClassListDeptName(istream& istr, ostream& ostr);
    bool GetClassListCourses(istream& istr, ostream& ostr);
    bool GetWeekdayName(istream& istr, ostream& ostr);
    bool GetShiftDuration(istream& istr, ostream& ostr);
    //bool GetCourseList(istream& istr, ostream& ostr);
    bool GetSelectedDepartmentServices(istream& istr, ostream& ostr);
    bool GetTutorFirstName(istream& istr, ostream& ostr);
    bool GetTermSeason(istream& istr, ostream& ostr);
    bool GetTermYear(istream& istr, ostream& ostr);
    bool GetTimeslotStart(istream& istr, ostream& ostr);
    bool GetTimeslotEnd(istream& istr, ostream& ostr);
    bool GetTimetableTutorList(istream& istr, ostream& ostr);
    bool GetTimetableSlotClass(istream& istr, ostream& ostr);

    bool Foreach_RolodexHeader(istream& istr, ostream& ostr);
    bool GetRolodexDescription(istream& istr, ostream& ostr);
    bool GetRolodexTutorList(istream& istr, ostream& ostr);
    bool Foreach_RolodexShift(istream& istr, ostream& ostr);
    bool GetRolodexShiftDays(istream& istr, ostream& ostr);
    bool GetRolodexShiftStartTime(istream& istr, ostream& ostr);
    bool GetRolodexShiftEndTime(istream& istr, ostream& ostr);

    void Export(const string templateFilename, const string outFilename);
    
    public:

    Exporter(AppData* _appData);
	void ExportTutorPage(Tutor&);
	void ExportSubjectPage(Department&);
	void ExportTimetable();
	void ExportRolodex();

};

#endif