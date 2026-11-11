#include "Player.h"
#include "Vector2.h"
#include <Novice.h>

Player::Player(Vector2 position, Vector2 speed, Vector2 range)
	:pos_{ position },
	velocity_{ speed },
	size_{ range } {
}
//こんにちわ
void Player::Jump(const char* keys, const char* preKeys, Vector2 speed) {
	
	if (keys[DIK_SPACE] && !preKeys[DIK_SPACE]) {

		jumpCounte_++;
		pos_.y += speed.y * (1.0f - jumpCounte_ / 10);//とんだ回数分ジャンプ力が下がるやつ
	
	}
	

}
void Player::Move(const char* keys, Vector2 speed) {
	
	if (keys[DIK_D]) {
		pos_.x += speed.x;
	}
	if (keys[DIK_A]) {
		pos_.x -= speed.x;
	}
	if (keys[DIK_S]) {
		pos_.y -= speed.y;
	}
}

Vector2 Player::Pos() {//場所確認
	return pos_;
}
Vector2 Player::Size(){//大きさ確認
	return size_;
}

Vector2 Player::Velocity() {//速度確認
	return velocity_;
}

