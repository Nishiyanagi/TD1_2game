#include "EnemyCloud.h"
#include "Random.h"
#include <math.h>

namespace {
	constexpr float kPi = 3.14159265f;

	float Clamp(float value, float min, float max)
	{
		if (value < min) { return min; }
		if (value > max) { return max; }
		return value;
	}

	// 回転した長方形(弾)と、軸に平行な矩形(プレイヤー)の当たり判定(分離軸判定)
	bool BoxHitsRect(const Vector2& center, const Vector2& size, float angle, const Rect& rect)
	{
		const float c = cosf(angle);
		const float s = sinf(angle);
		const float hl = size.x * 0.5f;   // 進む向きの半分の長さ
		const float hw = size.y * 0.5f;   // 幅の半分

		const float rectCx = (rect.left + rect.right) * 0.5f;
		const float rectCy = (rect.top + rect.bottom) * 0.5f;
		const float rx = (rect.right - rect.left) * 0.5f;
		const float ry = (rect.top - rect.bottom) * 0.5f;

		const float dx = center.x - rectCx;
		const float dy = center.y - rectCy;

		// 軸は4本: 画面のX・Y(矩形の辺)と、長方形の進む向き・その垂直
		if (fabsf(dx) > rx + hl * fabsf(c) + hw * fabsf(s)) { return false; }
		if (fabsf(dy) > ry + hl * fabsf(s) + hw * fabsf(c)) { return false; }
		if (fabsf(dx * c + dy * s) > hl + rx * fabsf(c) + ry * fabsf(s)) { return false; }
		if (fabsf(-dx * s + dy * c) > hw + rx * fabsf(s) + ry * fabsf(c)) { return false; }
		return true;
	}

	// 円と矩形の当たり判定(矩形の中で円の中心に一番近い点までの距離で判定)
	bool CircleHitsRect(const Vector2& center, float radius, const Rect& rect)
	{
		const float nearestX = Clamp(center.x, rect.left, rect.right);
		const float nearestY = Clamp(center.y, rect.bottom, rect.top);
		const float dx = center.x - nearestX;
		const float dy = center.y - nearestY;
		return dx * dx + dy * dy <= radius * radius;
	}
}

EnemyCloud::EnemyCloud(Vector2 position, Vector2 size, Vector2 velocity, float hp, int graphHandle)
	: Enemy{ position, size, velocity, hp, graphHandle }
{
	patrolDir_ = Random::Chance(50) ? 1.0f : -1.0f;
	EnterIdle();
}

// ------------------------------------------------------------
// 本体・分身
// ------------------------------------------------------------

Vector2 EnemyCloud::BodyPos(int index) const
{
	if (index == 0) { return pos_; }
	// 分身: 2体の中央の軸(axisX_)を中心にした鏡写し(高さは同じ)
	const float mirroredCenterX = 2.0f * axisX_ - CenterX();
	return { mirroredCenterX - size_.x * 0.5f, pos_.y };
}

Vector2 EnemyCloud::BodyCenter(int index) const
{
	const Vector2 p = BodyPos(index);
	return { p.x + size_.x * 0.5f, p.y - size_.y * 0.5f };
}

void EnemyCloud::KnockbackBody(int index, float velocityX)
{
	// 分身は本体の鏡写しなので、分身を右へ飛ばしたいときは本体を左へ飛ばす
	Knockback(index == 0 ? velocityX : -velocityX);
}

// ------------------------------------------------------------
// 毎フレームの更新
// ------------------------------------------------------------

