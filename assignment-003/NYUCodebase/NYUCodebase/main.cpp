#include <cstdlib>
#include <forward_list>
#include <iostream>
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
#define START_SCREEN_TOP PIXEL_FROM_TOP_TO_ORTHO(100)
#define START_SCREEN_BOTTOM PIXEL_FROM_BOTTOM_TO_ORTHO(370)
#define START_SCREEN_LEFT_RIGHT PIXEL_FROM_RIGHT_TO_ORTHO(40)
#define START_SCREEN_BUTTON_OUTER (164.0f / 260.25f * START_SCREEN_LEFT_RIGHT)
#define START_SCREEN_BUTTON_INNER (66.0f / 260.25f * START_SCREEN_LEFT_RIGHT)
#define CHARACTERS_WIDTH 40.0f // pixels
#define CHARACTERS_HEIGHT 80.0f // pixels
#define CHARACTERS_BASELINE 58.89f // pixels from top of line
#define CHARACTERS_SHEET_WIDTH 640.0f // pixels
#define CHARACTERS_SHEET_HEIGHT 1120.0f // pixels
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

bool DrawText(ShaderProgram& program, GLuint charactersT, const std::string& text, float baselineStartPixelX, float baselineStartPixelY, float scale = 1.0f) {
	// Dynamically allocate arrays of floats in memory for the vertices and texture coordinates.
	size_t s = 12 * sizeof(float) * text.size();
	float* vertices = (float*)malloc(s);
	float* texCoords = (float*)malloc(s);
	if (!(vertices && texCoords)) {
		return false;
	}
	// Add each character to the arrays.
	float top = PIXEL_FROM_TOP_TO_ORTHO((baselineStartPixelY - CHARACTERS_BASELINE * scale));
	for (size_t i = 0; i < text.size(); ++i) {
		// Set the vertices.
		float left = PIXEL_FROM_LEFT_TO_ORTHO((baselineStartPixelX + CHARACTERS_WIDTH * i * scale));
		Entity::SetBox(vertices + i * 12, top, left + PIXEL_TO_ORTHO_X((CHARACTERS_WIDTH * scale)), top - PIXEL_TO_ORTHO_Y((CHARACTERS_HEIGHT * scale)), left);
		// Set the texture coordinates.
		div_t gridPosition = div(text[i], 16);
		Entity::UVWrap uv(
			CHARACTERS_WIDTH / CHARACTERS_SHEET_WIDTH * gridPosition.rem,
			CHARACTERS_HEIGHT / CHARACTERS_SHEET_HEIGHT * (gridPosition.quot - 2),
			CHARACTERS_WIDTH / CHARACTERS_SHEET_WIDTH,
			CHARACTERS_HEIGHT / CHARACTERS_SHEET_HEIGHT
		);
		uv.GetTextureCoordinates(texCoords + i * 12);
	}
	// Draw the arrays.
	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program.positionAttribute);
	glEnableVertexAttribArray(program.texCoordAttribute);
	glBindTexture(GL_TEXTURE_2D, charactersT);
	glDrawArrays(GL_TRIANGLES, 0, 6 * text.size());
	glDisableVertexAttribArray(program.positionAttribute);
	glDisableVertexAttribArray(program.texCoordAttribute);
	// Free the allocated arrays.
	free(vertices);
	free(texCoords);
	return true;
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
		PlayerLaserCannon player(spriteSheet, 0.0f, -3.2f);
		std::forward_list<Bullet> bullets;
		Lives lives(
			spriteSheet,
			player.GetWidth(),
			4,
			PIXEL_FROM_LEFT_TO_ORTHO((20.0f + CHARACTERS_WIDTH * 4.0f)),
			PIXEL_FROM_TOP_TO_ORTHO((HEIGHT - 20.555f - CHARACTERS_BASELINE * 0.5f)) - PIXEL_TO_ORTHO_Y((CHARACTERS_BASELINE * 0.5f)) + player.GetHeight() * 0.4797297418f
		);
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
					}
				}
				// Draw the bullet.
				b.Draw(program);
			}
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
			SDL_GL_SwapWindow(displayWindow);
		}
	}

	SDL_Quit();
	return 0;
}
