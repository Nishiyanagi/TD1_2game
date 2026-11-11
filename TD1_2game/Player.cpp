#include "Player.h"
#include "Vector2.h"
#include <Novice.h>

Player::Player(Vector2 position, Vector2 speed, Vector2 range)
	:pos_{ position },
	velocity_{ speed },
	size_{ range } {
}
//こんにちわ
float Player::Jump(const char* keys, const char* preKeys, Vector2 speed) {
	float jumpSoeed = 0.0f;
	if (keys[DIK_SPACE] && !preKeys[DIK_SPACE]) {

		jumpCounte_++;
		jumpSoeed = speed.y * (1.0f - jumpCounte_ / 10);//とんだ回数分ジャンプ力が下がるやつ
	
	}
	

	return jumpSoeed;
}
void Player::Move(const char* keys, Vector2 speed) {
	
	if (keys[DIK_D]) {

	}
	if (keys[DIK_A]) {

	}
	if (keys[DIK_S]) {

	}
}

Vector2 Pos();//場所確認
Vector2 Size();//大きさ確認
Vector2 Velocity();//速度確認


