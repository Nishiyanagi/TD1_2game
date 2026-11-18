#include "HealDrop.h"
#include "Random.h"
#include "World.h"

HealDrop::HealDrop(Vector2 centerPos)
	: life_{ kLifeFrames }
{
	pos_ = { centerPos.x - size_.x * 0.5f, centerPos.y + size_.y * 0.5f };
	velocity_ = { Random::RangeF(-kPopSpeedX, kPopSpeedX), kPopSpeedY };
}

void HealDrop::Update()
{
	if (!isActive_) { return; }

	if (!isOnGround_) {
		velocity_.y -= kGravity;
		pos_.x += velocity_.x;
		pos_.y += velocity_.y;

		// ステージの外に出ない
		if (pos_.x < 0.0f) { pos_.x = 0.0f; }
		if (pos_.x + size_.x > kWorldWidth) { pos_.x = kWorldWidth - size_.x; }

		// 着地(足元は pos_.y - size_.y)
		if (velocity_.y < 0.0f && pos_.y - size_.y <= kGroundY) {
			pos_.y = kGroundY + size_.y;
			velocity_ = { 0.0f, 0.0f };
			isOnGround_ = true;
		}
	}
	else {
		// 地面に落ちてから時間を数える
		--life_;
		if (life_ <= 0) { isActive_ = false; }
	}
}

void HealDrop::Collect()
{
	isActive_ = false;
}

bool HealDrop::ShouldDraw() const
{
	if (life_ > kBlinkFrames) { return true; }
	return (life_ / 6) % 2 == 0;   // 6フレームごとに表示・非表示
}