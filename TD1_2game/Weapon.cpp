#include "Weapon.h"
#include "Vector2.h"
#include "InputDevice.h"
#include "Pad.h"
#include "Player.h"
#include "Theta.h"
#include "InputDevice.h" 
#include <Novice.h>
#define _USE_MATH_DEFINES
#include <math.h>
//ここにthetaいれて、ブーメランの返ってくる挙動を弧にしたい

Weapon::Weapon(Vector2 pos, Vector2 size, Vector2 velocity)
	: pos_(pos), size_(size), velocity_(velocity)
{

}

void Weapon::SetActive(bool isActive)
{
	isActive_ = isActive;
}

void Weapon::Move(const char* keys, Vector2 velocity, Pads* pad)
{
	int n = 0;

	if (n <= 1) {
		if (keys[DIK_D] || keys[DIK_A]) {
			n++;

			if (keys[DIK_D]) {

				pos_.x += velocity.x;
			}
			if (keys[DIK_A]) {
				velocity.x *= -1;
				pos_.x += velocity.x;
			}
		}

		if (keys[DIK_W] || keys[DIK_S]) {
			n++;

			if (keys[DIK_W]) {

				velocity.y *= 1;
				pos_.y += velocity.y;
			}
			if (keys[DIK_S]) {
				velocity.y *= -1;
				pos_.y += velocity.y;
			}
		}

	}
	else if (n > 1) {
		velocity.x *= thetaMove.x;
		velocity.y *= thetaMove.y;

		pos_.x += velocity.x;
		pos_.y += velocity.y;
	}
	InputDevice::GetAnalogInputLeft(pad->Lx(), pad->Ly());//Lスティックが入力された時の値を返す関数
	velocity.x *= static_cast<float>(*pad->Lx()) / 32768;
	velocity.y *= static_cast<float>(*pad->Ly()) / 32768;

	pos_.x += velocity.x;
	pos_.y -= velocity.y;
}

bool Weapon::SingleAttack(Vector2 mouseCursor, Vector2 ScrollPos, Vector2 returnTarget)
{
	// 飛び始め: 飛ばす向きを決める(飛んでいる間は変えない)
	if (throwState_ == ThrowState::None) {
		const float angle = atan2f(mouseCursor.y, mouseCursor.x + ScrollPos.x);
		throwDir_ = { cosf(angle), sinf(angle) };
		traveled_ = 0.0f;
		throwState_ = ThrowState::Outbound;
	}

	bool isFinished = false;

	if (throwState_ == ThrowState::Outbound) {
		// 向きに沿って一定の速さで進む。一定距離を超えたら、戻りに切り替える
		pos_.x += throwDir_.x * kThrowSpeed_;
		pos_.y += throwDir_.y * kThrowSpeed_;
		traveled_ += kThrowSpeed_;

		if (traveled_ >= kThrowDistance_) {
			throwState_ = ThrowState::Returning;
		}
	}
	else {
		isFinished = UpdateReturn(returnTarget/*, ScrollPos*/);
	}

	Novice::ScreenPrintf(500, 40, "weaponPos:%.1f", pos_.x);
	Novice::ScreenPrintf(500, 60, "weaponPos:%.1f", pos_.y);
	Novice::ScreenPrintf(500, 90, "cosAngle:%.1f", throwDir_.x);
	Novice::ScreenPrintf(500, 110, "sinAngle:%.1f", throwDir_.y);
	Novice::ScreenPrintf(500, 130, "throw:%s traveled:%.0f", throwState_ == ThrowState::Returning ? "RETURN" : "OUT", traveled_);

	return isFinished;
}

void Weapon::StartReturn()
{
	throwState_ = ThrowState::Returning;
}

bool Weapon::IsReturning() const
{
	return throwState_ == ThrowState::Returning;
}

bool Weapon::UpdateReturn(Vector2 returnTarget/*, Vector2 ScrollPos*/)
{
	// プレイヤーの今いる位置へ向かって戻る(プレイヤーが動いても追いかける)


	float dx = returnTarget.x - pos_.x;
	float dy = returnTarget.y - pos_.y;
	float distance = sqrtf(dx * dx + dy * dy);

	
	
	if (targetPosTime_ <= 1) {
		float angle = atan2f(dy, dx /*+ ScrollPos.x*/);
		thetaMove = { cosf(angle), sinf(angle) };
	}
	else if (targetPosTime_ >= 25) {
		targetPosTime_ = 0;
	}
	
	targetPosTime_++;
	if (distance <= kReturnSpeed_) {
		// 戻りきった
		pos_ = returnTarget;
		throwState_ = ThrowState::None;
		returnSpeed_ = {kReturnSpeed_,kReturnSpeed_ };
		return true;
	}
	pos_.x += (returnSpeed_.x) * thetaMove.x;
	pos_.y += (returnSpeed_.y) * thetaMove.y;
	Novice::ScreenPrintf(0, 300, "targetPosTime_:%.1f", targetPosTime_);
	return false;

}

void Weapon::PosReset(const Vector2 pos)
{
	pos_ = pos;
	throwState_ = ThrowState::None;
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