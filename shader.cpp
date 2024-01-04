#include "shader.hpp"

// point shader

void PointShader::buffers(){
	glGenBuffers(1, &vbo);
	glGenVertexArrays(1, &vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * elements * 4, data, GL_STATIC_DRAW);
	glBindVertexArray(vao);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 3, NULL);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat), (GLvoid*)(sizeof(GLfloat) * elements * 3));
	glEnableVertexAttribArray(1);
}

void PointShader::sources(const char **v, const char **f){
	const char *vShaderSrc = 
		"#version 330 core\n"
		"layout (location = 0) in vec3 pos;"
		"layout (location = 1) in float value;"
		"uniform mat4 model;"
		"uniform mat4 view;"
		"uniform mat4 projection;"
		"out vec3 vert_colour;"
		"void main(){"
			"if(value > 0.2) vert_colour = vec3(1);"
			"else vert_colour = vec3(0);"
			"gl_Position = projection * view * model * vec4(pos, 1);"
		"}";
	const char *fShaderSrc = 
		"#version 330 core\n"
		"in vec3 vert_colour;"
		"out vec4 frag_colour;"
		"void main(){"
			"frag_colour = vec4(vert_colour, 1);"
		"}";
	*v = vShaderSrc;
	*f = fShaderSrc;
}

void PointShader::display(){
	glUseProgram(program);
	glBindVertexArray(vao);
	glDrawArrays(GL_POINTS, 0, elements);
	glBindVertexArray(0);
	glUseProgram(0);
}

// triangle shader

void TriangleShader::buffers(){
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * elements * 6, data, GL_STATIC_DRAW);
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 6, NULL);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 6, (GLvoid*)(sizeof(GLfloat) * 3));
	glEnableVertexAttribArray(1);
}

void TriangleShader::sources(const char **v, const char **f){
	const char *vShaderSrc = 
		"#version 330 core\n"
		"layout (location = 0) in vec3 pos;"
		"layout (location = 1) in vec3 normal;"
		"uniform mat4 model;"
		"uniform mat4 view;"
		"uniform mat4 projection;"
		"out vec3 vert_normal;"
		"out vec3 vert_pos;"
		"void main(){"
			"vert_normal = normal;"
			"vert_pos = vec3(model * vec4(pos, 1));"
			"gl_Position = projection * view * model * vec4(pos, 1);"
		"}";
	const char *fShaderSrc = 
		"#version 330 core\n"
		"in vec3 vert_normal;"
		"in vec3 vert_pos;"
		"uniform mat4 model;"
		"uniform mat4 view;"
		"uniform mat4 projection;"
		"uniform vec3 viewPos;"
		"out vec4 frag_colour;"
		"void main(){"
			
			"mat4 mvp = projection * view * model;"
			
			"vec3 viewDir = normalize(vert_pos - viewPos);"
			"vec3 lightDir = viewDir;"
			"vec3 reflectDir = reflect(-lightDir, vert_normal);"
			
			"float ambient = 0.2f;"
			"float diffuse = max(dot(vert_normal, lightDir), 0.0f);"
			"float specular = 0.5f * pow(max(dot(viewDir, reflectDir), 0.0f), 32.0f);"
			"vec3 colour = vec3(0.7, 0.7, 0) * (ambient + diffuse + specular);"
			
			"frag_colour = vec4(colour, 1);"
		"}";
	*v = vShaderSrc;
	*f = fShaderSrc;
}

void TriangleShader::display(){
	glUseProgram(program);
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, elements);
	glBindVertexArray(0);
	glUseProgram(0);
}