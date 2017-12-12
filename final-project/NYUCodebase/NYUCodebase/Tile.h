#pragma once
#include "Rectangle.h"
// The spacing from one tile to the next
#define TILE_TEXTURE_WIDTH 1.0f
#define TILE_TEXTURE_HEIGHT 0.58203125f
// Convert isometric coordinates to screen coordinates.
// Every tile is translated from the origin by (TILE_TEXTURE_WIDTH / 2, TILE_TEXTURE_HEIGHT / 2) for every column from the left
// and by (-TILE_TEXTURE_WIDTH / 2, -TILE_TEXTURE_HEIGHT / 2) for every row from the top.
//     (x', y') = (0, 0) + x * (TILE_TEXTURE_WIDTH / 2, -TILE_TEXTURE_HEIGHT / 2) + y * (-TILE_TEXTURE_WIDTH / 2, -TILE_TEXTURE_HEIGHT / 2)
// Now, just some algebra:
//     x' = x * TILE_TEXTURE_WIDTH / 2 + y * -TILE_TEXTURE_WIDTH / 2 = (x - y) * TILE_TEXTURE_WIDTH / 2
//     y' = x * -TILE_TEXTURE_HEIGHT / 2 + y * -TILE_TEXTURE_HEIGHT / 2 = (x + y) * -TILE_TEXTURE_HEIGHT / 2
#define ISOMETRIC_TO_SCREEN_X(row, column) (((column) - (row)) * (TILE_TEXTURE_WIDTH / 2))
#define ISOMETRIC_TO_SCREEN_Y(row, column) (((column) + (row)) * (-TILE_TEXTURE_HEIGHT / 2))
#define ISOMETRIC_TO_SCREEN(row, column) ISOMETRIC_TO_SCREEN_X(row, column), ISOMETRIC_TO_SCREEN_Y(row, column)
// A class for isometric tiles
class Tile : public Rectangle {
public:
	typedef int TileType;
	Tile(float row, float column, TileType type);
	TileType GetType() const;
	bool IsOpenDoor() const;
	bool IsClosedDoor() const;
	bool IsDoor() const;
	void CloseDoor();
	void OpenDoor();
	TileType GetDoorZCover() const;
private:
	void SetType(TileType type);
	TileType type;
};

