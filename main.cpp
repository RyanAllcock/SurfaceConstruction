#include "lib/window/window.hpp"
#include "lib/shader.hpp"
#include "lib/camera.hpp"
#include "terrain.hpp"

#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif
#include "gl/glew.h"

#include <vector>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define WINDOW_PERSEC 144.f
#define INPUT_PERSEC 60.f

#define CAMERA_ROTATE_SENS .8f
#define CAMERA_MOVE_SENS .002f
#define CAMERA_FIELD_OF_VISION 70.f
#define PROJECTION_NEAR .1f
#define PROJECTION_FAR 100.f

#define TERRAIN_X 100
#define TERRAIN_Y 50
#define TERRAIN_Z 100
#define MC_THRESHOLD 0.2

enum Input{
	InputForward, InputBackward, InputLeft, InputRight, InputUp, InputDown,  // movement
	InputSelect, InputRelease, // camera
	InputFocus, InputSpin, // mouse
	InputTest, // shader
	InputSwitch, // terrain
	InputTabout}; // window

int main(int argc, char *argv[]){
	
	// window
	Window window("Marching Cubes", WindowResize | WindowGraphic, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_PERSEC, INPUT_PERSEC);
	InputBind input(window.getMouseMotionHandle(), window.getMousePositionHandle());
	input.bindAll(std::vector<std::pair<int,WindowKey>>{
		{InputForward, KeyW}, {InputBackward, KeyW}, {InputLeft, KeyA}, {InputRight, KeyD}, 
		{InputUp, KeySpace}, {InputDown, KeyLeftControl}, {InputSelect, KeyE}, {InputRelease, KeyTab}, 
		{InputTest, KeyT}, {InputSwitch, KeyP}, {InputTabout, KeyLeftAlt}}, window);
	input.bindAll(std::vector<std::pair<int,WindowButton>>{
		{InputFocus, MouseLeftClick}, {InputSpin, MouseRightClick}}, window);
	
	// camera
	Camera camera(glm::vec3(0, 0, 3), CAMERA_ROTATE_SENS, CAMERA_MOVE_SENS, window.getAspectRatio(), CAMERA_FIELD_OF_VISION, PROJECTION_NEAR, PROJECTION_FAR);
	
	// terrain
	std::vector<float> points, triangles;
	Terrain3D terrain(TERRAIN_X, TERRAIN_Y, TERRAIN_Z);
	points.resize(terrain.pointTotal * 4);
	terrain.points(points.data());
	float noiseScale[3] = { (float)TERRAIN_X / 4, (float)TERRAIN_Y / 2, (float)TERRAIN_Z / 4 };
	terrain.values(noiseScale, points.data() + terrain.pointTotal * 3);
	terrain.triangles(points.data() + terrain.pointTotal * 3, MC_THRESHOLD, triangles);
	
	// shaders
	PointShader pshader(points.data(), terrain.pointTotal);
	TriangleShader tshader(triangles.data(), terrain.faceTotal);
	
	// loop
	window.timer();
	bool isRunning = true;
	while(isRunning){
		
		// input
		WindowState windowState;
		while((windowState = window.get(), windowState) != WindowDefault){
			switch(windowState){
				case WindowQuit:
					isRunning = false;
					break;
				case WindowResized:
					camera.setProjection(window.getAspectRatio(), CAMERA_FIELD_OF_VISION, PROJECTION_NEAR, PROJECTION_FAR);
					break;
			}
		}
		
		// screen
		if(window.cap(WindowDisplay)){
			
			// camera
			float move[3] = { 0.f, 0.f, 0.f };
			float look[2];
			input.getMouseMotion(look);
			camera.input(input.getPress(InputSelect), input.getHold(InputSpin), input.getPress(InputRelease), move, look);
			glm::mat4 view, projection, rotation;
			glm::vec3 position;
			camera.uniforms(view, projection, rotation, position);
			
			// view
			glm::mat4 defmat = glm::mat4(1.f);
			pshader.uniform(glm::value_ptr(defmat), glm::value_ptr(view), glm::value_ptr(projection), glm::value_ptr(position));
			tshader.uniform(glm::value_ptr(defmat), glm::value_ptr(view), glm::value_ptr(projection), glm::value_ptr(position));
			
			// display
			window.clear();
			if(input.getHold(InputTest))
				pshader.display();
			else
				tshader.display();
			window.swap();
		}
		
		// simulation
		if(window.cap(WindowInput)){
			
			// camera
			if(input.getInactivePress(InputFocus)){
				window.lockCursor();
				input.setActive(true);
			}
			if(input.getPress(InputTabout)){
				window.unlockCursor();
				input.setActive(false);
			}
			
			// movement
			float look[2] = { 0.f, 0.f };
			float move[3];
			move[0] = input.getHold(InputRight) - input.getHold(InputLeft);
			move[1] = input.getHold(InputUp) - input.getHold(InputDown);
			move[2] = input.getHold(InputForward) - input.getHold(InputBackward);
			camera.input(input.getPress(InputSelect), input.getHold(InputSpin), input.getPress(InputRelease), move, look);
			
			// scene
			if(input.getPress(InputSwitch)){
				terrain.switchSurface(points.data() + terrain.pointTotal * 3, MC_THRESHOLD, triangles);
				tshader.reset(triangles.data(), terrain.faceTotal);
			}
		}
	}
	
	// cleanup
	std::vector<float>().swap(points);
	std::vector<float>().swap(triangles);
	
	return 0;
}