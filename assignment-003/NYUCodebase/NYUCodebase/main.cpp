#include <iostream>
#include <forward_list>
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
#define START_SCREEN_TOP PIXEL_FROM_TOP_TO_ORTHO(100)
#define START_SCREEN_BOTTOM PIXEL_FROM_BOTTOM_TO_ORTHO(370)
#define START_SCREEN_LEFT_RIGHT PIXEL_FROM_RIGHT_TO_ORTHO(40)
#define START_SCREEN_BUTTON_OUTER (164.0f / 260.25f * START_SCREEN_LEFT_RIGHT)
#define START_SCREEN_BUTTON_INNER (66.0f / 260.25f * START_SCREEN_LEFT_RIGHT)
enum GameMode {
	GAME_MODE_QUIT,
	GAME_MODE_START,
	GAME_MODE_PLAY,
	NUM_GAME_MODES
};

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

bool ProcessInput(GLuint spriteSheet, PlayerLaserCannon& player, std::forward_list<Bullet>& bullets) {
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
		player.CurrentMovement = PlayerLaserCannon::Movements::LEFT;
	}
	else if (keys[SDL_SCANCODE_RIGHT]) {
		player.CurrentMovement = PlayerLaserCannon::Movements::RIGHT;
	}
	else {
		player.CurrentMovement = PlayerLaserCannon::Movements::STATIONARY;
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
	GLuint startScreen = LoadTexture(RESOURCE_FOLDER"Images/start.png");

	// Start screen coordinates
	// 40 pixels from left of screen
	// 100 pixels from top of screen
	// 40 pixels from right of screen
	// 370 pixels from bottom of screen
	float startScreenVertices[] = {
		-START_SCREEN_LEFT_RIGHT, START_SCREEN_TOP,
		-START_SCREEN_LEFT_RIGHT, START_SCREEN_BOTTOM,
		START_SCREEN_LEFT_RIGHT,  START_SCREEN_TOP,
		START_SCREEN_LEFT_RIGHT,  START_SCREEN_TOP,
		-START_SCREEN_LEFT_RIGHT, START_SCREEN_BOTTOM,
		START_SCREEN_LEFT_RIGHT,  START_SCREEN_BOTTOM
	};
	float startScreenTexCoords[] = {
		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 0.0f,
		1.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f
	};

	// Create the player's cannon
	PlayerLaserCannon player(spriteSheet);
	// It starts in the middle of the screen. Move it down.
	player.MoveY(-3.2f);

	// Use linked lists to store active sprites.
	std::forward_list<Bullet> bullets;

	// Main game loops
	Uint32 lastFrameTick = 0;
	GameMode mode = GAME_MODE_START;
	while (mode != GAME_MODE_QUIT && mode < NUM_GAME_MODES) {
		// Loop for the start screen
		while (mode == GAME_MODE_START) {
			Uint32 thisFrameTick = SDL_GetTicks(), millisecondsElapsed = thisFrameTick - lastFrameTick;
			lastFrameTick = thisFrameTick;
			if (ProcessInput(spriteSheet, player, bullets)) {
				mode = GAME_MODE_QUIT;
				break;
			}
			glClear(GL_COLOR_BUFFER_BIT);
			// Draw start screen
			glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, startScreenVertices);
			glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, startScreenTexCoords);
			glEnableVertexAttribArray(program.positionAttribute);
			glEnableVertexAttribArray(program.texCoordAttribute);
			glBindTexture(GL_TEXTURE_2D, startScreen);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glDisableVertexAttribArray(program.positionAttribute);
			glDisableVertexAttribArray(program.texCoordAttribute);
			// Draw player
			player.CalculateMotion(millisecondsElapsed);
			player.Draw(program);
			// Delete bullets that have gone off the screen.
			bullets.remove_if([](const Bullet& b) { return b.IsOffScreen(); });
			// Draw bullets
			for (auto i = bullets.begin(); i != bullets.end(); i++) {
				i->CalculateMotion(millisecondsElapsed);
				// Check whether the bullet has hit one of the buttons.
				if (i->GetTopBoxBound() > START_SCREEN_BOTTOM) {
					if (i->GetLeftBoxBound() <= START_SCREEN_BUTTON_OUTER && i->GetRightBoxBound() >= START_SCREEN_BUTTON_INNER) {
						// The bullet hit the Quit button.
						mode = GAME_MODE_QUIT;
					}
					else if (i->GetLeftBoxBound() <= -START_SCREEN_BUTTON_INNER && i->GetRightBoxBound() >= -START_SCREEN_BUTTON_OUTER) {
						// The bullet hit the Start button.
						mode = GAME_MODE_PLAY;
					}
				}
				// Draw the bullet.
				i->Draw(program);
			}
			SDL_GL_SwapWindow(displayWindow);
		}
		// Loop for gameplay
		// TODO
		while (mode == GAME_MODE_PLAY) {
			// For now, just exit.
			mode = GAME_MODE_QUIT;
		}
	}

	SDL_Quit();
	return 0;
}
