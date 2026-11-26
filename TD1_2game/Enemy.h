#pragma once
#include "Vector2.h"
#include "Collision.h"

class Enemy
{
public:
	Enemy(Vector2 position, Vector2 size, Vector2 velocity, float hp, int graphHandle);
	virtual ~Enemy() = default;

	float Hp() const { return hp_; }
	const Vector2& Pos() const { return pos_; }
	const Vector2& Size() const { return size_; }
	const int& GraphHandle() const { return graphHandle_; }
	Rect GetRect() const { return MakeRect(pos_, size_); }
	void Damage(float amount);
	void Knockback(float velocityX);   // 横方向に吹っ飛ばす(正: 右 / 負: 左)。少しずつ減速して止まる

protected:
	void ApplyKnockback();   // 毎フレーム呼ぶ(吹っ飛びの移動と減速)

	Vector2 pos_{};
	Vector2 size_{};
	Vector2 velocity_{};
	Vector2 acceleration_{};

	int graphHandle_{};

	float hp_;
	float knockVelX_ = 0.0f;   // 吹っ飛びの横速度

	static constexpr float kKnockbackDamping = 0.85f;   // 吹っ飛びの減速(小さいほどすぐ止まる)
};