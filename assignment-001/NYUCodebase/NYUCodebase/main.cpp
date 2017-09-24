#ifdef _WINDOWS
	#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>

#ifdef _WINDOWS
	#define RESOURCE_FOLDER ""
#else
	#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "ShaderProgram.h"

SDL_Window* displayWindow;

// From slide 76
GLuint LoadTexture(const char *filePath) {
	int w, h, comp;
	unsigned char* image = stbi_load(filePath, &w, &h, &comp, STBI_rgb_alpha);
	if (image == NULL) {
		std::cout << "Unable to load image: " << filePath << std::endl;
		exit(1);
	}
	GLuint result;
	glGenTextures(1, &result);
	glBindTexture(GL_TEXTURE_2D, result);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	stbi_image_free(image);
	return result;
}

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
	#ifdef _WINDOWS
		glewInit();
	#endif

	// Viewport setup
	glViewport(0, 0, 640, 360);
	glClearColor(0.4f, 0.2f, 0.4f, 1.0f);
	Matrix projectionMatrix;
	projectionMatrix.SetOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
	Matrix modelviewMatrix;

	// Load textures
	ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
	GLuint happyTexture = LoadTexture(RESOURCE_FOLDER"Images/happy.gif");
	GLuint laughTexture = LoadTexture(RESOURCE_FOLDER"Images/laugh.gif");
	GLuint pleasedTexture = LoadTexture(RESOURCE_FOLDER"Images/pleased.gif");

	SDL_Event event;
	bool done = false;
	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
		}
		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(program.programID);
		program.SetProjectionMatrix(projectionMatrix);
		program.SetModelviewMatrix(modelviewMatrix);
		modelviewMatrix.Identity();
		modelviewMatrix.Rotate(0.1f);

		// Set up vertices and texture coordinates
		float texCoords[] = {
			0.0, 1.0, 1.0, 1.0, 1.0, 0.0,
			0.0, 1.0, 1.0, 0.0, 0.0, 0.0
		};
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);

		// Draw happy face in the middle
		float happyVertices[] = {
			-0.5, -0.5, 0.5, -0.5,  0.5, 0.5,
			-0.5, -0.5, 0.5,  0.5, -0.5, 0.5
		};
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, happyVertices);
		glEnableVertexAttribArray(program.positionAttribute);
		glBindTexture(GL_TEXTURE_2D, happyTexture);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program.positionAttribute);

		// Draw laugh face
		float laughVertices[] = {
			-2.0, -0.5, -1.0, -0.5, -1.0, 0.5,
			-2.0, -0.5, -1.0,  0.5, -2.0, 0.5
		};
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, laughVertices);
		glEnableVertexAttribArray(program.positionAttribute);
		glBindTexture(GL_TEXTURE_2D, laughTexture);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program.positionAttribute);

		// Draw laugh face
		float pleasedVertices[] = {
			1.0, -0.5, 2.0, -0.5, 2.0, 0.5,
			1.0, -0.5, 2.0,  0.5, 1.0, 0.5
		};
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, pleasedVertices);
		glEnableVertexAttribArray(program.positionAttribute);
		glBindTexture(GL_TEXTURE_2D, pleasedTexture);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program.positionAttribute);

		glDisableVertexAttribArray(program.texCoordAttribute);
		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}
