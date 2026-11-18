#pragma once
#include <vector>

// リジェネ(じわじわ回復)。tickIntervalFrames ごとに amountPerTick ずつ回復する。
// ドロップを拾うたびに重ね掛けされ、それぞれが独立して回復し続けるので、
// 2個重なっていれば回復の速さは2倍になる
class RegenEffect
{
public:
	// frames: 1個分の持続フレーム
	// tickIntervalFrames: 何フレームに一回回復するか(60 = 1秒に一回)
	// amountPerTick: 一回の回復量
	RegenEffect(int frames, int tickIntervalFrames, float amountPerTick);

	void AddStack();      // 拾ったとき(重ね掛け)。拾った瞬間に1回目の回復が入る
	float Update();       // 毎フレーム呼ぶ。このフレームの回復量を返す(回復しないフレームは0)
	int Stacks() const { return static_cast<int>(stacks_.size()); }

private:
	struct Stack {
		int remaining;   // 持続の残りフレーム
		int tickTimer;   // 次の回復までの残りフレーム(0 以下で回復)
	};

	std::vector<Stack> stacks_;
	int frames_;
	int tickInterval_;
	float amountPerTick_;
};