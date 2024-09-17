#ifndef HEADER_CAMERA
#define HEADER_CAMERA

#include <glm/glm.hpp> // matrix operations
#include <glm/ext.hpp> // matrix value references

#include <memory> // mode allocation

#ifndef PI
#define PI 3.14159
#endif

struct Transform{
	
	// properties
	float rotateX, rotateY;
	glm::vec3 position;
	
	// matrices
	glm::mat4 rotation, placement;
	
	// general
	Transform(glm::vec3 p);
	
	// transformation
	void setRotate();
	void setPosition();
	glm::mat4 getView();
	
	// operations
	void rotate(float x, float y);
	void lookAt(glm::vec3 target);
	void move(glm::vec3 m);
	void dolly(glm::vec3 pivot, float distance);
};

struct CameraMode{
	Transform &transform;
	CameraMode(Transform &t);
	virtual glm::mat4 look(int cond, float x, float y) = 0;
	virtual glm::mat4 move(glm::vec3 m) = 0;
};

struct CameraFree : CameraMode{ // spin and move
	CameraFree(Transform &t);
	glm::mat4 look(int cond, float x, float y); // turn
	glm::mat4 move(glm::vec3 m); // fly
};

struct CameraFocus : CameraMode{ // spin and select
	glm::vec3 centre;
	float distance;
	CameraFocus(Transform &t, glm::vec3 target);
	glm::mat4 look(int cond, float x, float y); // pivot
	glm::mat4 move(glm::vec3 m); // nothing
};

class Camera{
	
	// settings
	float rotateSensitivity, moveSensitivity;
	
	// properties
	Transform transform;
	glm::mat4 view, projection;
	std::unique_ptr<CameraMode> mode;
	
	// modes
	glm::mat4 setFree();
	glm::mat4 setFocus(glm::vec3 target);
	
	// input
public:
	Camera(glm::vec3 p, float rSens, float mSens, float aspectRatio, float fov, float near, float far);
	void input(int &select, int spin, int &release, float motion[3], float turning[2]);
	void uniforms(glm::mat4 &view, glm::mat4 &projection, glm::mat4 &rotation, glm::vec3 &position);
	void setProjection(float aspectRatio, float fov, float near, float far);
};

#endif