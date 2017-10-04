#include <iostream>
#include <list>
#ifdef _WINDOWS
	#define RESOURCE_FOLDER ""
	#include <GL/glew.h>
#else
	#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "ShaderProgram.h"
#include "GameEntity.h"
#include "Dimensions.h"

// From assignment 1
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

bool ProcessInput(GLuint spriteSheet, PlayerLaserCannon& player, std::list<Bullet>& bullets) {
	// Scan for input events.
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_QUIT:
		case SDL_WINDOWEVENT_CLOSE:
			return true;
		case SDL_KEYDOWN:
			switch (event.key.keysym.scancode) {
			case SDL_SCANCODE_SPACE:
				// The spacebar was pressed.
				bullets.emplace_front(spriteSheet, true, (player.GetLeftBoxBound() + player.GetRightBoxBound()) / 2.0f, -2.8f);
				break;
			}
			break;
		}
	}
	// Poll for pressed keys.
	const Uint8* keys = SDL_GetKeyboardState(NULL);
	if (keys[SDL_SCANCODE_LEFT]) {
		player.CurrentMovement = PlayerLaserCannon::MOVEMENTS::LEFT;
	}
	else if (keys[SDL_SCANCODE_RIGHT]) {
		player.CurrentMovement = PlayerLaserCannon::MOVEMENTS::RIGHT;
	}
	else {
		player.CurrentMovement = PlayerLaserCannon::MOVEMENTS::STATIONARY;
	}
	return false;
}

SDL_Window* displayWindow;

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Space Invaders", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif

	// Viewport setup
	glViewport(0, 0, WIDTH, HEIGHT);
	Matrix projectionMatrix, modelviewMatrix;
	projectionMatrix.SetOrthoProjection(-ORTHO_X_BOUND, ORTHO_X_BOUND, -ORTHO_Y_BOUND, ORTHO_Y_BOUND, -1.0f, 1.0f);
	ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
	glUseProgram(program.programID);
	program.SetProjectionMatrix(projectionMatrix);
	program.SetModelviewMatrix(modelviewMatrix);

	// Load textures
	GLuint spriteSheet = LoadTexture(RESOURCE_FOLDER"Images/sprites.png");

	// Create the player's cannon
	PlayerLaserCannon player(spriteSheet);
	// It starts in the middle of the screen. Move it down.
	player.MoveY(-3.2f);

	// Use linked lists to store active sprites.
	std::list<Bullet> bullets;

	// Main game loops
	Uint32 lastFrameTick = 0;
	bool done = false;
	while (!done) {
		// Loop for the start screen
		while (!done) {
			Uint32 thisFrameTick = SDL_GetTicks(), millisecondsElapsed = thisFrameTick - lastFrameTick;
			lastFrameTick = thisFrameTick;
			done = ProcessInput(spriteSheet, player, bullets);
			glClear(GL_COLOR_BUFFER_BIT);
			player.CalculateMotion(millisecondsElapsed);
			player.Draw(program);
			for (auto i = bullets.begin(); i != bullets.end();) {
				i->CalculateMotion(millisecondsElapsed);
				if (i->IsOffScreen()) {
					i = bullets.erase(i);
				}
				else {
					i->Draw(program);
					i++;
				}
			}
			SDL_GL_SwapWindow(displayWindow);
		}
		// Loop for gameplay
		// TODO
	}

	SDL_Quit();
	return 0;
}
