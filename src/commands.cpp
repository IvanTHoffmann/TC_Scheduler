#include "commands.hpp"


PrintCommand::PrintCommand(string s) : str(s){

}

PrintCommand::~PrintCommand() {

}

void PrintCommand::exec(AppData& appData){
    
}

void PrintCommand::undo(AppData& appData){

}

string PrintCommand::getString(AppData& appData){
    return str;
}