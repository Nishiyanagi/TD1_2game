#include "Enemy.h"
#include <stdio.h>


Enemy::Enemy(Vector2 position, Vector2 size, Vector2 velocity, float hp)
	: pos_{ position },
	size_{ size },
	velocity_{ velocity },
	hp_{ int(hp) } {
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
