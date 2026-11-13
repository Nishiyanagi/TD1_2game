#include "Weapon.h"
#include <Novice.h>

Weapon::Weapon(Vector2 pos, Vector2 size, Vector2 velocity)
	: pos_(pos), size_(size), velocity_(velocity)
{

}

void Weapon::SetActive(bool isActive)
{
	isActive_ = isActive;
}

void Weapon::Move(const char* keys, const Vector2 velocity)
{
	if (keys[DIK_D]) {
		pos_.x += velocity.x;
	}
	if (keys[DIK_A]) {
		pos_.x -= velocity.x;
	}
	if (keys[DIK_W]) {
		pos_.y += velocity.y;
	}
	if (keys[DIK_S]) {
		pos_.y -= velocity.y;
	}
}

void Weapon::PosReset()
{
	pos_ = Vector2(0.0f, 0.0f);
}

Vector2 Weapon::Pos()
{
	return pos_;
}

Vector2 Weapon::Size()
{
	return size_;
}

Vector2 Weapon::Velocity()
{
	return velocity_;
}

bool Weapon::IsActive()
{
	return isActive_;
}