void EnemyCloud::Move(const Vector2& targetPos, float screenCenterX, float screenHalfWidth)
{
	target_ = targetPos;

	screenCenterX_ = screenCenterX;
	screenHalfWidth_ = screenHalfWidth;

	// 【移動】分裂中の2体の中央の軸は、Idle のときだけ画面の中心へゆっくりついていく(置いていかれたら戻る)
	// 【攻撃】攻撃中は、画面が動いてもワールド座標で止まっている(ついていかない)
	if (isSplit_ && state_ == State::Idle) {
		const float delta = Clamp(screenCenterX_ - axisX_, -kFollowSpeed, kFollowSpeed);
		axisX_ += delta;
		pos_.x += delta;
	}

	ApplyKnockback();   // 攻撃を受けた反動の移動(行動の状態にかかわらず入る)
	UpdateBullets();

	switch (state_) {
	case State::Idle:
		Patrol();
		--timer_;
		if (timer_ <= 0 && IsInsideBounds()) { ChooseAttack(); }   // 画面の外に置いていかれている間は攻撃しない
		break;

		// ---- ①単発弾 ----
	case State::ShotCharge:
		--timer_;
		if (timer_ <= 0) {
			FireBullets();
			state_ = State::ShotRecover;
			timer_ = kShotRecoverFrames;
		}
		break;

	case State::ShotRecover:
		--timer_;
		if (timer_ <= 0) { EnterIdle(); }
		break;

		// ---- ②回転する電気玉 ----
	case State::OrbitAttack:
		++timer_;
		orbitAngle_ -= kOrbitSpeed;
		if (timer_ >= kOrbitTotalFrames) { EnterIdle(); }
		break;

		// ---- ③分裂 ----
	case State::SplitMove:
		if (MoveToward(attackHomeX_, kSplitMoveSpeed)) {   // 分裂する場所(攻撃を始めたときの画面の中央)へ
			axisX_ = attackHomeX_;                         // ここが分裂の中央。以降、攻撃が終わるまで動かさない
			state_ = State::SplitCharge;
			timer_ = kSplitChargeFrames;
		}
		break;

	case State::SplitCharge:
		--timer_;
		if (timer_ <= 0) {
			isSplit_ = true;   // ここで2体になる(まだ重なっている)
			state_ = State::SplitApart;
		}
		break;

	case State::SplitApart:
		// 本体は左へ、分身は鏡写しで右へ
		if (MoveToward(axisX_ - kSplitSeparation, kSplitMoveSpeed)) {
			splitAttacksLeft_ = kSplitAttackCount;
			EnterIdle();
		}
		break;

		// ---- ④雷 ----
	case State::ThunderCircle:
	{
		// 移動の前に、その場で小さく円を描く(クルクルと数周)。始めと終わりは円が小さく、出発の位置はずれない
		++timer_;
		const float t = static_cast<float>(timer_) / kThunderCircleFrames;
		const float rise = Clamp(t / 0.2f, 0.0f, 1.0f);
		const float fade = Clamp((1.0f - t) / 0.2f, 0.0f, 1.0f);
		const float radius = kThunderCircleRadius * rise * fade;
		const float angle = 2.0f * kPi * kThunderCircleLaps * t;
		SetCenterX(Anchor() + circleBaseX_ + cosf(angle) * radius);
		pos_.y = kHoverY + sinf(angle) * radius;   // 分身は左右が鏡写しなので、回る向きが逆になる
		if (timer_ >= kThunderCircleFrames) {
			SetCenterX(Anchor() + circleBaseX_);   // 円の中心(元の位置)に戻してから出発
			pos_.y = kHoverY;
			BeginThunderTravel();
		}
		break;
	}

	case State::ThunderMove:
	{
		// 落とす場所へ、ふわっと弧を描いて移動(ゆっくり出て、ゆっくり止まる)
		++timer_;
		float t = static_cast<float>(timer_) / static_cast<float>(moveFrames_);
		if (t > 1.0f) { t = 1.0f; }
		const float eased = t * t * (3.0f - 2.0f * t);   // 出だしと終わりがゆるやか
		SetCenterX(Anchor() + moveStartX_ + (moveTargetX_ - moveStartX_) * eased);
		pos_.y = kHoverY + moveArcHeight_ * sinf(kPi * t);   // 上へ持ち上がってから降りる弧
		if (t >= 1.0f) {
			SetCenterX(Anchor() + moveTargetX_);
			pos_.y = kHoverY;
			state_ = State::ThunderWarn;
			timer_ = kThunderWarnFrames;
		}
		break;
	}

	case State::ThunderWarn:
		--timer_;
		if (timer_ <= 0) {
			state_ = State::ThunderStrike;
			timer_ = kThunderStrikeFrames;
		}
		break;

	case State::ThunderStrike:
		--timer_;
		if (timer_ <= 0) {
			state_ = State::ThunderRecover;
			timer_ = kThunderRecoverFrames;
		}
		break;

	case State::ThunderRecover:
		--timer_;
		if (timer_ <= 0) {
			++thunderIndex_;
			if (thunderIndex_ >= ThunderWaypointCount()) {
				EnterIdle();
			}
			else {
				BeginThunderCircle();
			}
		}
		break;

		// ---- 分身が本体に戻る ----
	case State::Merge:
		if (MoveToward(axisX_, kMergeSpeed)) {
			isSplit_ = false;
			EnterIdle();
		}
		break;
	}

	// 一定の高さに保つ(最初に低い位置にいたときも、ここで浮かぶ高さへ上がる)。円・弧を描いて動いている間は、その動きが高さを決める
	if (state_ != State::ThunderMove && state_ != State::ThunderCircle) {
		pos_.y += (kHoverY - pos_.y) * kHoverGain;
		if (fabsf(kHoverY - pos_.y) < 0.5f) { pos_.y = kHoverY; }
	}

	ClampBody();
}

