#include "noise.hpp"

// general functions

namespace {

template <unsigned int D> float dotProduct(float a[D], float b[D]){
	float out = 0;
	for(int d = 0; d < D; d++) out += a[d] * b[d];
	return out;
}

float lerp(float a, float b, float w){
	return (b - a) * w + a;
}

}

// general noise functions

template <unsigned int D> FractalNoise<D>::FractalNoise(float p){
	persistence = p;
}

template <unsigned int D> FractalNoise<D>::~FractalNoise(){
	for(Noise<D> *l : layers)
		if(l != NULL) delete l;
}

template <unsigned int D> void FractalNoise<D>::add(Noise<D> *l){
	layers.push_back(l);
}

template <unsigned int D> float FractalNoise<D>::at(int in[3]){
	float total = 0;
	float maxValue = 0;
	float frequency = 1;
	float amplitude = 1;
	for(Noise<3> *l : layers){
		for(int d = 0; d < 3; d++) in[d] *= frequency;
		total += amplitude * l->at(in);
		maxValue += amplitude;
		amplitude *= persistence;
		frequency *= 2;
	}
	return total / maxValue;
}

// main functions

template <unsigned int D> PerlinNoise<D>::PerlinNoise(int p[D], float s[D]){
	for(int d = 0; d < D; d++){
		this->size[d] = p[d];
		this->scale[d] = s[d];
	}
}

PerlinNoiseOriginal::PerlinNoiseOriginal(int p[3], float s[3]) : PerlinNoise<3>(p, s){
	srand(time(NULL));
	for(int i = 0; i < 256; i++){ // generate random coherent gradient vector hash
		float theta = acos(2.f * ((float)rand() / RAND_MAX) - 1.f);
		float phi = 2.f * ((float)rand() / RAND_MAX) * PI;
		G[i * 3 + 0] = cos(phi) * sin(theta);
		G[i * 3 + 1] = sin(phi) * sin(theta);
		G[i * 3 + 2] = cos(theta);
		float total = 0;
		for(int d = 0; d < 3; d++) total += G[i * 3 + d];
		for(int d = 0; d < 3; d++) G[i * 3 + d] / total;
	}
}

template <unsigned int D> float PerlinNoise<D>::at(int in[D]){
	
	// coordinates
	float p[D];
	for(int d = 0; d < D; d++) p[d] = (float)in[d] / this->scale[d];
	
	// grid cells
	int i0[D], i1[D];
	for(int d = 0; d < D; d++){
		i0[d] = (int)p[d];
		i1[d] = (i0[d] + 1);
	}
	
	// grid distances
	float d0[D], d1[D];
	for(int d = 0; d < D; d++){
		d0[d] = (float)p[d] - (float)i0[d];
		d1[d] = d0[d] - 1;
	}
	
	// smoothing
	float s[D];
	interpolate(d0, s);
	
	// products
	std::array<float, 1 << D> n;
	for(int i = 0; i < 1 << D; i++){
		float dir[D]; // direction
		int c[D]; // corner
		for(int d = 0; d < D; d++){
			if(((unsigned int)i >> d) % 2 == 1){
				dir[d] = d1[d];
				c[d] = i1[d];
			}
			else{
				dir[d] = d0[d];
				c[d] = i0[d];
			}
		}
		n[i] = gradientProduct(c, dir);
	}
	
	// interpolation
	int inc = 1;
	for(int d = 0; d < D; d++){ // products 11111111 -> 10101010 -> 10001000 -> final value 10000000
		for(int i = 0; i < 1 << D; i += inc << 1) n[i] = lerp(n[i], n[i + inc], s[d]);
		inc <<= 1;
	}
	return n[0];
}

