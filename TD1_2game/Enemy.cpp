#include "Enemy.h"
#include <stdio.h>


Enemy::Enemy(Vector2 position, Vector2 size, Vector2 velocity, float hp)
	: pos_{ position },
	size_{ size },
	velocity_{ velocity },
	hp_{ hp } {
}

void Enemy::Damage(int amount)
{
	if (amount <= 0) { return; }
	if (amount >= hp_) {
		hp_ = 0;
	}
	else {
		hp_ -= amount;
	}
}
