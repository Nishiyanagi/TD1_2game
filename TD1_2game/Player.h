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

	Player(Vector2 position, Vector2 speed, Vector2 range, float hp, Camera* camera, int graphHandle);
	void Jump(const char* keys, const char* preKeys, Vector2& speed);
	void Move(const char* keys, Vector2 speed, Pads* pad);
	void ClampX(float minX, float maxX); // プレイヤーの行動範囲のX座標
	void Show();
	void UpdateAttack(Vector2& mouseDirection);//毎フレーム1回呼ぶ。攻撃ボタンを押した瞬間にブーメランを投げる状態にする
	bool Defence();
	void Damage(float amount);
	bool SetSingleAttack(bool isSingleAttack);//単発攻撃の状態を設定(キャッチされたら false に戻す)
	void GetCursorDirection(Vector2& mouseDirection);//プレイヤーの向いてる方向をマウスカーソルを追尾する関数
	bool WeaponHit();//自傷で吹っ飛ぶ時の、ヒット処理
	void Heal(float amount);

	Vector2 Pos();//場所確認
	Vector2 Size();//大きさ確認
	Vector2* Velocity();//速度確認
	float JumpCount();//ジャンプ回数確認
	float HP();//HP確認
	bool isRight();//向き確認
	bool isSingleAttack();//単発攻撃確認

	const int& GraphHandle() const { return graphHandle_; }

private:
	Vector2 pos_{};
	Vector2 size_{};
	Vector2 velocity_{};
	Vector2 acceleration_{};

	int graphHandle_{};

	Camera* camera_{};

	float hp_{};
	float maxHp_{};   // 最大HP(回復の上限)

	float jumpCount_{};

	bool isJump_ = false;
	bool prevAttackPressed_ = false;//前フレームで攻撃ボタンが押されていたか(押した瞬間の検出用)
	bool isSingleAttack_ = false;
	bool isDefence_ = false;
	bool isRight_ = false;//右向いてるかどうか　true:右向き false:左向き

	bool isAwayAttackWeapon_ = false;//プレイヤー自身の攻撃がプレイヤーから離れたら検知する
	bool isHitAttackWeapon_ = false;//プレイヤー自身の攻撃がプレイヤーに接触したら検知する
};