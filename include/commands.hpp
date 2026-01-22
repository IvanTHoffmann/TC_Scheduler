#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <string>

class Command;

#include "application.hpp"


using namespace std;


class Command {
    public:
    virtual void exec(AppData& appData) = 0;
    virtual void undo(AppData& appData) = 0;
    virtual string getString(AppData& appData) = 0;
};


class PrintCommand : public Command {
    private:
    string str;

    public:
    PrintCommand(string s);
    ~PrintCommand();
    
    void exec(AppData& appData);
    void undo(AppData& appData);
    string getString(AppData& appData);
};

#endif