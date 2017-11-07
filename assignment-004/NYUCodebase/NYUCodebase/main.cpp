#include <algorithm>
#include <chrono>
#include <forward_list>
#include <fstream>
#include <iostream>
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
#include "Coin.h"
#include "Dimensions.h"
#include "Matrix.h"
#include "Player.h"
#include "ShaderProgram.h"
#include "Tile.h"
#include "TileFile.h"
#define TILE_FILE "demo.txt"
#ifdef _WINDOWS
	#define RESOURCE_FOLDER ""
#else
	#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif
using std::cerr;
using std::forward_list;
using std::max;
using std::min;
using std::vector;

SDL_Window* displayWindow;
ShaderProgram* program;

// From assignment 1
GLuint LoadTexture(const char *filePath) {
	int w, h, comp;
	unsigned char* image = stbi_load(filePath, &w, &h, &comp, STBI_rgb_alpha);
	if (image == NULL) {
		cerr << "Unable to load image: " << filePath << '\n';
		exit(7);
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

// From assignment 3
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

// From assignment 3
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

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Platform Demo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	Matrix projectionMatrix;
	projectionMatrix.SetOrthoProjection(-ORTHO_X_BOUND, ORTHO_X_BOUND, -ORTHO_Y_BOUND, ORTHO_Y_BOUND, -1.0f, 1.0f);
	program = new ShaderProgram(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
	program->SetProjectionMatrix(projectionMatrix);
	glClearColor(0.0f, 0.3f, 0.6f, 1.0f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// Read the level data from the file.
	TileFile tileFile;
	try {
		tileFile = TileFile(std::ifstream(TILE_FILE));
	}
	catch (const TileFile::ParseError& e) {
		cerr << "Unable to parse level data!\n" << e.message << '\n' << e.line << '\n';
		return 1;
	}
	// Make sure that the Platform layer and the Start and Finish entities are there.
	auto platform = tileFile.GetLayers().find("Platform");
	if (platform == tileFile.GetLayers().end()) {
		cerr << "The Platform layer is missing.\n";
		return 2;
	}
	auto start = tileFile.GetEntities().find("Start");
	if (start == tileFile.GetEntities().end()) {
		cerr << "The Start location is missing.\n";
		return 3;
	}
	if (start->second.size() != 1) {
		cerr << "There must be exactly one Start location.\n";
		return 4;
	}
	auto finish = tileFile.GetEntities().find("Finish");
	if (finish == tileFile.GetEntities().end()) {
		cerr << "The Finish location is missing.\n";
		return 5;
	}
	if (finish->second.size() != 1) {
		cerr << "There must be exactly one End location.\n";
		return 6;
	}
	// Load textures
	GLuint Tsnow = LoadTexture(RESOURCE_FOLDER"snow.png");
	GLuint Tcoin = LoadTexture(RESOURCE_FOLDER"coin.png");
	GLuint Tplayer = LoadTexture(RESOURCE_FOLDER"Player.png");
	// Create tiles
	vector<Tile> tiles;
	for (unsigned int i = 0; i < tileFile.GetMapHeight(); ++i) {
		for (unsigned int j = 0; j < tileFile.GetMapWidth(); ++j) {
			if (platform->second[i][j] >= 0) {
				tiles.emplace_back(platform->second[i][j], tileFile.RowFromTopToRowFromBottom(i), j);
			}
		}
	}
	// Create coins
	forward_list<Coin> coins;
	{
		auto coinLocations = tileFile.GetEntities().find("Coin");
		if (coinLocations != tileFile.GetEntities().end()) {
			for (const auto& location : coinLocations->second) {
				coins.emplace_front(tileFile.RowFromTopToRowFromBottom(location.row - 1), location.column);
			}
		}
	}
	// Main game loop
	float limitX = tileFile.GetMapWidth() * 0.5f - 2 * ORTHO_X_BOUND;
	Matrix view;
	Player player(tileFile.RowFromTopToRowFromBottom(start->second.begin()->row), start->second.begin()->column + 1);
	SDL_Event event;
	bool done = false;
	while (!done) {
		Uint32 mse = MillisecondsElapsed();
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
		}
		player.ProcessInput(mse);
		view.SetPosition(-min(max(player.GetCenterX(), 0.0f), limitX), -player.GetCenterY(), 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		for (const auto& tile : tiles) {
			if (tile.GetLeftBoxBound() < player.GetCenterX() && player.GetCenterX() <= tile.GetRightBoxBound()) {
				// Check for a collision with the player's bottom edge.
				if (tile.GetBottomBoxBound() < player.GetBottomBoxBound()) {
					register float topBound = tile.GetTopBoxBound();
					// Some tiles are sloped.
					switch (tile.GetID()) {
					case 25:
						// Diagonal from bottom left to top right
						if (player.GetRightBoxBound() < tile.GetRightBoxBound()) {
							topBound -= (tile.GetRightBoxBound() - player.GetRightBoxBound()) / tile.GetWidth() * tile.GetHeight();
						}
						break;
					case 32:
						// Diagonal from top left to bottom right
						if (tile.GetLeftBoxBound() < player.GetLeftBoxBound()) {
							topBound -= (player.GetLeftBoxBound() - tile.GetLeftBoxBound()) / tile.GetWidth() * tile.GetHeight();
						}
						break;
					}
					// Check whether the player is touching the top of this tile.
					if (player.GetBottomBoxBound() <= topBound) {
						player.StayAbove(topBound);
					}
				}
				// Check for a collision with the player's top edge.
				else if (tile.GetBottomBoxBound() < player.GetTopBoxBound() && player.GetTopBoxBound() <= tile.GetTopBoxBound()) {
					player.StayBelow(tile.GetBottomBoxBound());
				}
			}
			if (tile.GetBottomBoxBound() < player.GetCenterY() && player.GetCenterY() <= tile.GetTopBoxBound()) {
				// Check for a collision with the player's right edge.
				if (tile.GetLeftBoxBound() < player.GetRightBoxBound() && player.GetRightBoxBound() <= tile.GetRightBoxBound()) {
					player.StayToLeftOf(tile.GetLeftBoxBound());
				}
				// Check for a collision with the player's left edge.
				else if (tile.GetLeftBoxBound() < player.GetLeftBoxBound() && player.GetLeftBoxBound() <= tile.GetRightBoxBound()) {
					player.StayToRightOf(tile.GetRightBoxBound());
				}
			}
			DrawTrianglesWithTexture(tile.model * view, 2, tile.VERTICES, tile.texture, Tsnow);
		}
		coins.remove_if(player.ContainsCenterOf);
		for (const auto& coin : coins) {
			DrawTrianglesWithTexture(coin.model * view, 2, coin.VERTICES, coin.texture, Tcoin);
		}
		DrawTrianglesWithTexture(player.model * view, 2, player.GetVertices(), player.GetTexture(), Tplayer);
		SDL_GL_SwapWindow(displayWindow);
	}
	delete program;
	SDL_Quit();
	return 0;
}
