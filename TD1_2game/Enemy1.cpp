#include "Enemy1.h"
#include "Random.h"

namespace {
	float Clamp(float value, float min, float max)
	{
		if (value < min) { return min; }
		if (value > max) { return max; }
		return value;
	}
}

Enemy1::Enemy1(Vector2 position, Vector2 size, Vector2 velocity, float hp)
	: Enemy{ position, size, velocity, hp }
{
	idleCooldown_.Start();
}

void Enemy1::Move(const Vector2& targetPos)
{
	const float centerX = pos_.x + size_.x * 0.5f;

	switch (state_) {
	case State::Idle:
		idleCooldown_.Update();
		if (idleCooldown_.IsReady()) {
			if (isFirstAction_) {
				// 最初は必ず小ジャンプ
				isFirstAction_ = false;
				StartChaseJump(targetPos, false);
			}
			else {
				// 0〜99 の値で振り分ける
				const int roll = Random::Range(0, 99);
				if (roll < kSlamChance) {
					StartSlam();
				}
				else if (roll < kSlamChance + kBigJumpChance) {
					StartChaseJump(targetPos, true);
				}
				else {
					StartChaseJump(targetPos, false);
				}
			}
		}
		break;

	case State::Jump:
		velocity_.y -= kGravity;
		pos_.x += velocity_.x;
		pos_.y += velocity_.y;
		ClampToScreen();

		// 着地判定(足元は pos_.y - size_.y)
		if (velocity_.y < 0.0f && pos_.y - size_.y <= kGroundY) {
			pos_.y = kGroundY + size_.y;
			velocity_ = { 0.0f, 0.0f };
			state_ = State::Land;
			if (isBigJump_) { bigLandCooldown_.Start(); }
			else { landCooldown_.Start(); }
		}
		break;

	case State::Land:
	{
		Cooldown& cooldown = isBigJump_ ? bigLandCooldown_ : landCooldown_;
		cooldown.Update();
		if (cooldown.IsReady()) {
			idleCooldown_.Start();
			state_ = State::Idle;
		}
		break;
	}

	case State::SlamTrack:
	{
		// 目標速度: 目標までの距離に比例(近づくほど遅くなる)
		const float targetVx = Clamp((targetPos.x - centerX) * kTrackGain, -kTrackSpeed, kTrackSpeed);
		const float targetVy = Clamp((kSlamHeight - pos_.y) * kRiseGain, 0.0f, kRiseSpeed);

		// 現在の速度を目標速度へ少しずつ近づける(加速・減速がなめらかになる)
		velocity_.x += (targetVx - velocity_.x) * kBlend;
		velocity_.y += (targetVy - velocity_.y) * kBlend;

		// 縦と横を同時に動かすので、斜めの弧を描いて頭上へ向かう
		pos_.x += velocity_.x;
		pos_.y += velocity_.y;
		ClampToScreen();

		trackCooldown_.Update();
		if (trackCooldown_.IsReady()) {
			waitCooldown_.Start();
			state_ = State::SlamWait;
		}
		break;
	}

	case State::SlamWait:
		// 急停止せず、速度を落としながら止まる
		velocity_.x *= kBrake;
		velocity_.y *= kBrake;
		pos_.x += velocity_.x;
		pos_.y += velocity_.y;
		ClampToScreen();

		waitCooldown_.Update();
		if (waitCooldown_.IsReady()) {
			velocity_ = { 0.0f, 0.0f };
			state_ = State::SlamFall;
		}
		break;

	case State::SlamFall:
		// 加速しながら落下
		velocity_.y = Clamp(velocity_.y - kFallAccel, -kFallSpeed, kFallSpeed);
		pos_.y += velocity_.y;
		if (pos_.y - size_.y <= kGroundY) {
			pos_.y = kGroundY + size_.y;
			velocity_ = { 0.0f, 0.0f };
			slamLandCooldown_.Start();
			state_ = State::SlamLand;
		}
		break;

	case State::SlamLand:
		slamLandCooldown_.Update();
		if (slamLandCooldown_.IsReady()) {
			idleCooldown_.Start();
			state_ = State::Idle;
		}
		break;
	}
}

void Enemy1::StartChaseJump(const Vector2& targetPos, bool isBig)
{
	isBigJump_ = isBig;

	const float jumpY = isBig ? kBigJumpY : kSmallJumpY;
	const float maxSpeedX = isBig ? kBigMaxSpeedX : kSmallMaxSpeedX;

	const float centerX = pos_.x + size_.x * 0.5f;
	const float dx = targetPos.x - centerX;

	// 滞空フレーム数の目安(上がって同じ高さに戻るまで)
	const float airTime = 2.0f * jumpY / kGravity - 1.0f;

	velocity_.x = Clamp(dx / airTime, -maxSpeedX, maxSpeedX);
	velocity_.y = jumpY;
	state_ = State::Jump;
}

void Enemy1::StartSlam()
{
	velocity_ = { 0.0f, 0.0f };
	trackCooldown_.Start();
	state_ = State::SlamTrack;
}

void Enemy1::ClampToScreen()
{
	pos_.x = Clamp(pos_.x, 0.0f, kScreenWidth - size_.x);
}