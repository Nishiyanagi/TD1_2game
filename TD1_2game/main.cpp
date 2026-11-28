#include <Novice.h>
#include "Player.h"
#include "Pad.h"
#include "Weapon.h"
#include "Vector2.h"
#include "Line.h"
#include "EnemyCloud.h"
#include "Random.h"
#include "Collision.h"
#include "Cooldown.h"
#include "Camera.h"   // 追加
#include "World.h"    // 追加
#include "HealFlask.h"    // 追加(回復)
#include "HealDrop.h"     // 追加(回復)
#include "RegenEffect.h"  // 追加(回復)
#include <vector>        // 追加(回復)
#include <algorithm>     // 追加(回復)
#include <math.h>         // 追加(弾の回転の描画: cosf / sinf)
#include "InputDevice.h"  // 追加(操作デバイスの切り替え)
#ifdef USE_IMGUI
#include <imgui.h>
#endif

const int kWindowWidth = 1920;   // 追加
const int kWindowHeight = 1080;  // 追加

// ステージ全体の幅(kWorldWidth)まで伸ばす
Line* line = new Line(Vector2(0.0f, 0.0f), Vector2(kWorldWidth, 0.0f));

Pads* pad = new Pads;

static int IsTriggerButton(int stickNo, PadButton button);

const char kWindowTitle[] = "LC1B_16_ニシヤナギ_リョウマ_タイトル";

// ワールド座標の矩形(pos は左上・Y上向き)をカメラ越しに描画する
static void DrawWorldBox(const Camera& camera, const Vector2& pos, const Vector2& size, const int graphHandle, unsigned int color)
{
	const Vector2 leftTop = camera.WorldToScreen(pos);
	const Vector2 rightTop = camera.WorldToScreen({ pos.x + size.x, pos.y });
	const Vector2 leftBottom = camera.WorldToScreen({ pos.x, pos.y - size.y });
	const Vector2 rightBottom = camera.WorldToScreen({ pos.x + size.x, pos.y - size.y });

	/*Novice::DrawBox(
		static_cast<int>(leftTop.x), static_cast<int>(leftTop.y),
		static_cast<int>(rightBottom.x - leftTop.x), static_cast<int>(rightBottom.y - leftTop.y),
		0.0f, color, kFillModeSolid);*/

	Novice::DrawQuad(
		static_cast<int>(leftTop.x), static_cast<int>(leftTop.y),
		static_cast<int>(rightTop.x), static_cast<int>(rightTop.y),
		static_cast<int>(leftBottom.x), static_cast<int>(leftBottom.y),
		static_cast<int>(rightBottom.x), static_cast<int>(rightBottom.y),
		0, 0, static_cast<int>(size.x), static_cast<int>(size.y), graphHandle, color);

}

// ワールド座標の回転した長方形(中心・大きさ・角度)をカメラ越しに描画する
// size.x が角度の向きの長さ、size.y が幅。angle は 0 が右向きで、反時計回りが正(Y上向きのワールド座標)
static void DrawWorldRotatedBox(const Camera& camera, const Vector2& center, const Vector2& size, float angle, const int graphHandle, unsigned int color)
{
	const float c = cosf(angle);
	const float s = sinf(angle);
	const float hl = size.x * 0.5f;
	const float hw = size.y * 0.5f;

	// 角度の向き(u)と、その垂直(v)に沿った四隅(DrawWorldBox と同じ並び: 左上・右上・左下・右下)
	const Vector2 leftTop = camera.WorldToScreen({ center.x - c * hl - s * hw, center.y - s * hl + c * hw });
	const Vector2 rightTop = camera.WorldToScreen({ center.x + c * hl - s * hw, center.y + s * hl + c * hw });
	const Vector2 leftBottom = camera.WorldToScreen({ center.x - c * hl + s * hw, center.y - s * hl - c * hw });
	const Vector2 rightBottom = camera.WorldToScreen({ center.x + c * hl + s * hw, center.y + s * hl - c * hw });

	Novice::DrawQuad(
		static_cast<int>(leftTop.x), static_cast<int>(leftTop.y),
		static_cast<int>(rightTop.x), static_cast<int>(rightTop.y),
		static_cast<int>(leftBottom.x), static_cast<int>(leftBottom.y),
		static_cast<int>(rightBottom.x), static_cast<int>(rightBottom.y),
		0, 0, static_cast<int>(size.x), static_cast<int>(size.y), graphHandle, color);
}