// ------------------------------------------------------------
// 状態の開始
// ------------------------------------------------------------

void EnemyCloud::EnterIdle()
{
	state_ = State::Idle;
	timer_ = kIdleFrames;
}

void EnemyCloud::ChooseAttack()
{
	// 分裂してから決まった回数だけ攻撃したら、分身が本体に戻る
	if (isSplit_ && splitAttacksLeft_ <= 0) {
		StartMerge();
		return;
	}

	// 最初は必ず単発弾
	if (isFirstAction_) {
		isFirstAction_ = false;
		StartShot();
		return;
	}

	const int roll = Random::Range(0, 99);   // 0〜99 の値で振り分ける

	if (isSplit_) {
		--splitAttacksLeft_;
		if (roll < kSplitShotChance) { StartShot(); }
		else if (roll < kSplitShotChance + kSplitOrbitChance) { StartOrbit(); }
		else { StartThunder(); }
		return;
	}

	if (roll < kShotChance) { StartShot(); }
	else if (roll < kShotChance + kOrbitChance) { StartOrbit(); }
	else if (roll < kShotChance + kOrbitChance + kSplitChance) { StartSplit(); }
	else { StartThunder(); }
}

void EnemyCloud::StartShot()
{
	state_ = State::ShotCharge;
	timer_ = kShotChargeFrames;
}

void EnemyCloud::StartOrbit()
{
	state_ = State::OrbitAttack;
	timer_ = 0;   // この状態だけ「経過フレーム」を数える
	orbitAngle_ = Random::RangeF(0.0f, 2.0f * kPi);
}

void EnemyCloud::StartSplit()
{
	attackHomeX_ = screenCenterX_;   // 分裂する場所は、攻撃を始めたときの画面の中央に決める(あとで画面が動いても変えない)
	state_ = State::SplitMove;
}

void EnemyCloud::StartThunder()
{
	thunderIndex_ = 0;
	attackHomeX_ = HomeX();   // 雷を落とす場所の中心は、攻撃を始めたときに決める(あとで画面が動いても変えない)
	BeginThunderCircle();
}

void EnemyCloud::BeginThunderCircle()
{
	circleBaseX_ = CenterX() - Anchor();
	timer_ = 0;
	state_ = State::ThunderCircle;
}

