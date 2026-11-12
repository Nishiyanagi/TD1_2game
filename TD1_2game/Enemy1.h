#pragma once
#include "Enemy.h"
#include "Cooldown.h"

class Enemy1 :
	public Enemy
{
public:
	Enemy1(Vector2 position, Vector2 size, Vector2 velocity, int hp);

	// targetPos: プレイヤーの中心座標
	void Move(const Vector2& targetPos);

private:
	enum class State {
		Idle,       // 待機(溜め)
		Jump,       // 追尾ジャンプ中
		Land,       // ジャンプ着地の硬直
		SlamTrack,  // 頭上攻撃: 上昇しながらプレイヤーの真上へ移動
		SlamWait,   // 頭上攻撃: 落下前の減速・停止(予兆)
		SlamFall,   // 頭上攻撃: 落下
		SlamLand,   // 頭上攻撃: 着地の大きな硬直
	};

	void StartChaseJump(const Vector2& targetPos);
	void StartSlam();
	void ClampToScreen();

	State state_ = State::Idle;
	Cooldown idleCooldown_{ 40 };      // 攻撃前の待ち
	Cooldown landCooldown_{ 20 };      // ジャンプ着地後の硬直
	Cooldown trackCooldown_{ 90 };     // 上昇+追尾の時間
	Cooldown waitCooldown_{ 20 };      // 落下前の停止時間
	Cooldown slamLandCooldown_{ 50 };  // 落下着地後の硬直

	// 共通
	static constexpr float kGravity = 0.8f;
	static constexpr float kGroundY = 0.0f;        // 地面の高さ(Y上向き)
	static constexpr float kScreenWidth = 1920.0f;

	// 追尾ジャンプ
	static constexpr float kJumpY = 22.0f;         // ジャンプの初速(上向き)
	static constexpr float kMaxJumpSpeedX = 14.0f; // 横速度の上限

	// 頭上落下攻撃
	static constexpr int kSlamChance = 30;         // 頭上攻撃を選ぶ確率(%)
	static constexpr float kSlamHeight = 500.0f;   // 滞空する高さ(posのY)
	static constexpr float kRiseSpeed = 30.0f;     // 上昇速度の上限
	static constexpr float kRiseGain = 0.08f;      // 目標高さまでの距離に対する速度の割合
	static constexpr float kTrackSpeed = 14.0f;    // 横追尾速度の上限
	static constexpr float kTrackGain = 0.08f;     // プレイヤーまでの距離に対する速度の割合
	static constexpr float kBlend = 0.12f;         // 速度の変化のなめらかさ(小さいほど滑らか)
	static constexpr float kBrake = 0.75f;         // 予兆中の減速率(毎フレーム掛ける)
	static constexpr float kFallAccel = 4.0f;      // 落下の加速
	static constexpr float kFallSpeed = 40.0f;     // 落下速度の上限
};