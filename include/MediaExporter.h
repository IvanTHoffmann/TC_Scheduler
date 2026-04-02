#ifndef MEDIA_EXPORTER_H
#define MEDIA_EXPORTER_H

#include <string>

using namespace std;

class MediaCreator {
    public:

    void CopyTutorCanvasPage(string tutorName);
    void CopySubjectCanvasPage(string subject);
    void ExportRolodex();
    void ExportTimetable();
};

#endif