#include <fstream>
#include <iostream>

#include "style.hpp"
#include "application.hpp"
#include "raylib.h"

using namespace std;


Application::Application(){
    data.individualPage.tutorDropdown.index = 0;
    data.individualPage.tutorDropdown.getLabel = &getTutorName;
    data.individualPage.tutorDropdown.nVisible = 7;
    data.individualPage.tutorDropdown.nOptions = 0;

    data.individualPage.classesTab.categoryDropdown.index = 0;
    data.individualPage.classesTab.categoryDropdown.getLabel = &getCategoryLabel;
    data.individualPage.classesTab.categoryDropdown.nVisible = 7;
    data.individualPage.classesTab.categoryDropdown.nOptions = 0;

    data.individualPage.scheduleTab.brushDropdown.index = 0;
    data.individualPage.scheduleTab.brushTypes.push_back({label: "Embedded", color: {255, 0, 0}});
    data.individualPage.scheduleTab.brushTypes.push_back({label: "eTutoring", color: {0, 255, 0}});
    data.individualPage.scheduleTab.brushTypes.push_back({label: "Drop-In", color: {0, 0, 255}});
    data.individualPage.scheduleTab.brushTypes.push_back({label: "By Appointment", color: {255, 255, 0}});
    //data.summaryPage.filters.push_back(FilterCommand());

    // initialize
    loadConfig();

    

    load();
}


Application::~Application(){
    UnloadFont(font);

    //saveConfig();
}

/*
ostream& operator << (const ostream& ostr, const PAGE_ENUM& page) {
    return ostr << (int)page;
}

istream& operator >> (const istream& istr, PAGE_ENUM& page) {
    return istr >> page;
}
*/

#define FOREACH_CONFIG_ENTRY(f) \
f(data.config.resolution.x) \
f(data.config.resolution.y) \
f(data.config.fontSize) \
f(data.config.fontSpacing) \

#define WRITE_CONFIG(e) << e
#define READ_CONFIG(e) >> e

void Application::loadConfig(){
    fstream configFile;
    configFile.open(CONFIG_FILENAME, fstream::in);
    if (configFile.is_open()){
        data.cout << "successfully opened " << CONFIG_FILENAME << endl;
        configFile FOREACH_CONFIG_ENTRY(READ_CONFIG);
        configFile.close();
    }
    else {
        data.cout << "failed to open " << CONFIG_FILENAME << ". Using default settings" << endl;
        data.curPage = PAGE_CONFIG;
        data.config.resolution.x = DEFAULT_RESOLUTION_X;
        data.config.resolution.y = DEFAULT_RESOLUTION_Y;
        data.config.fontSize = DEFAULT_FONT_SIZE;
        data.config.fontSpacing = DEFAULT_FONT_SPACING;
    }

    setResolution(data.config.resolution.x, data.config.resolution.y);
}

void Application::saveConfig(){
    fstream configFile;
    configFile.open(CONFIG_FILENAME, fstream::out);
    configFile FOREACH_CONFIG_ENTRY(WRITE_CONFIG);
    configFile.close();
}


void Application::load() {
    addTutor("Ivan", "Hoffmann");
    addTutor("Eric", "Nitardy");
    addTutor("Angel", "Weibe-Wright");
    addTutor("Mitch", "Newcomb");
    addTutor("Daniel", "Kimeneteski");
    addTutor("Maddy", "Pulliam");
    addTutor("Sebastian", "Santiago");
    addTutor("Ivan", "Hoffmann");
    addTutor("Eric", "Nitardy");
    addTutor("Angel", "Weibe-Wright");
    addTutor("Mitch", "Newcomb");
    addTutor("Daniel", "Kimeneteski");
    addTutor("Maddy", "Pulliam");
    addTutor("Sebastian", "Santiago");
}


void Application::save() {

}

void Application::setResolution(int w, int h) {
    data.config.resolution.x = (float)w;
    data.config.resolution.y = (float)h;

    unitX = data.config.resolution.x / GRID_W;
    unitY = data.config.resolution.y / GRID_H;
}


void Application::run(){
    InitWindow(data.config.resolution.x, data.config.resolution.y, "TC Scheduler");

    // load font
    font = LoadFontEx("./assets/fonts/arial.ttf", 32, 0, 250);

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        pollInput();
        draw();
    }

    CloseWindow();
}

