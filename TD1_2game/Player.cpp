#include "Player.h"
#include "Line.h"
#include "Vector2.h"
#include "Pad.h"
#include <Novice.h>

Player::Player(Vector2 position, Vector2 speed, Vector2 range, float hp)
	:pos_{ position },
	velocity_{ speed },
	size_{ range },
	acceleration_{ 0.0f,0.5f },
	hp_{ hp }
{
}
//こんにちわ
 
bool Player::Defence() {
	if (Novice::IsPressMouse(1) || Novice::IsPressButton(0, kPadButton11) ) {
		return isDefence_ = true;
	}
	else {
		return isDefence_ = false;
	}
}
void Player::Jump(const char* keys, const char* preKeys, Vector2& speed) {

	if ((keys[DIK_SPACE] && !preKeys[DIK_SPACE] || Novice::IsTriggerButton(0, kPadButton10))&& !Player::Defence()) {
		isJump_ = true;
		jumpCount_++;
		speed.y = 10.0f * (1.0f - jumpCount_ / 15);
	}
	if (isJump_) {

		if ((pos_.y < line->StartPos().y + size_.y || pos_.y < line->EndPos().y + size_.y)) {
			pos_.y = line->StartPos().y + size_.y;
			speed.y = 0;
			jumpCount_ = 0;
		}
		else {
			pos_.y += speed.y;
			speed.y -= acceleration_.y;
		}
	}
}
void Player::Move(const char* keys, Vector2 speed, Pads* pad) {
	if (keys[DIK_D]) {
		pos_.x += speed.x;
		isRight_ = true;
	}
	if (keys[DIK_A]) {
		pos_.x -= speed.x;
		isRight_ = false;
	}

Novice::GetAnalogInputLeft(0, pad->Lx(), pad->Ly());//Lスティックが入力された時の値を返す関数
	speed.x *= static_cast<float>(*pad->Lx()) / 32768;
	pos_.x += speed.x;

	if (keys[DIK_S]) {
		//足場を降りる処理
	}
}

void Player::Show()
{
	Novice::ScreenPrintf(0, 40, "AttackTimer:%f", attackTimer_);
}

bool Player::WeaponHit() {

	return false;
}

bool Player::Attack(Vector2 weaponPos, Vector2 weaponSize, float screenWidth, float screenHeight) {
	if (Novice::IsPressMouse(0) || Novice::IsPressButton(0, kPadButton15) || Novice::IsPressButton(0, kPadButton9)) {
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
		if (attackTimer_ >= 2 && attackTimer_ <= 6)
		{
			isSingleAttack_ = true;
		}
		if (weaponPos.x >= screenWidth + weaponSize.x || weaponPos.x <= -weaponSize.x ||
			weaponPos.y >= screenHeight + weaponSize.y || weaponPos.y <= -weaponSize.y)
		{
			isSingleAttack_ = false;
		}
		attackTimer_ = 0;
		return isAttack_ = false;
	}
}

void Player::Damage(float amount)
{
	if (amount <= 0.0f) { return; }
	hp_ -= amount;
	if (hp_ < 0.0f) { hp_ = 0.0f; }
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
float  Player::JumpCount() {//ジャンプ回数確認
	return jumpCount_;
}
float  Player::HP() {//HP確認
	return hp_;
}

float Player::attackTimer() {//攻撃タイマー増加
	attackTimer_++;
	return attackTimer_;
}

bool Player::isRight() {
	return isRight_;
}

bool Player::isSingleAttack() {
	return isSingleAttack_;
}