// internal functions
template <> int PerlinNoise<3>::permutation(int c[3]){ // fetch coherent pseudo-random permutation value
	int perm[512] = {
		151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 
        103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23, 190, 6, 148, 247, 120, 234, 75, 0, 
        26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33, 88, 237, 149, 56, 
        87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71, 134, 139, 48, 27, 166, 
        77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 
        46, 245, 40, 244, 102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 
        187, 208, 89, 18, 169, 200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109, 
        198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123, 5, 202, 38, 147, 118, 126, 
        255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42, 223, 183, 
        170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 
        172, 9, 129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 
        104, 218, 246, 97, 228, 251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 
        241, 81, 51, 145, 235, 249, 14, 239, 107, 49, 192, 214, 31, 181, 199, 106, 
        157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254, 138, 236, 205, 
        93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180, 
		// ... repeat ...
		151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 
        103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23, 190, 6, 148, 247, 120, 234, 75, 0, 
        26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33, 88, 237, 149, 56, 
        87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71, 134, 139, 48, 27, 166, 
        77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 
        46, 245, 40, 244, 102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 
        187, 208, 89, 18, 169, 200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109, 
        198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123, 5, 202, 38, 147, 118, 126, 
        255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42, 223, 183, 
        170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 
        172, 9, 129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 
        104, 218, 246, 97, 228, 251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 
        241, 81, 51, 145, 235, 249, 14, 239, 107, 49, 192, 214, 31, 181, 199, 106, 
        157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254, 138, 236, 205, 
        93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180};
	return perm[perm[perm[c[0] & 255] + (c[1] & 255)] + (c[2] & 255)];
}

// interpolation functions

template <unsigned int D> void PerlinNoise<D>::interpolate(float t[D], float (&s)[D]){ // smoothstep
	for(int d = 0; d < D; d++) s[d] = (-2.f * t[d] + 3.f) * t[d]*t[d];
}

void PerlinNoiseImproved::interpolate(float t[3], float (&s)[3]){ // smootherstep
	for(int d = 0; d < 3; d++) s[d] = ((6.f * t[d] - 15.f) * t[d] + 10.f) * t[d]*t[d]*t[d];
}

// gradient functions

void PerlinNoiseBasic::gradient(int c[2], float (&g)[2]){
	const unsigned w = 8 * sizeof(unsigned);
	const unsigned s = w / 2;
	unsigned a = c[0], b = c[1];
	a *= 3284157443; b ^= a << s | a >> w-s;
	b *= 1911520717; a ^= b << s | b >> w-s;
	a *= 2048419325;
	float random = a * (3.14159265 / ~(~0u >> 1)); // in [0,2PI]
	g[0] = sin(random);
	g[1] = cos(random);
}

void PerlinNoiseOriginal::gradient(int c[3], float (&g)[3]){ // fetch hashed gradient vectors
	for(int d = 0; d < 3; d++) g[d] = G[permutation(c) + d]; // random directions
}

void PerlinNoiseImproved::gradient(int c[3], float (&g)[3]){ // twelve directions
	float G[16 * 3] = { // lookup index taken from lo-value 4 bits sample
		 1, 1, 0, -1, 1, 0,  1,-1, 0, -1,-1, 0, 
		 1, 0, 1, -1, 0, 1,  1, 0,-1, -1, 0,-1, 
		 0, 1, 1,  0,-1, 1,  0, 1,-1,  0,-1,-1, 
		 1, 1, 0, -1, 1, 0,  0,-1, 1,  0,-1,-1};
	for(int d = 0; d < 3; d++) g[d] = G[(permutation(c) & (2 << 3 - 1) * 3 + d)];
}

// product functions

template <unsigned int D> float PerlinNoise<D>::gradientProduct(int c[D], float dir[D]){ // calculate dot product from direction vector
	float g[D]; // gradient
	gradient(c, g);
	return dotProduct<D>(dir, g);
}

float PerlinNoiseImplicit::gradientProduct(int c[3], float dir[3]){ // implicit values
	// since the chosen gradients are along the dimensional axes, dot products are sums of the position's components
	int h = permutation(c) & 15; // taken lo-value 4 bits as sample from hash
	float u = h<8 ? dir[0] : dir[1]; // distribute as x/y/z based on bit-2/3/4 of sample
	float v = h<4 ? dir[1] : h==12||h==14 ? dir[0] : dir[2];
	return ((h&1) == 0 ? u : -u) + ((h&2) == 0 ? v : -v); // sign as +/- based on bit-1 of sample
}