#pragma once
#include "Enemy.h"
class Enemy1 :
    public Enemy
{
public:
	Enemy1(Vector2 position, Vector2 size, Vector2 velocity, int hp);
	void Move();
};

