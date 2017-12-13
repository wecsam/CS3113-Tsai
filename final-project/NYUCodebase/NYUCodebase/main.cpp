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
	program = new ShaderProgram(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
	// Set the projection matrix.
	{
		Matrix projectionMatrix;
		projectionMatrix.SetOrthoProjection(-ORTHO_X_BOUND, ORTHO_X_BOUND, -ORTHO_Y_BOUND, ORTHO_Y_BOUND, -1.0f, 1.0f);
		program->SetProjectionMatrix(projectionMatrix);
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
	vector<Tile> tilesFloor;
	vector<Tile*> tilesWalls;
	vector<Tile> tilesDoorZCovers;
	{
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
		// The two layers have the same dimensions, so we can take care of both with one loop.
		for (size_t i = 0; i < tileFile.GetMapHeight(); ++i) {
			for (size_t j = 0; j < tileFile.GetMapWidth(); ++j) {
				// Add a floor tile.
				if (tilesFloorTypes->second[i][j] > 0) {
					tilesFloor.emplace_back((float)i, (float)j, tilesFloorTypes->second[i][j]);
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
	}
	if (!helperDoor) {
		cerr << "The helper's door was not found.\n";
		return 5;
	}
	if (!helperSwitch) {
		cerr << "The helper's switch was not found.\n";
		return 6;
	}
	// Find the starting locations of the player and the helper AI.
	auto playerStart = tileFile.GetEntities().find("Player");
	if (playerStart == tileFile.GetEntities().end()) {
		cerr << "The player's start location is missing.\n";
		return 7;
	}
	if (playerStart->second.size() != 1) {
		cerr << "There must be exactly one starting location for the player.\n";
		return 8;
	}
	auto helperStart = tileFile.GetEntities().find("AI");
	if (helperStart == tileFile.GetEntities().end()) {
		cerr << "The helper AI's start location is missing.\n";
		return 9;
	}
	if (helperStart->second.size() != 1) {
		cerr << "There must be exactly one starting location for the helper AI.\n";
		return 10;
	}
	// Load textures.
	auto Ttiles = LoadTexture(RESOURCE_FOLDER"Images/Prototype_31x6.png");
	auto Tbetty = LoadTexture(RESOURCE_FOLDER"Images/Betty.png");
	auto Tgeorge = LoadTexture(RESOURCE_FOLDER"Images/George.png");
	auto Tstart = LoadTexture(RESOURCE_FOLDER"Images/Start.png");
	// Start the game.
	GameMode mode = MODE_START;
	while (mode != MODE_QUIT) {
		{
			Rectangle startScreen(0.0f, 0.0f, ORTHO_X_BOUND, ORTHO_Y_BOUND, 0.0f, 0.0f, 1.0f, 1.0f);
			Character marker(0.0f, 0.0f);
			bool selectionOnQuit = false;
			while (mode == MODE_START) {
				// Process input.
				Uint32 ms = MillisecondsElapsed();
				Input input;
				if (input.QuitRequested) {
					mode = MODE_QUIT;
				}
				else if (input.UpPressed) {
					selectionOnQuit = false;
				}
				else if (input.DownPressed) {
					selectionOnQuit = true;
				}
				else if (input.EnterPressed) {
					if (selectionOnQuit) {
						mode = MODE_QUIT;
					}
					else {
						mode = MODE_PLAY;
					}
				}
				marker.Model.SetPosition(-1.75f, selectionOnQuit ? -0.6f : -0.1f, 0.0f);
				// Clear screen.
				glClear(GL_COLOR_BUFFER_BIT);
				// Draw.
				DrawTrianglesWithTexture(startScreen.Model, 2, startScreen.GetVertices(), startScreen.GetTextureCoordinates(), Tstart);
				DrawTrianglesWithTexture(marker.Model, 2, marker.GetVertices(), marker.GetTextureCoordinates(), Tbetty);
				// Update screen.
				SDL_GL_SwapWindow(displayWindow);
			}
		}
		{
			Matrix view;
			Character player(ISOMETRIC_TO_SCREEN(playerStart->second.begin()->Row, playerStart->second.begin()->Column));
			Character helper(ISOMETRIC_TO_SCREEN(helperStart->second.begin()->Row, helperStart->second.begin()->Column));
			float playerLastX = player.GetCenterX(), playerLastY = player.GetCenterY(), playerAdvancingTargetY;
			Switch* playerNearSwitch = nullptr;
			Tile* playerAdvancingToNextLevel = nullptr;
			while (mode == MODE_PLAY) {
				// Set the view matrix so that the view is centered on the player.
				view.SetPosition(-player.GetCenterX(), -player.GetCenterY(), 0.0f);
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
						mode = MODE_END;
						dy = 0.0f;
					}
					else if (theTile->GetCenterY() >= playerAdvancingTargetY) {
						// The player has reached the next level.
						dy = playerAdvancingTargetY - theTile->GetCenterY();
						playerAdvancingToNextLevel = nullptr;
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
						}
					}
					MoveCharacter(player, input.PlayerDirection, ms);
				}
				// Make the helper move.
				if (helperDoor->IsOpenDoor() && !helperSwitch->IsOn()) {
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
					}
					MoveCharacter(helper, helperDirection, ms);
				}
				// Create a line segment from the player's last position to the player's current position.
				LineSegment playerPath(playerLastX, playerLastY - PLAYER_FEET_OFFSET_Y, player.GetCenterX(), player.GetCenterY() - PLAYER_FEET_OFFSET_Y);
				// Clear screen.
				glClear(GL_COLOR_BUFFER_BIT);
				// Draw the floor.
				for (auto& tile : tilesFloor) {
					// Check whether the player is on this tile.
					if (
						fabs(player.GetCenterX() - tile.GetCenterX()) < TILE_TEXTURE_WIDTH / 4.0f &&
						tile.GetCenterY() - TILE_TEXTURE_HEIGHT * 0.675f < player.GetCenterY() &&
						player.GetCenterY() < tile.GetCenterY() - TILE_TEXTURE_HEIGHT * 0.125f
					) {
						// Check whether this is the tile that marks the completion of the level.
						if (!playerAdvancingToNextLevel && tile.GetType() == TILE_TYPE_LEVEL_COMPLETION) {
							// The user completed this level.
							playerAdvancingToNextLevel = &tile;
							// Put the player in the middle of the tile.
							player.Model.SetPosition(tile.GetCenterX(), tile.GetCenterY() - TILE_TEXTURE_HEIGHT / 4.0f, 0.0f);
							player.Face(Character::DOWN);
							player.Stand();
							// The player should rise 4 spaces. Set the destination.
							playerAdvancingTargetY = tile.GetCenterY() + TILE_TEXTURE_HEIGHT * 4.0f;
							tile.SetType(TILE_TYPE_LEVEL_START);
						}
					}
					// Draw the tile.
					DrawRectangleWithTexture(tile, view, Ttiles);
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
				// If the player is advancing to the next level, just draw the player and the tile on top of everything.
				if (playerAdvancingToNextLevel) {
					DrawRectangleWithTexture(*playerAdvancingToNextLevel, view, Ttiles);
					DrawRectangleWithTexture(player, view, Tbetty);
				}
				// Save the player's position.
				playerLastX = player.GetCenterX();
				playerLastY = player.GetCenterY();
				// Update screen.
				SDL_GL_SwapWindow(displayWindow);
			}
		}
		while (mode == MODE_END) {
			// TODO: make an end screen
			mode = MODE_QUIT;
		}
	}
	for (Tile* tile : tilesWalls) {
		delete tile;
	}
	for (Switch* toggler : switches) {
		delete toggler;
	}
	delete program;
	SDL_Quit();
	return 0;
}
