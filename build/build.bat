set PATH=C:\msys64\ucrt64\bin;%PATH%
set PATH=C:\raylib;C:\raylib\raylib\src;%PATH%

g++ ../src/*.cpp -o main -O1 -Wall -std=c++17 -Wno-missing-braces -L"../lib/" -lraylib -lopengl32 -lgdi32 -lwinmm -I"C:/raylib/raylib/src" -I"../include"

pause

main.exe
