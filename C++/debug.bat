cls
del bin\Particles.exe
g++ -gdwarf-2 -std=c++11 -LC:\MinGW\lib -IC:\MinGW\include -Ofast particles.cpp main.cpp ball.cpp quad.cpp blackHole.cpp -lsfml-graphics -lsfml-window -lsfml-system -lsfgui -o bin\Particles
cd bin
gdb Particles.exe
cd ..