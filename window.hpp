#ifndef HEADER_WINDOW
#define HEADER_WINDOW

#include "gl/glew.h" // post-context GLEW initialisation
#include "SDL2/SDL.h" // context creation
#include "SDL2/SDL_opengl.h" // OpenGL options

#include <map> // input binding
#include <ctime> // update rate

#define WINDOW_KEYCODES (128 + 226)
#define WINDOW_MOUSECODES 8

#define INPUT_SENS_KEY 0.01f
#define INPUT_SENS_MOUSE 0.01f
#define INPUT_SENS_SCROLL 0.05f

enum Input{
	InputForward, InputBackward, InputLeft, InputRight, InputUp, InputDown,  // movement
	InputSelect, InputRelease, // camera
	InputFocus, InputSpin, // mouse
	InputTest, // shader
	InputSwitch, // terrain
	InputTabout}; // window

class Window{
	
	// context
	int width, height;
	SDL_Window *window;
	SDL_GLContext context;
	
	// time
	clock_t timeStart;
	clock_t timePeriod[2]; // frame, input
	clock_t timePrev[2];
	
	// focus
	bool isWindowFocused;
	bool isCursorPresent;
	bool isCursorNewlyFocused;
	
	// bindings
	std::map<int, int*> bindings;
	int keyMap[WINDOW_KEYCODES];
	int mouseMap[WINDOW_MOUSECODES];
	
	// cursor
	float mouseMove[2], mousePos[2];
	
	// internal operations
	int keysym(int code);
	
public:
	// setup
	Window(const char *name, int w, int h, float frameRate, float inputRate);
	void bind(int id, int *ref);
	void timer();
	~Window();
	
	// update
	bool cap(int type);
	int get();
	int& output(int id);
	void swap();
	
	// motion
	void keyMovement(float (&k)[3]);
	void mouseMovement(float (&m)[2]);
	
	// properties
	float getAspectRatio();
};

#endif