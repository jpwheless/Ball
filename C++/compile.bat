cls
del bin\Balls.exe
g++ -std=c++11 -LC:\MinGW\lib -IC:\MinGW\include -Ofast -o bin\Balls main.cpp -lsfml-graphics -lsfml-window -lsfml-system -lsfgui
cd bin
.\Balls.exe
cd ..