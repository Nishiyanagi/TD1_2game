#include "Weapon.h"
#include "Matrix3x3.h"
#include <math.h>

namespace {
	constexpr float kPi = 3.14159265f;
}

Weapon::Weapon(Vector2 pos, Vector2 size, Vector2 velocity, int graphHandle)
	: pos_(pos), size_(size), velocity_(velocity), graphHandle_(graphHandle)
{

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

// ------------------------------------------------------------
// ブーメラン
// ------------------------------------------------------------

Vector2 Weapon::CenterToPos(const Vector2& center) const
{
	// pos_ は左上(Y上向き)なので、中心から半分ずらす
	return { center.x - size_.x * 0.5f, center.y + size_.y * 0.5f };
}

float Weapon::DecideTurnDir(const Vector2& playerCenter) const
{
	// 往路の向きに対する左側の垂直方向。曲がる側(円の中心がある側)の候補は +/- の2通り
	const Vector2 normal = { -dir_.y, dir_.x };

	// 折り返し地点がプレイヤーの左右どちら側か
	const float side = apex_.x - playerCenter.x;

	float sign = 1.0f;
	if (fabsf(normal.x) >= kHorizontalBulgeThreshold) {
		// 上下に近い方向へ投げたとき: 左側にあれば左へ、右側にあれば右へ膨らませる
		if (fabsf(side) >= 1.0f) {   // ほぼ真上・真下のときは固定の向き
			sign = ((side >= 0.0f) == (normal.x >= 0.0f)) ? 1.0f : -1.0f;
		}
	}
	else {
		// 横に近い方向へ投げたとき: 左右には膨らませにくいので、上側へ膨らませる
		sign = (normal.y >= 0.0f) ? 1.0f : -1.0f;
	}

	// 曲がる円の一番下が地面にめり込むなら、反対側へ曲がる(地面を滑るのを防ぐ)
	const float half = size_.y * 0.5f;
	const float bottom = apex_.y + normal.y * sign * kTurnRadius - kTurnRadius;
	const float otherBottom = apex_.y - normal.y * sign * kTurnRadius - kTurnRadius;
	if (bottom < kGroundY + half && otherBottom > bottom) {
		sign = -sign;
	}

	return sign;   // 円の中心を置く側(+1: 左側 = 左回り)
}

void Weapon::StepReturn(float stepLen, const Vector2& target)
{
	const float tx = target.x - retPos_.x;
	const float ty = target.y - retPos_.y;
	const float dist = sqrtf(tx * tx + ty * ty);
	if (dist < 0.0001f) { return; }

	// プレイヤーの向きと今の進行方向のずれ(-π〜π)
	const float desired = atan2f(ty, tx);
	float delta = desired - heading_;
	while (delta > kPi) { delta -= 2.0f * kPi; }
	while (delta < -kPi) { delta += 2.0f * kPi; }

	// 曲がる半径。プレイヤーに近づいたら小さくして、周りをぐるぐる回らないようにする
	float radius = kTurnRadius;
	if (dist * 0.5f < radius) { radius = dist * 0.5f; }
	if (radius < kMinTurnRadius) { radius = kMinTurnRadius; }

	// 1回に曲がれる角度は、進む距離 / 半径(速さが遅いほど小さくなる → 常に同じ半径の円を描く)
	const float maxTurn = stepLen / radius;

	float turn = 0.0f;
	if (fabsf(delta) > kCommitAngle) {
		turn = turnDir_ * maxTurn;                         // プレイヤーが真後ろ寄り: 決めた向きで曲がり続ける
	}
	else {
		turn = (delta > maxTurn) ? maxTurn : (delta < -maxTurn ? -maxTurn : delta);   // 近道の向きへ回す
	}
	heading_ += turn/1.5f;

	retPos_.x += cosf(heading_) * stepLen;
	retPos_.y += sinf(heading_) * stepLen;
}

void Weapon::Throw(const Vector2& originCenter, const Vector2& direction)
{
	float len = sqrtf(direction.x * direction.x + direction.y * direction.y);
	if (len < 0.0001f) {
		dir_ = { 1.0f, 0.0f };   // 方向が無いときは右
	}
	else {
		dir_ = { direction.x / len, direction.y / len };
	}

	origin_ = originCenter;
	apex_ = { origin_.x + dir_.x * kRange, origin_.y + dir_.y * kRange };

	phase_ = Phase::Out;
	speed_ = kStartSpeed;
	outTravelled_ = 0.0f;
	outLength_ = kRange;
	hitThisPhase_ = false;
	recoilOffset_ = { 0.0f, 0.0f };
	recoilVel_ = { 0.0f, 0.0f };
	returnFrames_ = 0;
	touchingGround_ = false;
	isActive_ = true;
	pos_ = CenterToPos(origin_);
}

bool Weapon::UpdateBoomerang(const Vector2& playerCenter)
{
	if (phase_ == Phase::Idle) { return false; }

	const float half = size_.y * 0.5f;
	const bool isOut = (phase_ == Phase::Out);

	// 敵に当たった反動: 押し戻されたあと、少しずつ本来の軌道へ戻る
	recoilOffset_.x += recoilVel_.x;
	recoilOffset_.y += recoilVel_.y;
	recoilVel_.x *= kRecoilDamping;
	recoilVel_.y *= kRecoilDamping;
	recoilOffset_.x *= kRecoilSpring;
	recoilOffset_.y *= kRecoilSpring;

	Vector2 center{};
	bool reachedApex = false;
	float leftover = 0.0f;    // 折り返し地点を通り過ぎた分の距離(復路へ持ち越す)

	if (isOut) {
		// 往路は曲げずに、投げた方向へ真っすぐ飛ぶ
		outTravelled_ += speed_;
		if (outTravelled_ >= outLength_) {
			reachedApex = true;
			leftover = outTravelled_ - outLength_;
			outTravelled_ = outLength_;
		}
		const float s = (outLength_ > 0.0001f) ? outTravelled_ / outLength_ : 1.0f;
		center = {
			origin_.x + (apex_.x - origin_.x) * s,
			origin_.y + (apex_.y - origin_.y) * s };

		// 下向きに飛んで地面に触れたら、Y方向を反転して跳ね返る(残りの距離はそのまま使う)
		if (dir_.y < 0.0f && center.y - half <= kGroundY) {
			const float remaining = outLength_ - outTravelled_;

			center.y = kGroundY + half;
			dir_.y = -dir_.y;
			origin_ = center;
			apex_ = { center.x + dir_.x * remaining, center.y + dir_.y * remaining };
			outLength_ = remaining;
			outTravelled_ = 0.0f;
		}

		// 折り返し地点に着いたら、通り過ぎた分の距離をそのまま復路で進める(折り返しで速さが途切れない)
		if (reachedApex) {
			phase_ = Phase::Return;
			hitThisPhase_ = false;   // 復路でもう一度当たれる
			retPos_ = apex_;
			heading_ = atan2f(dir_.y, dir_.x);   // 往路の向きのまま曲がり始める(角ができない)
			turnDir_ = DecideTurnDir(playerCenter);
			returnFrames_ = 0;
			if (leftover > 0.0f) { StepReturn(leftover, playerCenter); }
			center = retPos_;
		}
	}
	else {
		// 復路: プレイヤーの現在位置へ向けて、進行方向を少しずつ回しながら進む(動いても追ってくる)
		++returnFrames_;
		StepReturn(speed_, playerCenter);
		center = retPos_;
	}

	// 投げてから少しずつ遅くなる。ただし kMinSpeed より遅くはならない
	speed_ -= kDeceleration;
	if (speed_ < kMinSpeed) { speed_ = kMinSpeed; }

	// 反動のずれを足す
	center.x += recoilOffset_.x;
	center.y += recoilOffset_.y;

	// 地面にめり込まない。復路で地面に触れた瞬間は、少し跳ね上がる
	bool onGround = false;
	if (center.y - half <= kGroundY) {
		center.y = kGroundY + half;
		onGround = true;
		if (!touchingGround_ && !isOut) {
			recoilVel_.y = kGroundHop;
		}
	}
	touchingGround_ = onGround;

	pos_ = CenterToPos(center);

	if (isOut) { return false; }

	const float dx = center.x - playerCenter.x;
	const float dy = center.y - playerCenter.y;
	const bool close = (dx * dx + dy * dy) <= kCatchDistance * kCatchDistance;

	if (close || returnFrames_ > kMaxReturnFrames) {
		phase_ = Phase::Idle;
		isActive_ = false;
		return true;   // キャッチ
	}
	return false;
}

bool Weapon::TryHit(const Vector2& enemyCenter)
{
	if (hitThisPhase_) { return false; }
	hitThisPhase_ = true;

	// 敵の中心から離れる向きに、少し押し戻される
	const Vector2 center = { pos_.x + size_.x * 0.5f, pos_.y - size_.y * 0.5f };
	float dx = center.x - enemyCenter.x;
	float dy = center.y - enemyCenter.y;
	float len = sqrtf(dx * dx + dy * dy);
	if (len < 0.0001f) {
		dx = -dir_.x;   // 中心が重なっているときは、来た方向へ戻す
		dy = -dir_.y;
		len = 1.0f;
	}
	recoilVel_ = { dx / len * kRecoilSpeed, dy / len * kRecoilSpeed };
	return true;
}