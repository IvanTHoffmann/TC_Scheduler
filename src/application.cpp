#include <fstream>
#include <iostream>

#include "application.hpp"
#include "raylib.h"

using namespace std;


Application::Application(){

    string configFilename = "config.ini";

    data.individualPage.tutorIndex = 0;
    data.individualPage.scheduleTab.brushIndex = 0;
    data.individualPage.scheduleTab.brushTypes.push_back({label: "Embedded", color: {255, 0, 0}});
    data.individualPage.scheduleTab.brushTypes.push_back({label: "eTutoring", color: {0, 255, 0}});
    data.individualPage.scheduleTab.brushTypes.push_back({label: "Drop-In", color: {0, 0, 255}});
    data.individualPage.scheduleTab.brushTypes.push_back({label: "By Appointment", color: {255, 255, 0}});
    //data.summaryPage.filters.push_back(FilterCommand());

    // initialize
    fstream configFile;
    configFile.open(configFilename, fstream::in);
    if (configFile.is_open()){
        data.cout << "successfully opened " << configFilename << endl;

        configFile 
        >> data.config.resolution.x
        >> data.config.resolution.y
        ;

        configFile.close();
    }
    else {
        data.cout << "failed to open " << configFilename << ". Using default settings" << endl;
        
        data.config.resolution.x = 800;
        data.config.resolution.y = 600;

        configFile.open(configFilename, fstream::out);

        configFile 
        << data.config.resolution.x << endl 
        << data.config.resolution.y << endl
        ;

        configFile.close();
    }

    cout << data.cout.str();

}


Application::~Application(){
    
}


void Application::load() {

}


void Application::save() {

}


void Application::run(){
    InitWindow(data.config.resolution.x, data.config.resolution.y, "TC Scheduler");

    data.ballPositionX = (float)data.config.resolution.x / 2; 
    data.ballPositionY = (float)data.config.resolution.y / 2;

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        pollInput();
        draw();
    }

    CloseWindow();
}


void Application::draw(){
    BeginDrawing();
    
    ClearBackground(RAYWHITE);

    DrawText("move the ball with arrow keys", 10, 10, 20, DARKGRAY);

    DrawCircleV(data.ballPosition, 50, MAROON);

    EndDrawing();
}


void Application::pollInput(){
    if (IsKeyDown(KEY_RIGHT)) data.ballPosition.x += 2.0f;
    if (IsKeyDown(KEY_LEFT)) data.ballPosition.x -= 2.0f;
    if (IsKeyDown(KEY_UP)) data.ballPosition.y -= 2.0f;
    if (IsKeyDown(KEY_DOWN)) data.ballPosition.y += 2.0f;
}