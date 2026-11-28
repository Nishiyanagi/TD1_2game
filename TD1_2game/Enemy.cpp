#include "Enemy.h"
#include "World.h"
#include <stdio.h>


Enemy::Enemy(Vector2 position, Vector2 size, Vector2 velocity, float hp, int graphHandle)
	: pos_{ position },
	size_{ size },
	velocity_{ velocity },
	hp_{ float(hp) },
	graphHandle_{ graphHandle} {
}

void Enemy::Damage(float amount)
{
	if (amount <= 0.0f) { return; }
	if (amount >= hp_) {
		hp_ = 0.0f;
	}
	else {
		hp_ -= amount;
	}
}

void Enemy::Knockback(float velocityX)
{
	knockVelX_ = velocityX;
}

void Enemy::ApplyKnockback()
{
	if (knockVelX_ == 0.0f) { return; }

	pos_.x += knockVelX_;
	knockVelX_ *= kKnockbackDamping;
	if (knockVelX_ > -0.1f && knockVelX_ < 0.1f) { knockVelX_ = 0.0f; }

	// ステージの外に出ない
	if (pos_.x < 0.0f) { pos_.x = 0.0f; }
	if (pos_.x + size_.x > kWorldWidth) { pos_.x = kWorldWidth - size_.x; }
}