CXX := g++
CXXFLAGS := -o

LIB := lib/
SRC := source/
BIN := temp/
OUT := deploy/

main: main.cpp $(BIN)camera.o $(BIN)window.o $(BIN)shader.o $(BIN)terrain3d.o $(BIN)noise.o $(BIN)marchingcubes.o
	$(CXX) $(CXXFLAGS) $(OUT)cubes.exe $(BIN)camera.o $(BIN)window.o $(BIN)shader.o $(BIN)terrain3d.o $(BIN)noise.o $(BIN)marchingcubes.o main.cpp -lopenGL32 -lmingw32 -lSDL2main -lSDL2 -lglew32

$(BIN)camera.o: $(LIB)camera.cpp
	$(CXX) -c $(CXXFLAGS) $(BIN)camera.o $(LIB)camera.cpp

$(BIN)window.o: $(LIB)window.cpp
	$(CXX) -c $(CXXFLAGS) $(BIN)window.o $(LIB)window.cpp

$(BIN)shader.o: $(LIB)shader.cpp
	$(CXX) -c $(CXXFLAGS) $(BIN)shader.o $(LIB)shader.cpp

$(BIN)terrain3d.o: $(LIB)terrain3d.cpp
	$(CXX) -c $(CXXFLAGS) $(BIN)terrain3d.o $(LIB)terrain3d.cpp

$(BIN)noise.o: $(LIB)noise.cpp
	$(CXX) -c $(CXXFLAGS) $(BIN)noise.o $(LIB)noise.cpp

$(BIN)marchingcubes.o: $(SRC)marchingcubes.cpp
	$(CXX) -c $(CXXFLAGS) $(BIN)marchingcubes.o $(SRC)marchingcubes.cpp

prepare:
	mkdir $(BIN) $(OUT)

clean:
	cd temp & del /q /s "*.o" & cd .. & $(MAKE) --no-print-directory main