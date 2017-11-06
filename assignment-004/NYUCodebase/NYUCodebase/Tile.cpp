#include "Tile.h"
#include <cstdlib>
#define TILES_SHEET_WIDTH_TILES 7
#define TILES_SHEET_HEIGHT_TILES 7
#define TILE_WIDTH_PIXELS 70
#define TILE_HEIGHT_PIXELS 70
#define TILES_SHEET_WIDTH_PIXELS 512
#define TILES_SHEET_HEIGHT_PIXELS 512
Tile::Tile(unsigned int id, unsigned int row, unsigned int column)
	: Rectangle(row, column) {
	// Set the texture coordinates to show the correct square from the image.
	div_t position = div(id, TILES_SHEET_WIDTH_TILES);
	SetBox(
		texture,
		position.quot * TILE_HEIGHT_PIXELS / (float)TILES_SHEET_HEIGHT_PIXELS,
		(position.rem + 1) * TILE_WIDTH_PIXELS / (float)TILES_SHEET_WIDTH_PIXELS,
		(position.quot + 1) * TILE_HEIGHT_PIXELS / (float)TILES_SHEET_HEIGHT_PIXELS,
		position.rem * TILE_WIDTH_PIXELS / (float)TILES_SHEET_WIDTH_PIXELS
	);
}
const float Tile::VERTICES[] = {
	0.0f, 0.5f, 0.0f, 0.0f, 0.5f, 0.5f,
	0.5f, 0.5f, 0.0f, 0.0f, 0.5f, 0.0f
};
