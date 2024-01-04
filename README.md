# Marching Cubes

## How To Use
- preview and make program using .DLL files located in deploy folder
- make sure GLEW and GLM header files are present
- make using Makefile on Windows operating system
- execute using cubes.exe
	- *p* changes Marching Cubes type
	- *w* *a* *s* *d* *lctrl* *space* to move free camera
	- *e* to tether camera to origin
		- *right-click* to pivot camera around origin
		- *tab* to free camera
	- *alt* to free mouse from window

## Modules
- Marching Cubes
- Fractal Noise

## Marching Cubes https://en.wikipedia.org/wiki/Marching_cubes
- Early method of 3D surface generation using density-assigned voxels
- Originally used to limit ambiguities in automated surface construction
- Preludes a wide range of modern surface generation research techniques
- Produces a set of vertices and their normals for a (usually triangular-faced) mesh

### In This Project
- Original Marching Cubes implementation, with lookup table generation
- Improved Marching Cubes optional lookup table version (given "Mystery" alias for now; source of lookup table generation by "Cory Gene Bloyd" as of yet undiscovered)

### General Algorithm of Marching Cubes
- Create a lookup table for the 256 permutations of a cube's triangulated surface (2^8 permutations of binary values for the cube's corner points)
- Acquire voxel-density data set
- For each cube of voxel-density points ... (split into n-sized slices perpendicular to the z-axis, for optimisation techniques like parallel computing and limited memory usage)
	- Use the cube's corner density values to select a triangulation from the permutation lookup table, then fetch the vertices from the edge vertices lookup table
	- For each vertex ... 
		- Interpolate the vertex's position between the two cube's corner points it lies between, using the corners' density values
		- Calculate the gradient vectors of these two corner points, using the central differences method with their respective surrounding voxel point density values
		- Interpolate between these two gradient vectors, using the corner's density values, to obtain the vertex's normal

### Types of Marching Cubes
- Original (maximum of 12 triangles per permutation) https://dl.acm.org/doi/pdf/10.1145/37401.37422
- Improved (maximum of 15 triangles per permutation, removes ambiguous surfaces) http://lemur.cmp.uea.ac.uk/Research/ivis/backup/PhD/Ronan%20iViS%20Stuff%20(Website)/Polygonising%20a%20scalar%20field.pdf
- Extended (selects sub-permutations to remove triangulation ambiguities) https://journal-bcs.springeropen.com/articles/10.1186/s13173-019-0086-6
- Dual (uses quad patches to improve triangle formation) https://dl.acm.org/doi/pdf/10.1109/VISUAL.2004.28
- Neural (improved geometric feature preservation, for e.g. sharp edges and smooth curves on pre-constructed surfaces) https://arxiv.org/pdf/2106.11272.pdf

### What to add next
- [Z-Slices] stay true to original MC paper's algorithm by computing mesh in slices (and maybe generate point values in slices & quadrants(?) too)
- [MCTerminology] check for more original MC paper terminology to utilise inside code (e.g. instead of array of "value"s, call it "density" instead)
- [PhongShading] find out if there's a way to shade using Phong shading instead of Gouraud shading, perhaps using shader-calculated normals (or utilising shader-calculated noise values when using coherent noise generated point values)
- [MarchingCubesDemystify] find source & name of "mystery" version, then compute lookup table manually
- [MarchingCubesOther] other MC algorithms & implementations from other papers etc.
- [StaticLookupTable] make lookup table generation static; only call once for each type of uniquely-tabled MC subclass
- [VertexSmoothingLookupTable] create lookup table to find "surface intersection edge" (s=0) quicker
- [Octrees]
- [DynamicMeshAlteration/Updating]
- [DualContouring&LaterResearch]
- [Improved&DualMarchingCubesEtc.]
- [AddNormalsFromNoise/ValueGeneration] Perlin Noise can possibly produce it's own normals, meaning they don't need to be generated using MC (expensive/unnecessary)
- [UtiliseTriangleEBO] since each vertex will ideally have it's own single normal to enable smoother surfaces, ebo's (vertex-normal pairs) are now practical again ... ?
- [NoRecalculatingSlices] somehow avoid recomputing values within adjacent slices, e.g. gradient vectors for normal smoothing could be used for up to 4 slices in total so save the values that long

### More Interesting/Extra Links
- (finding other MC types) https://www.bing.com/images/search?q=marching+cubes+configurations&form=HDRSC3&first=1
- (finding other MC papers) https://www.bing.com/search?q=marching+cubes+paper&form=QBLH&sp=-1&lq=0&pq=marching+cubes+pa&sc=10-17&qs=n&sk=&cvid=5DE23558BF46482AA5548D3A799CCB53&ghsh=0&ghacc=0&ghpl=
- (MC implementation video presentations) https://www.youtube.com/results?search_query=dual+marching+cubes
- (MC alternative methods and broader scope of MC) http://www.paulbourke.net/geometry/polygonise/
- (supposed MC octree approach) https://www.researchgate.net/publication/228454597_Isovox_A_Brick-Octree_Approach_to_Indirect_Visualization
- (MC lookup table generation/methods research/acquisition) https://www.bing.com/images/search?q=marching%20cubes%20lookup%20table&form=IQFRBA&id=8EF53EA6A22BF4C002453FC495FF6AE80FD06104&first=1&disoverlay=1