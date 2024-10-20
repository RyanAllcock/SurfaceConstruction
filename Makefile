CXX := g++
CXXFLAGS := -o

LIB := lib/
SRC := source/
BIN := temp/
OUT := deploy/

main: main.cpp $(BIN)camera.o $(BIN)window.o $(BIN)shader.o $(BIN)terrain.o $(BIN)noise.o $(BIN)marchingcubes.o
	$(CXX) $(CXXFLAGS) $(OUT)cubes.exe $(BIN)camera.o $(BIN)window.o $(BIN)shader.o $(BIN)terrain.o $(BIN)noise.o $(BIN)marchingcubes.o main.cpp -lopenGL32 -lmingw32 -lSDL2main -lSDL2 -lglew32

$(BIN)camera.o: $(LIB)camera.cpp $(LIB)camera.hpp
	$(CXX) -c $(CXXFLAGS) $(BIN)camera.o $(LIB)camera.cpp

$(BIN)window.o: $(LIB)window/window.cpp $(LIB)window/window.hpp
	$(CXX) -c $(CXXFLAGS) $(BIN)window.o $(LIB)window/window.cpp

$(BIN)shader.o: $(LIB)shader.cpp $(LIB)shader.hpp
	$(CXX) -c $(CXXFLAGS) $(BIN)shader.o $(LIB)shader.cpp

$(BIN)terrain.o: terrain.cpp terrain.hpp
	$(CXX) -c $(CXXFLAGS) $(BIN)terrain.o terrain.cpp

$(BIN)noise.o: $(LIB)noise/noise.cpp $(LIB)noise/noise.hpp
	$(CXX) -c $(CXXFLAGS) $(BIN)noise.o $(LIB)noise/noise.cpp

$(BIN)marchingcubes.o: $(SRC)marchingcubes.cpp $(SRC)marchingcubes.hpp
	$(CXX) -c $(CXXFLAGS) $(BIN)marchingcubes.o $(SRC)marchingcubes.cpp

prepare:
	mkdir $(BIN) $(OUT)

clean:
	cd temp & del /q /s "*.o" & cd .. & $(MAKE) --no-print-directory main