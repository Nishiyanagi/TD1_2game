#include "HealFlask.h"
#include <Novice.h>
#include "InputDevice.h"

HealFlask::HealFlask(int cooldownFrames, float healAmount)
	: cooldown_{ cooldownFrames },
	healAmount_{ healAmount }
{
	// Cooldown は最初から使える状態(remaining_ = 0)
}

bool HealFlask::HealTrigger(const char* keys, const char* preKeys)
{
	const bool keyboard = keys[DIK_E] && !preKeys[DIK_E];   // E を押した瞬間
	const bool pad = InputDevice::IsTriggerButton(kPadButton13);   // パッドの Y を押した瞬間
	return keyboard || pad;
}

void HealFlask::Update()
{
	cooldown_.Update();
}

bool HealFlask::TryUse()
{
	if (!cooldown_.IsReady()) { return false; }
	cooldown_.Start();
	return true;
}

bool HealFlask::IsReady() const
{
	return cooldown_.IsReady();
}