set PATH=C:\_path\msys64\ucrt64\bin;%PATH%

g++ -static ../src/*.cpp -o main -O1 -Wall -std=c++17 -Wno-missing-braces -L"../lib/" -lraylib -lopengl32 -lgdi32 -lwinmm -I"C:/_path/raylib/raylib/src" -I"../include"

::pause

main.exe
