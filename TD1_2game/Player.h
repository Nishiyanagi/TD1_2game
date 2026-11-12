#pragma once
#include "Vector2.h"

enum Actions {
	NONE = 0,
	ATTACK = 1,
	DEFENCE = 2
};
class Player {
public:

	Player(Vector2 position, Vector2 speed, Vector2 range);
	void Jump(const char* keys, const char* preKeys, Vector2& speed);
	void Move(const char* keys, Vector2 speed);
	bool Attack();
	bool Defence();


	Vector2 Pos();//場所確認
	Vector2 Size();//大きさ確認
	Vector2* Velocity();//速度確認
	float JumpCounte();//速度確認

private:
	Vector2 pos_{};
	Vector2 size_{};
	Vector2 velocity_{};
	Vector2 acceleration_{};

	float jumpCounte_{};

	bool isJump_ = false;
	bool isAttack_ = false;
	bool isDefece_ = false;

};
