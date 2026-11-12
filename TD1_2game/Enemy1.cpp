#include "Enemy1.h"
#include "Random.h"

Enemy1::Enemy1(Vector2 position, Vector2 size, Vector2 velocity, int hp)
	: Enemy{ position, size, velocity, hp }
{
	direction_ = Random::Chance(50) ? 1.0f : -1.0f;
	idleCooldown_.Start();
}

void Enemy1::Move()
{
	switch (state_) {
	case State::Idle:
		idleCooldown_.Update();
		if (idleCooldown_.IsReady()) {
			// 画面端に近いときは中央へ向かう
			if (pos_.x < size_.x * 2.0f) {
				direction_ = 1.0f;
			}
			else if (pos_.x + size_.x > kScreenWidth - size_.x * 2.0f) {
				direction_ = -1.0f;
			}

			// 小ジャンプ or 大ジャンプ
			bool isBig = Random::Chance(kBigJumpChance);
			velocity_.x = direction_ * (isBig ? kBigJumpX : kSmallJumpX);
			velocity_.y = isBig ? kBigJumpY : kSmallJumpY;
			state_ = State::Jump;
		}
		break;

	case State::Jump:
		velocity_.y -= kGravity;
		pos_.x += velocity_.x;
		pos_.y += velocity_.y;

		// 壁で跳ね返る
		if (pos_.x < 0.0f) {
			pos_.x = 0.0f;
			velocity_.x *= -1.0f;
			direction_ = 1.0f;
		}
		else if (pos_.x + size_.x > kScreenWidth) {
			pos_.x = kScreenWidth - size_.x;
			velocity_.x *= -1.0f;
			direction_ = -1.0f;
		}

		// 着地判定(posは左上なので、足元は pos_.y - size_.y)
		if (velocity_.y < 0.0f && pos_.y - size_.y <= kGroundY) {
			pos_.y = kGroundY + size_.y;
			velocity_ = { 0.0f, 0.0f };
			state_ = State::Land;
			landCooldown_.Start();
		}
		break;

	case State::Land:
		landCooldown_.Update();
		if (landCooldown_.IsReady()) {
			// 次のジャンプの向きをランダムに決める
			if (Random::Chance(30)) { direction_ *= -1.0f; }
			idleCooldown_.Start();
			state_ = State::Idle;
		}
		break;
	}
}