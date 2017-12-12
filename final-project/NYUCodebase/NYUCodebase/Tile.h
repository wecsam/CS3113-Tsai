#pragma once
#include "Rectangle.h"
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

