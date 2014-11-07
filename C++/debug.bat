cls
del bin\Particles.exe
g++ -std=c++11 -g -LC:\MinGW\lib -IC:\MinGW\include -Ofast -o bin\Particles main.cpp -lsfml-graphics -lsfml-window -lsfml-system -lsfgui
cd bin
gdb Particles.exe
cd ..