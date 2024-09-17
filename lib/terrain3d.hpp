#ifndef HEADER_TERRAIN3D
#define HEADER_TERRAIN3D

#include "noise.hpp"
#include "../source/marchingcubes.hpp"

#include <vector> // data storage
#include <memory> // permutation class persistence
#include <iterator> // terrain type cycling

struct Terrain3D{
	int pointSize[3];
	int pointTotal;
	int cubeSize[3];
	float cubeLength;
	float cubeBase[3];
	float cubeInc;
	int cubeTotal;
	int faceTotal;
	std::vector<MarchingCubes> marchers;
	std::vector<MarchingCubes>::iterator mc;
	Terrain3D(int x, int y, int z);
	void points(float *points);
	void values(float scale[3], float *values);
	void triangles(float *values, float threshold, std::vector<float> &triangles);
	void switchSurface(float *values, float threshold, std::vector<float> &tris);
};

#endif