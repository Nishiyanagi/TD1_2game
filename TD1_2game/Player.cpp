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
	// 吹っ飛びから着地するまではガードできない
	if (isLaunchLocked_) {
		return isDefence_ = false;
	}
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
}

void Player::ClampX(float minX, float maxX)
{
	if (pos_.x < minX) { pos_.x = minX; if (isLaunched_) { launchVel_.x = 0.0f; } }
	if (pos_.x + size_.x > maxX) { pos_.x = maxX - size_.x; if (isLaunched_) { launchVel_.x = 0.0f; } }

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
	const bool isPressed = InputDevice::IsPressMouse(0) || InputDevice::IsPressButton(kPadButton15) || InputDevice::IsPressButton(kPadButton9);

	// 吹っ飛びから着地するまでは攻撃できない。押しっぱなしのまま着地しても、いったん離すまでは始まらない
	if (isLaunchLocked_) {
		attackTimer_ = 0.0f;
		isHoldExpired_ = isPressed;
		return isAttack_ = false;
	}

	if (isPressed) {
		// 長押しの時間切れのあとは、いったんボタンを離すまで攻撃しない
		if (isHoldExpired_) {
			return isAttack_ = false;
		}

		// 長押しの時間が来た(1回だけ)。武器を戻し始める合図を出す
		if (attackTimer_ >= kHoldMaxFrames) {
			isHoldExpired_ = true;
			isHoldTimeUp_ = true;
			attackTimer_ = 0;
			return isAttack_ = false;
		}

		return isAttack_ = true;
	}

	// ボタンを離している
	isHoldExpired_ = false;

	// 短押しで離した → 飛ばす攻撃
	if (attackTimer_ >= kSingleAttackMinFrames && attackTimer_ <= kSingleAttackMaxFrames) {
		GetCursorDirection(weaponPos, mouseDirection);
		isSingleAttack_ = true;
	}
	attackTimer_ = 0;
	return isAttack_ = false;
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

bool Player::ConsumeHoldTimeUp() {
	const bool result = isHoldTimeUp_;
	isHoldTimeUp_ = false;
	return result;
}

void Player::StartLaunch(Vector2 direction)
{
	float length = sqrtf(direction.x * direction.x + direction.y * direction.y);
	if (length < 0.001f) {
		// 真上から重なった等で向きが取れないときは、向いている方向の逆へ
		direction = { isRight_ ? -1.0f : 1.0f, 0.0f };
		length = 1.0f;
	}
	launchVel_ = { direction.x / length * kLaunchSpeed, direction.y / length * kLaunchSpeed };
	isLaunched_ = true;
	isLaunchLocked_ = true;
	launchFrames_ = 0;

	// 攻撃入力は打ち切り。ボタンを離すまで次の攻撃はしない
	attackTimer_ = 0.0f;
	isHoldExpired_ = true;
	isAttack_ = false;
}

void Player::UpdateLaunch(const char* keys, Pads* pad)
{
	const float groundY = line->StartPos().y + size_.y;

	if (!isLaunched_) {
		// 吹っ飛びが終わったあと、空中にいる間はロックを続ける。着地したら解除
		if (isLaunchLocked_ && pos_.y <= groundY + 0.01f) {
			isLaunchLocked_ = false;
		}
		return;
	}
	launchFrames_++;

	// 吹っ飛び中の操作(キー or 左スティック)
	InputDevice::GetAnalogInputLeft(pad->Lx(), pad->Ly());
	float steerX = static_cast<float>(*pad->Lx()) / 32768.0f;
	float steerY = static_cast<float>(*pad->Ly()) / -32768.0f;   // スティックは上が負なので反転
	if (keys[DIK_D]) { steerX += 1.0f; }
	if (keys[DIK_A]) { steerX -= 1.0f; }
	if (keys[DIK_W]) { steerY += 1.0f; }
	if (keys[DIK_S]) { steerY -= 1.0f; }
	launchVel_.x += steerX * kLaunchAirControl;
	launchVel_.y += steerY * kLaunchAirControl;

	// 慣性
	launchVel_.y -= kLaunchGravity;
	launchVel_.x *= kLaunchDrag;
	launchVel_.y *= kLaunchDrag;

	pos_.x += launchVel_.x;
	pos_.y += launchVel_.y;

	// 地面
	if (pos_.y <= groundY) {
		pos_.y = groundY;
		if (launchVel_.y < 0.0f) { launchVel_.y = 0.0f; }
		launchVel_.x *= kLaunchGroundFriction;
	}

	if (launchVel_.x > 1.0f) { isRight_ = true; }
	else if (launchVel_.x < -1.0f) { isRight_ = false; }

	// 終了判定
	const float speed = sqrtf(launchVel_.x * launchVel_.x + launchVel_.y * launchVel_.y);
	if (speed < kLaunchEndSpeed || launchFrames_ >= kLaunchMaxFrames) {
		isLaunched_ = false;
		jumpCount_ = 0.0f;
		velocity_.y = launchVel_.y;       // 空中で終わったら、通常のジャンプ処理に落下を引き継ぐ
		isJump_ = pos_.y > groundY;
		if (!isJump_) { isLaunchLocked_ = false; }   // 地面で終わったならそのままロック解除
	}
}

bool Player::IsLaunched() const
{
	return isLaunched_;
}