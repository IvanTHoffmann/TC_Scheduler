#ifndef APPLICATION_HPP
#define APPLICATION_HPP

struct AppData;
class Application;

#include "raylib.h"
#include "commands.hpp"

#include <cstdint>
#include <string>
#include <sstream>
#include <vector>
#include <array>

using namespace std;

typedef uint8_t BrushIndex_t;


struct BrushType{
    string label;
    array<uint8_t, 3> color; 
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
    string firstName, lastName;

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

struct AppData {
    Vector2 ballPosition;

    Vector2 mousePos;
    vector<Tutor> tutors;
    stringstream cout, cin;

    struct {
        Vector2 resolution;
        string filename;
    } config;

    struct {
        uint8_t tutorIndex;

        struct {
            BrushIndex_t brushIndex;
            vector<BrushType> brushTypes;
        } scheduleTab;

        struct {
            string categoryID;
        } classesTab;
    } individualPage;
    
    struct {
        vector<ScheduleFilter> filters;
        WeekSchedule schedule;
    } summaryPage;
};

typedef void (*Callback)(AppData& appData);



struct EventData{
    enum {
        EVENT_DRAW,
        EVENT_MOUSEMOVE,
        EVENT_MOUSEDOWN,
        EVENT_MOUSEUP,
        EVENT_KEYDOWN,
        EVENT_KEYUP
    } type;
    Vector2 mousePos;
    uint8_t mouseButton;
    char key;
};

class Application {
    private:
        vector<Command*> commands;
        AppData data;

    public:
        Application();
        ~Application();

        void doCommand(Command* command);

        void load();
        void save();

        void run();
        void draw();
        void pollInput();
        void handleEvent(EventEnum eventType);

        void moveCursor(Vector2 diff);
        void button(string label, Callback onClick);
};

#endif