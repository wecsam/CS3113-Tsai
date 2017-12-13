#include "Tile.h"
#include <cstdlib>
// The dimensions of the texture sheet
#define TEXTURE_SHEET_WIDTH_PX 7936
#define TEXTURE_SHEET_HEIGHT_PX 1794
#define TEXTURE_SHEET_TEXTURES_PER_ROW 31
#define TEXTURE_SHEET_TEXTURES_PER_COL 6
#define TEXTURE_WIDTH (1.0f / TEXTURE_SHEET_TEXTURES_PER_ROW)
#define TEXTURE_HEIGHT (1.0f / TEXTURE_SHEET_TEXTURES_PER_COL)
// The dimensions of the draw area
#define TILE_RECT_WIDTH 1.0f
#define TILE_RECT_HEIGHT 1.16796875f
// When a player is walking over a door, draw this tile over the player
// to make it look like the player is walking through the doorway.
const Tile::TileType DOOR_Z_COVERS[] = { 137, 138, 131, 132 };
// Place the tile on the isometric grid.
Tile::Tile(float row, float column, Tile::TileType type)
	: Rectangle(ISOMETRIC_TO_SCREEN(row, column), TILE_RECT_WIDTH * 0.5f, TILE_RECT_HEIGHT * 0.5f) {
	SetType(type);
}
Tile::TileType Tile::GetType() const {
	return type;
}
bool Tile::GetBottomEdge(LineSegment& result) const {
	// Find the line segment that runs along the isometric bottom edge of this tile.
	// To do that, we need to know what the bottom edge of this tile looks like.
	enum Situation {
		LOWER_RIGHT = 0,
		UPPER_RIGHT = 1,
		LOWER_LEFT = 2,
		UPPER_LEFT = 3,
		NUM_SITUATIONS = 4
	} situation = NUM_SITUATIONS;
	// The bottom edges have to be hardcoded.
	// Only these tile types have been hardcoded.
	if (117 <= type && type <= 124) {
		situation = static_cast<Situation>((type - 117) % NUM_SITUATIONS);
	}
	else if (149 <= type && type <= 156) {
		situation = static_cast<Situation>((type - 149) % NUM_SITUATIONS);
	}
	else if (type == 146) {
		result.X1 = GetLeftBoxBound();
		result.X2 = GetRightBoxBound();
		result.Y1 = result.Y2 = GetCenterY() - (TILE_RECT_HEIGHT * 0.3076923192f);
		return true;
	}
	// If this is one of the supported tiles, then find the bottom edge.
	switch (situation) {
	case LOWER_RIGHT:
		result.X1 = GetCenterX();
		result.Y1 = GetBottomBoxBound();
		result.X2 = GetRightBoxBound();
		result.Y2 = GetCenterY() - (TILE_RECT_HEIGHT / 4);
		return true;
	case UPPER_RIGHT:
		result.X1 = GetCenterX() - (TILE_RECT_WIDTH / 8);
		result.Y1 = GetCenterY() - (TILE_RECT_HEIGHT * 0.06020066887f);
		result.X2 = GetRightBoxBound() - (TILE_RECT_WIDTH / 8);
		result.Y2 = GetCenterY() - (TILE_RECT_HEIGHT * 0.3076923192f);
		return true;
	case LOWER_LEFT:
		result.X1 = GetLeftBoxBound();
		result.Y1 = GetCenterY() - (TILE_RECT_WIDTH / 4);
		result.X2 = GetCenterX();
		result.Y2 = GetBottomBoxBound();
		return true;
	case UPPER_LEFT:
		result.X1 = GetLeftBoxBound() + (TILE_RECT_WIDTH / 8);
		result.Y1 = GetCenterY() - (TILE_RECT_HEIGHT * 0.3076923192f);
		result.X2 = GetCenterX() + (TILE_RECT_WIDTH / 8);
		result.Y2 = GetCenterY() - (TILE_RECT_HEIGHT * 0.06020066887f);
		return true;
	}
	return false;
}
bool Tile::IsOpenDoor() const {
	return 121 <= type && type <= 124;
}
bool Tile::IsClosedDoor() const {
	return 117 <= type && type <= 120;
}
bool Tile::IsDoor() const {
	return 117 <= type && type <= 124;
}
void Tile::CloseDoor() {
	if (IsOpenDoor()) {
		SetType(type - 4);
	}
}
void Tile::OpenDoor() {
	if (IsClosedDoor()) {
		SetType(type + 4);
	}
}
Tile::TileType Tile::GetDoorZCover() const {
	if (IsClosedDoor()) {
		return DOOR_Z_COVERS[type - 117];
	}
	if (IsOpenDoor()) {
		return DOOR_Z_COVERS[type - 121];
	}
	throw "This is not a door.";
}
void Tile::SetType(Tile::TileType type) {
	this->type = type;
	div_t position = div(type - 1, TEXTURE_SHEET_TEXTURES_PER_ROW);
	SetBox(
		texture,
		position.quot * TEXTURE_HEIGHT,
		(position.rem + 1) * TEXTURE_WIDTH,
		(position.quot + 1) * TEXTURE_HEIGHT,
		position.rem * TEXTURE_WIDTH
	);
}
