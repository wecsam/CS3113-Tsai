#include <chrono>
#include <cstdlib>
#include <forward_list>
#include <iostream>
#include <list>
#include <string>
#include <thread>
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
#include "Input.h"
#include "Animation.h"
#define START_SCREEN_TOP PIXEL_FROM_TOP_TO_ORTHO(100)
#define START_SCREEN_BOTTOM PIXEL_FROM_BOTTOM_TO_ORTHO(370)
#define START_SCREEN_LEFT_RIGHT PIXEL_FROM_RIGHT_TO_ORTHO(39.5f)
#define START_SCREEN_BUTTON_OUTER (164.0f / 260.25f * START_SCREEN_LEFT_RIGHT)
#define START_SCREEN_BUTTON_INNER (66.0f / 260.25f * START_SCREEN_LEFT_RIGHT)
#define START_SCREEN_ANIMATION_MS 500
#define REMOVE_OFFSCREEN_BULLETS(bullets) bullets.remove_if([](const Bullet& b) { return b.IsOffScreen(); })
#define RETALIATE_EVERY_BULLETS 2
#define MAX_RETALIATION_DISTANCE 0.5f
#define MIN_RETALIATION_WAIT 100 // milliseconds
enum GameMode {
	GAME_MODE_QUIT,
	GAME_MODE_START,
	GAME_MODE_PLAY,
	GAME_MODE_DEAD,
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

Uint32 MillisecondsElapsed() {
	static Uint32 lastFrameTick = 0;
	Uint32 thisFrameTick, delta;
	while (true) {
		// Calculate the number of milliseconds that have elapsed since the last call to this function.
		thisFrameTick = SDL_GetTicks();
		delta = thisFrameTick - lastFrameTick;
		// At 60 FPS, there should be 16.67 milliseconds between frames.
		// If it has been shorter than 16 milliseconds, sleep.
		if (delta < 16) {
			std::this_thread::sleep_for(std::chrono::milliseconds(16 - delta));
		}
		else {
			lastFrameTick = thisFrameTick;
			return delta;
		}
	}
}

SDL_Window* displayWindow;
AnimationList animations;

float square(float x) {
	return x * x;
}
void AnimateY(GameMode& mode, float* vertices, float* texCoords, GLuint textureID, Uint32 animationDurationMilliseconds, float(*position)(float)) {
	Matrix modelviewMatrix;
	Uint32 startTicks = SDL_GetTicks(), delta = 0;
	do {
		// Position the start screen.
		modelviewMatrix.Identity();
		modelviewMatrix.Translate(0.0f, position((float)delta / animationDurationMilliseconds), 0.0f);
		// Process input.
		Input input;
		if (input.QuitRequested) {
			// Quit the game.
			mode = GAME_MODE_QUIT;
			break;
		}
		if (input.BulletsToFire || input.EscapePressed) {
			// Jump to the end of this animation.
			break;
		}
		// Draw the start screen.
		glClear(GL_COLOR_BUFFER_BIT);
		DrawTrianglesWithTexture(modelviewMatrix, 2, vertices, texCoords, textureID);
		animations.DrawAll();
		SDL_GL_SwapWindow(displayWindow);
		// Pause if this loop is running more quickly than the desired frame rate.
		MillisecondsElapsed();
		// Get the time that has passed since the last loop.
		delta = SDL_GetTicks() - startTicks;
	} while (delta < animationDurationMilliseconds);
}

int main(int argc, char *argv[])
{
	bool immortal = (argc == 2 && strcmp(argv[1], "immortal") == 0);
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Space Invaders", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif

	// Viewport setup
	glViewport(0, 0, WIDTH, HEIGHT);
	Matrix projectionMatrix;
	projectionMatrix.SetOrthoProjection(-ORTHO_X_BOUND, ORTHO_X_BOUND, -ORTHO_Y_BOUND, ORTHO_Y_BOUND, -1.0f, 1.0f);
	program = new ShaderProgram(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
	program->SetProjectionMatrix(projectionMatrix);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Load textures
	spriteSheet = LoadTexture(RESOURCE_FOLDER"Images/sprites.png");
	GLuint startScreen = LoadTexture(RESOURCE_FOLDER"Images/start.png");
	fontGrid = LoadTexture(RESOURCE_FOLDER"Images/characters.png");
	anExplosion = LoadTexture(RESOURCE_FOLDER"Images/explosion.png");

	// Start screen coordinates
	// 40 pixels from left of screen
	// 100 pixels from top of screen
	// 40 pixels from right of screen
	// 370 pixels from bottom of screen
	float startScreenVertices[12], startScreenTexCoords[12];
	Rectangle::SetBox(startScreenVertices, START_SCREEN_TOP, START_SCREEN_LEFT_RIGHT, START_SCREEN_BOTTOM, -START_SCREEN_LEFT_RIGHT);
	Rectangle::SetBox(startScreenTexCoords, 0.0f, 1.0f, 1.0f, 0.0f);

	// Main game loops
	GameMode mode = GAME_MODE_START;
	while (mode != GAME_MODE_QUIT && mode < NUM_GAME_MODES) {
		// The player controls this cannon. Bullets shoot from this cannon.
		PlayerLaserCannon player(0.0f, -3.2f);
		// We only need to be able to add and remove bullets efficiently.
		std::forward_list<Bullet> bullets;
		unsigned int bulletsFired = 0, lastRetaliation = 0;
		Uint32 lastRetaliationTicks = 0;
		// The invaders come in a grid. The outer list represents the columns.
		bool invadersGoingRight, invadersEntering;
		int level = 0;
		std::list<std::vector<Invader>> invaders;
		// In the lower-left corner, one icon will show for each life that is left.
		Lives lives(
			player.GetWidth(),
			4,
			PIXEL_FROM_LEFT_TO_ORTHO((20.0f + CHARACTERS_WIDTH * 4.0f)),
			PIXEL_FROM_TOP_TO_ORTHO((HEIGHT - 20.555f - CHARACTERS_BASELINE * 0.5f)) - PIXEL_TO_ORTHO_Y((CHARACTERS_BASELINE * 0.5f)) + player.GetHeight() * 0.4797297418f
		);
		// The player's score can be kept track of with an int.
		unsigned int score = 0;
		// Slide the start screen in from the top.
		AnimateY(
			mode,
			startScreenVertices,
			startScreenTexCoords,
			startScreen,
			START_SCREEN_ANIMATION_MS,
			[](float progress) { return (ORTHO_Y_BOUND - START_SCREEN_BOTTOM) * square(progress - 1); }
		);
		// Loop for the start screen
		while (mode == GAME_MODE_START) {
			Uint32 millisecondsElapsed = MillisecondsElapsed();
			Input input;
			if (input.QuitRequested) {
				mode = GAME_MODE_QUIT;
				break;
			}
			input.Process(player, bullets);
			glClear(GL_COLOR_BUFFER_BIT);
			// Draw start screen
			DrawTrianglesWithTexture(IDENTITY_MATRIX, 2, startScreenVertices, startScreenTexCoords, startScreen);
			// Draw player
			player.CalculateMotion(millisecondsElapsed);
			player.Draw();
			// Draw bullets
			for (Bullet& b : bullets) {
				b.CalculateMotion(millisecondsElapsed);
				// Check whether the bullet has hit one of the buttons.
				if (b.GetTopBoxBound() > START_SCREEN_BOTTOM) {
					if (b.GetLeftBoxBound() <= START_SCREEN_BUTTON_OUTER && b.GetRightBoxBound() >= START_SCREEN_BUTTON_INNER) {
						// The bullet hit the Quit button.
						animations.Add(ExplosionAnimation(b.GetCenterX(), b.GetTopBoxBound()));
						mode = GAME_MODE_QUIT;
					}
					else if (b.GetLeftBoxBound() <= -START_SCREEN_BUTTON_INNER && b.GetRightBoxBound() >= -START_SCREEN_BUTTON_OUTER) {
						// The bullet hit the Start button.
						animations.Add(ExplosionAnimation(b.GetCenterX(), b.GetTopBoxBound()));
						mode = GAME_MODE_PLAY;
						b.MoveOffScreen();
					}
				}
				// Draw the bullet.
				b.Draw();
			}
			REMOVE_OFFSCREEN_BULLETS(bullets);
			SDL_GL_SwapWindow(displayWindow);
		}
		// Slide the start screen out toward the top.
		AnimateY(
			mode,
			startScreenVertices,
			startScreenTexCoords,
			startScreen,
			START_SCREEN_ANIMATION_MS,
			[](float progress) { return (ORTHO_Y_BOUND - START_SCREEN_BOTTOM) * square(progress); }
		);
		// Loop for gameplay
		while (mode == GAME_MODE_PLAY) {
			Uint32 millisecondsElapsed = MillisecondsElapsed();
			Input input;
			bulletsFired += input.BulletsToFire;
			if (input.QuitRequested) {
				mode = GAME_MODE_QUIT;
				break;
			}
			if (input.EscapePressed) {
				mode = GAME_MODE_START;
				break;
			}
			input.Process(player, bullets);
			glClear(GL_COLOR_BUFFER_BIT);
			// Draw player's current score
			DrawText("Score: " + std::to_string(score), 20.0f, 49.445f, 0.5f);
			if (!immortal) {
				// Draw player's number of lives
				DrawText("Lives:", 20.0f, HEIGHT - 20.555f, 0.5f);
				lives.Draw();
				// Check for bullets hitting the player.
				for (Bullet& b : bullets) {
					if (b.GetBottomBoxBound() < player.GetTopBoxBound() &&
						b.GetLeftBoxBound() <= player.GetRightBoxBound() &&
						b.GetRightBoxBound() >= player.GetLeftBoxBound()) {
						// The player was hit!
						// Add an explosion animation.
						animations.Add(ExplosionAnimation(b.GetCenterX(), b.GetBottomBoxBound()));
						// Dispose of the bullet.
						b.MoveOffScreen();
						// Deduct a life from the player.
						lives.RemoveLife();
						// Once the player loses all lives, the game is over.
						if (!lives.NumberLeft()) {
							mode = GAME_MODE_DEAD;
						}
						// Only allow the player to get hit once per frame.
						break;
					}
				}
			}
			REMOVE_OFFSCREEN_BULLETS(bullets);
			// Draw player
			player.CalculateMotion(millisecondsElapsed);
			player.Draw();
			// If there are no more invaders, make a new fleet and animate it in.
			invaders.remove_if([](const auto& c) { return c.empty(); });
			if (invaders.empty()) {
				++level;
				invadersGoingRight = false;
				invadersEntering = true;
				for (int column = 0; column < 10; ++column) {
					// Make a new column at the right.
					invaders.emplace_back();
					// Fill it with invaders of this type.
					register float x = ORTHO_X_BOUND + 0.25f + column * 0.5f;
					for (size_t row = 0; row < DESIRED_INVADERS.size(); ++row) {
						invaders.back().emplace_back(
							DESIRED_INVADERS[row],
							x,
							ORTHO_Y_BOUND - PIXEL_TO_ORTHO_Y(CHARACTERS_HEIGHT) - row * 0.5f
						);
					}
				}
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
					invadersEntering = false;
				}
			}
			// An invader may shoot back at the player.
			if (bulletsFired - lastRetaliation > RETALIATE_EVERY_BULLETS && SDL_GetTicks() - lastRetaliationTicks > MIN_RETALIATION_WAIT) {
				// Find the column whose invaders has the closest X position to the player.
				register float minDistance = FLT_MAX;
				const Invader* closestInvader = NULL;
				for (const auto& c : invaders) {
					if (!c.empty()) {
						register float curDistance = fabs((c.back().GetLeftBoxBound() + c.back().GetRightBoxBound()) / 2.0f - (player.GetLeftBoxBound() + player.GetRightBoxBound()) / 2.0f);
						if (curDistance < minDistance) {
							minDistance = curDistance;
							closestInvader = &c.back();
						}
					}
				}
				// Only retaliate if the player is within range.
				if (minDistance <= MAX_RETALIATION_DISTANCE) {
					bullets.emplace_front(false, (closestInvader->GetLeftBoxBound() + closestInvader->GetRightBoxBound()) / 2.0f, closestInvader->GetBottomBoxBound() - 0.2f);
					lastRetaliation += RETALIATE_EVERY_BULLETS;
					lastRetaliationTicks = SDL_GetTicks();
				}
			}
			// Check for bullets hitting invaders and invaders reaching the player.
			for (auto& c : invaders) {
				// If an invader makes it past the player, the game is over.
				if (c.back().GetTopBoxBound() < player.GetBottomBoxBound()) {
					if (immortal) {
						animations.Add(ExplosionAnimation(c.back().GetCenterX(), c.back().GetCenterY()));
						c.pop_back();
						continue;
					}
					else {
						mode = GAME_MODE_DEAD;
					}
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
						// Add an explosion animation.
						animations.Add(ExplosionAnimation(b.GetCenterX(), b.GetTopBoxBound()));
						// Increase the player's score.
						bool below1K = score < 1000;
						score += c.back().GetPointValue();
						// Dispose of the bullet and the invader.
						b.MoveOffScreen();
						c.pop_back();
						// When the score reaches 1,000, the player gets an additional life.
						if (below1K && score >= 1000) {
							lives.AddLife();
						}
						break;
					}
				}
			}
			// Draw invaders
			{
				register float invaderXDelta = (invadersEntering ? -0.5f : (invadersGoingRight ? 0.005f : -0.005f)) / invaders.size() / invaders.size() * millisecondsElapsed;
				register float invaderYDelta = -0.000005f * level * level * millisecondsElapsed;
				for (auto& c : invaders) {
					for (Invader& a : c) {
						a.Move(invaderXDelta, invaderYDelta);
						a.Draw();
					}
				}
			}
			// Draw bullets
			for (Bullet& b : bullets) {
				b.CalculateMotion(millisecondsElapsed);
				b.Draw();
			}
			// Draw animations
			animations.DrawAll();
			// Send the graphics to the screen.
			SDL_GL_SwapWindow(displayWindow);
		}
		// The player died.
		if (mode == GAME_MODE_DEAD) {
			glClear(GL_COLOR_BUFFER_BIT);
			DrawText("You died!", 20.0f, 325.0f, 0.5f);
			DrawText("Score: " + std::to_string(score), 20.0f, 375.0f, 0.5f);
			DrawText("Press Esc to return to the main menu.", 20.0f, 415.0f, 0.25f);
			SDL_GL_SwapWindow(displayWindow);
			while (true) {
				MillisecondsElapsed();
				Input input;
				if (input.QuitRequested) {
					mode = GAME_MODE_QUIT;
					break;
				}
				if (input.EscapePressed) {
					mode = GAME_MODE_START;
					break;
				}
			}
		}
	}

	delete program;
	SDL_Quit();
	return 0;
}
