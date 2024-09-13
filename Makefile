CXX := g++
CXXFLAGS := -o

BIN := temp/
OUT := deploy/

main: main.cpp $(BIN)camera.o $(BIN)window.o $(BIN)shader.o $(BIN)terrain3d.o $(BIN)noise.o $(BIN)marchingcubes.o
	$(CXX) $(CXXFLAGS) $(OUT)cubes.exe $(BIN)camera.o $(BIN)window.o $(BIN)shader.o $(BIN)terrain3d.o $(BIN)noise.o $(BIN)marchingcubes.o main.cpp -lopenGL32 -lmingw32 -lSDL2main -lSDL2 -lglew32

$(BIN)camera.o: camera.cpp
	$(CXX) -c $(CXXFLAGS) $(BIN)camera.o camera.cpp

$(BIN)window.o: window.cpp
	$(CXX) -c $(CXXFLAGS) $(BIN)window.o window.cpp

$(BIN)shader.o: shader.cpp
	$(CXX) -c $(CXXFLAGS) $(BIN)shader.o shader.cpp

$(BIN)terrain3d.o: terrain3d.cpp
	$(CXX) -c $(CXXFLAGS) $(BIN)terrain3d.o terrain3d.cpp

$(BIN)noise.o: noise.cpp
	$(CXX) -c $(CXXFLAGS) $(BIN)noise.o noise.cpp

$(BIN)marchingcubes.o: marchingcubes.cpp
	$(CXX) -c $(CXXFLAGS) $(BIN)marchingcubes.o marchingcubes.cpp

prepare:
	mkdir $(BIN) $(OUT)

clean:
	cd temp & del /q /s "*.o" & cd .. & $(MAKE) --no-print-directory main