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
#include "Matrix.h"
#include "ShaderProgram.h"
#include "Tile.h"
#include "TileFile.h"
#define TILE_FILE "Levels.txt"
#define PLAYER_VELOCITY 0.001f
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
	// Process the Floor and Walls layers.
	vector<Tile> tilesFloor;
	vector<Tile> tilesWalls;
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
		for (size_t i = 0; i < tileFile.GetMapHeight(); ++i) {
			for (size_t j = 0; j < tileFile.GetMapWidth(); ++j) {
				if (tilesFloorTypes->second[i][j] > 0) {
					tilesFloor.emplace_back((float)i, (float)j, tilesFloorTypes->second[i][j]);
				}
				if (tilesWallsTypes->second[i][j] > 0) {
					tilesWalls.emplace_back((float)i, (float)j, tilesWallsTypes->second[i][j]);
				}
			}
		}
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
		return 5;
	}
	if (helperStart->second.size() != 1) {
		cerr << "There must be exactly one starting location for the helper AI.\n";
		return 6;
	}
	// Load textures.
	auto Ttiles = LoadTexture(RESOURCE_FOLDER"Images/Prototype_31x6.png");
	auto Tbetty = LoadTexture(RESOURCE_FOLDER"Images/Betty.png");
	auto Tgeorge = LoadTexture(RESOURCE_FOLDER"Images/George.png");
	// Start the game.
	GameMode mode = MODE_START;
	while (mode != MODE_QUIT) {
		while (mode == MODE_START) {
			// TODO: make a start screen
			mode = MODE_PLAY;
		}
		{
			Matrix view;
			Character player(ISOMETRIC_TO_SCREEN(playerStart->second.begin()->Row, playerStart->second.begin()->Column));
			Character helper(ISOMETRIC_TO_SCREEN(helperStart->second.begin()->Row, helperStart->second.begin()->Column));
			while (mode == MODE_PLAY) {
				// Process input.
				auto ms = MillisecondsElapsed();
				Input input;
				if (input.QuitRequested) {
					mode = MODE_QUIT;
				}
				else if (input.EscapePressed) {
					mode = MODE_START;
				}
				switch (input.PlayerDirection) {
				case Input::DOWN:
					player.Model.Translate(0.0f, ms * -PLAYER_VELOCITY, 0.0f);
					player.Face(Character::DOWN);
					player.Walk();
					break;
				case Input::LEFT:
					player.Model.Translate(ms * -PLAYER_VELOCITY, 0.0f, 0.0f);
					player.Face(Character::LEFT);
					player.Walk();
					break;
				case Input::UP:
					player.Model.Translate(0.0f, ms * PLAYER_VELOCITY, 0.0f);
					player.Face(Character::UP);
					player.Walk();
					break;
				case Input::RIGHT:
					player.Model.Translate(ms * PLAYER_VELOCITY, 0.0f, 0.0f);
					player.Face(Character::RIGHT);
					player.Walk();
					break;
				default:
					player.Stand();
				}
				// Clear screen.
				glClear(GL_COLOR_BUFFER_BIT);
				// Set the view matrix so that the view is centered on the player.
				view.SetPosition(-player.GetCenterX(), -player.GetCenterY(), 0.0f);
				// Draw the floor.
				for (const auto& tile : tilesFloor) {
					DrawTrianglesWithTexture(tile.Model * view, 2, tile.GetVertices(), tile.GetTextureCoordinates(), Ttiles);
				}
				// Draw walls above the player or helper, draw the player or helper, draw the walls between the
				// player and helper, draw the other character, and then draw walls below that character.
				{
					list<RectangleAndTextureID<float>> drawList;
					// Add all walls to the draw list.
					for (const auto& tile : tilesWalls) {
						// TODO: use the diagonal edge of the wall instead of a flat Y value
						drawList.emplace_back(&tile, Ttiles, tile.GetCenterY() - TILE_TEXTURE_HEIGHT * 0.125f);
					}
					// Add characters to the other list.
					drawList.emplace_back(&player, Tbetty, player.GetCenterY());
					drawList.emplace_back(&helper, Tgeorge, helper.GetCenterY());
					// Sort the list from top to bottom.
					drawList.sort(RectangleAndTextureID<float>::GreaterThan);
					// Draw the rectangles.
					for (const auto& r : drawList) {
						DrawTrianglesWithTexture(r.Rectangle->Model * view, 2, r.Rectangle->GetVertices(), r.Rectangle->GetTextureCoordinates(), r.TextureID);
					}
				}
				// Update screen.
				SDL_GL_SwapWindow(displayWindow);
			}
		}
		while (mode == MODE_END) {
			// TODO: make an end screen
			mode = MODE_QUIT;
		}
	}
	delete program;
	SDL_Quit();
	return 0;
}