void Application::pollInput(){
    event.mouseScroll = GetMouseWheelMove();
    if (event.mouseScroll){
        handleEvent(EVENT_SCROLL);
    }

    event.mouseDelta = GetMouseDelta();
    if (event.mouseDelta.x || event.mouseDelta.y){
        event.mousePos = GetMousePosition();
        handleEvent(EVENT_MOUSEMOVE);
    }

    // LEFT MOUSE BUTTON
    for (int button=0; button<1; button++){
        
        if (IsMouseButtonUp(button)){
            event.mouseLB.state = BUTTON_UP;
        }
        if (IsMouseButtonPressed(button)){
            event.mouseLB.state = BUTTON_PRESSED;
            handleEvent(EVENT_CLICK);
        }
        if (IsMouseButtonDown(button)){
            event.mouseLB.state = BUTTON_DOWN;
        }
        if (IsMouseButtonReleased(button)){
            event.mouseLB.state = BUTTON_RELEASED;
        }
    }
}

void Application::draw(){
    BeginDrawing();
    ClearBackground(RAYWHITE);
    
    handleEvent(EVENT_DRAW);

    if (!data.cout.str().empty()){
        cout << data.cout.str() << endl;
        data.cout = stringstream();
    }

    EndDrawing();
}

void Application::tabs(){
    bool summarySelected = (data.curPage == PAGE_SUMMARY);
    bool individualSelected = (data.curPage == PAGE_INDIVIDUAL);
    bool configSelected = (data.curPage == PAGE_CONFIG);

    setRect(0, 0, 7, summarySelected ? TAB_SELECTED_HEIGHT : TAB_UNSELECTED_HEIGHT);
    if (button("Summary", summarySelected ? COLOR_BUTTON_SELECTED : COLOR_BUTTON_DEFAULT) 
            && event.type == EVENT_CLICK) {
        data.curPage = PAGE_SUMMARY;
    }

    stepRect(1, 0);
    setRect(rectX, rectY, rectW, individualSelected ? TAB_SELECTED_HEIGHT : TAB_UNSELECTED_HEIGHT);
    if (button("Individual", individualSelected ? COLOR_BUTTON_SELECTED : COLOR_BUTTON_DEFAULT)
            && event.type == EVENT_CLICK) {
        data.curPage = PAGE_INDIVIDUAL;
    }

    setRect(GRID_W - rectW, 0, rectW, configSelected ? TAB_SELECTED_HEIGHT : TAB_UNSELECTED_HEIGHT);
    if (button("Config", configSelected ? COLOR_BUTTON_SELECTED : COLOR_BUTTON_DEFAULT)
            && event.type == EVENT_CLICK) {
        data.curPage = PAGE_CONFIG;
    }
}

void Application::configPage(){
    tabs();

}

void Application::summaryPage(){
    tabs();
}

void Application::individualPage(){
    tabs();

    bool scheduleSelected = (data.individualPage.curTab == TAB_SCHEDULE);
    bool classesSelected = (data.individualPage.curTab == TAB_CLASSES);

    moveRect(-1, 0);
    stepRect(-1, 0);
    if (button("Schedule", scheduleSelected ? COLOR_BUTTON_SELECTED : COLOR_BUTTON_DEFAULT)
            && event.type == EVENT_CLICK){
        data.individualPage.curTab = TAB_SCHEDULE;
    }
    stepRect(-1, 0);
    if (button("Classes", classesSelected ? COLOR_BUTTON_SELECTED : COLOR_BUTTON_DEFAULT)
            && event.type == EVENT_CLICK){
        data.individualPage.curTab = TAB_CLASSES;
    }

    switch(data.individualPage.curTab){
    case TAB_SCHEDULE:
        
        break;
    case TAB_CLASSES:
        setRect(2, 8, 5, rectH);
        drawLabelRect("Category", COLOR_BUTTON_DEFAULT, 0, COLOR_BUTTON_BORDER);
        stepRect(1, 0);
        resizeRect(10, rectH);
        dropdown(data.individualPage.classesTab.categoryDropdown);
        break;
    }

    setRect(15, 0, 12, TAB_UNSELECTED_HEIGHT);
    if (dropdown(data.individualPage.tutorDropdown) 
            && event.type == EVENT_CLICK){
        // selected a tutor
        
    }
}