void EnemyCloud::BeginThunderTravel()
{
	// 出発点と目標は、ここで決めて動かさない(途中で画面が動いても、軌道がぶれない)
	moveStartX_ = circleBaseX_;
	moveTargetX_ = ThunderTargetX(thunderIndex_) - Anchor();

	const float distance = fabsf(moveTargetX_ - moveStartX_);
	if (distance <= kArriveDistance) {
		// すでにその場所にいる
		SetCenterX(Anchor() + moveTargetX_);
		state_ = State::ThunderWarn;
		timer_ = kThunderWarnFrames;
		return;
	}

	// 遠いほど時間をかける(ただし上限・下限あり)。速さは出だしと終わりがゆるやか
	const float frames = Clamp(distance / kThunderAvgSpeed,
		static_cast<float>(kThunderMinTravelFrames), static_cast<float>(kThunderMaxTravelFrames));
	moveFrames_ = static_cast<int>(ceilf(frames));
	moveArcHeight_ = fminf(kThunderArcHeight, distance * kThunderArcRatio);
	timer_ = 0;
	state_ = State::ThunderMove;
}

void EnemyCloud::StartMerge()
{
	state_ = State::Merge;
}

void EnemyCloud::FireBullets()
{
	// 分裂中は2体とも同時に、プレイヤーへ向けて撃つ
	for (int i = 0; i < BodyCount(); ++i) {
		const Vector2 from = BodyCenter(i);
		float dx = target_.x - from.x;
		float dy = target_.y - from.y;
		const float len = sqrtf(dx * dx + dy * dy);
		if (len < 0.0001f) {
			dx = 0.0f;
			dy = -1.0f;   // 真下
		}
		else {
			dx /= len;
			dy /= len;
		}
		// 長方形はプレイヤーの方向(進む向き)を向く
		bullets_.push_back({ from,{ dx * kBulletSpeed, dy * kBulletSpeed }, atan2f(dy, dx), kBulletLife });
	}
}

// ------------------------------------------------------------
// 移動
// ------------------------------------------------------------

void EnemyCloud::UpdateBullets()
{
	for (Bullet& bullet : bullets_) {
		bullet.center.x += bullet.velocity.x;
		bullet.center.y += bullet.velocity.y;
		--bullet.life;
	}

	// 時間切れ・地面・ステージの外に出たものは消す
	for (size_t i = 0; i < bullets_.size();) {
		const Bullet& b = bullets_[i];
		// 回転した長方形が画面のX・Yにはみ出す大きさ(半分)
		const float extentX = fabsf(cosf(b.angle)) * kBulletLength * 0.5f + fabsf(sinf(b.angle)) * kBulletWidth * 0.5f;
		const float extentY = fabsf(sinf(b.angle)) * kBulletLength * 0.5f + fabsf(cosf(b.angle)) * kBulletWidth * 0.5f;
		const bool dead = b.life <= 0 ||
			b.center.y - extentY <= kGroundY ||
			b.center.x < -extentX || b.center.x > kWorldWidth + extentX;
		if (dead) {
			bullets_[i] = bullets_.back();
			bullets_.pop_back();
		}
		else {
			++i;
		}
	}
}

void EnemyCloud::Patrol()
{
	// 一定の高さのまま、基準(画面の中心)から決まった範囲の中を左右に往復する(自分から画面の外には出ない)
	float minX = HomeX() - kPatrolHalfRange;
	float maxX = HomeX() + kPatrolHalfRange;
	if (isSplit_) { maxX = axisX_ - SplitMinHalfGap(); }   // 分裂中は、2体がぶつからない範囲
	if (minX < MinCenterX()) { minX = MinCenterX(); }
	if (maxX > MaxCenterX()) { maxX = MaxCenterX(); }

	float x = CenterX();
	if (x < minX) {
		// 画面から置いていかれた: ゆっくり範囲まで戻る(プレイヤーより遅いので、走れば逃げ切れる)
		x += kFollowSpeed;
		if (x > minX) { x = minX; }
		patrolDir_ = 1.0f;
	}
	else if (x > maxX) {
		x -= kFollowSpeed;
		if (x < maxX) { x = maxX; }
		patrolDir_ = -1.0f;
	}
	else {
		x += patrolDir_ * kPatrolSpeed;
		if (x >= maxX) { x = maxX; patrolDir_ = -1.0f; }
		else if (x <= minX) { x = minX; patrolDir_ = 1.0f; }
	}
	SetCenterX(x);
}

