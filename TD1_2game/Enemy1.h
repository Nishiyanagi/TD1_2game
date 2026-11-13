#pragma once
#include "Enemy.h"
#include "Cooldown.h"

class Enemy1 :
	public Enemy
{
public:
	Enemy1(Vector2 position, Vector2 size, Vector2 velocity, float hp);

	// targetPos: プレイヤーの中心座標
	void Move(const Vector2& targetPos);

	// 今、プレイヤーに接触ダメージを与える状態か
	bool IsAttacking() const;

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


	void StartChaseJump(const Vector2& targetPos, bool isBig);
	void StartSlam();
	void ClampToScreen();

	State state_ = State::Idle;
	bool isFirstAction_ = true;        // 最初の行動かどうか(最初は必ず小ジャンプ)
	bool isBigJump_ = false;           // 今のジャンプが大ジャンプか
	Cooldown idleCooldown_{ 40 };      // 攻撃前の待ち
	Cooldown landCooldown_{ 20 };      // 小ジャンプ着地後の硬直
	Cooldown bigLandCooldown_{ 35 };   // 大ジャンプ着地後の硬直
	Cooldown trackCooldown_{ 90 };     // 上昇+追尾の時間
	Cooldown waitCooldown_{ 20 };      // 落下前の停止時間
	Cooldown slamLandCooldown_{ 50 };  // 落下着地後の硬直

	// 共通
	static constexpr float kGravity = 0.8f;
	static constexpr float kGroundY = 0.0f;        // 地面の高さ(Y上向き)
	static constexpr float kScreenWidth = 1920.0f;

	// 追尾ジャンプ(小)
	static constexpr float kSmallJumpY = 16.0f;         // 初速(上向き)
	static constexpr float kSmallMaxSpeedX = 10.0f;     // 横速度の上限

	// 追尾ジャンプ(大)
	static constexpr float kBigJumpY = 24.0f;
	static constexpr float kBigMaxSpeedX = 16.0f;

	// 行動の確率(%)。残りが小ジャンプ
	static constexpr int kSlamChance = 30;              // 頭上落下
	static constexpr int kBigJumpChance = 25;           // 大ジャンプ

	// 頭上落下攻撃
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