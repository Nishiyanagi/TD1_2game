#pragma once
#include "Enemy.h"
#include "World.h"
#include <vector>

// 丸い攻撃(回転する電気玉)。当たり判定は円
struct AttackCircle {
	Vector2 center;
	float radius;
};

// 回転する長方形の攻撃(単発弾)。angle は進む向き(ラジアン。0 が右、反時計回りが正)
// size.x が進む向きの長さ、size.y が幅
struct AttackBox {
	Vector2 center;
	Vector2 size;
	float angle;
};

// 雷。isStriking が false の間は予兆(当たり判定なし。落雷と同じ太さで、赤く点滅する)
// isVisible が false のフレームは描画しない(予兆の点滅用)
struct ThunderBolt {
	Rect rect;
	bool isStriking;
	bool isVisible;
};

// 雷雲の敵
//   Idle: 一定の高さで左右に移動
//   攻撃: ①単発弾 ②回転する電気玉(ガーディアン) ③分裂 ④雷を落として移動
//   分裂中は2体になり、画面中央を基準に鏡写しで動く。HPは共通
//
// 【移動】と【攻撃】は完全に分かれている
//   移動(Idle): 画面の中を左右に往復する。自分からは画面の外に出ない。画面から置いていかれたら、ゆっくり戻ってくる
//   攻撃: 始める前に、攻撃する場所(その場 / 雷や分裂はその場所)を決め、そこへ行ってからワールド座標で止まる。
//         攻撃が終わるまで、画面(カメラ)が動いても場所は変わらない
//   → プレイヤーは、走って逃げることも、敵に近づいて攻撃することもしやすい
class EnemyCloud :
	public Enemy
{
public:
	EnemyCloud(Vector2 position, Vector2 size, Vector2 velocity, float hp, int graphHandle);

	// targetPos: プレイヤーの中心座標 / screenCenterX: 画面の中心のワールドX(カメラの中心) / screenHalfWidth: 画面の幅の半分
	// 毎フレーム呼ぶ。敵はこの画面の中に収まるように動く
	void Move(const Vector2& targetPos, float screenCenterX, float screenHalfWidth);

	// ---- 本体(分裂中は2体。index 0 が本体、1 が分身) ----
	int BodyCount() const { return isSplit_ ? 2 : 1; }
	Vector2 BodyPos(int index) const;                                   // 左上(Y上向き)
	Rect BodyRect(int index) const { return MakeRect(BodyPos(index), size_); }
	void KnockbackBody(int index, float velocityX);                     // 当たった体を吹っ飛ばす(分身は鏡写しの向きになる)

	// ---- 攻撃の当たり判定 ----
	// プレイヤーに弾・電気玉・雷が当たったら true(当たった弾は消える)。接触ダメージは含まない
	bool HitsPlayer(const Rect& playerRect);

	// ---- 描画用 ----
	void GetBullets(std::vector<AttackBox>& out) const;            // 単発弾(プレイヤーの方向を向いた長方形)
	void GetDamageCircles(std::vector<AttackCircle>& out) const;   // 回転する電気玉
	void GetThunders(std::vector<ThunderBolt>& out) const;         // 雷(予兆を含む)
	bool IsTelegraphing() const;                                   // 攻撃の予兆中(色を変えるなどに使う)
	const char* StateName() const;                                 // デバッグ表示用

private:
	enum class State {
		Idle,           // 一定の高さで左右に移動
		ShotCharge,     // ①単発弾: 溜め
		ShotRecover,    // ①単発弾: 撃った後の硬直
		OrbitAttack,    // ②回転する電気玉
		SplitMove,      // ③分裂: 中央へ移動
		SplitCharge,    // ③分裂: 中央で溜め
		SplitApart,     // ③分裂: 左右に分かれる
		ThunderCircle,  // ④雷: 移動の前に、その場で小さく円を描く(クルクル)
		ThunderMove,    // ④雷: 落とす場所へ、ふわっと弧を描いて移動
		ThunderWarn,    // ④雷: 予兆
		ThunderStrike,  // ④雷: 落雷(当たり判定あり)
		ThunderRecover, // ④雷: 落とした後の硬直
		Merge,          // 分身が本体に戻る
	};

	struct Bullet {
		Vector2 center;
		Vector2 velocity;
		float angle;   // 進む向き(長方形の向き)
		int life;
	};

	// 状態の開始
	void EnterIdle();
	void ChooseAttack();
	void StartShot();
	void StartOrbit();
	void StartSplit();
	void StartThunder();
	void StartMerge();
	void BeginThunderCircle();   // 雷の落とし場所へ向かう前の、その場での円の動きを始める
	void BeginThunderTravel();   // 円の動きが終わったら、落とし場所へ弧を描いて移動を始める
	void FireBullets();

	// 毎フレームの処理
	void UpdateBullets();
	void Patrol();
	bool MoveToward(float targetCenterX, float speed);   // 本体を横へ移動(着いたら true)
	void ClampBody();
	float ThunderTargetX(int waypointIndex) const;
	int ThunderWaypointCount() const { return isSplit_ ? 2 : 3; }
	float OrbitScale() const;                             // 電気玉の腕の伸び具合(0〜1)
	void ComputeOrbitCircles(std::vector<AttackCircle>& out) const;

	// 動き回る基準の位置。1体のときは画面の中心、分裂中は2体の中央の軸
	float HomeX() const { return isSplit_ ? axisX_ : screenCenterX_; }
	// 分裂中は2体の中央の軸からの相対位置で動かす(軸が動いても、2体の鏡写しが崩れないように)
	float Anchor() const { return isSplit_ ? axisX_ : 0.0f; }
	float MinCenterX() const;   // 本体の中心が動ける左端(基準 - 画面の半分 + 余白)
	float MaxCenterX() const;   // 本体の中心が動ける右端(基準 + 画面の半分 - 余白)
	bool IsInsideBounds() const;   // 本体が画面の中に戻っているか(外にいる間は攻撃しない)

	float CenterX() const { return pos_.x + size_.x * 0.5f; }
	void SetCenterX(float x) { pos_.x = x - size_.x * 0.5f; }
	Vector2 BodyCenter(int index) const;
	float SplitMinHalfGap() const { return size_.x * 0.5f + kSplitGap; }   // 分裂中、本体が中央に近づける限界(中心から)

	State state_ = State::Idle;
	int timer_ = 0;                  // 状態の残りフレーム(状態によっては経過フレーム)
	float patrolDir_ = 1.0f;         // Idle の移動方向(+1 右 / -1 左)
	bool isFirstAction_ = true;      // 最初の行動かどうか(最初は必ず単発弾)
	bool isSplit_ = false;           // 分裂中か
	int splitAttacksLeft_ = 0;       // 分裂が解けるまでの残り攻撃回数
	float orbitAngle_ = 0.0f;        // 電気玉の腕の回転角
	int thunderIndex_ = 0;           // 雷の何か所目か
	Vector2 target_{};               // プレイヤーの中心(毎フレーム更新)
	float screenCenterX_ = kWorldWidth * 0.5f;   // 画面の中心X(毎フレーム更新)。分裂の中央・鏡写しの軸にもなる
	float screenHalfWidth_ = 960.0f;             // 画面の幅の半分(毎フレーム更新)
	float axisX_ = kWorldWidth * 0.5f;           // 分裂中の鏡写しの軸(2体の中央)。移動(Idle)のときだけ画面の中心へゆっくりついていく。攻撃中は動かない
	float attackHomeX_ = 0.0f;                   // いまの攻撃(雷・分裂)の中心。攻撃を始めたときに決めて、ワールド座標で動かさない

	// ④雷の移動(その場で円 → 弧を描く移動)。位置は Anchor() からの相対で持つ
	float circleBaseX_ = 0.0f;      // 円を描く前の位置(円の中心)
	float moveStartX_ = 0.0f;       // 移動の出発点
	float moveTargetX_ = 0.0f;      // 移動の目標(落とす場所)
	int moveFrames_ = 1;            // 移動にかかる時間
	float moveArcHeight_ = 0.0f;    // 弧の高さ
	std::vector<Bullet> bullets_;

	// ---- 画面・高さ ----
	static constexpr float kScreenMargin = 40.0f;                // 画面の端からあける余白(雷の左端・右端もここ)
	static constexpr float kFollowSpeed = 4.0f;                  // 画面から置いていかれたとき、戻ってくる速さ(プレイヤーが走る速さ5より遅いと、走って逃げ切れる)
	static constexpr float kGroundY = 0.0f;                      // 地面の高さ(Y上向き)
	static constexpr float kHoverY = 420.0f;                     // Idle で浮かぶ高さ(posのY = 雲の上端)
	static constexpr float kHoverGain = 0.1f;                    // 浮かぶ高さへ戻る速さ

	// ---- Idle ----
	static constexpr int kIdleFrames = 120;                      // 攻撃前に左右へ動く時間
	static constexpr float kPatrolSpeed = 3.0f;                  // 左右の移動速度
	static constexpr float kPatrolHalfRange = 700.0f;            // 左右に動く範囲(画面の中心から。画面に収まるよう自動で狭まる)

	// ---- 行動の確率(%)。分裂していないとき ----
	static constexpr int kShotChance = 35;                       // ①単発弾
	static constexpr int kOrbitChance = 30;                      // ②電気玉
	static constexpr int kSplitChance = 15;                      // ③分裂   (残りが④雷)
	// 分裂中(③は選ばれない): ①35% / ②30% / ④35%
	static constexpr int kSplitShotChance = 35;
	static constexpr int kSplitOrbitChance = 30;

	// ---- ①単発弾 ----
	static constexpr int kShotChargeFrames = 30;                 // 撃つ前の溜め
	static constexpr int kShotRecoverFrames = 35;                // 撃った後の硬直
	static constexpr float kBulletSpeed = 15.0f;                 // 弾の速さ
	static constexpr float kBulletLength = 56.0f;                // 弾の長さ(進む向き)
	static constexpr float kBulletWidth = 20.0f;                 // 弾の幅
	static constexpr int kBulletLife = 360;                      // 弾が消えるまでのフレーム

	// ---- ②回転する電気玉(ガーディアン) ----
	static constexpr int kOrbitArms = 2;                         // 腕の数(等間隔)
	static constexpr int kOrbitBalls = 7;                        // 1本の腕の電気玉の数
	static constexpr float kOrbitInnerRadius = 150.0f;           // 一番内側の電気玉までの距離(雲の幅の半分より大きくする)
	static constexpr float kOrbitSpacing = 44.0f;                // 電気玉どうしの間隔
	static constexpr float kOrbitBallRadius = 22.0f;             // 電気玉1個の半径(当たり判定)
	static constexpr float kOrbitSpeed = 0.05f;                  // 回転速度(ラジアン/フレーム。0.05 で約2.1秒に1周)
	static constexpr int kOrbitTotalFrames = 300;                // 攻撃の長さ
	static constexpr int kOrbitExtendFrames = 40;                // 腕が伸びる時間
	static constexpr int kOrbitRetractFrames = 40;               // 腕が縮む時間

	// ---- ③分裂 ----
	static constexpr float kSplitMoveSpeed = 10.0f;              // 中央へ向かう速さ
	static constexpr int kSplitChargeFrames = 40;                // 中央での溜め
	static constexpr float kSplitSeparation = 360.0f;            // 分かれた後の、中央から各体までの距離
	static constexpr float kSplitGap = 20.0f;                    // 2体のあいだに空ける余白
	static constexpr int kSplitAttackCount = 3;                  // 分裂が解けるまでに行う攻撃の回数
	static constexpr float kMergeSpeed = 8.0f;                   // 本体に戻る速さ

	// ---- ④雷 ----
	static constexpr int kThunderCircleFrames = 48;              // 移動前に、その場で円を描く時間
	static constexpr float kThunderCircleLaps = 2.0f;            // 円を何周するか
	static constexpr float kThunderCircleRadius = 30.0f;         // 円の大きさ(半径px。始めと終わりは小さく、真ん中で最大)
	static constexpr float kThunderAvgSpeed = 11.0f;             // 移動の平均の速さ(px/フレーム。ゆっくり出て、ゆっくり止まる)
	static constexpr int kThunderMinTravelFrames = 45;           // 移動にかかる時間の下限(近いときも急がない)
	static constexpr int kThunderMaxTravelFrames = 130;          // 移動にかかる時間の上限(遠いときの最大)
	static constexpr float kThunderArcHeight = 90.0f;            // 移動の弧の高さの上限(px。上へふわっと持ち上がる)
	static constexpr float kThunderArcRatio = 0.15f;             // 弧の高さ = 移動距離 × この値(短い移動では弧も小さい)
	static constexpr int kThunderWarnFrames = 30;                // 予兆の長さ
	static constexpr int kThunderBlinkInterval = 6;              // 予兆の点滅の間隔(フレーム。この間隔ごとに 表示 ⇔ 非表示)
	static constexpr int kThunderStrikeFrames = 18;              // 落雷の長さ(当たり判定あり)
	static constexpr int kThunderRecoverFrames = 15;             // 落とした後の硬直
	static constexpr float kThunderWidth = 256.0f;                // 雷の太さ

	// ---- 移動の共通 ----
	static constexpr float kApproachGain = 0.1f;                 // 目標に近づくほど遅くする割合
	static constexpr float kMinApproachStep = 2.0f;              // 近づいたときの最低速度(止まりきらないのを防ぐ)
	static constexpr float kArriveDistance = 2.0f;               // これ以内なら「着いた」とみなす
};