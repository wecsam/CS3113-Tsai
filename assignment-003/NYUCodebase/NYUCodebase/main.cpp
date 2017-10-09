#include <cstdlib>
#include <forward_list>
#include <iostream>
#include <list>
#include <string>
#include <vector>
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
#include "Lives.h"
#include "Draw.h"
#define START_SCREEN_TOP PIXEL_FROM_TOP_TO_ORTHO(100)
#define START_SCREEN_BOTTOM PIXEL_FROM_BOTTOM_TO_ORTHO(370)
#define START_SCREEN_LEFT_RIGHT PIXEL_FROM_RIGHT_TO_ORTHO(40)
#define START_SCREEN_BUTTON_OUTER (164.0f / 260.25f * START_SCREEN_LEFT_RIGHT)
#define START_SCREEN_BUTTON_INNER (66.0f / 260.25f * START_SCREEN_LEFT_RIGHT)
#define REMOVE_OFFSCREEN_BULLETS(bullets) bullets.remove_if([](const Bullet& b) { return b.IsOffScreen(); })
enum GameMode {
	GAME_MODE_QUIT,
	GAME_MODE_START,
	GAME_MODE_PLAY,
	NUM_GAME_MODES
};
const std::vector<Invader::InvaderType> DESIRED_INVADERS = {
	Invader::InvaderType::BACK,
	Invader::InvaderType::MIDDLE,
	Invader::InvaderType::MIDDLE,
	Invader::InvaderType::FRONT,
	Invader::InvaderType::FRONT
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

Uint32 MillisecondsElapsed() {
	static Uint32 lastFrameTick = 0;
	Uint32 thisFrameTick = SDL_GetTicks(), result = thisFrameTick - lastFrameTick;
	lastFrameTick = thisFrameTick;
	return result;
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
	GLuint charactersT = LoadTexture(RESOURCE_FOLDER"Images/characters.png");

	// Start screen coordinates
	// 40 pixels from left of screen
	// 100 pixels from top of screen
	// 40 pixels from right of screen
	// 370 pixels from bottom of screen
	float startScreenVertices[12], startScreenTexCoords[12];
	Entity::SetBox(startScreenVertices, START_SCREEN_TOP, START_SCREEN_LEFT_RIGHT, START_SCREEN_BOTTOM, -START_SCREEN_LEFT_RIGHT);
	Entity::SetBox(startScreenTexCoords, 0.0f, 1.0f, 1.0f, 0.0f);

	// Main game loops
	GameMode mode = GAME_MODE_START;
	while (mode != GAME_MODE_QUIT && mode < NUM_GAME_MODES) {
		// The player controls this cannon. Bullets shoot from this cannon.
		PlayerLaserCannon player(spriteSheet, 0.0f, -3.2f);
		// We only need to be able to add and remove bullets efficiently.
		std::forward_list<Bullet> bullets;
		// The invaders come in a grid. The outer list represents the columns.
		bool invadersGoingRight = true;
		std::list<std::vector<Invader>> invaders;
		for (int column = 0; column < 10; ++column) {
			// Make a new column at the right.
			invaders.emplace_back();
			// Fill it with invaders of this type.
			register float x = column * 0.5f - ORTHO_X_BOUND + 0.25f;
			for (size_t row = 0; row < DESIRED_INVADERS.size(); ++row) {
				invaders.back().emplace_back(
					spriteSheet,
					DESIRED_INVADERS[row],
					x,
					ORTHO_Y_BOUND - PIXEL_TO_ORTHO_Y(CHARACTERS_HEIGHT) - row * 0.5f
				);
			}
		}
		// In the lower-left corner, one icon will show for each life that is left.
		Lives lives(
			spriteSheet,
			player.GetWidth(),
			4,
			PIXEL_FROM_LEFT_TO_ORTHO((20.0f + CHARACTERS_WIDTH * 4.0f)),
			PIXEL_FROM_TOP_TO_ORTHO((HEIGHT - 20.555f - CHARACTERS_BASELINE * 0.5f)) - PIXEL_TO_ORTHO_Y((CHARACTERS_BASELINE * 0.5f)) + player.GetHeight() * 0.4797297418f
		);
		// The player's score can be kept track of with an int.
		unsigned int score = 0;
		// Loop for the start screen
		while (mode == GAME_MODE_START) {
			Uint32 millisecondsElapsed = MillisecondsElapsed();
			if (ProcessInput(spriteSheet, player, bullets)) {
				mode = GAME_MODE_QUIT;
				break;
			}
			glClear(GL_COLOR_BUFFER_BIT);
			// Draw start screen
			DrawTrianglesWithTexture(program, 2, startScreenVertices, startScreenTexCoords, startScreen);
			// Draw player
			player.CalculateMotion(millisecondsElapsed);
			player.Draw(program);
			// Draw bullets
			for (Bullet& b : bullets) {
				b.CalculateMotion(millisecondsElapsed);
				// Check whether the bullet has hit one of the buttons.
				if (b.GetTopBoxBound() > START_SCREEN_BOTTOM) {
					if (b.GetLeftBoxBound() <= START_SCREEN_BUTTON_OUTER && b.GetRightBoxBound() >= START_SCREEN_BUTTON_INNER) {
						// The bullet hit the Quit button.
						mode = GAME_MODE_QUIT;
					}
					else if (b.GetLeftBoxBound() <= -START_SCREEN_BUTTON_INNER && b.GetRightBoxBound() >= -START_SCREEN_BUTTON_OUTER) {
						// The bullet hit the Start button.
						mode = GAME_MODE_PLAY;
						b.MoveOffScreen();
					}
				}
				// Draw the bullet.
				b.Draw(program);
			}
			REMOVE_OFFSCREEN_BULLETS(bullets);
			SDL_GL_SwapWindow(displayWindow);
		}
		// Loop for gameplay
		while (mode == GAME_MODE_PLAY) {
			Uint32 millisecondsElapsed = MillisecondsElapsed();
			if (ProcessInput(spriteSheet, player, bullets)) {
				mode = GAME_MODE_QUIT;
				break;
			}
			glClear(GL_COLOR_BUFFER_BIT);
			// Draw player's current score
			DrawText(program, charactersT, "Score: " + std::to_string(score), 20.0f, 49.445f, 0.5f);
			// Draw player's number of lives
			DrawText(program, charactersT, "Lives:", 20.0f, HEIGHT - 20.555f, 0.5f);
			lives.Draw(program);
			// Draw player
			player.CalculateMotion(millisecondsElapsed);
			player.Draw(program);
			// If there are no more invaders, the game is over.
			invaders.remove_if([](const auto& c) { return c.empty(); });
			if (invaders.empty()) {
				mode = GAME_MODE_START;
				break;
			}
			// If the invaders have hit a side of the screen, reverse their direction.
			if (invadersGoingRight) {
				if (invaders.back().front().GetRightBoxBound() > ORTHO_X_BOUND) {
					invadersGoingRight = false;
				}
			}
			else {
				if (invaders.front().front().GetLeftBoxBound() < -ORTHO_X_BOUND) {
					invadersGoingRight = true;
				}
			}
			// Check for bullets hitting invaders and invaders reaching the player.
			for (auto& c : invaders) {
				// If an invader makes it past the player, the game is over.
				if (c.back().GetTopBoxBound() < player.GetBottomBoxBound()) {
					mode = GAME_MODE_START;
				}
				// We only need to check for the invader at the bottom of the list
				// because a bullet from the player cannot hit any invaders behind
				// that invader.
				for (Bullet& b : bullets) {
					if (
						b.GetTopBoxBound() > c.back().GetBottomBoxBound() &&
						b.GetLeftBoxBound() <= c.back().GetRightBoxBound() &&
						b.GetRightBoxBound() >= c.back().GetLeftBoxBound()
						) {
						// This bullet hit this invader.
						score += c.back().GetPointValue();
						b.MoveOffScreen();
						c.pop_back();
					}
				}
			}
			// Draw invaders
			{
				register float invaderXDelta = (invadersGoingRight ? 0.005f : -0.005f) / invaders.size() / invaders.size() * millisecondsElapsed;
				register float invaderYDelta = -0.000005 * millisecondsElapsed;
				for (auto& c : invaders) {
					for (Invader& a : c) {
						a.MoveX(invaderXDelta);
						a.MoveY(invaderYDelta);
						a.Draw(program);
					}
				}
			}
			// Draw bullets
			REMOVE_OFFSCREEN_BULLETS(bullets);
			for (Bullet& b : bullets) {
				b.CalculateMotion(millisecondsElapsed);
				b.Draw(program);
			}
			SDL_GL_SwapWindow(displayWindow);
		}
	}

	SDL_Quit();
	return 0;
}
