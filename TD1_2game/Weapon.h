#pragma once
#include "Vector2.h"
#include "Pad.h"

class Weapon
{
public:
	Weapon(Vector2 pos, Vector2 size, Vector2 velocity);

	void SetActive(bool isActive);
	void Move(const char* keys, const Vector2 velocity, Pads* pad);
	// 短押しで飛ばす攻撃。一定距離まで飛んだら returnTarget(プレイヤーの位置)へ戻る。戻りきったフレームに true を返す
	bool SingleAttack(Vector2 mouseCursor, Vector2 ScrollPos, Vector2 returnTarget);

	// 戻る動き(飛ばす攻撃の戻りと、長押しの時間切れで共通)
	void StartReturn();                        // 今いる位置から、プレイヤーへ戻り始める
	bool IsReturning() const;                  // 戻っている最中か
	bool UpdateReturn(Vector2 returnTarget/*, Vector2 ScrollPos*/);   // 戻る動きを1フレーム進める。戻りきったフレームに true
	void PosReset(const Vector2 pos);

	Vector2 Pos();//場所確認
	Vector2 Size();//大きさ確認
	Vector2 Velocity();//速度確認	
	bool IsActive();//アクティブ確認
private:
	Vector2 pos_{};
	Vector2 size_{};
	Vector2 velocity_{};
	Vector2 returnSpeed_ = { 15.0f,15.0f };    // 戻ってくる速さ
	float targetPosTime_{};//対象の座標に向くタイミング
	bool isActive_ = false;

	// 飛ばす攻撃(行って戻ってくる)
	enum class ThrowState {
		None,       // 飛ばしていない
		Outbound,   // 飛んでいく
		Returning,  // 戻ってくる
	};
	ThrowState throwState_ = ThrowState::None;
	Vector2 throwDir_{};        // 飛ばす向き(長さ1)。飛び始めに決めて、飛んでいる間は変えない
	float traveled_ = 0.0f;     // 飛んだ距離

	// 調整用(1フレームあたりの移動量は px)
	static constexpr float kThrowSpeed_ = 15.0f;     // 飛んでいく速さ
	static constexpr float kThrowDistance_ = 400.0f; // ここまで飛んだら戻り始める
	static constexpr float kReturnSpeed_ = { 15.0f };    // 戻ってくる速さ
	
};