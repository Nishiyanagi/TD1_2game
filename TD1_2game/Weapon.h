#pragma once
#include "Vector2.h"

class Weapon
{
public:
	Weapon(Vector2 pos, Vector2 size);

	void SetActive(bool isActive);

	Vector2 Pos();//場所確認
	Vector2 Size();//大きさ確認
	bool IsActive();//アクティブ確認
private:
	Vector2 pos_{};
	Vector2 size_{};
	bool isActive_ = false;
};

