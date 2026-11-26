#pragma once
#include "Vector2.h"

class Weapon
{
public:
	enum class Phase {
		Idle,    // 投げていない
		Out,     // 往路(弧を描いて飛んでいく)
		Return,  // 復路(プレイヤーへ戻ってくる)
	};

	Weapon(Vector2 pos, Vector2 size, Vector2 velocity);

	// ---- ブーメラン ----
	// originCenter: 投げる位置(プレイヤーの中心) / direction: 狙う方向(長さは何でもよい)
	void Throw(const Vector2& originCenter, const Vector2& direction);
	// 毎フレーム呼ぶ。playerCenter は復路の目標。キャッチした「そのフレームだけ」true を返す
	bool UpdateBoomerang(const Vector2& playerCenter);
	// 今の往路/復路でまだ当たっていなければ true を返し、「当たった」を記録する(多段ヒット防止)
	// 当たったとき、敵の中心から離れる向きに少し跳ね返る(反動)。enemyCenter: 敵の中心
	bool TryHit(const Vector2& enemyCenter);

	bool IsFlying() const { return phase_ != Phase::Idle; }
	bool IsReturning() const { return phase_ == Phase::Return; }

	Vector2 Pos();//場所確認
	Vector2 Size();//大きさ確認
	Vector2 Velocity();//速度確認	
	bool IsActive();//アクティブ確認
private:
	Vector2 CenterToPos(const Vector2& center) const;
	float DecideTurnDir(const Vector2& playerCenter) const;   // 折り返しで曲がる向きを決める(+1: 左回り / -1: 右回り)
	void StepReturn(float stepLen, const Vector2& target);     // 復路を stepLen だけ進める(向きを少しずつ target へ回す)

	Vector2 pos_{};   // 左上(Y上向き)
	Vector2 size_{};
	Vector2 velocity_{};
	bool isActive_ = false;

	// ブーメラン用
	Phase phase_ = Phase::Idle;
	float speed_ = 0.0f;          // 今の速さ(px/フレーム)。投げた直後が最大で、少しずつ遅くなる
	bool hitThisPhase_ = false;   // 今のフェーズで当てたか
	Vector2 origin_{};            // 投げた位置(中心)
	Vector2 apex_{};              // 折り返し地点(中心)
	Vector2 dir_{ 1.0f, 0.0f };   // 正規化した投げ方向
	Vector2 retPos_{};            // 復路の現在位置(中心。反動のずれは含まない)
	float heading_ = 0.0f;        // 復路の進行方向(ラジアン)
	float turnDir_ = 1.0f;        // 折り返しで曲がる向き(+1: 左回り / -1: 右回り)。復路に入るときに1回だけ決める
	int returnFrames_ = 0;        // 復路に入ってからの経過フレーム
	Vector2 recoilOffset_{};      // 敵に当たった反動で、本来の軌道からずれている量
	Vector2 recoilVel_{};         // 反動の速度
	float outTravelled_ = 0.0f;   // 往路(今の直線)で進んだ距離(px)
	float outLength_ = 0.0f;      // 往路(今の直線)の長さ(px)。地面で跳ね返ると、残りの距離になる
	bool touchingGround_ = false; // 前フレームで地面に触れていたか

	static constexpr float kStartSpeed = 24.0f;    // 投げた直後の速さ(px/フレーム)
	static constexpr float kMinSpeed = 18.0f;      // これ以下には遅くならない速さ(px/フレーム)
	static constexpr float kDeceleration = 0.5f;   // 1フレームごとに遅くなる量(px/フレーム)
	static constexpr float kRange = 700.0f;      // 飛ぶ距離
	static constexpr float kTurnRadius = 160.0f; // 折り返しの丸さ(曲がる半径px。大きいほど大きく丸く曲がる)
	static constexpr float kMinTurnRadius = 12.0f; // プレイヤーのすぐ近くで曲がるときの、半径の下限
	static constexpr float kCommitAngle = 2.4f;    // プレイヤーがこれ以上後ろ(ラジアン)にいる間は、決めた向きで曲がり続ける
	static constexpr int kMaxReturnFrames = 600;   // 復路がこれ以上続いたら、強制的にキャッチ(安全装置)
	static constexpr float kCatchDistance = 32.0f; // 復路でこの距離まで近づいたらキャッチ
	static constexpr float kGroundY = 0.0f;           // 地面の高さ(Y上向き)
	static constexpr float kRecoilSpeed = 16.0f;      // 敵に当たったときの反動の初速
	static constexpr float kRecoilDamping = 0.78f;    // 反動の速度の減衰(小さいほどすぐ止まる)
	static constexpr float kRecoilSpring = 0.9f;      // ずれが軌道へ戻る強さ(小さいほど早く戻る)
	static constexpr float kGroundHop = 14.0f;        // 復路で地面に触れたときの跳ね上がり
	static constexpr float kHorizontalBulgeThreshold = 0.3f; // 投げた方向が横寄りか上下寄りかの境目(垂直方向のX成分の大きさ)
};