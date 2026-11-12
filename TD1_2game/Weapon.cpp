#include "Weapon.h"

Weapon::Weapon(Vector2 pos, Vector2 size)
	: pos_(pos), size_(size)
{

}

void Weapon::SetActive(bool isActive)
{
	isActive_ = isActive;
}

Vector2 Weapon::Pos()
{
	return pos_;
}

Vector2 Weapon::Size()
{
	return size_;
}

bool Weapon::IsActive()
{
	return isActive_;
}