bool EnemyCloud::MoveToward(float targetCenterX, float speed)
{
	const float dx = targetCenterX - CenterX();
	if (fabsf(dx) <= kArriveDistance) {
		SetCenterX(targetCenterX);
		return true;
	}

	// 目標に近いほどゆっくり(ただし最低速度はある)
	float step = Clamp(fabsf(dx) * kApproachGain, kMinApproachStep, speed);
	if (step > fabsf(dx)) { step = fabsf(dx); }
	SetCenterX(CenterX() + (dx > 0.0f ? step : -step));
	return false;
}

float EnemyCloud::MinCenterX() const
{
	const float half = size_.x * 0.5f;
	return fmaxf(HomeX() - screenHalfWidth_ + kScreenMargin + half, half);
}

float EnemyCloud::MaxCenterX() const
{
	const float half = size_.x * 0.5f;
	return fminf(HomeX() + screenHalfWidth_ - kScreenMargin - half, kWorldWidth - half);
}

bool EnemyCloud::IsInsideBounds() const
{
	const float x = CenterX();
	return x >= MinCenterX() - kArriveDistance && x <= MaxCenterX() + kArriveDistance;
}

void EnemyCloud::ClampBody()
{
	// ステージの外には出ない。画面の端で止めることはしない(画面に押されて動かないように)
	const float half = size_.x * 0.5f;
	const float minX = half;
	float maxX = kWorldWidth - half;

	// 分裂中は本体が左側。中央を越えて2体が重ならないようにする
	// (中央へ向かう途中・分かれる途中・戻る途中は、中央までは行ける)
	if (isSplit_) {
		const bool free = (state_ == State::SplitApart || state_ == State::Merge);
		const float limit = free ? axisX_ : axisX_ - SplitMinHalfGap();
		if (limit < maxX) { maxX = limit; }
	}

	SetCenterX(Clamp(CenterX(), minX, maxX));
}

float EnemyCloud::ThunderTargetX(int waypointIndex) const
{
	// 落とす場所は、攻撃を始めたときに決めた中心(attackHomeX_)から、画面の半分の幅だけ左右へ
	const float half = size_.x * 0.5f;
	const float reach = screenHalfWidth_ - kScreenMargin - half;
	const float left = fmaxf(attackHomeX_ - reach, half);
	const float right = fminf(attackHomeX_ + reach, kWorldWidth - half);

	if (isSplit_) {
		// 分裂中: 本体は 左端 → 中央(2体がぶつからない位置)。分身は鏡写しで 右端 → 中央
		return (waypointIndex == 0) ? left : (attackHomeX_ - SplitMinHalfGap());
	}
	// 1体: 左 → 右 → 真ん中
	if (waypointIndex == 0) { return left; }
	if (waypointIndex == 1) { return right; }
	return attackHomeX_;
}

// ------------------------------------------------------------
// ②回転する電気玉
// ------------------------------------------------------------

float EnemyCloud::OrbitScale() const
{
	// 始めに腕が伸びて、終わりに縮む
	const float t = static_cast<float>(timer_);
	float scale = 1.0f;
	if (t < kOrbitExtendFrames) {
		scale = t / kOrbitExtendFrames;
	}
	else if (t > kOrbitTotalFrames - kOrbitRetractFrames) {
		scale = (kOrbitTotalFrames - t) / kOrbitRetractFrames;
	}
	return Clamp(scale, 0.0f, 1.0f);
}

void EnemyCloud::ComputeOrbitCircles(std::vector<AttackCircle>& out) const
{
	const float scale = OrbitScale();

	for (int body = 0; body < BodyCount(); ++body) {
		const Vector2 center = BodyCenter(body);
		// 分身は鏡写し(回転の向きが逆になる)
		const float baseAngle = (body == 0) ? orbitAngle_ : (kPi - orbitAngle_);

		for (int arm = 0; arm < kOrbitArms; ++arm) {
			const float angle = baseAngle + (2.0f * kPi) * arm / kOrbitArms;
			for (int k = 0; k < kOrbitBalls; ++k) {
				const float r = (kOrbitInnerRadius + kOrbitSpacing * k) * scale;
				out.push_back({ { center.x + cosf(angle) * r, center.y + sinf(angle) * r }, kOrbitBallRadius });
			}
		}
	}
}

