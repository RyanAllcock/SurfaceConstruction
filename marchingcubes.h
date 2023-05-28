#ifndef MARCHINGCUBES
#define MARCHINGCUBES

#ifndef GLEW_STATIC
#define GLEW_STATIC
#include "GL/glew.h"
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/string_cast.hpp>
#endif

#define THRES 0.2

struct MarchingCubes{
	
	// usage functions
	MarchingCubes(){} // if no lookup tables were obtained elsewhere, initialise MC using subclasses instead
	MarchingCubes(int *perm, float *edge, int *corner, int maxV); // use if already have pre-computed lookup tables
	void computeMesh(float *value, glm::ivec3 points, float **triangle, int *tn);
	
	// lookup table vertices
	protected:
		int maxVertices;
		int *permutation;
		float edgeVertex[12 * 3];
		int cornerIndex[8 * 3];
		
		// internal functions
		virtual void computeLookupTable(){} // get mesh-faces lookup table with permutations
		void transformPoints(int *p, int r);
		void transformEdges(int *e, int r);
		void addPermutation(int p, int *v);
		void countTriangles(float *value, glm::ivec3 points, glm::ivec3 faces, int *tn, int *perm);
		void computeTriangles(float *value, int *perm, glm::ivec3 points, glm::ivec3 faces, float *triangle);
};

struct MarchingCubesOriginal : MarchingCubes{
	
	// usage functions
	MarchingCubesOriginal();
	~MarchingCubesOriginal();
	
	// internal functions
	protected:
		virtual void computeLookupTable();
};

struct MarchingCubesMystery : MarchingCubes{
	
	// usage functions
	MarchingCubesMystery();
	~MarchingCubesMystery();
	
	// internal functions
	protected:
		virtual void computeLookupTable();
};

// general marching cubes

MarchingCubes::MarchingCubes(int *perm, float *edge, int *corner, int maxV){
	permutation = perm;
	for(int i = 0; i < 12 * 3; i++) edgeVertex[i] = edge[i];
	for(int i = 0; i < 8 * 3; i++) cornerIndex[i] = corner[i];
	maxVertices = maxV;
}

void MarchingCubes::transformPoints(int *p, int r){
	
	int vertexRotate[8 * 3] = { // rotation lookup table
		// 0  1  2  3  4  5  6  7 -> ...
		4, 5, 1, 0, 7, 6, 2, 3, // x
		1, 5, 6, 2, 0, 4, 7, 3, // y
		1, 2, 3, 0, 5, 6, 7, 4}; // z
	
	// rotate active vertices
	int out = 0;
	int b = 0b00000001;
	for(int i = 0; i < 8; i++){
		if(*p & b)
			out += 0b00000001 << vertexRotate[r * 8 + i];
		b <<= 1;
	}
	*p = out;
}

void MarchingCubes::transformEdges(int *e, int r){
	
	int edgeRotate[12 * 3] = { // rotation lookup table
		// 0  1  2  3  4  5  6  7  8  9  10 11 -> ...
		4 , 9 , 0 , 8 , 6 , 11, 2 , 10, 7 , 5 , 3 , 1 , // x
		9 , 5 , 11, 1 , 8 , 7 , 10, 3 , 0 , 4 , 2 , 6 , // y
		1 , 2 , 3 , 0 , 5 , 6 , 7 , 4 , 9 , 11, 8 , 10}; // z
	
	// rotate each edge in list
	int in[12];
	for(int i = 0; i < 12; i++)
		in[i] = e[i];
	for(int i = 0; i < 12 && in[i] != -1; i++){
		e[i] = edgeRotate[r * 12 + in[i]];
	}
}

void MarchingCubes::addPermutation(int p, int *v){ // add permutation and it's complementary-symmetric counterpart
	for(int m = 0; m < maxVertices; m++) permutation[p * maxVertices + m] = v[m];
	int antip = p ^ 0b11111111;
	for(int m = 0; m < maxVertices; m++) permutation[antip * maxVertices + m] = v[(m / 3 + 1) * 3 - (m % 3) - 1]; // add vertices backwards to reverse winding order
}

void MarchingCubes::countTriangles(float *value, glm::ivec3 points, glm::ivec3 faces, int *tn, int *perm){
	*tn = 0;
	int index = 0;
	for(int k = 0; k < faces.z; k++){
		for(int j = 0; j < faces.y; j++){
			for(int i = 0; i < faces.x; i++){
				int permVal = 
					(value[((k + cornerIndex[2]) * points.y + j + cornerIndex[1]) * points.x + i + cornerIndex[0]] > THRES) * 0b00000001 + 
					(value[((k + cornerIndex[5]) * points.y + j + cornerIndex[4]) * points.x + i + cornerIndex[3]] > THRES) * 0b00000010 + 
					(value[((k + cornerIndex[8]) * points.y + j + cornerIndex[7]) * points.x + i + cornerIndex[6]] > THRES) * 0b00000100 + 
					(value[((k + cornerIndex[11]) * points.y + j + cornerIndex[10]) * points.x + i + cornerIndex[9]] > THRES) * 0b00001000 + 
					(value[((k + cornerIndex[14]) * points.y + j + cornerIndex[13]) * points.x + i + cornerIndex[12]] > THRES) * 0b00010000 + 
					(value[((k + cornerIndex[17]) * points.y + j + cornerIndex[16]) * points.x + i + cornerIndex[15]] > THRES) * 0b00100000 + 
					(value[((k + cornerIndex[20]) * points.y + j + cornerIndex[19]) * points.x + i + cornerIndex[18]] > THRES) * 0b01000000 + 
					(value[((k + cornerIndex[23]) * points.y + j + cornerIndex[22]) * points.x + i + cornerIndex[21]] > THRES) * 0b10000000;
				for(int m = 0; permutation[permVal * maxVertices + m] != -1; m += 3) *tn += 3;
				perm[index++] = permVal;
			}
		}
	}
}

