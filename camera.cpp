#include "camera.hpp"

// Transform

Transform::Transform(glm::vec3 p){
	rotateX = rotateY = 0;
	position = p;
	setRotate();
	setPosition();
}

// transformation methods

void Transform::setRotate(){
	glm::mat4 pitch = glm::rotate(glm::mat4(1), rotateY, glm::vec3(-1, 0, 0));
	glm::mat4 yaw = glm::rotate(glm::mat4(1), rotateX, glm::vec3(0, 1, 0));
	rotation = pitch * yaw;
}

void Transform::setPosition(){
	placement = glm::translate(glm::mat4(1), -position);
}

glm::mat4 Transform::getView(){
	return rotation * placement;
}

// general operations

void Transform::rotate(float x, float y){
	rotateX += x;
	rotateY += y;
	rotateX = (rotateX > PI * 2 ? rotateX - PI * 2 : (rotateX < 0 ? rotateX + PI * 2 : rotateX));
	rotateY = (rotateY > PI / 2 ? PI / 2 : (rotateY < -PI / 2 ? -PI / 2 : rotateY));
	setRotate();
}

void Transform::lookAt(glm::vec3 target){
	glm::vec3 upGlobal = glm::vec3(0, 1.f, 0);
	glm::vec3 dir = glm::normalize(target - position);
	glm::vec3 right = glm::normalize(glm::cross(upGlobal, -dir));
	glm::vec3 up = glm::normalize(glm::cross(-dir, right));
	glm::vec3 forward = glm::normalize(glm::cross(upGlobal, right));
	rotateX = right.z < 0 ? PI * 2 - acos(right.x) : acos(right.x);
	rotateY = acos(glm::dot(forward, up) / (glm::length(forward) * glm::length(up))) - PI / 2.f;
	setRotate();
}

void Transform::move(glm::vec3 m){
	position += glm::vec3(transpose(rotation) * glm::vec4(m.x, m.y, -m.z, 1));
	setPosition();
}

void Transform::dolly(glm::vec3 pivot, float distance){
	glm::vec3 offset = glm::vec3(glm::transpose(rotation) * glm::vec4(0, 0, -distance, 1));
	position = pivot + offset;
	setPosition();
}

// Camera

// mode methods

glm::mat4 Camera::setFree(){
	mode.reset(new CameraFree(transform));
	return transform.getView();
}

glm::mat4 Camera::setFocus(glm::vec3 target){
	mode.reset(new CameraFocus(transform, target));
	return transform.getView();
}

// input methods

Camera::Camera(glm::vec3 p, float rSens, float mSens, float aspectRatio, float fov, float near, float far) : transform(p), 
	rotateSensitivity(rSens), moveSensitivity(mSens), // settings
	mode(new CameraFree(transform)) { // mode
	
	// projection
	view = transform.getView();
	float fovx = (fov * PI / 180.f);
	float fovy = ((float)(2.0 * atan(tan(fovx / 2.0) / aspectRatio)));
	projection = glm::perspective(fovy, aspectRatio, near, far);
}

void Camera::input(int &select, int spin, int &release, float motion[3], float turning[2]){
	view = mode->look(spin, rotateSensitivity * turning[0], rotateSensitivity * turning[1]);
	view = mode->move(moveSensitivity * glm::vec3(motion[0], motion[1], motion[2]));
	if(select == 1){ // set pivot focus mode
		view = setFocus(glm::vec3(0.f, 0.f, 0.f));
		select = -1;
	}
	if(release == 1){ // set free move mode
		view = setFree();
		release = -1;
	}
}

void Camera::uniforms(glm::mat4 &view, glm::mat4 &projection, glm::mat4 &rotation, glm::vec3 &position){
	view = this->view;
	projection = this->projection;
	rotation = transform.rotation;
	position = transform.position;
}

// Camera Mode

CameraMode::CameraMode(Transform &t) : transform(t) {}

// Free Camera

CameraFree::CameraFree(Transform &t) : CameraMode(t) {}

// free methods

glm::mat4 CameraFree::look(int cond, float x, float y){
	transform.rotate(x, y);
	return transform.getView();
}

glm::mat4 CameraFree::move(glm::vec3 m){
	transform.move(m);
	return transform.getView();
}

// Focus Camera

CameraFocus::CameraFocus(Transform &t, glm::vec3 target) : CameraMode(t) {
	transform.lookAt(target);
	centre = target;
	distance = glm::length(centre - transform.position);
}

// focus methods

glm::mat4 CameraFocus::look(int cond, float x, float y){
	if(cond){
		transform.rotate(x, y);
		transform.dolly(centre, -distance); // rig camera at pivot
	}
	return transform.getView();
}

glm::mat4 CameraFocus::move(glm::vec3 m){
	return transform.getView();
}