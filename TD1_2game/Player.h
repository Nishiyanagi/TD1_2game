#pragma once
#include "Vector2.h"
#include "Camera.h"
#include "Pad.h"

enum Actions {
	NONE = 0,
	ATTACK = 1,
	DEFENCE = 2
};
class Camera;

class Player {
public:

	Player(Vector2 position, Vector2 speed, Vector2 range, float hp, Camera* camera);
	void Jump(const char* keys, const char* preKeys, Vector2& speed);
	void Move(const char* keys, Vector2 speed, Pads* pad);
	void ClampX(float minX, float maxX);
	void Show();
	bool Attack(Vector2 weaponPos, Vector2& mouseDirection);
	bool Defence();
	void Damage(float amount);
	float SetAttackTimer(float times);//攻撃タイマーを入力
	bool SetSingleAttack(bool isSingleAttack);//攻撃タイマーを入力
	void GetCursorDirection(Vector2 Pos, Vector2& mouseDirection);//プレイヤーの向いてる方向をマウスカーソルを追尾する関数
	bool WeaponHit();//自傷で吹っ飛ぶ時の、ヒット処理
	void Heal(float amount);
	void StartLaunch(Vector2 direction);              // 自分の武器に当たって吹っ飛び始める(direction は飛ぶ向き。長さは問わない)
	void UpdateLaunch(const char* keys, Pads* pad);   // 吹っ飛び中の移動を1フレーム進める
	bool IsLaunched() const;                          // 吹っ飛び中か

	Vector2 Pos();//場所確認
	Vector2 Size();//大きさ確認
	Vector2* Velocity();//速度確認
	float JumpCount();//ジャンプ回数確認
	float attackTimer();//攻撃タイマー確認
	float HP();//HP確認
	bool isRight();//向き確認
	bool isSingleAttack();//単発攻撃確認
	bool ConsumeHoldTimeUp();//長押しの時間が来たら true(1回だけ)。武器を戻し始める合図

private:
	Vector2 pos_{};
	Vector2 size_{};
	Vector2 velocity_{};
	Vector2 acceleration_{};

	Camera* camera_{};

	float hp_{};
	float maxHp_{};   // 最大HP(回復の上限)

	float attackTimer_{};

	float jumpCount_{};

	bool isJump_ = false;
	bool isAttack_ = false;
	bool isSingleAttack_ = false;
	bool isDefence_ = false;
	bool isRight_ = false;//右向いてるかどうか　true:右向き false:左向き

	bool isAwayAttackWeapon_ = false;//プレイヤー自身の攻撃がプレイヤーから離れたら検知する
	bool isHitAttackWeapon_ = false;//プレイヤー自身の攻撃がプレイヤーに接触したら検知する

	// 攻撃ボタンの押し方。attackTimer_ は「攻撃ボタンを押しているフレーム数」
	bool isHoldExpired_ = false;   // 長押しの時間切れのあと。ボタンを離すまで攻撃しない
	bool isHoldTimeUp_ = false;    // 時間切れの合図(ConsumeHoldTimeUp で1回だけ取り出す)
	static constexpr float kSingleAttackMinFrames = 1.0f;   // 短押し(飛ばす攻撃)になる、押していたフレーム数の下限
	static constexpr float kSingleAttackMaxFrames = 10.0f;   // 同・上限。これより長く押すと長押し(動かせる攻撃)
	static constexpr float kHoldMaxFrames = 150.0f;         // 長押しを続けられる時間。超えると武器が戻る

	// 吹っ飛び(自分の攻撃に当たったとき)
	bool isLaunched_ = false;
	bool isLaunchLocked_ = false;   // 吹っ飛びが始まってから着地するまで true。この間は攻撃・ガードできない
	Vector2 launchVel_{};
	int launchFrames_ = 0;
	static constexpr float kLaunchSpeed = 24.0f;          // 吹っ飛び始めの速さ
	static constexpr float kLaunchAirControl = 0.2f;      // 吹っ飛び中の操作の効き(1フレームの加速量)
	static constexpr float kLaunchGravity = 0.3f;         // 重力(弱めにして飛距離を出す)
	static constexpr float kLaunchDrag = 0.965f;          // 空気抵抗(1に近いほど慣性が残る)
	static constexpr float kLaunchGroundFriction = 0.9f;  // 地面を滑るときの摩擦
	static constexpr float kLaunchEndSpeed = 4.0f;        // これより遅くなったら吹っ飛び終了
	static constexpr int kLaunchMaxFrames = 120;          // 最大継続フレーム
};