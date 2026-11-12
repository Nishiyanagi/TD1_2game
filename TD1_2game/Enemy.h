#pragma once
#include "Vector2.h"

class Enemy
{
public:
	Enemy(Vector2 position, Vector2 size, Vector2 velocity, int hp);
	int Hp() const { return hp_; }
	void Damage(int amount);

private:
	Vector2 pos_{};
	Vector2 size_{};
	Vector2 velocity_{};
	Vector2 acceleration_{};

	int hp_;
};