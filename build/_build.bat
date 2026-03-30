::set PATH=%PATH%;C:\msys64\ucrt64\bin
g++ -g -std=c++20 -I../include ../src/*.cpp ../lib/*.a -lshlwapi -o TCScheduler

TCScheduler.exe