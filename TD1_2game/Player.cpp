#include "Player.h"
#include "Line.h"
#include "Vector2.h"
#include "Camera.h"
#include "Pad.h"
#include "InputDevice.h"
#include <math.h>
#include <Novice.h>

Player::Player(Vector2 position, Vector2 speed, Vector2 range, float hp, Camera* camera)
	:pos_{ position },
	velocity_{ speed },
	size_{ range },
	acceleration_{ 0.0f,0.5f },
	hp_{ hp },
	maxHp_{ hp },
	camera_{ camera }
{
}

bool Player::Defence() {
	if (InputDevice::IsPressMouse(1) || InputDevice::IsPressButton(kPadButton11)) {
		return isDefence_ = true;
	}
	else {
		return isDefence_ = false;
	}
}
void Player::Jump(const char* keys, const char* preKeys, Vector2& speed) {

	if ((keys[DIK_SPACE] && !preKeys[DIK_SPACE] || InputDevice::IsTriggerButton(kPadButton10)) && !Player::Defence()) {
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

	InputDevice::GetAnalogInputLeft(pad->Lx(), pad->Ly());//Lスティックが入力された時の値を返す関数
	// パッド操作中も、スティックを倒した向きに合わせて向き(isRight_)を更新する
	if (*pad->Lx() > 8000) { isRight_ = true; }
	else if (*pad->Lx() < -8000) { isRight_ = false; }
	speed.x *= static_cast<float>(*pad->Lx()) / 32768;
	pos_.x += speed.x;

	if (keys[DIK_S]) {
		//足場を降りる処理
	}
		//足場を降りる処理
	}
}

void Player::ClampX(float minX, float maxX)
{
	if (pos_.x < minX) { pos_.x = minX; }
	if (pos_.x + size_.x > maxX) { pos_.x = maxX - size_.x; }

}

void Player::ClampX(float minX, float maxX)
{
	if (pos_.x < minX) { pos_.x = minX; }
	if (pos_.x + size_.x > maxX) { pos_.x = maxX - size_.x; }

}

void Player::ClampX(float minX, float maxX)
{
	if (pos_.x < minX) { pos_.x = minX; }
	if (pos_.x + size_.x > maxX) { pos_.x = maxX - size_.x; }

}

void Player::Show()
{
	Novice::ScreenPrintf(0, 40, "AttackTimer:%f", attackTimer_);
}

bool Player::WeaponHit() {

	return false;
}

void Player::Heal(float amount)
{
	if (amount <= 0.0f || hp_ <= 0.0f) { return; }   // HP0(やられた後)は回復しない
	hp_ += amount;
	if (hp_ > maxHp_) { hp_ = maxHp_; }
}

void Player::GetCursorDirection(Vector2 Pos, Vector2& mouseDirection) {
	// パッド操作中はマウスを使わず、左スティックの向きで狙う(倒していなければ向いている方向)
	if (InputDevice::UsePad()) {
		constexpr float kStickMax = 32768.0f;
		constexpr float kDeadZone = 0.3f;     // これより小さい入力は「倒していない」とみなす
		constexpr float kAimLength = 300.0f;  // 方向ベクトルの長さ(マウス操作での、プレイヤーからカーソルまでの距離に相当)

		int lx = 0;
		int ly = 0;
		InputDevice::GetAnalogInputLeft(&lx, &ly);

		float x = static_cast<float>(lx) / kStickMax;
		float y = static_cast<float>(ly) / -kStickMax;   // スティックは上が負なので、ワールド(Y上向き)に合わせて反転
		const float length = sqrtf(x * x + y * y);

		if (length < kDeadZone) {
			x = isRight_ ? 1.0f : -1.0f;
			y = 0.0f;
		}
		else {
			x /= length;
			y /= length;
		}

		mouseDirection = { x * kAimLength, y * kAimLength };
		return;
	}

	int mousePositionX{};
	int mousePositionY{};
	Vector2 playerInversePos{};
	Novice::GetMousePosition(&mousePositionX, &mousePositionY);//マウスのポジション取得
	playerInversePos = { camera_->ScreenToWorld(Pos) };

	mouseDirection.x = (static_cast<float>(mousePositionX) - playerInversePos.x) - size_.x * 0.5f;
	mouseDirection.y = ((static_cast<float>(mousePositionY) - playerInversePos.y) * -1 + size_.y * 0.5f);

	Novice::ScreenPrintf(200, 0, "mouceDirection.x:%.1f", mouseDirection.x);
	Novice::ScreenPrintf(200, 20, "mouceDirection.y:%.1f", mouseDirection.y);
}

bool Player::Attack(Vector2 weaponPos, Vector2& mouseDirection) {
	if (InputDevice::IsPressMouse(0) || InputDevice::IsPressButton(kPadButton15) || InputDevice::IsPressButton(kPadButton9)) {
		if (attackTimer_ >= 300)
		{
			attackTimer_ = 0;
			return isAttack_ = false, isSingleAttack_ = false;
		}
		else
		{
			return isAttack_ = true;
		}
	}
	else {
		if (attackTimer_ >= 2 && attackTimer_ <= 12)
		{
			GetCursorDirection(weaponPos, mouseDirection);
			isSingleAttack_ = true;
		}
		//if () //武器が消える条件
		//{
		//	isSingleAttack_ = false;
		//}
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

void Player::Damage(float amount)
{
	if (amount <= 0.0f) { return; }
	hp_ -= amount;
	if (hp_ < 0.0f) { hp_ = 0.0f; }
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
float  Player::JumpCounte() {//速度確認
	return jumpCounte_;
}
float  Player::HP() {//速度確認
	return hp_;
}

float Player::attackTimer() {//攻撃タイマー増加
	attackTimer_++;
	return attackTimer_;
}
float Player::SetAttackTimer(float times) {//攻撃タイマー代入
	attackTimer_ = times;
	return attackTimer_;
}


bool Player::isRight() {
	return isRight_;
}

bool Player::isSingleAttack() {
	return isSingleAttack_;
}
bool Player::SetSingleAttack(bool isSingleAttack) {

	return isSingleAttack_ = isSingleAttack;
}