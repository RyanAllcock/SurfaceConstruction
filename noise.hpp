#ifndef HEADER_NOISE
#define HEADER_NOISE

#include <list> // fractal layer storage
#include <array> // dimension-dependent component storage
#include <ctime> // random gradient seed
#include <cmath> // coherent gradient components

#ifndef PI
#define PI 3.14159
#endif

// general noise classes

template <unsigned int D> class Noise{
protected:
	int size[D];
	float scale[D];
public:
	virtual float at(int in[D]) = 0; // retrieve single noise positional value
};

template <unsigned int D> class FractalNoise{
	std::list<Noise<D>*> layers; // octaves
	float persistence;
public:
	FractalNoise(float p);
	~FractalNoise();
	void add(Noise<D> *l); // add layer of noise
	float at(int in[3]);
};

// Perlin Noise classes

template <unsigned int D> class PerlinNoise : Noise<D>{
	
	// general internal functionality
protected:
	int permutation(int c[D]);
	
	// later-updated functionality
	virtual void interpolate(float t[D], float (&s)[D]);
	virtual float gradientProduct(int c[D], float dir[D]);
	virtual void gradient(int c[D], float (&g)[D]){}
	
	// external usage
public:
	PerlinNoise(int p[D], float s[D]);
	virtual float at(int in[D]);
};

class PerlinNoiseBasic : public PerlinNoise<2>{
	void gradient(int i0[2], float (&g)[2]);
public:
	PerlinNoiseBasic(int p[2], float s[2]) : PerlinNoise<2>(p, s) {}
};

class PerlinNoiseOriginal : public PerlinNoise<3>{
	float G[256 * 3];
	void gradient(int c[3], float (&g)[3]);
public:
	PerlinNoiseOriginal(int p[3], float s[3]); // random gradient lookup table for grid intersections
};

class PerlinNoiseImproved : public PerlinNoise<3>{
protected:
	virtual void gradient(int c[3], float (&g)[3]); // replace random gradient distribution with 12 predetermined axis-aligned gradient vectors
	virtual void interpolate(float t[3], float (&s)[3]); // smootherstep, or "quintic", weights
public:
	PerlinNoiseImproved(int p[3], float s[3]) : PerlinNoise<3>(p, s){}
};

class PerlinNoiseImplicit : public PerlinNoiseImproved{
	float gradientProduct(int c[3], float dir[3]); // replace gradient vector lookup and dot product with optimised implicit calculation
public:
	PerlinNoiseImplicit(int p[3], float s[3]) : PerlinNoiseImproved(p, s){}
};

#endif