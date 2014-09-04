cls
del bin\Balls.exe
g++ -std=c++11 -O -o bin\Balls main.cpp -lsfml-graphics -lsfml-window -lsfml-system
cd bin
.\Balls.exe
cd ..