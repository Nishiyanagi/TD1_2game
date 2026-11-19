#include "Weapon.h"
#include "Pad.h"
#include "Player.h"
#include "Theta.h"
#include <Novice.h>

Weapon::Weapon(Vector2 pos, Vector2 size, Vector2 velocity)
	: pos_(pos), size_(size), velocity_(velocity)
{

}

void Weapon::SetActive(bool isActive)
{
	isActive_ = isActive;
}

void Weapon::Move(const char* keys, Vector2 velocity, Pads* pad)
{
	int n = 0;

	if (n <= 1) {
		if (keys[DIK_D] || keys[DIK_A]) {
			n++;
			
			if (keys[DIK_D]) {
				
				pos_.x += velocity.x;
			}
			if (keys[DIK_A]) {
				velocity.x *= -1;
				pos_.x += velocity.x;
			}
		}

		if (keys[DIK_W] || keys[DIK_S]) {
			n++;

			if (keys[DIK_W]) {

				velocity.y *= 1;
				pos_.y += velocity.y;
			}
			if (keys[DIK_S]) {
				velocity.y *= -1;
				pos_.y += velocity.y;
			}
		}

	}
	else if (n > 1) {
		velocity.x *= thetaMove.x;
		velocity.y *= thetaMove.y;

		pos_.x += velocity.x;
		pos_.y += velocity.y;
	}
	Novice::GetAnalogInputLeft(0, pad->Lx(), pad->Ly());//Lスティックが入力された時の値を返す関数
	velocity.x *= static_cast<float>(*pad->Lx()) / 32768;
	velocity.y *= static_cast<float>(*pad->Ly()) / 32768;

	pos_.x += velocity.x;
	pos_.y -= velocity.y;
}

void Weapon::SingleAttack(Vector2 velocity, Vector2 mouseCursor, Vector2 ScrollPos/*, Pads* pad*/)
{
	float angle = {90.0f};
	
	if (angle >= 90.0f) {
angle = atan2f(mouseCursor.y , mouseCursor.x + ScrollPos.x);
	
	}
	
	pos_.x += velocity.x * cosf(angle);
	pos_.y += velocity.y * sinf(angle);

	
	Novice::ScreenPrintf(500, 40, "weaponPos:%.1f", pos_.x);
	Novice::ScreenPrintf(500, 60, "weaponPos:%.1f", pos_.y);
	Novice::ScreenPrintf(500, 90, "cosAngle:%.1f", cosf(angle));
	Novice::ScreenPrintf(500, 110, "sinAngle:%.1f", sinf(angle));

}

void Weapon::PosReset(const Vector2 pos)
{
	pos_ = pos;
}

Vector2 Weapon::Pos()
{
	return pos_;
}

Vector2 Weapon::Size()
{
	return size_;
}

Vector2 Weapon::Velocity()
{
	return velocity_;
}

bool Weapon::IsActive()
{
	return isActive_;
}
