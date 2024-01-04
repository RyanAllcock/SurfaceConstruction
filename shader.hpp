#ifndef HEADER_SHADER
#define HEADER_SHADER

#include "gl/glew.h" // OpenGL & GLSL

// classes

template <typename T> class Shader{
	void buffers();
	void sources(const char **v, const char **f);
protected:
	GLuint vbo, vao, program;
	GLfloat *data;
	int elements;
public:
	Shader(GLfloat *d, int e);
	~Shader();
	void uniform(float *m, float *v, float *p, float *vp);
	void reset(GLfloat *d, int e);
	void display();
};

class PointShader : public Shader<PointShader>{
public:
	void buffers();
	void sources(const char **v, const char **f);
	PointShader(GLfloat *d, int e) : Shader(d, e) {}
	void display();
};

class TriangleShader : public Shader<TriangleShader>{
public:
	void buffers();
	void sources(const char **v, const char **f);
	TriangleShader(GLfloat *d, int e) : Shader(d, e) {}
	void display();
};

// general shader

template <typename T>
Shader<T>::Shader(GLfloat *d, int e){
	
	// initialisation
	vbo = vao = program = 0;
	data = d;
	elements = e;
	
	// buffers
	buffers();
	
	// sources
	const char *vShaderSrc, *fShaderSrc;
	sources(&vShaderSrc, &fShaderSrc);
	
	// shaders
	GLint status;
	GLchar infoLog[1024];
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(vs, 1, &vShaderSrc, NULL);
	glShaderSource(fs, 1, &fShaderSrc, NULL);
	
	// compilation
	glCompileShader(vs);
	glCompileShader(fs);
	if(!(glGetShaderiv(vs, GL_COMPILE_STATUS, &status), status)){
		glGetShaderInfoLog(vs, sizeof(infoLog), NULL, infoLog);
		// printf("vs compile error: %s\n", infoLog); // WIP
	}
	if(!(glGetShaderiv(fs, GL_COMPILE_STATUS, &status), status)){
		glGetShaderInfoLog(fs, sizeof(infoLog), NULL, infoLog);
		// printf("fs compile error: %s\n", infoLog); // WIP
	}
	
	// program
	if((program = glCreateProgram()) == 0);
		// printf("shader program not created\n"); // WIP
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	if(!(glGetProgramiv(program, GL_LINK_STATUS, &status), status)){
		glGetProgramInfoLog(program, sizeof(infoLog), NULL, infoLog);
		// printf("shader program error: %s\n", infoLog); // WIP
	}
	glDeleteShader(vs);
	glDeleteShader(fs);
}

template <typename T>
Shader<T>::~Shader(){
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	glDeleteProgram(program);
}

template <typename T>
void Shader<T>::uniform(float *m, float *v, float *p, float *vp){
	glUseProgram(program);
	glUniformMatrix4fv(glGetUniformLocation(program,"model"), 1, GL_FALSE, m);
	glUniformMatrix4fv(glGetUniformLocation(program,"view"), 1, GL_FALSE, v);
	glUniformMatrix4fv(glGetUniformLocation(program,"projection"), 1, GL_FALSE, p);
	glUniform3fv(glGetUniformLocation(program,"viewPos"), 1, vp);
	glUseProgram(0);
}

template <typename T>
void Shader<T>::reset(GLfloat *d, int e){
	data = d;
	elements = e;
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	buffers();
}

template <typename T>
void Shader<T>::buffers(){
	static_cast<T*>(this)->buffers();
}

template <typename T>
void Shader<T>::sources(const char **v, const char **f){
	static_cast<T*>(this)->sources(v, f);
}

template <typename T>
void Shader<T>::display(){
	static_cast<T*>(this)->display();
}

#endif