#include <chrono>
#include <fstream>
#include <iostream>
#include <list>
#include <thread>
#include <vector>
#ifdef _WINDOWS
	#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Character.h"
#include "Dimensions.h"
#include "Input.h"
#include "LineSegment.h"
#include "Matrix.h"
#include "ShaderProgram.h"
#include "Switch.h"
#include "Tile.h"
#include "TileFile.h"
#define TILE_FILE "Levels.txt"
#define PLAYER_VELOCITY 0.001f
#define ROCK_VELOCITY 0.003f
#define TILE_TYPE_LEVEL_COMPLETION 53
#define TILE_TYPE_LEVEL_START 57
#ifdef _WINDOWS
	#define RESOURCE_FOLDER ""
#else
	#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif
using std::cerr;
using std::ifstream;
using std::list;
using std::vector;
enum GameMode {	MODE_QUIT, MODE_START, MODE_PLAY, MODE_END };
SDL_Window* displayWindow;
ShaderProgram* program;
ShaderProgram* faderProgram;
const Matrix IDENTITY;
GLuint LoadTexture(const char *filePath) {
	int w, h, comp;
	unsigned char* image = stbi_load(filePath, &w, &h, &comp, STBI_rgb_alpha);
	if (image == NULL) {
		cerr << "Unable to load image: " << filePath << '\n';
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
	static Uint32 lastFrameTick = SDL_GetTicks();
	Uint32 thisFrameTick, delta;
	while (true) {
		thisFrameTick = SDL_GetTicks();
		delta = thisFrameTick - lastFrameTick;
		if (delta < 1000 / FPS) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1000 / FPS - delta));
		}
		else {
			lastFrameTick = thisFrameTick;
			return delta;
		}
	}
}
void DrawTrianglesWithTexture(const Matrix& ModelviewMatrix, GLsizei numTriangles, const float* vertices, const float* texCoords, GLuint textureID) {
	program->SetModelviewMatrix(ModelviewMatrix);
	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program->positionAttribute);
	glEnableVertexAttribArray(program->texCoordAttribute);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glDrawArrays(GL_TRIANGLES, 0, 3 * numTriangles);
	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}
