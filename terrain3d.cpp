#include "terrain3d.hpp"

Terrain3D::Terrain3D(int x, int y, int z){
	pointSize[0] = x;
	pointSize[1] = y;
	pointSize[2] = z;
	pointTotal = pointSize[0] * pointSize[1] * pointSize[2];
	cubeSize[0] = pointSize[0] - 1;
	cubeSize[1] = pointSize[1] - 1;
	cubeSize[2] = pointSize[2] - 1;
	cubeLength = 1.f / (cubeSize[0] > cubeSize[1] 
		? (cubeSize[1] > cubeSize[2] ? cubeSize[1] : cubeSize[2]) 
		: (cubeSize[0] > cubeSize[2] ? cubeSize[0] : cubeSize[2]));
	cubeBase[0] = (cubeLength - pointSize[0]) * cubeLength;
	cubeBase[1] = (cubeLength - pointSize[1]) * cubeLength;
	cubeBase[2] = (cubeLength - pointSize[2]) * cubeLength;
	cubeInc = cubeLength * 2;
	cubeTotal = (pointSize[0] - 1) * (pointSize[1] - 1) * (pointSize[2] - 1);
	faceTotal = 0;
	marchers.push_back(MarchingCubes(std::make_unique<MCOriginal>().get()));
	marchers.push_back(MarchingCubes(std::make_unique<MCMystery>().get()));
	mc = marchers.begin();
}

void Terrain3D::points(float *points){
	int index = 0;
	for(int k = 0; k < pointSize[2]; k++){
		for(int j = 0; j < pointSize[1]; j++){
			for(int i = 0; i < pointSize[0]; i++){
				points[index++] = cubeInc * i + cubeBase[0];
				points[index++] = cubeInc * j + cubeBase[1];
				points[index++] = cubeInc * k + cubeBase[2];
			}
		}
	}
}

void Terrain3D::values(float scale[3], float *values){
	PerlinNoiseImplicit noise(pointSize, scale);
	int index = 0;
	for(int k = 0; k < pointSize[2]; k++){
		int atBorderZ = k % (pointSize[2] - 1) != 0;
		for(int j = 0; j < pointSize[1]; j++){
			int atBorderY = (j % (pointSize[1] - 1) != 0) * atBorderZ;
			for(int i = 0; i < pointSize[0]; i++){
				int atBorderX = (i % (pointSize[0] - 1) != 0) * atBorderY;
				int in[3] = { i, j, k };
				values[index++] = (float)atBorderX * noise.at(in);
			}
		}
	}
}

void Terrain3D::triangles(float *values, float threshold, std::vector<float> &tris){
	std::vector<int> mcPerm(cubeTotal);
	faceTotal = mc->count(values, pointSize, threshold, mcPerm.data());
	tris.resize(faceTotal * 6);
	mc->compute(values, mcPerm.data(), pointSize, threshold, tris);
	std::vector<int>().swap(mcPerm);
}

void Terrain3D::switchSurface(float *values, float threshold, std::vector<float> &tris){
	if(std::next(mc) == marchers.end()) mc = marchers.begin();
	else mc++;
	triangles(values, threshold, tris);
}