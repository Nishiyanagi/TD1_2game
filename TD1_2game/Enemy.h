#pragma once
#include "Vector2.h"
#include "Collision.h"

class Enemy
{
public:
	Enemy(Vector2 position, Vector2 size, Vector2 velocity, float hp);
	virtual ~Enemy() = default;

	float Hp() const { return hp_; }
	const Vector2& Pos() const { return pos_; }
	const Vector2& Size() const { return size_; }
	Rect GetRect() const { return MakeRect(pos_, size_); }
	void Damage(float amount);

protected:
	Vector2 pos_{};
	Vector2 size_{};
	Vector2 velocity_{};
	Vector2 acceleration_{};

	float hp_;
};