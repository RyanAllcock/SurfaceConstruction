#include "window.hpp"

// setup methods

Window::Window(const char *name, int w, int h, float frameRate, float inputRate){
	
	// SDL
	window = NULL;
	width = w;
	height = h;
	if(SDL_Init(SDL_INIT_EVERYTHING) < 0){
		// printf("SDL init failed: %s\n", SDL_GetError()); // WIP
		return;
	}
	if((window = SDL_CreateWindow(name, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL)) == NULL){
		// printf("SDL window creation failed: %s\n", SDL_GetError()); // WIP
		return;
	}
	
	// SDL OpenGL
	context = NULL;
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	if((context = SDL_GL_CreateContext(window)) == NULL){
		// printf("GL context creation failed: %s\n", SDL_GetError()); // WIP
		return;
	}
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	
	// OpenGL
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glClearColor(.2f, .2f, .2f, 1.f);
	
	// GLEW
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if(err != GLEW_OK){
		// printf("GLEW init failed: %s\n", glewGetErrorString(err)); // WIP
		return;
	}
	
	// time
	timeStart = 0;
	timePrev[0] = timePrev[1] = 0;
	timePeriod[0] = 1.f / frameRate;
	timePeriod[1] = 1.f / inputRate;
	
	// control
	mousePos[0] = mousePos[1] = 0;
	mouseMove[0] = mouseMove[1] = 0;
	
	// bindings
	bind(InputForward, &keyMap[keysym(SDLK_w)]);
	bind(InputBackward, &keyMap[keysym(SDLK_s)]);
	bind(InputLeft, &keyMap[keysym(SDLK_a)]);
	bind(InputRight, &keyMap[keysym(SDLK_d)]);
	bind(InputUp, &keyMap[keysym(SDLK_SPACE)]);
	bind(InputDown, &keyMap[keysym(SDLK_LCTRL)]);
	bind(InputSelect, &keyMap[keysym(SDLK_e)]);
	bind(InputRelease, &keyMap[keysym(SDLK_TAB)]);
	bind(InputFocus, &mouseMap[SDL_BUTTON_LEFT]);
	bind(InputSpin, &mouseMap[SDL_BUTTON_RIGHT]);
	bind(InputTest, &keyMap[keysym(SDLK_t)]);
	bind(InputSwitch, &keyMap[keysym(SDLK_p)]);
	bind(InputTabout, &keyMap[keysym(SDLK_LALT)]);
	for(int i = 0; i < WINDOW_KEYCODES; i++) keyMap[i] = 0;
	for(int i = 0; i < WINDOW_MOUSECODES; i++) mouseMap[i] = 0;
	
	// focus
	isWindowFocused = false;
	isCursorPresent = false;
	isCursorNewlyFocused = false;
	SDL_SetHintWithPriority(SDL_HINT_MOUSE_RELATIVE_MODE_WARP, "1", SDL_HINT_OVERRIDE);
}

void Window::bind(int id, int *ref){
	bindings.insert({id, ref});
}

void Window::timer(){
	timeStart = time(NULL);
	timePrev[0] = timePrev[1] = timeStart;
}

Window::~Window(){
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

// update methods

bool Window::cap(int type){
	if(time(NULL) - timePrev[type] > timePeriod[type]){
		timePrev[type] += timePeriod[type];
		return true;
	}
	return false;
}

int Window::get(){
	
	// events
	SDL_Event event;
	int key;
	while(SDL_PollEvent(&event)){
		switch(event.type){
			case SDL_QUIT:
				return 0;
				break;
			case SDL_MOUSEBUTTONDOWN:
				if(event.button.button < WINDOW_MOUSECODES) mouseMap[event.button.button] = (mouseMap[event.button.button] == 0 ? 1 : -1);
				// else std::cout << "Input error: code out of bounds (MOUSEDOWN " << event.button.button << ")\n"; // WIP
				break;
			case SDL_MOUSEBUTTONUP:
				if(event.button.button < WINDOW_MOUSECODES) mouseMap[event.button.button] = 0;
				// else std::cout << "Input error: code out of bounds (MOUSEUP " << event.button.button << ")\n"; // WIP
				break;
			case SDL_KEYDOWN:
				key = keysym(event.key.keysym.sym);
				if(key < WINDOW_KEYCODES) keyMap[key] = (keyMap[key] == 0 ? 1 : -1);
				// else std::cout << "Input error: code out of bounds (KEYDOWN " << key << ")\n"; // WIP
				break;
			case SDL_KEYUP:
				key = keysym(event.key.keysym.sym);
				if(key < WINDOW_KEYCODES) keyMap[key] = 0;
				// else std::cout << "Input error: code out of bounds (KEYUP " << key << ")\n"; // WIP
				break;
			case SDL_MOUSEMOTION:
				if(isCursorNewlyFocused){
					mouseMove[0] = 0;
					mouseMove[1] = 0;
					isCursorNewlyFocused = false;
				}
				else{
					mouseMove[0] += event.motion.xrel;
					mouseMove[1] -= event.motion.yrel;
				}
				break;
			case SDL_WINDOWEVENT:
				switch(event.window.event){
					case SDL_WINDOWEVENT_ENTER:
						isCursorPresent = true;
						break;
					case SDL_WINDOWEVENT_LEAVE:
						isCursorPresent = false;
						break;
				}
				break;
			default:
				break;
		}
	}
	
	// focusing
	if(output(InputFocus) == 1){
		if(isCursorPresent && !isWindowFocused){
			isWindowFocused = true;
			isCursorNewlyFocused = true;
			SDL_SetRelativeMouseMode(SDL_TRUE);
			output(InputFocus) = -1;
		}
	}
	if(output(InputTabout) == 1){
		SDL_SetRelativeMouseMode(SDL_FALSE);
		isWindowFocused = false;
		output(InputTabout) = -1;
	}
	
	return 1;
}

int& Window::output(int id){
	return *bindings[id];
}

void Window::swap(){
	SDL_GL_SwapWindow(window);
}

// motion methods

void Window::keyMovement(float (&k)[3]){
	k[0] = (abs(output(InputRight)) - abs(output(InputLeft))) * (float)isWindowFocused;
	k[1] = (abs(output(InputUp)) - abs(output(InputDown))) * (float)isWindowFocused;
	k[2] = (abs(output(InputForward)) - abs(output(InputBackward))) * (float)isWindowFocused;
}

void Window::mouseMovement(float (&m)[2]){
	m[0] = ((float)(2.f * mouseMove[0]) / (float)width) * (float)isWindowFocused;
	m[1] = ((float)(2.f * mouseMove[1]) / (float)height) * (float)isWindowFocused;
	mouseMove[0] = mouseMove[1] = 0;
}

// internal methods

int Window::keysym(int code){
	int key = code;
	if(key >= 128) key -= 0x40000039;
	if(key < 0) return WINDOW_KEYCODES;
	return key + 128;
}