void MarchingCubes::computeTriangles(float *value, int *perm, glm::ivec3 points, glm::ivec3 faces, float *triangle){
	int size = 0;
	float maxFaces = (faces.x > faces.z ? (faces.x > faces.y ? faces.x : faces.y) : (faces.y > faces.z ? faces.y : faces.z));
	glm::vec3 cubeLength = glm::vec3(1.f / maxFaces);
	glm::vec3 base = (cubeLength - (glm::vec3)faces) / maxFaces;
	glm::vec3 inc = 2.f * cubeLength;
	int index = 0;
	for(int k = 0; k < faces.z; k++){
		for(int j = 0; j < faces.y; j++){
			for(int i = 0; i < faces.x; i++){
				int *edgeIndex = &permutation[perm[index++] * maxVertices];
				for(int t = 0; edgeIndex[t * 3] != -1 && t * 3 < maxVertices; t++){
					glm::vec3 tri[3];
					glm::vec3 norm[3];
					for(int v = 0; v < 3; v++){ //vertices
						tri[v] = glm::vec3(
							edgeVertex[edgeIndex[t * 3 + v] * 3 + 0], 
							edgeVertex[edgeIndex[t * 3 + v] * 3 + 1], 
							edgeVertex[edgeIndex[t * 3 + v] * 3 + 2]);
						
						// interpolation
						float *vp = glm::value_ptr(tri[v]);
						glm::ivec3 iv = tri[v];
						int *ivp = glm::value_ptr(iv);
						for(int s = 0; s < 3; s++){
							if(ivp[s] == 0){
								iv = (iv + 1) / 2 + glm::ivec3(i, j, k);
								float wa = value[(iv.z * points.y + iv.y) * points.x + iv.x];
								glm::vec3 g0 = glm::normalize(glm::vec3(
									value[(iv.z * points.y + iv.y) * points.x + (iv.x + 1 == points.x ? iv.x : iv.x + 1)] - value[(iv.z * points.y + iv.y) * points.x + (iv.x == 0 ? iv.x : iv.x - 1)], 
									value[(iv.z * points.y + (iv.y + 1 == points.y ? iv.y : iv.y + 1)) * points.x + iv.x] - value[(iv.z * points.y + (iv.y == 0 ? iv.y : iv.y - 1)) * points.x + iv.x], 
									value[((iv.z + 1 == points.z ? iv.z : iv.z + 1) * points.y + iv.y) * points.x + iv.x] - value[((iv.z == 0 ? iv.z : iv.z - 1) * points.y + iv.y) * points.x + iv.x]) / cubeLength);
								ivp[s]++;
								float wb = value[(iv.z * points.y + iv.y) * points.x + iv.x];
								glm::vec3 g1 = glm::normalize(glm::vec3(
									value[(iv.z * points.y + iv.y) * points.x + (iv.x + 1 == points.x ? iv.x : iv.x + 1)] - value[(iv.z * points.y + iv.y) * points.x + (iv.x == 0 ? iv.x : iv.x - 1)], 
									value[(iv.z * points.y + (iv.y + 1 == points.y ? iv.y : iv.y + 1)) * points.x + iv.x] - value[(iv.z * points.y + (iv.y == 0 ? iv.y : iv.y - 1)) * points.x + iv.x], 
									value[((iv.z + 1 == points.z ? iv.z : iv.z + 1) * points.y + iv.y) * points.x + iv.x] - value[((iv.z == 0 ? iv.z : iv.z - 1) * points.y + iv.y) * points.x + iv.x]) / cubeLength);
								float w = (THRES - wa) / (wb - wa);
								vp[s] += 2.f * w - 1.f; // vertex smoothing
								norm[v] = g0 * (1.f - w) + g1 * w; // normal smoothing
								break;
							}
						}
						
						tri[v] = tri[v] * cubeLength + glm::vec3(i, j, k) * inc + base;
					}
					
					// pass triangle into data
					for(int v = 0; v < 3; v++){
						triangle[size + v * 6] = tri[v].x;
						triangle[size + v * 6 + 1] = tri[v].y;
						triangle[size + v * 6 + 2] = tri[v].z;
						triangle[size + v * 6 + 3] = norm[v].x;
						triangle[size + v * 6 + 4] = norm[v].y;
						triangle[size + v * 6 + 5] = norm[v].z;
					}
					size += 18;
				}
			}
		}
	}
}