void Application::handleEvent(EventTypeEnum eventType) {
    event.type = eventType;

    switch(data.curPage){
    case PAGE_CONFIG:
        configPage();
        break;
    case PAGE_SUMMARY:
        summaryPage();
        break;
    case PAGE_INDIVIDUAL:
        individualPage();
        break;
    default:
        break;
    }
    
}

void Application::setRect(int x, int y, int w, int h) {
    rectX = x;
    rectY = y;
    rectW = w;
    rectH = h;
    drawRect = {unitX * rectX, unitY * rectY, unitX * rectW, unitY * rectH};
}

void Application::resizeRect(int w, int h) {
    setRect(rectX, rectY, w, h);
}

void Application::moveRect(int dx, int dy) {
    setRect(rectX + dx, rectY + dy, rectW, rectH);
}

void Application::stepRect(int dx, int dy) {
    setRect(rectX + rectW*dx, rectY + rectH*dy, rectW, rectH);
}

bool Application::button(const string& text, Color fillColor) {
    bool mouseOver = CheckCollisionPointRec(event.mousePos, drawRect);

    if (event.type == EVENT_DRAW) {
        unsigned char tint = 0;
        if (mouseOver){
            tint = (event.mouseLB.state == BUTTON_DOWN ? HOLD_TINT : HOVER_TINT);
        }
        drawLabelRect(text, fillColor, tint, COLOR_BUTTON_BORDER);
    }

    return mouseOver;
}

bool Application::dropdown(DropdownData& dropdown) {
    if (!dropdown.nOptions){
        return button("N/A", COLOR_BUTTON_DEFAULT);
    }

    if (!dropdown.opened){
        dropdown.opened = button(dropdown.getLabel(data, dropdown.index), COLOR_BUTTON_SELECTED);
        return dropdown.opened;
    }

    // DROPDOWN IS OPENED
    int addIndex = dropdown.nVisible / 2;
    int endIndex, startIndex;

    endIndex = dropdown.index + addIndex + 1;
    startIndex = endIndex - dropdown.nVisible;
    endIndex = min(max(endIndex, dropdown.nVisible), dropdown.nOptions);
    startIndex = max(min(startIndex, dropdown.nOptions - dropdown.nVisible), 0);

    dropdown.opened = false;
    Rectangle bkpRect = drawRect;
    for (int i=startIndex; i<endIndex; i++){
        Color c = (i==dropdown.index ? COLOR_BUTTON_SELECTED : COLOR_BUTTON_DEFAULT);
        bool mouseOverOption = button(dropdown.getLabel(data, i), c);
        if (mouseOverOption && event.type == EVENT_CLICK){
            dropdown.index = i;
        }
        drawRect.y += drawRect.height;
        dropdown.opened |= mouseOverOption;
    }
    drawRect = bkpRect;

    switch (event.type){
        case EVENT_SCROLL:
            if (event.mouseScroll > 0){
                if (dropdown.index != 0){
                    dropdown.index--;
                }
            }
            else{
                if (dropdown.index != dropdown.nOptions-1) {
                    dropdown.index++;
                }
            }
            break;
        default:
            break;
    }

    return dropdown.opened;
}


void Application::drawLabelRect(string label, Color fillColor, unsigned char tint, Color borderColor){
    fillColor.r = max(fillColor.r - tint, 0);
    fillColor.g = max(fillColor.g - tint, 0);
    fillColor.b = max(fillColor.b - tint, 0);

    DrawRectangleRec(drawRect, fillColor);
    DrawRectangleLinesEx(drawRect, BUTTON_OUTLINE_THICKNESS, borderColor);

    Vector2 textSize = MeasureTextEx(font, label.c_str(), data.config.fontSize, data.config.fontSpacing);
    float textX = drawRect.x + (drawRect.width - textSize.x) * 0.5f;
    float textY = drawRect.y + (drawRect.height - textSize.y) * 0.5f;

    DrawTextEx(font, label.c_str(), {textX, textY}, data.config.fontSize, data.config.fontSpacing, COLOR_BUTTON_TEXT);
}

const string& getTutorName(AppData& data, int index){
    return data.tutors[index].fullName;
}

const string& getCategoryLabel(AppData& data, int index){
    return data.individualPage.classesTab.categories[index];
}

void Application::addTutor(const string& first, const string& last){
    Tutor t;
    t.firstName = first;
    t.lastName = last;
    t.fullName = last + ", " + first;
    data.tutors.push_back(t);
    data.individualPage.tutorDropdown.nOptions++;
}