// ワールド座標の線分をカメラ越しに描画する
static void DrawWorldLine(const Camera& camera, const Vector2& start, const Vector2& end, unsigned int color)
{
	const Vector2 s = camera.WorldToScreen(start);
	const Vector2 e = camera.WorldToScreen(end);

	Novice::DrawLine(
		static_cast<int>(s.x), static_cast<int>(s.y),
		static_cast<int>(e.x), static_cast<int>(e.y), color);
}
// ワールド座標の円(中心・半径)をカメラ越しに描画する(カメラの拡大率は1なので、半径はそのまま)
static void DrawWorldCircle(const Camera& camera, const Vector2& center, float radius, unsigned int color)
{
	const Vector2 s = camera.WorldToScreen(center);
	Novice::DrawEllipse(
		static_cast<int>(s.x), static_cast<int>(s.y),
		static_cast<int>(radius), static_cast<int>(radius),
		0.0f, color, kFillModeSolid);
}

Camera* camera;
Player* player;

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, kWindowWidth, kWindowHeight);

	Random::Init();

	EnemyCloud* enemycloud = new EnemyCloud(Vector2(1200.0f, 420.0f), Vector2(256.0f, 128.0f), Vector2(0.0f, 0.0f), 100.0f,
		Novice::LoadTexture("./NoviceResources/white1x1.png"));

	Weapon* weapon = new Weapon(Vector2(640.0f, 25.0f), Vector2(48.0f, 48.0f), Vector2(5.0f, 5.0f),
		Novice::LoadTexture("./NoviceResources/white1x1.png"));

	Cooldown playerInvincible{ 60 };	// プレイヤーの被弾後の無敵時間(フレーム)
	const int kWeaponDamage = 5;
	const float kReturnDamageMultiplier = 1.5f;	// ブーメランの復路のダメージ倍率
	const float kContactDamage = 2.0f;
	const float kEnemyAttackDamage = 3.0f;	// 敵の弾・電気玉・雷が当たったときのダメージ
	float kWorldOriginY = 800.0f; // 従来の「ワールドのY=0が画面のどこか」(画面の見た目を変えないために使う)

	//bool isPadAttackTrigger = false; //パッドで攻撃したか

	// カメラ: 今までと同じ見た目(Y=0が画面の800px)になるよう、カメラ中心のYを決める
	const float kCameraY = kWorldOriginY - kWindowHeight * 0.5f;

	camera = new Camera(
		Vector2(kWindowWidth * 0.5f, kCameraY),
		Vector2(static_cast<float>(kWindowWidth), static_cast<float>(kWindowHeight)),
		kWorldWidth,
		kCameraY,            // minY: 今の見た目(地面が画面の800px)より下には行かない
		kCameraY + 1000.0f); // maxY: 上方向にどこまでスクロールするか

	player = new Player(Vector2(640.0f, 64.0f), Vector2(5.0f, 5.0f), Vector2(64.0f, 64.0f), 100.0f, camera,
		Novice::LoadTexture("./NoviceResources/white1x1.png"));
	// 回復(数値はここで調整)
	const int kFlaskCooldown = 600;       // 常備の回復のクールタイム(フレーム。600 = 10秒)
	const float kFlaskAmount = 20.0f;     // 常備の回復の回復量
	const int kRegenFrames = 300;         // リジェネ1個分の持続時間(フレーム。300 = 5秒)
	const int kRegenTickInterval = 60;    // 何フレームに一回回復するか(60 = 1秒に一回)
	const float kRegenTickAmount = 2.0f;  // 一回の回復量(1個分の合計 = 回復する回数 × この値)
	const int kDropChance = 20;           // 攻撃を当てたときにドロップする確率(%)

	HealFlask flask(kFlaskCooldown, kFlaskAmount);
	RegenEffect regen(kRegenFrames, kRegenTickInterval, kRegenTickAmount);
	std::vector<HealDrop> drops;

	Vector2 mouseDirection = {};
	// キー入力結果を受け取る箱
	char keys[256] = { 0 };
	char preKeys[256] = { 0 };

	// ウィンドウの×ボタンが押されるまでループ
	while (Novice::ProcessMessage() == 0) {
		// フレームの開始
		Novice::BeginFrame();

		// キー入力を受け取る
		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		// 操作デバイスの切り替え(仮): K キー。K はどちらのデバイスの操作中でも効く
		// タイトル/ポーズ画面ができたら、ここの代わりに InputDevice::SetUsePad(true / false) を呼ぶ
		if (keys[DIK_K] && !preKeys[DIK_K]) {
			InputDevice::Toggle();
		}

		// ゲーム操作用のキー入力。パッド操作中は全部 0 になる(ゲームの処理にはこちらを渡す)
		char inputKeys[256];
		char inputPreKeys[256];
		InputDevice::FilterKeys(keys, inputKeys);
		InputDevice::FilterKeys(preKeys, inputPreKeys);

		///
		/// ↓更新処理ここから
		///
	/*	ImGui::Begin("");

		ImGui::End();*/

#pragma region Enemy1

		Vector2 playerCenter{
		player->Pos().x + player->Size().x * 0.5f,
		player->Pos().y - player->Size().y * 0.5f
		};
		// 画面(カメラ)の中心と幅の半分を渡す。敵はこの画面の中に収まり、スクロールにもついていく
		enemycloud->Move(playerCenter, camera->Pos().x, kWindowWidth * 0.5f);

#pragma endregion

		player->Jump(inputKeys, inputPreKeys, *player->Velocity());
		if (!player->Defence()) {
			player->Move(inputKeys, *player->Velocity(), pad);
		}
		pad->Input();

		// 重力は常にかかる(攻撃中も、空中にいれば落ちる)
		player->UpdateGravity();

		//if (player->isSingleAttack())
		//{
		//	// パッド操作中の mouseDirection は純粋な方向なので、マウス用のスクロール補正は入れない
		//	const Vector2 scrollForAim = InputDevice::UsePad() ? Vector2{ 0.0f, 0.0f } : camera->ScrollPos();
		//	weapon->SingleAttack(weapon->Velocity(), mouseDirection, scrollForAim, isPadAttackTrigger/*, pad*/);
		//}

		player->Defence();

		// 追加: プレイヤーがステージの外に出ないようにする(Player に ClampX を追加する必要あり)
		player->ClampX(0.0f, kWorldWidth);

		// 攻撃ボタンを押した瞬間にブーメランを投げる状態になる
		player->UpdateAttack(mouseDirection);

		// ブーメラン: 投げる → 飛行の更新 → キャッチ
		const Vector2 playerCenterNow{
			player->Pos().x + player->Size().x * 0.5f,
			player->Pos().y - player->Size().y * 0.5f
		};
		if (player->isSingleAttack() && !weapon->IsFlying()) {
			weapon->Throw(playerCenterNow, mouseDirection);
		}
		if (weapon->UpdateBoomerang(playerCenterNow)) {
			player->SetSingleAttack(false);   // キャッチ → 次の攻撃が可能に
			// ここにキャッチ成功の演出やクールタイム短縮を入れられる
		}

#pragma region Collision

		playerInvincible.Update();

		const Rect playerRect = MakeRect(player->Pos(), player->Size());

		// 敵 → プレイヤー
		if (playerInvincible.IsReady()) {
			// 弾・電気玉・雷(無敵時間中は当たらずに通り抜ける)
			if (enemycloud->HitsPlayer(playerRect)) {
				player->Damage(kEnemyAttackDamage);
				playerInvincible.Start();
			}
			else {
				// 接触ダメージ。敵の状態にかかわらず、重なっていれば入る(分裂中は2体とも)
				for (int i = 0; i < enemycloud->BodyCount(); ++i) {
					if (IsHit(playerRect, enemycloud->BodyRect(i))) {
						player->Damage(kContactDamage);
						playerInvincible.Start();
						break;
					}
				}
			}
		}

		// 武器(ブーメラン) → 敵: 往路と復路で1回ずつ当たる(復路はダメージ増)
		// 分裂中は2体のどちらに当たってもHPは共通。1回の当たりで入るのは1体分だけ
		if (weapon->IsFlying()) {
			const Rect weaponRect = MakeRect(weapon->Pos(), weapon->Size());
			for (int i = 0; i < enemycloud->BodyCount(); ++i) {
				const Vector2 bodyPos = enemycloud->BodyPos(i);
				const Vector2 enemyCenter{
					bodyPos.x + enemycloud->Size().x * 0.5f,
					bodyPos.y - enemycloud->Size().y * 0.5f };

				if (IsHit(weaponRect, enemycloud->BodyRect(i)) && weapon->TryHit(enemyCenter)) {
					const float damage = weapon->IsReturning()
						? kWeaponDamage * kReturnDamageMultiplier
						: static_cast<float>(kWeaponDamage);
					enemycloud->Damage(damage);

					// 一定確率でリジェネのアイテムが敵の位置に出る
					if (Random::Chance(kDropChance)) {
						drops.push_back(HealDrop(enemyCenter, 0));
					}
					break;
				}
			}
		}

#pragma endregion

#pragma region Heal

		// 常備の回復: E キー or パッドの Y(押した瞬間)
		flask.Update();
		const bool healPressed = HealFlask::HealTrigger(inputKeys, inputPreKeys);
		if (healPressed && flask.TryUse()) {
			player->Heal(flask.Amount());
		}

		// リジェネ: 重ね掛けの数だけ、毎フレームの回復量が増える
		player->Heal(regen.Update());

		// ドロップ: 落下・消滅の管理と、プレイヤーが触れたらリジェネ開始
		for (HealDrop& drop : drops) {
			drop.Update();
			if (drop.IsActive() && IsHit(playerRect, drop.GetRect())) {
				regen.AddStack();
				drop.Collect();   // その時点でアイテムは消える
			}
		}
		drops.erase(
			std::remove_if(drops.begin(), drops.end(), [](const HealDrop& drop) { return !drop.IsActive(); }),
			drops.end());

#pragma endregion

		// 追加: カメラの更新(プレイヤーの位置が確定した後、描画の前に行う)
		camera->Update(Vector2(
			player->Pos().x + player->Size().x * 0.5f,
			player->Pos().y - player->Size().y * 0.5f));


		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

#ifdef _DEBUG

		// 仮背景
		//Novice::DrawBox(0, 0, kWindowWidth, kWindowHeight, 0.0f, 0x90ee90FF, kFillModeSolid);

		// 【仮】スクロールが分かりやすいよう、ステージに縦線を引く(不要になったら消してください)
		for (float x = 0.0f; x <= kWorldWidth; x += 240.0f) {
			DrawWorldLine(*camera, Vector2(x, 800.0f), Vector2(x, 0.0f), 0x404040ff);
		}

#endif // DEBUG	

#pragma region Enemy1

		// 雷(予兆は落雷と同じ太さで、赤く点滅。落雷は黄色い柱)
		{
			std::vector<ThunderBolt> bolts;
			enemycloud->GetThunders(bolts);
			for (const ThunderBolt& bolt : bolts) {
				if (!bolt.isVisible) { continue; }   // 点滅で消えているフレーム
				DrawWorldBox(*camera, { bolt.rect.left, bolt.rect.top },
					{ bolt.rect.right - bolt.rect.left, bolt.rect.top - bolt.rect.bottom },
					enemycloud->GraphHandle(), bolt.isStriking ? 0xFFFF40ff : 0xFF2020B0);
			}
		}

		// 雲本体(分裂中は2体)。攻撃の予兆中は黄色くなる
		for (int i = 0; i < enemycloud->BodyCount(); ++i) {
			DrawWorldBox(*camera, enemycloud->BodyPos(i), enemycloud->Size(), enemycloud->GraphHandle(),
				enemycloud->IsTelegraphing() ? 0xFFE040ff : 0x8090B0ff);
		}

		// 単発弾(プレイヤーの方向を向いた長方形)
		{
			std::vector<AttackBox> bullets;
			enemycloud->GetBullets(bullets);
			for (const AttackBox& bullet : bullets) {
				DrawWorldRotatedBox(*camera, bullet.center, bullet.size, bullet.angle, enemycloud->GraphHandle(), 0xFFF060ff);
			}
		}

		// 回転する電気玉
		{
			std::vector<AttackCircle> circles;
			enemycloud->GetDamageCircles(circles);
			for (const AttackCircle& circle : circles) {
				DrawWorldCircle(*camera, circle.center, circle.radius, 0xFFF060ff);
			}
		}

#pragma endregion

		// 回復アイテム(ドロップ)
		for (const HealDrop& drop : drops) {
			if (drop.ShouldDraw()) {
				DrawWorldBox(*camera, drop.Pos(), drop.Size(), drop.GraphHandle(), 0x60FF90ff);
			}
		}

		if (weapon->IsFlying()) {//攻撃時
			DrawWorldBox(*camera, player->Pos(), player->Size(), player->GraphHandle(), 0xD06060ff);
		}
		else if (player->Defence()) {//ガード時
			DrawWorldBox(*camera, player->Pos(), player->Size(), player->GraphHandle(), 0x6060D0ff);
		}
		else {//攻撃、ガード無し
			DrawWorldBox(*camera, player->Pos(), player->Size(), player->GraphHandle(), WHITE);
		}

		// 武器の描画
		if (weapon->IsActive()) {
			DrawWorldBox(*camera, weapon->Pos(), weapon->Size(), weapon->GraphHandle(), BLACK);

		}

		// 地面
		DrawWorldLine(*camera, line->StartPos(), line->EndPos(), 0xffffffff);

#ifdef _DEBUG
		Novice::ScreenPrintf(0, 0, "JumpCount:%.1f", player->JumpCount());
		if (weapon->IsFlying()) {//攻撃時
			Novice::ScreenPrintf(0, 20, "Acction:Attack");

		}
		else if (player->Defence()) {//ガード時
			Novice::ScreenPrintf(0, 20, "Acction:Defence");

		}
		else {//攻撃、ガード無し
			Novice::ScreenPrintf(0, 20, "Acction:None");

		}


		Novice::ScreenPrintf(0, 60, "PlayerHP:%.1f", player->HP());
		Novice::ScreenPrintf(200, 60, "PlayerSpeed:%.1f %.1f", player->Velocity()->x, player->Velocity()->y);
		Novice::ScreenPrintf(0, 80, "EnemyHP:%.1f", enemycloud->Hp());
		Novice::ScreenPrintf(0, 180, "Cloud:%s  Bodies:%d", enemycloud->StateName(), enemycloud->BodyCount());
		Novice::ScreenPrintf(0, 120, "Camera:%.1f %.1f", camera->Pos().x, camera->Pos().y);
		Novice::ScreenPrintf(0, 140, "Flask:%s  Regen:x%d", flask.IsReady() ? "READY" : "COOLDOWN", regen.Stacks());
		Novice::ScreenPrintf(0, 160, "Input:%s  (Kで切り替え)", InputDevice::UsePad() ? "PAD" : "KEYBOARD+MOUSE");

		pad->Show();
		player->Show();

#endif


		///
		/// ↑描画処理ここまで
		///

		// フレームの終了
		Novice::EndFrame();

		// ESCキーが押されたらループを抜ける
		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
			break;
		}
	}

	// ライブラリの終了
	Novice::Finalize();
	return 0;
}