void MarchingCubes::computeMesh(float *value, glm::ivec3 points, float **triangle, int *tn){
	
	// mesh properties
	glm::ivec3 faces = glm::ivec3(points.x - 1, points.y - 1, points.z - 1);
	int fn = faces.x * faces.y * faces.z;
	
	// triangle counting
	int *meshPermutation = (int*)malloc(sizeof(int) * fn);
	countTriangles(value, points, faces, tn, meshPermutation);
	*triangle = (float*)malloc(sizeof(float) * (3 + 3) * *tn); // vertex & normal values per vertex in triangle * number of triangles
	
	// vertex data
	computeTriangles(value, meshPermutation, points, faces, *triangle);
	free(meshPermutation);
}

// original marching cubes

MarchingCubesOriginal::MarchingCubesOriginal(){
	maxVertices = 12;
	computeLookupTable();
}

MarchingCubesOriginal::~MarchingCubesOriginal(){
	free(permutation);
}

void MarchingCubesOriginal::computeLookupTable(){
	
	int patternPermutation[15] = { 
		0b00000000, // empty/full
		0b00000001, // 1 corner triangle
		0b00000011, // 1 corner plane
		0b00000101, // 2 corner triangles
		0b01000001, // 2 opposite corner triangles
		0b00110010, // 1 curved plane (1 trapezium plane, 1 flat triangle)
		0b01000011, // 1 corner plane, 1 corner triangle
		0b01001010, // 3 corner triangles
		0b00110011, // 1 flat plane
		0b10110001, // 1 diagonal plane, 2 wide triangles
		0b01101001, // 2 corner planes
		0b01110001, // 1 convex plane (1 cubic triangle, 2 tall triangles, 1 wide triangle)
		0b00111010, // 1 curved plane, 1 corner triangle
		0b10100101, // 4 corner triangles
		0b10110010};// 1 convex plane
	
	int patternVertex[15 * 12] = { 
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
		0 , 8 , 3 , -1, -1, -1, -1, -1, -1, -1, -1, -1, 
		3 , 1 , 8 , 1 , 9 , 8 , -1, -1, -1, -1, -1, -1, 
		0 , 8 , 3 , 1 , 2 , 11, -1, -1, -1, -1, -1, -1, 
		0 , 8 , 3 , 5 , 11, 6 , -1, -1, -1, -1, -1, -1, 
		0 , 1 , 8 , 8 , 1 , 7 , 1 , 5 , 7 , -1, -1, -1, 
		3 , 1 , 8 , 1 , 9 , 8 , 11, 6 , 5 , -1, -1, -1, 
		1 , 2 , 9 , 2 , 3 , 10, 11, 6 , 5 , -1, -1, -1, 
		3 , 1 , 5 , 3 , 5 , 7 , -1, -1, -1, -1, -1, -1, 
		3 , 6 , 10, 3 , 0 , 6 , 0 , 5 , 6 , 0 , 9 , 5 , 
		0 , 8 , 2 , 8 , 10, 2 , 4 , 9 , 11, 4 , 11, 6 , 
		3 , 0 , 7 , 0 , 11 , 7 , 0 , 9 , 11, 7 , 11, 6 , 
		8 , 1 , 7 , 8 , 0 , 1 , 7 , 1 , 5 , 3 , 10, 2 , 
		0 , 8 , 3 , 9 , 5 , 4 , 1 , 2 , 11, 7 , 6 , 10, 
		8 , 0 , 10, 0 , 5 , 10, 0 , 1 , 5 , 10, 5 , 6 };
	
	// set permutation vertices
	permutation = (int*)malloc(sizeof(int) * 256 * maxVertices);
	if(permutation == NULL) printf("Error: malloc failed\n");
	for(int v = 0; v < 256 * maxVertices; v++) permutation[v] = -1;
	for(int p = 0; p < 15; p++){
		
		// set current pattern
		int perm = patternPermutation[p];
		int *edges = (int*)&patternVertex[p * 12];
		
		// add lookup table triangles using rotated permutations and patterns
		for(int i = 0; i < 4; i++){ // closest wall
			addPermutation(perm, edges);
			transformPoints(&perm, 2);
			transformEdges(edges, 2);
		}
		transformPoints(&perm, 1);
		transformEdges(edges, 1);
		for(int i = 0; i < 4; i++){ // outer walls
			for(int j = 0; j < 2; j++){
				for(int k = 0; k < 4; k++){ // left and right walls
					addPermutation(perm, edges);
					transformPoints(&perm, 0);
					transformEdges(edges, 0);
				}
				transformPoints(&perm, 2);
				transformEdges(edges, 2);
				for(int k = 0; k < 4; k++){ // top and bottom walls
					addPermutation(perm, edges);
					transformPoints(&perm, 1);
					transformEdges(edges, 1);
				}
				transformPoints(&perm, 2);
				transformEdges(edges, 2);
			}
		}
		transformPoints(&perm, 1);
		transformEdges(edges, 1);
		for(int i = 0; i < 4; i++){ // furthest wall
			addPermutation(perm, edges);
			transformPoints(&perm, 2);
			transformEdges(edges, 2);
		}
	}
	
	// set edge vertex coordinates
	GLfloat vertex[12 * 3] = { 
		0, -1, 1, 
		1, 0, 1, 
		0, 1, 1, 
		-1, 0, 1, 
		0, -1, -1, 
		1, 0, -1, 
		0, 1, -1, 
		-1, 0, -1, 
		-1, -1, 0, 
		1, -1, 0, 
		-1, 1, 0, 
		1, 1, 0};
	for(int i = 0; i < 12 * 3; i++)
		edgeVertex[i] = vertex[i];
	
	// set cube corner vertex indices
	int corner[8 * 3] = {
		0, 0, 1, 
		1, 0, 1, 
		1, 1, 1, 
		0, 1, 1, 
		0, 0, 0, 
		1, 0, 0, 
		1, 1, 0, 
		0, 1, 0};
	for(int i = 0; i < 8 * 3; i++)
		cornerIndex[i] = corner[i];
}

