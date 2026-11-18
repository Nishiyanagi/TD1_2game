#pragma once
#include "Cooldown.h"

// 常に持っていて、クールタイムで使えるようになる回復アイテム
class HealFlask
{
public:
	HealFlask(int cooldownFrames, float healAmount);

	// 回復ボタンを押した瞬間か(キーボード E / パッド Y)。入力の割り当てはここだけで変えられる
	static bool HealTrigger(const char* keys, const char* preKeys);

	void Update();      // 毎フレーム呼ぶ
	bool TryUse();      // 使えたら true(クールタイム開始)。使えなければ false
	bool IsReady() const;
	float Amount() const { return healAmount_; }

private:
	Cooldown cooldown_;
	float healAmount_;
};