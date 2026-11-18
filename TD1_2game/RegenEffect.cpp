#include "RegenEffect.h"
#include <algorithm>

RegenEffect::RegenEffect(int frames, int tickIntervalFrames, float amountPerTick)
	: frames_{ frames },
	tickInterval_{ std::max(1, tickIntervalFrames) },   // 0 以下だと回復が止まらないので 1 以上にする
	amountPerTick_{ amountPerTick }
{
}

void RegenEffect::AddStack()
{
	// 前のリジェネとは独立して、新しく1個分を始める(tickTimer = 0 なので、次の Update で1回目が入る)
	stacks_.push_back({ frames_, 0 });
}

float RegenEffect::Update()
{
	float heal = 0.0f;

	for (Stack& stack : stacks_) {
		if (stack.tickTimer <= 0) {
			heal += amountPerTick_;          // 重ね掛けの数だけ加算される
			stack.tickTimer = tickInterval_;
		}
		--stack.tickTimer;
		--stack.remaining;
	}

	// 持続が終わったものを取り除く
	stacks_.erase(
		std::remove_if(stacks_.begin(), stacks_.end(), [](const Stack& stack) { return stack.remaining <= 0; }),
		stacks_.end());

	return heal;
}