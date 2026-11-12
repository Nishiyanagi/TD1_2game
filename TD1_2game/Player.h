#pragma once
#include "Vector2.h"
#include "Camera.h"
#include "Pad.h"

enum Actions {
	NONE = 0,
	ATTACK = 1,
	DEFENCE = 2
};
enum Actions {
	NONE = 0,
	ATTACK = 1,
	DEFENCE = 2
};
class Camera;

class Player {
public:

	Player(Vector2 position, Vector2 speed, Vector2 range,float hp,Camera* camera);
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


	Vector2 Pos();//場所確認
	Vector2 Size();//大きさ確認
	Vector2* Velocity();//速度確認
	float JumpCount();//ジャンプ回数確認
	float attackTimer();//攻撃タイマー確認
	float HP();//HP確認
	bool isRight();//向き確認
	bool isSingleAttack();//単発攻撃確認

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
};