// ------------------------------------------------------------
// 当たり判定・描画用の情報
// ------------------------------------------------------------

bool EnemyCloud::HitsPlayer(const Rect& playerRect)
{
	bool hit = false;

	// ①弾(当たった弾は消える)
	for (size_t i = 0; i < bullets_.size();) {
		if (BoxHitsRect(bullets_[i].center, { kBulletLength, kBulletWidth }, bullets_[i].angle, playerRect)) {
			bullets_[i] = bullets_.back();
			bullets_.pop_back();
			hit = true;
		}
		else {
			++i;
		}
	}

	// ②電気玉(伸びきっていない間も、いまある位置で当たる)
	if (state_ == State::OrbitAttack) {
		std::vector<AttackCircle> circles;
		ComputeOrbitCircles(circles);
		for (const AttackCircle& c : circles) {
			if (CircleHitsRect(c.center, c.radius, playerRect)) { hit = true; }
		}
	}

	// ④雷(落雷中だけ)
	if (state_ == State::ThunderStrike) {
		std::vector<ThunderBolt> bolts;
		GetThunders(bolts);
		for (const ThunderBolt& bolt : bolts) {
			if (bolt.isStriking && IsHit(playerRect, bolt.rect)) { hit = true; }
		}
	}

	return hit;
}

void EnemyCloud::GetBullets(std::vector<AttackBox>& out) const
{
	for (const Bullet& bullet : bullets_) {
		out.push_back({ bullet.center,{ kBulletLength, kBulletWidth }, bullet.angle });
	}
}

void EnemyCloud::GetDamageCircles(std::vector<AttackCircle>& out) const
{
	if (state_ == State::OrbitAttack) {
		ComputeOrbitCircles(out);
	}
}

void EnemyCloud::GetThunders(std::vector<ThunderBolt>& out) const
{
	if (state_ != State::ThunderWarn && state_ != State::ThunderStrike) { return; }

	const bool striking = (state_ == State::ThunderStrike);

	// 予兆は、一定の間隔で 表示 ⇔ 非表示 を繰り返して点滅する(落雷は常に表示)
	bool visible = true;
	if (!striking) {
		const int elapsed = kThunderWarnFrames - timer_;   // 予兆に入ってからの経過フレーム
		visible = ((elapsed / kThunderBlinkInterval) % 2) == 0;
	}

	for (int i = 0; i < BodyCount(); ++i) {
		const Vector2 center = BodyCenter(i);
		const float bodyBottom = center.y - size_.y * 0.5f;
		// 雲の下から地面まで
		const Rect rect = { center.x - kThunderWidth * 0.5f, center.x + kThunderWidth * 0.5f, bodyBottom, kGroundY };
		out.push_back({ rect, striking, visible });   // 予兆も落雷と同じ太さ
	}
}

bool EnemyCloud::IsTelegraphing() const
{
	return state_ == State::ShotCharge || state_ == State::SplitCharge || state_ == State::ThunderWarn;
}

const char* EnemyCloud::StateName() const
{
	switch (state_) {
	case State::Idle:           return "Idle";
	case State::ShotCharge:     return "ShotCharge";
	case State::ShotRecover:    return "ShotRecover";
	case State::OrbitAttack:    return "OrbitAttack";
	case State::SplitMove:      return "SplitMove";
	case State::SplitCharge:    return "SplitCharge";
	case State::SplitApart:     return "SplitApart";
	case State::ThunderCircle:  return "ThunderCircle";
	case State::ThunderMove:    return "ThunderMove";
	case State::ThunderWarn:    return "ThunderWarn";
	case State::ThunderStrike:  return "ThunderStrike";
	case State::ThunderRecover: return "ThunderRecover";
	case State::Merge:          return "Merge";
	}
	return "?";
}