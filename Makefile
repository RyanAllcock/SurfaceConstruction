CXX := g++
CXXFLAGS := -o

main: main.cpp temp/camera.o temp/window.o temp/shader.o temp/terrain3d.o temp/noise.o temp/marchingcubes.o
	$(CXX) $(CXXFLAGS) cubes.exe temp/camera.o temp/window.o temp/shader.o temp/terrain3d.o temp/noise.o temp/marchingcubes.o main.cpp -lopenGL32 -lmingw32 -lSDL2main -lSDL2 -lglew32

temp/camera.o: camera.cpp temp
	$(CXX) -c $(CXXFLAGS) temp/camera.o camera.cpp

temp/window.o: window.cpp temp
	$(CXX) -c $(CXXFLAGS) temp/window.o window.cpp

temp/shader.o: shader.cpp temp
	$(CXX) -c $(CXXFLAGS) temp/shader.o shader.cpp

temp/terrain3d.o: terrain3d.cpp temp
	$(CXX) -c $(CXXFLAGS) temp/terrain3d.o terrain3d.cpp

temp/noise.o: noise.cpp temp
	$(CXX) -c $(CXXFLAGS) temp/noise.o noise.cpp

temp/marchingcubes.o: marchingcubes.cpp temp
	$(CXX) -c $(CXXFLAGS) temp/marchingcubes.o marchingcubes.cpp

temp:
	mkdir temp

clean:
	cd temp & del /q /s "*.o" & cd .. & $(MAKE) --no-print-directory main