void DrawRectangleWithTexture(const Rectangle& r, const Matrix& view, GLuint textureID) {
	DrawTrianglesWithTexture(r.Model * view, 2, r.GetVertices(), r.GetTextureCoordinates(), textureID);
}
void MoveCharacter(Character& c, Input::Direction d, Uint32 ms) {
	switch (d) {
	case Input::DOWN:
		c.Model.Translate(0.0f, ms * -PLAYER_VELOCITY, 0.0f);
		c.Face(Character::DOWN);
		c.Walk();
		break;
	case Input::LEFT:
		c.Model.Translate(ms * -PLAYER_VELOCITY, 0.0f, 0.0f);
		c.Face(Character::LEFT);
		c.Walk();
		break;
	case Input::UP:
		c.Model.Translate(0.0f, ms * PLAYER_VELOCITY, 0.0f);
		c.Face(Character::UP);
		c.Walk();
		break;
	case Input::RIGHT:
		c.Model.Translate(ms * PLAYER_VELOCITY, 0.0f, 0.0f);
		c.Face(Character::RIGHT);
		c.Walk();
		break;
	default:
		c.Stand();
	}
}
float square(float x) {
	return x * x;
}
template<typename CompareType>
struct RectangleAndTextureID {
	RectangleAndTextureID(const Rectangle* Rectangle, unsigned int TextureID, CompareType Comparee)
		: Rectangle(Rectangle), TextureID(TextureID), Comparee(Comparee) {}
	static bool GreaterThan(const RectangleAndTextureID& a, const RectangleAndTextureID& b) {
		return a.Comparee > b.Comparee;
	}
	const Rectangle* Rectangle;
	unsigned int TextureID;
	CompareType Comparee;
};
namespace Fader {
	float FADER_VERTICES[12];
	Uint32 FadeFinishTime = 0;
	bool FadingIn = true;
	void DimScreen(float blackness) {
		faderProgram->SetModelviewMatrix(IDENTITY);
		glUniform1f(glGetUniformLocation(faderProgram->programID, "alphaValue"), blackness);
		glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, FADER_VERTICES);
		glEnableVertexAttribArray(program->positionAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program->positionAttribute);
	}
	bool FadeDone() {
		return SDL_GetTicks() >= FadeFinishTime;
	}
	void StartFade(bool fadeIn) {
		Uint32 now = SDL_GetTicks();
		// If the previous fade hasn't finished yet,
		// then just reverse that fade. If the fade
		// direction is the same, then don't do anything.
		if (now < FadeFinishTime && fadeIn != FadingIn) {
			FadeFinishTime = now + (now - (FadeFinishTime - 500));
		}
		else {
			FadeFinishTime = now + 500;
		}
		FadingIn = fadeIn;
	}
	void DrawShade() {
		Uint32 now = SDL_GetTicks();
		if (now < FadeFinishTime) {
			// Interpolate the alpha value over time.
			if (FadingIn) {
				DimScreen((FadeFinishTime - now) / 500.0f);
			}
			else {
				DimScreen(1.0f - (FadeFinishTime - now) / 500.0f);
			}
		}
		else if (!FadingIn) {
			// Black out the whole screen.
			DimScreen(1.0f);
		}
	}
}
int main(int argc, char *argv[]) {
	// Set up the SDL and OpenGL.
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Levels", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	glClearColor(0.0f, 0.3f, 0.6f, 1.0f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 4096);
	program = new ShaderProgram(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
	faderProgram = new ShaderProgram(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_black_alpha.glsl");
	Rectangle::SetBox(Fader::FADER_VERTICES, ORTHO_Y_BOUND, ORTHO_X_BOUND, -ORTHO_Y_BOUND, -ORTHO_X_BOUND);
	// Set the projection matrix.
	{
		Matrix projectionMatrix;
		projectionMatrix.SetOrthoProjection(-ORTHO_X_BOUND, ORTHO_X_BOUND, -ORTHO_Y_BOUND, ORTHO_Y_BOUND, -1.0f, 1.0f);
		program->SetProjectionMatrix(projectionMatrix);
		faderProgram->SetProjectionMatrix(projectionMatrix);
	}
	// Read the level data from the file.
	TileFile tileFile;
	try {
		tileFile = TileFile(ifstream(TILE_FILE));
	}
	catch (const TileFile::ParseError& e) {
		cerr << "Unable to parse level data!\n" << e.message << '\n' << e.line << '\n';
		return 2;
	}
	auto tilesFloorTypes = tileFile.GetLayers().find("Floor");
	if (tilesFloorTypes == tileFile.GetLayers().end()) {
		cerr << "The Floor layer is missing.\n";
		return 3;
	}
	auto tilesWallsTypes = tileFile.GetLayers().find("Walls");
	if (tilesWallsTypes == tileFile.GetLayers().end()) {
		cerr << "The Walls layer is missing.\n";
		return 4;
	}
	// Find the starting locations of the player and the helper AI.
	auto playerStart = tileFile.GetEntities().find("Player");
	if (playerStart == tileFile.GetEntities().end()) {
		cerr << "The player's start location is missing.\n";
		return 5;
	}
	if (playerStart->second.size() != 1) {
		cerr << "There must be exactly one starting location for the player.\n";
		return 6;
	}
	auto helperStart = tileFile.GetEntities().find("AI");
	if (helperStart == tileFile.GetEntities().end()) {
		cerr << "The helper AI's start location is missing.\n";
		return 7;
	}
	if (helperStart->second.size() != 1) {
		cerr << "There must be exactly one starting location for the helper AI.\n";
		return 8;
	}
	// Find the location of the rock.
	auto rockStart = tileFile.GetEntities().find("Rock");
	if (rockStart == tileFile.GetEntities().end()) {
		cerr << "The rock's start location is missing.\n";
		return 9;
	}
	if (rockStart->second.size() != 1) {
		cerr << "There must be exactly one starting location for the rock.\n";
		return 10;
	}
	// Load textures.
	auto Ttiles = LoadTexture(RESOURCE_FOLDER"Images/Prototype_31x6.png");
	auto Tbetty = LoadTexture(RESOURCE_FOLDER"Images/Betty.png");
	auto Tgeorge = LoadTexture(RESOURCE_FOLDER"Images/George.png");
	auto Trock = LoadTexture(RESOURCE_FOLDER"Images/Rock.png");
	auto Tstart = LoadTexture(RESOURCE_FOLDER"Images/Start.png");
	auto Tend = LoadTexture(RESOURCE_FOLDER"Images/End.png");
	// Load sounds.
	auto Sdoor = Mix_LoadWAV(RESOURCE_FOLDER"Sounds/Door.wav");
	auto Sland = Mix_LoadWAV(RESOURCE_FOLDER"Sounds/Land.wav");
	auto Slaunch = Mix_LoadWAV(RESOURCE_FOLDER"Sounds/Launch.wav");
	auto SrockGet = Mix_LoadWAV(RESOURCE_FOLDER"Sounds/Rock_Get.wav");
	auto SrockThrow = Mix_LoadWAV(RESOURCE_FOLDER"Sounds/Rock_Throw.wav");
	auto Sselect = Mix_LoadWAV(RESOURCE_FOLDER"Sounds/Select.wav");
	auto Mbackground = Mix_LoadMUS(RESOURCE_FOLDER"Sounds/Podington_Bear_-_Starling.wav");
	// Start background music.
	Mix_VolumeMusic(MIX_MAX_VOLUME / 2);
	Mix_PlayMusic(Mbackground, -1);
	// Start the game.
	GameMode mode = MODE_START, nextMode;
	bool modeWaitingForFade = false;
	while (mode != MODE_QUIT) {
		{
			Rectangle startScreen(0.0f, 0.0f, ORTHO_X_BOUND, ORTHO_Y_BOUND, 0.0f, 0.0f, 1.0f, 1.0f);
			Character marker(0.0f, 0.0f);
			bool selectionOnQuit = false;
			Fader::StartFade(true);
			while (mode == MODE_START) {
				// Process input.
				Uint32 ms = MillisecondsElapsed();
				Input input;
				if (input.QuitRequested) {
					mode = MODE_QUIT;
				}
				else if (input.UpPressed) {
					if (selectionOnQuit) {
						Mix_PlayChannel(-1, Sselect, 0);
					}
					selectionOnQuit = false;
				}
				else if (input.DownPressed) {
					if (!selectionOnQuit) {
						Mix_PlayChannel(-1, Sselect, 0);
					}
					selectionOnQuit = true;
				}
				else if (input.EnterPressed) {
					if (selectionOnQuit) {
						mode = MODE_QUIT;
					}
					else {
						nextMode = MODE_PLAY;
						modeWaitingForFade = true;
						Fader::StartFade(false);
					}
				}
				marker.Model.SetPosition(-1.75f, selectionOnQuit ? -0.6f : -0.1f, 0.0f);
				// Clear screen.
				glClear(GL_COLOR_BUFFER_BIT);
				// Draw.
				DrawRectangleWithTexture(startScreen, IDENTITY, Tstart);
				DrawRectangleWithTexture(marker, IDENTITY, Tbetty);
				Fader::DrawShade();
				if (modeWaitingForFade && Fader::FadeDone()) {
					mode = nextMode;
					modeWaitingForFade = false;
				}
				// Update screen.
				SDL_GL_SwapWindow(displayWindow);
			}
		}
		{
			Matrix view;
			// The helper will interact with these objects.
			Tile* helperDoor = nullptr;
			Switch* helperSwitch = nullptr;
			// Process the Switches.
			vector<Switch*> switches;
			{
				auto switchEntities = tileFile.GetEntities().find("Switch");
				if (switchEntities != tileFile.GetEntities().end()) {
					for (const auto& entity : switchEntities->second) {
						Switch* newSwitch = new Switch((float)entity.Row, (float)entity.Column, entity.DoorX, entity.DoorY, entity.FacingLeft);
						switches.push_back(newSwitch);
						if (entity.Row == 19 && entity.Column == 5) {
							helperSwitch = newSwitch;
						}
					}
				}
			}
			// Process the Floor and Walls layers.
			vector<Tile*> tilesFloor;
			vector<Tile*> tilesWalls;
			vector<Tile> tilesDoorZCovers;
			// The two layers have the same dimensions, so we can take care of both with one loop.
			for (size_t i = 0; i < tileFile.GetMapHeight(); ++i) {
				for (size_t j = 0; j < tileFile.GetMapWidth(); ++j) {
					// Add a floor tile.
					if (tilesFloorTypes->second[i][j] > 0) {
						tilesFloor.push_back(new Tile((float)i, (float)j, tilesFloorTypes->second[i][j]));
					}
					// Add a wall tile.
					if (tilesWallsTypes->second[i][j] > 0) {
						Tile* newTile = new Tile((float)i, (float)j, tilesWallsTypes->second[i][j]);
						tilesWalls.push_back(newTile);
						// Check whether this is a door.
						if (newTile->IsDoor()) {
							// Check whether there is a switch that controls this wall.
							// We could create a map of coordinates, but there are not that many switches anyway.
							for (Switch* toggler : switches) {
								if (toggler->GetDoorX() == j && toggler->GetDoorY() == i) {
									auto test = newTile->GetType();
									toggler->Door = newTile;
									// Check whether this is the door for whose opening the helper is waiting.
									if (j == 7 && i == 13) {
										helperDoor = newTile;
									}
								}
							}
							// Add a Z cover for this door.
							// The purpose of the Z cover is to cover the player as the player is passing through a doorway.
							// Without the Z cover, the player would not look like the player was going through the doorway.
							tilesDoorZCovers.emplace_back((float)i, (float)j, newTile->GetDoorZCover());
						}
					}
				}
			}
			// Create the player, helper, and rock.
			Character player(0.0f, 0.0f);
			Character helper(ISOMETRIC_TO_SCREEN(helperStart->second.begin()->Row, helperStart->second.begin()->Column));
			Rectangle rock(ISOMETRIC_TO_SCREEN(rockStart->second.begin()->Row, rockStart->second.begin()->Column), 0.125f, 0.125f, 0.0f, 0.0f, 1.0f, 1.0f);
			// Create the tile that will bring the player to the first level.
			Tile* start = new Tile((float)playerStart->second.begin()->Row, (float)playerStart->second.begin()->Column, TILE_TYPE_LEVEL_START);
			float playerLastX = 0.0f, playerLastY = 0.0f, playerAdvancingTargetY = start->GetCenterY();
			start->Model.Translate(0.0f, TILE_TEXTURE_HEIGHT * -4.0f, 0.0f);
			player.Model.SetPosition(start->GetCenterX(), start->GetCenterY() - TILE_TEXTURE_HEIGHT / 4.0f, 0.0f);
			Tile* playerAdvancingToNextLevel = start;
			tilesFloor.push_back(start);
			// If the player is near a switch, this points to that switch.
			Switch* playerNearSwitch = nullptr;
			// Keep track of the status of the rock
			enum RockStatus { ROCK_SITTING, ROCK_FOLLOWING_PLAYER, ROCK_FLYING, ROCK_HIT_SWITCH, ROCK_DONE } rockStatus = ROCK_SITTING;
			Switch* rockTarget = nullptr;
			// Provide some basic screen shake
			Uint32 screenShakeFinishTime = 0;
			// Main game loop
			Fader::StartFade(true);
			while (mode == MODE_PLAY) {
				// Set the view matrix so that the view is centered on the player.
				{
					Uint32 now = SDL_GetTicks();
					float d = 0.0f;
					if (now < screenShakeFinishTime) {
						d = (screenShakeFinishTime - now) / 1000.0f;
						d = 0.05f * (1 - d) * sin(100.0f * d);
					}
					view.SetPosition(-player.GetCenterX() + d, -player.GetCenterY() + d, 0.0f);
				}
				// Process input.
				Uint32 ms = MillisecondsElapsed();
				Input input;
				if (input.QuitRequested) {
					mode = MODE_QUIT;
				}
				else if (input.EscapePressed) {
					mode = MODE_START;
				}
				else if (playerAdvancingToNextLevel) {
					// The player is advancing to the next level.
					float dy;
					Tile* theTile = playerAdvancingToNextLevel;
					if (theTile->GetCenterY() >= -TILE_TEXTURE_HEIGHT) {
						if (!modeWaitingForFade) {
							nextMode = MODE_END;
							modeWaitingForFade = true;
							Fader::StartFade(false);
						}
						dy = ms * PLAYER_VELOCITY;
					}
					else if (theTile->GetCenterY() >= playerAdvancingTargetY) {
						// The player has reached the next level.
						dy = playerAdvancingTargetY - theTile->GetCenterY();
						playerAdvancingToNextLevel = nullptr;
						Mix_PlayChannel(-1, Sland, 0);
						// Shake the screen.
						screenShakeFinishTime = SDL_GetTicks() + 150;
					}
					else {
						// Make the player rise.
						dy = ms * PLAYER_VELOCITY;
					}
					player.Model.Translate(0.0f, dy, 0.0f);
					theTile->Model.Translate(0.0f, dy, 0.0f);
				}
				else {
					// Press the spacebar to flip the switch.
					if (input.SpacePressed) {
						if (playerNearSwitch) {
							playerNearSwitch->Flip();
							Mix_PlayChannel(-1, Sdoor, 0);
						}
						// Note: the rock can also flip the switch. See below.
					}
					MoveCharacter(player, input.PlayerDirection, ms);
				}
				// Make the helper move.
				if (helperDoor && helperSwitch && helperDoor->IsOpenDoor() && !helperSwitch->IsOn()) {
					Input::Direction helperDirection = Input::NONE;
					if (helper.GetCenterX() >= ISOMETRIC_TO_SCREEN_X(14, 4)) {
						if (helper.GetCenterY() < ISOMETRIC_TO_SCREEN_Y(12, 9)) {
							helperDirection = Input::UP;
						}
						else {
							helperDirection = Input::LEFT;
						}
					}
					else if (helper.GetCenterX() >= ISOMETRIC_TO_SCREEN_X(17, 3)) {
						if (helper.GetCenterY() > ISOMETRIC_TO_SCREEN_Y(16, 7)) {
							helperDirection = Input::DOWN;
						}
						else {
							helperDirection = Input::LEFT;
						}
					}
					else {
						helperSwitch->Flip();
						helperDirection = Input::DOWN;
						Mix_PlayChannel(-1, Sdoor, 0);
					}
					MoveCharacter(helper, helperDirection, ms);
				}
				// Create a line segment from the player's last position to the player's current position.
				LineSegment playerPath(playerLastX, playerLastY - PLAYER_FEET_OFFSET_Y, player.GetCenterX(), player.GetCenterY() - PLAYER_FEET_OFFSET_Y);
				// If the player has the rock, make the rock follow the player.
				switch (rockStatus) {
				case ROCK_SITTING:
					if (square(rock.GetCenterX() - player.GetCenterX()) + square(rock.GetCenterY() - player.GetCenterY()) < 0.125f) {
						// The player touched the rock. Make the rock follow the player.
						rockStatus = ROCK_FOLLOWING_PLAYER;
						Mix_PlayChannel(-1, SrockGet, 0);
					}
					break;
				case ROCK_FOLLOWING_PLAYER:
					rock.Model.SetPosition(player.GetCenterX(), player.GetCenterY() + 0.125f, 0.0f);
					if (input.SpacePressed) {
						// Search for the nearest switch that is off.
						float distanceSquaredMax = 4.0f;
						Switch* target = nullptr;
						for (Switch* toggler : switches) {
							if (!toggler->IsOn()) {
								float distanceSquared = square(rock.GetCenterX() - toggler->GetCenterX()) + square(rock.GetCenterY() - toggler->GetCenterY());
								if (distanceSquared <= distanceSquaredMax) {
									distanceSquaredMax = distanceSquared;
									target = toggler;
								}
							}
						}
						// Throw the rock at the switch.
						if (target) {
							rockTarget = target;
							rockStatus = ROCK_FLYING;
							Mix_PlayChannel(-1, SrockThrow, 0);
						}
					}
					break;
				case ROCK_FLYING:
					if(rockTarget){
						float distanceX = rockTarget->GetCenterX() - rock.GetCenterX(),
							distanceY = rockTarget->GetCenterY() - rock.GetCenterY();
						// Check whether the rock has hit the switch.
						if (square(distanceX) + square(distanceY) < 0.125f) {
							rockStatus = ROCK_HIT_SWITCH;
						}
						else {
							float rockFlightAngle = atan2(distanceY, distanceX);
							rock.Model.Translate(cos(rockFlightAngle) * ROCK_VELOCITY * ms, sin(rockFlightAngle) * 2.0f * ROCK_VELOCITY * ms, 0.0f);
						}
					}
					else {
						rockStatus = ROCK_FOLLOWING_PLAYER;
					}
					break;
				case ROCK_HIT_SWITCH:
					rockTarget->Flip();
					Mix_PlayChannel(-1, Sdoor, 0);
					screenShakeFinishTime = SDL_GetTicks() + 200;
					rockStatus = ROCK_DONE;
					break;
				}
				// Clear screen.
				glClear(GL_COLOR_BUFFER_BIT);
				// Draw the floor.
				bool playerIsOnTheFloor = false;
				for (Tile* tile : tilesFloor) {
					// Check whether the player is on this tile.
					if (
						fabs(player.GetCenterX() - tile->GetCenterX()) < TILE_TEXTURE_WIDTH / 4.0f &&
						tile->GetCenterY() - TILE_TEXTURE_HEIGHT * 0.675f < player.GetCenterY() &&
						player.GetCenterY() < tile->GetCenterY() - TILE_TEXTURE_HEIGHT * 0.125f
					) {
						// Check whether this is the tile that marks the completion of the level.
						if (!playerAdvancingToNextLevel && tile->GetType() == TILE_TYPE_LEVEL_COMPLETION) {
							// The user completed this level.
							playerAdvancingToNextLevel = tile;
							// Put the player in the middle of the tile.
							player.Model.SetPosition(tile->GetCenterX(), tile->GetCenterY() - TILE_TEXTURE_HEIGHT / 4.0f, 0.0f);
							player.Face(Character::DOWN);
							player.Stand();
							// The player should rise 4 spaces. Set the destination.
							playerAdvancingTargetY = tile->GetCenterY() + TILE_TEXTURE_HEIGHT * 4.0f;
							tile->SetType(TILE_TYPE_LEVEL_START);
							// Shake the screen.
							screenShakeFinishTime = SDL_GetTicks() + 500;
							// Play a sound effect.
							Mix_PlayChannel(-1, Slaunch, 0);
						}
						playerIsOnTheFloor = true;
					}
					else if (
						tile->GetLeftBoxBound() <= player.GetCenterX() &&
						player.GetCenterX() <= tile->GetRightBoxBound() &&
						tile->GetCenterY() - TILE_TEXTURE_HEIGHT * 0.5f < player.GetCenterY() &&
						player.GetCenterY() < tile->GetCenterY()
					) {
						playerIsOnTheFloor = true;
					}
					// Draw the tile.
					DrawRectangleWithTexture(*tile, view, Ttiles);
				}
				if (!playerIsOnTheFloor && !playerAdvancingToNextLevel) {
					player.Model.Translate(0.0f, -0.003f * ms, 0.0f);
				}
				// Draw walls above the player or helper, draw the player or helper, draw the walls between the
				// player and helper, draw the other character, and then draw walls below that character.
				{
					list<RectangleAndTextureID<float>> drawList;
					// Add all walls to the draw list.
					for (const Tile* tile : tilesWalls) {
						// TODO: use the diagonal edge of the wall instead of a flat Y value
						drawList.emplace_back(tile, Ttiles, tile->GetCenterY() - TILE_TEXTURE_HEIGHT * 0.125f);
						// If this is not an open door and the player is not advancing to the next level,
						// check for a collision between this wall and the player.
						if (!tile->IsOpenDoor() && !playerAdvancingToNextLevel) {
							LineSegment wallBottomEdge;
							if (tile->GetBottomEdge(wallBottomEdge)) {
								float intersectionX, intersectionY;
								if (playerPath.IntersectionWith(wallBottomEdge, intersectionX, intersectionY)) {
									// The player collided!
									player.Model.SetPosition(playerLastX, playerLastY, 0.0f);
								}
							}
						}
					}
					// Add all door Z covers to the draw list.
					for (const auto& cover : tilesDoorZCovers) {
						drawList.emplace_back(&cover, Ttiles, cover.GetCenterY() - TILE_TEXTURE_HEIGHT * 0.5f);
					}
					// Add all switches to the draw list.
					// Also, check whether the player is near a switch.
					playerNearSwitch = nullptr;
					for (Switch* toggler : switches) {
						drawList.emplace_back(toggler, Ttiles, toggler->GetCenterY() - TILE_TEXTURE_HEIGHT * 0.126f);
						if (
							toggler->GetLeftBoxBound() <= player.GetCenterX() && player.GetCenterX() <= toggler->GetRightBoxBound() &&
							toggler->GetBottomBoxBound() <= player.GetCenterY() && player.GetCenterY() <= toggler->GetTopBoxBound()
						) {
							playerNearSwitch = toggler;
						}
					}
					// Add characters to the other list.
					drawList.emplace_back(&player, Tbetty, player.GetCenterY());
					drawList.emplace_back(&helper, Tgeorge, helper.GetCenterY());
					// Sort the list from top to bottom.
					drawList.sort(RectangleAndTextureID<float>::GreaterThan);
					// Draw the rectangles.
					for (const auto& r : drawList) {
						DrawRectangleWithTexture(*r.Rectangle, view, r.TextureID);
					}
				}
				// Draw the rock.
				if (rockStatus != ROCK_DONE) {
					DrawRectangleWithTexture(rock, view, Trock);
				}
				// If the player is advancing to the next level, just draw the player and the tile on top of everything.
				if (playerAdvancingToNextLevel) {
					DrawRectangleWithTexture(*playerAdvancingToNextLevel, view, Ttiles);
					DrawRectangleWithTexture(player, view, Tbetty);
				}
				// Save the player's position.
				playerLastX = player.GetCenterX();
				playerLastY = player.GetCenterY();
				// Do the screen fade.
				Fader::DrawShade();
				if (modeWaitingForFade && Fader::FadeDone()) {
					mode = nextMode;
					modeWaitingForFade = false;
				}
				// Update screen.
				SDL_GL_SwapWindow(displayWindow);
			}
			for (Tile* tile : tilesFloor) {
				delete tile;
			}
			for (Tile* tile : tilesWalls) {
				delete tile;
			}
			for (Switch* toggler : switches) {
				delete toggler;
			}
		}
		if (mode == MODE_END) {
			Rectangle endScreen(0.0f, 0.0f, ORTHO_X_BOUND, ORTHO_Y_BOUND, 0.0f, 0.0f, 1.0f, 1.0f);
			Fader::StartFade(true);
			while (mode == MODE_END) {
				// Process input.
				Uint32 ms = MillisecondsElapsed();
				Input input;
				if (input.QuitRequested) {
					mode = MODE_QUIT;
				}
				else if (input.SpacePressed || input.EnterPressed || input.EscapePressed) {
					nextMode = MODE_START;
					modeWaitingForFade = true;
					Fader::StartFade(false);
				}
				// Draw.
				if (Fader::FadeDone()) {
					if (modeWaitingForFade) {
						mode = nextMode;
						modeWaitingForFade = false;
					}
				}
				else {
					glClear(GL_COLOR_BUFFER_BIT);
					DrawRectangleWithTexture(endScreen, IDENTITY, Tend);
					Fader::DrawShade();
					SDL_GL_SwapWindow(displayWindow);
				}
			}
		}
	}
	Mix_FreeChunk(Sdoor);
	Mix_FreeChunk(Sland);
	Mix_FreeChunk(Slaunch);
	Mix_FreeChunk(SrockGet);
	Mix_FreeChunk(SrockThrow);
	Mix_FreeChunk(Sselect);
	Mix_FreeMusic(Mbackground);
	delete program;
	delete faderProgram;
	SDL_Quit();
	return 0;
}