// mystery marching cubes

MarchingCubesMystery::MarchingCubesMystery(){
	maxVertices = 16;
	computeLookupTable();
}

MarchingCubesMystery::~MarchingCubesMystery(){
	free(permutation);
}

void MarchingCubesMystery::computeLookupTable(){
	
	int perm[256 * maxVertices] =
		{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		0, 1, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		1, 8, 3, 9, 8, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		0, 8, 3, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		9, 2, 10, 0, 2, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		2, 8, 3, 2, 10, 8, 10, 9, 8, -1, -1, -1, -1, -1, -1, -1,
		3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		0, 11, 2, 8, 11, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		1, 9, 0, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		1, 11, 2, 1, 9, 11, 9, 8, 11, -1, -1, -1, -1, -1, -1, -1,
		3, 10, 1, 11, 10, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		0, 10, 1, 0, 8, 10, 8, 11, 10, -1, -1, -1, -1, -1, -1, -1,
		3, 9, 0, 3, 11, 9, 11, 10, 9, -1, -1, -1, -1, -1, -1, -1,
		9, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		4, 3, 0, 7, 3, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		0, 1, 9, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		4, 1, 9, 4, 7, 1, 7, 3, 1, -1, -1, -1, -1, -1, -1, -1,
		1, 2, 10, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		3, 4, 7, 3, 0, 4, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1,
		9, 2, 10, 9, 0, 2, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1,
		2, 10, 9, 2, 9, 7, 2, 7, 3, 7, 9, 4, -1, -1, -1, -1,
		8, 4, 7, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		11, 4, 7, 11, 2, 4, 2, 0, 4, -1, -1, -1, -1, -1, -1, -1,
		9, 0, 1, 8, 4, 7, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1,
		4, 7, 11, 9, 4, 11, 9, 11, 2, 9, 2, 1, -1, -1, -1, -1,
		3, 10, 1, 3, 11, 10, 7, 8, 4, -1, -1, -1, -1, -1, -1, -1,
		1, 11, 10, 1, 4, 11, 1, 0, 4, 7, 11, 4, -1, -1, -1, -1,
		4, 7, 8, 9, 0, 11, 9, 11, 10, 11, 0, 3, -1, -1, -1, -1,
		4, 7, 11, 4, 11, 9, 9, 11, 10, -1, -1, -1, -1, -1, -1, -1,
		9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		9, 5, 4, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		0, 5, 4, 1, 5, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		8, 5, 4, 8, 3, 5, 3, 1, 5, -1, -1, -1, -1, -1, -1, -1,
		1, 2, 10, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		3, 0, 8, 1, 2, 10, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1,
		5, 2, 10, 5, 4, 2, 4, 0, 2, -1, -1, -1, -1, -1, -1, -1,
		2, 10, 5, 3, 2, 5, 3, 5, 4, 3, 4, 8, -1, -1, -1, -1,
		9, 5, 4, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		0, 11, 2, 0, 8, 11, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1,
		0, 5, 4, 0, 1, 5, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1,
		2, 1, 5, 2, 5, 8, 2, 8, 11, 4, 8, 5, -1, -1, -1, -1,
		10, 3, 11, 10, 1, 3, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1,
		4, 9, 5, 0, 8, 1, 8, 10, 1, 8, 11, 10, -1, -1, -1, -1,
		5, 4, 0, 5, 0, 11, 5, 11, 10, 11, 0, 3, -1, -1, -1, -1,
		5, 4, 8, 5, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1,
		9, 7, 8, 5, 7, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		9, 3, 0, 9, 5, 3, 5, 7, 3, -1, -1, -1, -1, -1, -1, -1,
		0, 7, 8, 0, 1, 7, 1, 5, 7, -1, -1, -1, -1, -1, -1, -1,
		1, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		9, 7, 8, 9, 5, 7, 10, 1, 2, -1, -1, -1, -1, -1, -1, -1,
		10, 1, 2, 9, 5, 0, 5, 3, 0, 5, 7, 3, -1, -1, -1, -1,
		8, 0, 2, 8, 2, 5, 8, 5, 7, 10, 5, 2, -1, -1, -1, -1,
		2, 10, 5, 2, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1,
		7, 9, 5, 7, 8, 9, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1,
		9, 5, 7, 9, 7, 2, 9, 2, 0, 2, 7, 11, -1, -1, -1, -1,
		2, 3, 11, 0, 1, 8, 1, 7, 8, 1, 5, 7, -1, -1, -1, -1,
		11, 2, 1, 11, 1, 7, 7, 1, 5, -1, -1, -1, -1, -1, -1, -1,
		9, 5, 8, 8, 5, 7, 10, 1, 3, 10, 3, 11, -1, -1, -1, -1,
		5, 7, 0, 5, 0, 9, 7, 11, 0, 1, 0, 10, 11, 10, 0, -1,
		11, 10, 0, 11, 0, 3, 10, 5, 0, 8, 0, 7, 5, 7, 0, -1,
		11, 10, 5, 7, 11, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		0, 8, 3, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		9, 0, 1, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		1, 8, 3, 1, 9, 8, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1,
		1, 6, 5, 2, 6, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		1, 6, 5, 1, 2, 6, 3, 0, 8, -1, -1, -1, -1, -1, -1, -1,
		9, 6, 5, 9, 0, 6, 0, 2, 6, -1, -1, -1, -1, -1, -1, -1,
		5, 9, 8, 5, 8, 2, 5, 2, 6, 3, 2, 8, -1, -1, -1, -1,
		2, 3, 11, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		11, 0, 8, 11, 2, 0, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1,
		0, 1, 9, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1,
		5, 10, 6, 1, 9, 2, 9, 11, 2, 9, 8, 11, -1, -1, -1, -1,
		6, 3, 11, 6, 5, 3, 5, 1, 3, -1, -1, -1, -1, -1, -1, -1,
		0, 8, 11, 0, 11, 5, 0, 5, 1, 5, 11, 6, -1, -1, -1, -1,
		3, 11, 6, 0, 3, 6, 0, 6, 5, 0, 5, 9, -1, -1, -1, -1,
		6, 5, 9, 6, 9, 11, 11, 9, 8, -1, -1, -1, -1, -1, -1, -1,
		5, 10, 6, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		4, 3, 0, 4, 7, 3, 6, 5, 10, -1, -1, -1, -1, -1, -1, -1,
		1, 9, 0, 5, 10, 6, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1,
		10, 6, 5, 1, 9, 7, 1, 7, 3, 7, 9, 4, -1, -1, -1, -1,
		6, 1, 2, 6, 5, 1, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1,
		1, 2, 5, 5, 2, 6, 3, 0, 4, 3, 4, 7, -1, -1, -1, -1,
		8, 4, 7, 9, 0, 5, 0, 6, 5, 0, 2, 6, -1, -1, -1, -1,
		7, 3, 9, 7, 9, 4, 3, 2, 9, 5, 9, 6, 2, 6, 9, -1,
		3, 11, 2, 7, 8, 4, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1,
		5, 10, 6, 4, 7, 2, 4, 2, 0, 2, 7, 11, -1, -1, -1, -1,
		0, 1, 9, 4, 7, 8, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1,
		9, 2, 1, 9, 11, 2, 9, 4, 11, 7, 11, 4, 5, 10, 6, -1,
		8, 4, 7, 3, 11, 5, 3, 5, 1, 5, 11, 6, -1, -1, -1, -1,
		5, 1, 11, 5, 11, 6, 1, 0, 11, 7, 11, 4, 0, 4, 11, -1,
		0, 5, 9, 0, 6, 5, 0, 3, 6, 11, 6, 3, 8, 4, 7, -1,
		6, 5, 9, 6, 9, 11, 4, 7, 9, 7, 11, 9, -1, -1, -1, -1,
		10, 4, 9, 6, 4, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		4, 10, 6, 4, 9, 10, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1,
		10, 0, 1, 10, 6, 0, 6, 4, 0, -1, -1, -1, -1, -1, -1, -1,
		8, 3, 1, 8, 1, 6, 8, 6, 4, 6, 1, 10, -1, -1, -1, -1,
		1, 4, 9, 1, 2, 4, 2, 6, 4, -1, -1, -1, -1, -1, -1, -1,
		3, 0, 8, 1, 2, 9, 2, 4, 9, 2, 6, 4, -1, -1, -1, -1,
		0, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		8, 3, 2, 8, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1,
		10, 4, 9, 10, 6, 4, 11, 2, 3, -1, -1, -1, -1, -1, -1, -1,
		0, 8, 2, 2, 8, 11, 4, 9, 10, 4, 10, 6, -1, -1, -1, -1,
		3, 11, 2, 0, 1, 6, 0, 6, 4, 6, 1, 10, -1, -1, -1, -1,
		6, 4, 1, 6, 1, 10, 4, 8, 1, 2, 1, 11, 8, 11, 1, -1,
		9, 6, 4, 9, 3, 6, 9, 1, 3, 11, 6, 3, -1, -1, -1, -1,
		8, 11, 1, 8, 1, 0, 11, 6, 1, 9, 1, 4, 6, 4, 1, -1,
		3, 11, 6, 3, 6, 0, 0, 6, 4, -1, -1, -1, -1, -1, -1, -1,
		6, 4, 8, 11, 6, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		7, 10, 6, 7, 8, 10, 8, 9, 10, -1, -1, -1, -1, -1, -1, -1,
		0, 7, 3, 0, 10, 7, 0, 9, 10, 6, 7, 10, -1, -1, -1, -1,
		10, 6, 7, 1, 10, 7, 1, 7, 8, 1, 8, 0, -1, -1, -1, -1,
		10, 6, 7, 10, 7, 1, 1, 7, 3, -1, -1, -1, -1, -1, -1, -1,
		1, 2, 6, 1, 6, 8, 1, 8, 9, 8, 6, 7, -1, -1, -1, -1,
		2, 6, 9, 2, 9, 1, 6, 7, 9, 0, 9, 3, 7, 3, 9, -1,
		7, 8, 0, 7, 0, 6, 6, 0, 2, -1, -1, -1, -1, -1, -1, -1,
		7, 3, 2, 6, 7, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		2, 3, 11, 10, 6, 8, 10, 8, 9, 8, 6, 7, -1, -1, -1, -1,
		2, 0, 7, 2, 7, 11, 0, 9, 7, 6, 7, 10, 9, 10, 7, -1,
		1, 8, 0, 1, 7, 8, 1, 10, 7, 6, 7, 10, 2, 3, 11, -1,
		11, 2, 1, 11, 1, 7, 10, 6, 1, 6, 7, 1, -1, -1, -1, -1,
		8, 9, 6, 8, 6, 7, 9, 1, 6, 11, 6, 3, 1, 3, 6, -1,
		0, 9, 1, 11, 6, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		7, 8, 0, 7, 0, 6, 3, 11, 0, 11, 6, 0, -1, -1, -1, -1,
		7, 11, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		3, 0, 8, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		0, 1, 9, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		8, 1, 9, 8, 3, 1, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1,
		10, 1, 2, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		1, 2, 10, 3, 0, 8, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1,
		2, 9, 0, 2, 10, 9, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1,
		6, 11, 7, 2, 10, 3, 10, 8, 3, 10, 9, 8, -1, -1, -1, -1,
		7, 2, 3, 6, 2, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		7, 0, 8, 7, 6, 0, 6, 2, 0, -1, -1, -1, -1, -1, -1, -1,
		2, 7, 6, 2, 3, 7, 0, 1, 9, -1, -1, -1, -1, -1, -1, -1,
		1, 6, 2, 1, 8, 6, 1, 9, 8, 8, 7, 6, -1, -1, -1, -1,
		10, 7, 6, 10, 1, 7, 1, 3, 7, -1, -1, -1, -1, -1, -1, -1,
		10, 7, 6, 1, 7, 10, 1, 8, 7, 1, 0, 8, -1, -1, -1, -1,
		0, 3, 7, 0, 7, 10, 0, 10, 9, 6, 10, 7, -1, -1, -1, -1,
		7, 6, 10, 7, 10, 8, 8, 10, 9, -1, -1, -1, -1, -1, -1, -1,
		6, 8, 4, 11, 8, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		3, 6, 11, 3, 0, 6, 0, 4, 6, -1, -1, -1, -1, -1, -1, -1,
		8, 6, 11, 8, 4, 6, 9, 0, 1, -1, -1, -1, -1, -1, -1, -1,
		9, 4, 6, 9, 6, 3, 9, 3, 1, 11, 3, 6, -1, -1, -1, -1,
		6, 8, 4, 6, 11, 8, 2, 10, 1, -1, -1, -1, -1, -1, -1, -1,
		1, 2, 10, 3, 0, 11, 0, 6, 11, 0, 4, 6, -1, -1, -1, -1,
		4, 11, 8, 4, 6, 11, 0, 2, 9, 2, 10, 9, -1, -1, -1, -1,
		10, 9, 3, 10, 3, 2, 9, 4, 3, 11, 3, 6, 4, 6, 3, -1,
		8, 2, 3, 8, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1,
		0, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		1, 9, 0, 2, 3, 4, 2, 4, 6, 4, 3, 8, -1, -1, -1, -1,
		1, 9, 4, 1, 4, 2, 2, 4, 6, -1, -1, -1, -1, -1, -1, -1,
		8, 1, 3, 8, 6, 1, 8, 4, 6, 6, 10, 1, -1, -1, -1, -1,
		10, 1, 0, 10, 0, 6, 6, 0, 4, -1, -1, -1, -1, -1, -1, -1,
		4, 6, 3, 4, 3, 8, 6, 10, 3, 0, 3, 9, 10, 9, 3, -1,
		10, 9, 4, 6, 10, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		4, 9, 5, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		0, 8, 3, 4, 9, 5, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1,
		5, 0, 1, 5, 4, 0, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1,
		11, 7, 6, 8, 3, 4, 3, 5, 4, 3, 1, 5, -1, -1, -1, -1,
		9, 5, 4, 10, 1, 2, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1,
		6, 11, 7, 1, 2, 10, 0, 8, 3, 4, 9, 5, -1, -1, -1, -1,
		7, 6, 11, 5, 4, 10, 4, 2, 10, 4, 0, 2, -1, -1, -1, -1,
		3, 4, 8, 3, 5, 4, 3, 2, 5, 10, 5, 2, 11, 7, 6, -1,
		7, 2, 3, 7, 6, 2, 5, 4, 9, -1, -1, -1, -1, -1, -1, -1,
		9, 5, 4, 0, 8, 6, 0, 6, 2, 6, 8, 7, -1, -1, -1, -1,
		3, 6, 2, 3, 7, 6, 1, 5, 0, 5, 4, 0, -1, -1, -1, -1,
		6, 2, 8, 6, 8, 7, 2, 1, 8, 4, 8, 5, 1, 5, 8, -1,
		9, 5, 4, 10, 1, 6, 1, 7, 6, 1, 3, 7, -1, -1, -1, -1,
		1, 6, 10, 1, 7, 6, 1, 0, 7, 8, 7, 0, 9, 5, 4, -1,
		4, 0, 10, 4, 10, 5, 0, 3, 10, 6, 10, 7, 3, 7, 10, -1,
		7, 6, 10, 7, 10, 8, 5, 4, 10, 4, 8, 10, -1, -1, -1, -1,
		6, 9, 5, 6, 11, 9, 11, 8, 9, -1, -1, -1, -1, -1, -1, -1,
		3, 6, 11, 0, 6, 3, 0, 5, 6, 0, 9, 5, -1, -1, -1, -1,
		0, 11, 8, 0, 5, 11, 0, 1, 5, 5, 6, 11, -1, -1, -1, -1,
		6, 11, 3, 6, 3, 5, 5, 3, 1, -1, -1, -1, -1, -1, -1, -1,
		1, 2, 10, 9, 5, 11, 9, 11, 8, 11, 5, 6, -1, -1, -1, -1,
		0, 11, 3, 0, 6, 11, 0, 9, 6, 5, 6, 9, 1, 2, 10, -1,
		11, 8, 5, 11, 5, 6, 8, 0, 5, 10, 5, 2, 0, 2, 5, -1,
		6, 11, 3, 6, 3, 5, 2, 10, 3, 10, 5, 3, -1, -1, -1, -1,
		5, 8, 9, 5, 2, 8, 5, 6, 2, 3, 8, 2, -1, -1, -1, -1,
		9, 5, 6, 9, 6, 0, 0, 6, 2, -1, -1, -1, -1, -1, -1, -1,
		1, 5, 8, 1, 8, 0, 5, 6, 8, 3, 8, 2, 6, 2, 8, -1,
		1, 5, 6, 2, 1, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		1, 3, 6, 1, 6, 10, 3, 8, 6, 5, 6, 9, 8, 9, 6, -1,
		10, 1, 0, 10, 0, 6, 9, 5, 0, 5, 6, 0, -1, -1, -1, -1,
		0, 3, 8, 5, 6, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		10, 5, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		11, 5, 10, 7, 5, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		11, 5, 10, 11, 7, 5, 8, 3, 0, -1, -1, -1, -1, -1, -1, -1,
		5, 11, 7, 5, 10, 11, 1, 9, 0, -1, -1, -1, -1, -1, -1, -1,
		10, 7, 5, 10, 11, 7, 9, 8, 1, 8, 3, 1, -1, -1, -1, -1,
		11, 1, 2, 11, 7, 1, 7, 5, 1, -1, -1, -1, -1, -1, -1, -1,
		0, 8, 3, 1, 2, 7, 1, 7, 5, 7, 2, 11, -1, -1, -1, -1,
		9, 7, 5, 9, 2, 7, 9, 0, 2, 2, 11, 7, -1, -1, -1, -1,
		7, 5, 2, 7, 2, 11, 5, 9, 2, 3, 2, 8, 9, 8, 2, -1,
		2, 5, 10, 2, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1,
		8, 2, 0, 8, 5, 2, 8, 7, 5, 10, 2, 5, -1, -1, -1, -1,
		9, 0, 1, 5, 10, 3, 5, 3, 7, 3, 10, 2, -1, -1, -1, -1,
		9, 8, 2, 9, 2, 1, 8, 7, 2, 10, 2, 5, 7, 5, 2, -1,
		1, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		0, 8, 7, 0, 7, 1, 1, 7, 5, -1, -1, -1, -1, -1, -1, -1,
		9, 0, 3, 9, 3, 5, 5, 3, 7, -1, -1, -1, -1, -1, -1, -1,
		9, 8, 7, 5, 9, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		5, 8, 4, 5, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1,
		5, 0, 4, 5, 11, 0, 5, 10, 11, 11, 3, 0, -1, -1, -1, -1,
		0, 1, 9, 8, 4, 10, 8, 10, 11, 10, 4, 5, -1, -1, -1, -1,
		10, 11, 4, 10, 4, 5, 11, 3, 4, 9, 4, 1, 3, 1, 4, -1,
		2, 5, 1, 2, 8, 5, 2, 11, 8, 4, 5, 8, -1, -1, -1, -1,
		0, 4, 11, 0, 11, 3, 4, 5, 11, 2, 11, 1, 5, 1, 11, -1,
		0, 2, 5, 0, 5, 9, 2, 11, 5, 4, 5, 8, 11, 8, 5, -1,
		9, 4, 5, 2, 11, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		2, 5, 10, 3, 5, 2, 3, 4, 5, 3, 8, 4, -1, -1, -1, -1,
		5, 10, 2, 5, 2, 4, 4, 2, 0, -1, -1, -1, -1, -1, -1, -1,
		3, 10, 2, 3, 5, 10, 3, 8, 5, 4, 5, 8, 0, 1, 9, -1,
		5, 10, 2, 5, 2, 4, 1, 9, 2, 9, 4, 2, -1, -1, -1, -1,
		8, 4, 5, 8, 5, 3, 3, 5, 1, -1, -1, -1, -1, -1, -1, -1,
		0, 4, 5, 1, 0, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		8, 4, 5, 8, 5, 3, 9, 0, 5, 0, 3, 5, -1, -1, -1, -1,
		9, 4, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		4, 11, 7, 4, 9, 11, 9, 10, 11, -1, -1, -1, -1, -1, -1, -1,
		0, 8, 3, 4, 9, 7, 9, 11, 7, 9, 10, 11, -1, -1, -1, -1,
		1, 10, 11, 1, 11, 4, 1, 4, 0, 7, 4, 11, -1, -1, -1, -1,
		3, 1, 4, 3, 4, 8, 1, 10, 4, 7, 4, 11, 10, 11, 4, -1,
		4, 11, 7, 9, 11, 4, 9, 2, 11, 9, 1, 2, -1, -1, -1, -1,
		9, 7, 4, 9, 11, 7, 9, 1, 11, 2, 11, 1, 0, 8, 3, -1,
		11, 7, 4, 11, 4, 2, 2, 4, 0, -1, -1, -1, -1, -1, -1, -1,
		11, 7, 4, 11, 4, 2, 8, 3, 4, 3, 2, 4, -1, -1, -1, -1,
		2, 9, 10, 2, 7, 9, 2, 3, 7, 7, 4, 9, -1, -1, -1, -1,
		9, 10, 7, 9, 7, 4, 10, 2, 7, 8, 7, 0, 2, 0, 7, -1,
		3, 7, 10, 3, 10, 2, 7, 4, 10, 1, 10, 0, 4, 0, 10, -1,
		1, 10, 2, 8, 7, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		4, 9, 1, 4, 1, 7, 7, 1, 3, -1, -1, -1, -1, -1, -1, -1,
		4, 9, 1, 4, 1, 7, 0, 8, 1, 8, 7, 1, -1, -1, -1, -1,
		4, 0, 3, 7, 4, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		4, 8, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		9, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		3, 0, 9, 3, 9, 11, 11, 9, 10, -1, -1, -1, -1, -1, -1, -1,
		0, 1, 10, 0, 10, 8, 8, 10, 11, -1, -1, -1, -1, -1, -1, -1,
		3, 1, 10, 11, 3, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		1, 2, 11, 1, 11, 9, 9, 11, 8, -1, -1, -1, -1, -1, -1, -1,
		3, 0, 9, 3, 9, 11, 1, 2, 9, 2, 11, 9, -1, -1, -1, -1,
		0, 2, 11, 8, 0, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		3, 2, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		2, 3, 8, 2, 8, 10, 10, 8, 9, -1, -1, -1, -1, -1, -1, -1,
		9, 10, 2, 0, 9, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		2, 3, 8, 2, 8, 10, 0, 1, 8, 1, 10, 8, -1, -1, -1, -1,
		1, 10, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		1, 3, 8, 9, 1, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		0, 9, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		0, 3, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
	permutation = (int*)malloc(sizeof(int) * 256 * maxVertices);
	for(int i = 0; i < 256 * maxVertices; i++)
		permutation[i] = perm[i];
	
	//translation data
	float vertex[12 * 3] = {
		0, -1, -1,
		1, -1, 0,
		0, -1, 1,
		-1, -1, 0,
		0, 1, -1,
		1, 1, 0,
		0, 1, 1,
		-1, 1, 0,
		-1, 0, -1,
		1, 0, -1,
		1, 0, 1,
		-1, 0, 1};
	for(int i = 0; i < 12 * 3; i++)
		edgeVertex[i] = vertex[i];
	
	//vertex data
	int corner[8 * 3] = {
		0, 0, 0, 
		1, 0, 0, 
		1, 0, 1, 
		0, 0, 1, 
		0, 1, 0, 
		1, 1, 0, 
		1, 1, 1, 
		0, 1, 1};
	for(int i = 0; i < 8 * 3; i++)
		cornerIndex[i] = corner[i];
}

#endif