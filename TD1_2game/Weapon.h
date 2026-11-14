#pragma once
#include "Vector2.h"

class Weapon
{
public:
	Weapon(Vector2 pos, Vector2 size, Vector2 velocity);

	void SetActive(bool isActive);
	void Move(const char* keys, const Vector2 velocity);
	void PosReset(const Vector2 pos);

	Vector2 Pos();//場所確認
	Vector2 Size();//大きさ確認
	Vector2 Velocity();//速度確認
	bool IsActive();//アクティブ確認
private:
	Vector2 pos_{};
	Vector2 size_{};
	Vector2 velocity_{};
	bool isActive_ = false;
};

