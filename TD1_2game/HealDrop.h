#pragma once
#include "Vector2.h"
#include "Collision.h"

// 敵に攻撃を当てたときに落ちる、リジェネ回復のアイテム
class HealDrop
{
public:
	// centerPos: 出現する位置(中心)
	explicit HealDrop(Vector2 centerPos);

	void Update();            // 毎フレーム呼ぶ(落下・消滅の管理)
	void Collect();           // 拾われた(消える)

	bool IsActive() const { return isActive_; }
	bool ShouldDraw() const;  // 消える直前は点滅させる(false のフレームは描画しない)

	const Vector2& Pos() const { return pos_; }
	const Vector2& Size() const { return size_; }
	Rect GetRect() const { return MakeRect(pos_, size_); }

private:
	Vector2 pos_{};        // 左上(Y上向き)
	Vector2 size_{ 30.0f, 30.0f };
	Vector2 velocity_{};
	bool isActive_ = true;
	bool isOnGround_ = false;
	int life_;             // 消えるまでの残りフレーム

	static constexpr float kGravity = 0.8f;
	static constexpr float kGroundY = 0.0f;
	static constexpr float kPopSpeedX = 4.0f;    // 出現時の横方向のばらつき
	static constexpr float kPopSpeedY = 10.0f;   // 出現時に跳ね上がる速さ
	static constexpr int kLifeFrames = 600;      // 地面に出てから消えるまで(10秒)
	static constexpr int kBlinkFrames = 120;     // 消える前に点滅する時間
};