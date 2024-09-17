#ifndef HEADER_MARCHINGCUBES
#define HEADER_MARCHINGCUBES

#include <cmath> // square root for normalisation
#include <vector> // permutation storage

struct MCPermutations{
	void compute(std::vector<int> &permutation, float (&edgeVertex)[12 * 3], int (&cornerIndex)[8 * 3], int &maxVertices);
	virtual void permutations(std::vector<int> &permutation, int &maxVertices) = 0;
	virtual void tables(float (&edgeVertex)[12 * 3], int (&cornerIndex)[8 * 3]) = 0;
};

struct MCOriginal : MCPermutations{
	void permutations(std::vector<int> &permutation, int &maxVertices);
	void tables(float (&edgeVertex)[12 * 3], int (&cornerIndex)[8 * 3]);
	
	// auxiliary functions
	void transformPoints(int *p, int r);
	void transformEdges(int *e, int r);
	void addPermutation(int p, int *v, std::vector<int> &permutation, int maxVertices);
};

struct MCMystery : MCPermutations{
	void permutations(std::vector<int> &permutation, int &maxVertices);
	void tables(float (&edgeVertex)[12 * 3], int (&cornerIndex)[8 * 3]);
};

class MarchingCubes{
	
	// lookup table
	int maxVertices;
	std::vector<int> permutation;
	float edgeVertex[12 * 3];
	int cornerIndex[8 * 3];
	
	// usage
public:
	MarchingCubes(MCPermutations *p);
	int count(float *density, int size[3], float threshold, int *perm);
	void compute(float *density, int *perm, int size[3], float threshold, std::vector<float> &triangle);
};

#endif