#pragma once
#include "Enemy.h"
#include "Cooldown.h"

class Enemy1 :
	public Enemy
{
public:
	Enemy1(Vector2 position, Vector2 size, Vector2 velocity, int hp);
	void Move();

private:
	enum class State {
		Idle,   // 待機(溜め)
		Jump,   // 空中
		Land,   // 着地の硬直
	};

	State state_ = State::Idle;
	Cooldown idleCooldown_{ 40 };  // ジャンプ前の待ち時間(フレーム)
	Cooldown landCooldown_{ 20 };  // 着地後の硬直(フレーム)
	float direction_ = 1.0f;       // 1:右 / -1:左

	// 調整用パラメータ
	static constexpr float kGravity = 0.8f;
	static constexpr float kGroundY = 0.0f;       // 地面の高さ(Y上向き座標)
	static constexpr float kScreenWidth = 1920.0f;
	static constexpr float kSmallJumpX = 7.0f;
	static constexpr float kSmallJumpY = 18.0f;
	static constexpr float kBigJumpX = 12.0f;
	static constexpr float kBigJumpY = 28.0f;
	static constexpr int kBigJumpChance = 30;     // 大ジャンプの確率(%)
};