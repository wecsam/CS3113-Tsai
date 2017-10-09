#pragma once
#include <forward_list>
#include "GameEntity.h"
struct Input
{
	Input();
	void Process(GLuint spriteSheet, PlayerLaserCannon& player, std::forward_list<Bullet>& bullets);
	bool QuitRequested = false;
	bool EscapePressed = false;
	unsigned int BulletsToFire = 0;
	PlayerLaserCannon::Movements PlayerMovement;
};
