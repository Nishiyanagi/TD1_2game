#pragma once
#include "Vector2.h"
#include "Collision.h"

class Enemy
{
public:
	Enemy(Vector2 position, Vector2 size, Vector2 velocity, int hp);
	virtual ~Enemy() = default;

	int Hp() const { return hp_; }
	const Vector2& Pos() const { return pos_; }
	const Vector2& Size() const { return size_; }
	Rect GetRect() const { return MakeRect(pos_, size_); }
	void Damage(int amount);

protected:
	Vector2 pos_{};
	Vector2 size_{};
	Vector2 velocity_{};
	Vector2 acceleration_{};

	int hp_;
};