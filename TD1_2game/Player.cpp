#include "Player.h"
#include "Line.h"
#include "Vector2.h"
#include <Novice.h>

Player::Player(Vector2 position, Vector2 speed, Vector2 range)
	:pos_{ position },
	velocity_{ speed },
	size_{ range },
	acceleration_{ 0.0f,0.5f }
{
}
//こんにちわ
void Player::Jump(const char* keys, const char* preKeys, Vector2& speed) {

	if (keys[DIK_SPACE] && !preKeys[DIK_SPACE]) {

		isJump_ = true;
		jumpCounte_++;
		speed.y = 10.0f * (1.0f - jumpCounte_ / 15);
	}
	if (isJump_) {
		if ((pos_.y < line->StartPos().y + size_.y || pos_.y < line->EndPos().y + size_.y)) {
			pos_.y = line->StartPos().y + size_.y;
			speed.y = 0;
			jumpCounte_ = 0;
		}
		else {
			pos_.y += speed.y;
			speed.y -= acceleration_.y;
		}
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

void Player::Show()
{
	Novice::ScreenPrintf(0, 40, "AttackTimer:%f", attackTimer_);
}

bool Player::Attack() {
	if (Novice::IsPressMouse(0)) {
		if (attackTimer_ >= 300)
		{
			attackTimer_ = 300;
			return isAttack_ = false;
		}
		else
		{
			return isAttack_ = true;
		}
	}
	else {
		attackTimer_ = 0;
		return isAttack_ = false;
	}
}
bool Player::Defence() {
	if (Novice::IsPressMouse(1)) {
		return isDefece_ = true;
	}
	else {
		return isDefece_ = false;
	}
}

Vector2 Player::Pos() {//場所確認
	return pos_;
}
Vector2 Player::Size() {//大きさ確認
	return size_;
}

Vector2* Player::Velocity() {//速度確認
	return &velocity_;
}
float  Player::JumpCounte() {//速度確認
	return jumpCounte_;
}

float  Player::attackTimer() {//速度確認
	attackTimer_++;
	return attackTimer_;
}

