#ifndef APPLICATION_HPP
#define APPLICATION_HPP

struct AppData;
class Application;

typedef void (*Callback)(AppData& appData);

#include "raylib.h"
#include "commands.hpp"
#include "callbacks.hpp"

#include <cstdint>
#include <string>
#include <sstream>
#include <vector>
#include <array>

using namespace std;

typedef uint8_t BrushIndex_t;


const string CONFIG_FILENAME = "config.ini";


struct BrushType{
    string label;
    Color color; 
};

struct ClassList{
    uint8_t categoryID;
    bool excludeList;
    array<uint16_t, 16> classNums;
};

struct DaySchedule{
    array<BrushIndex_t, 96> segments;
};

struct WeekSchedule {
    array<DaySchedule, 7> days;
};

struct Tutor {
    string firstName, lastName, fullName;

    uint16_t totalHours;
    uint16_t minHours;
    uint16_t maxHours;

    WeekSchedule schedule;
    array<ClassList, 12> classes;
};

struct ScheduleFilter{
    string label;
    void (*exec)(AppData& appData);
};

struct DropdownData{
    bool opened;
    int index;
    int nVisible;
    int nOptions;
    const string& (*getLabel)(AppData&, int);
};

enum PAGE_ENUM{
    PAGE_CONFIG,
    PAGE_SUMMARY,
    PAGE_INDIVIDUAL,
};

enum INDIVIDUAL_TAB_ENUM{
    TAB_CLASSES,
    TAB_SCHEDULE,
};

struct AppData {
    PAGE_ENUM curPage;

    vector<Tutor> tutors;
    stringstream cout, cin;

    struct {
        Vector2 resolution;
        float fontSize, fontSpacing;
        string filename;
    } config;

    struct {
        INDIVIDUAL_TAB_ENUM curTab;

        DropdownData tutorDropdown;

        struct {
            vector<BrushType> brushTypes;
            DropdownData brushDropdown;
        } scheduleTab;

        struct {
            vector<string> categories;
            DropdownData categoryDropdown;
            string categoryID;
        } classesTab;
    } individualPage;
    
    struct {
        vector<ScheduleFilter> filters;
        DropdownData filterDropdown;
        WeekSchedule schedule;
    } summaryPage;
};

enum EventTypeEnum {
    EVENT_DRAW,
    EVENT_MOUSEMOVE,
    EVENT_SCROLL,
    EVENT_CLICK,
    EVENT_DOUBLECLICK,
    EVENT_KEYDOWN,
};

enum MouseButtonState {
    BUTTON_UP,
    BUTTON_DOWN,
    BUTTON_PRESSED,
    BUTTON_RELEASED,
};

struct MouseButtonData {
    MouseButtonState state;
    double changeTime;
};


struct EventData{
    EventTypeEnum type;
    Vector2 mousePos, mouseDelta;
    float mouseScroll;
    MouseButtonData mouseLB;
    char key;
};


const string& getTutorName(AppData& data, int index);
const string& getCategoryLabel(AppData& data, int index);


class Application {
    private:
        vector<Command*> commands;
        AppData data;
        EventData event;
        Rectangle drawRect;
        int rectX, rectY, rectW, rectH;
        float unitX, unitY;
        Font font;

    public:
        Application();
        ~Application();

        void doCommand(Command* command);

        void loadConfig();
        void saveConfig();
        void load();
        void save();

        void run();
        void draw();
        void pollInput();

        void tabs();
        void configPage();
        void summaryPage();
        void individualPage();
        void handleEvent(EventTypeEnum eventType);

        void setRect(int x, int y, int w, int h);
        void resizeRect(int w, int h);
        void moveRect(int x, int y);
        void stepRect(int dx, int dy);

        void setResolution(int w, int h);

        // ui elements return true if they are under the mouse
        bool button(const string& label, Color fillColor);
        bool dropdown(DropdownData& data);

        void drawLabelRect(string label, Color fillColor, unsigned char tint, Color borderColor);

        void addTutor(const string& first, const string& last);
};

/*
Vector2 MeasureTextEx(Font font, const char *text, float fontSize, float spacing);


*/

#endif