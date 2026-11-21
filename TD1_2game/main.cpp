#include <Novice.h>
#include "Player.h"
#include "Pad.h"
#include "Weapon.h"
#include "Vector2.h"
#include "Line.h"
#include "Enemy1.h"
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
static void DrawWorldBox(const Camera& camera, const Vector2& pos, const Vector2& size, unsigned int color)
{
	const Vector2 leftTop = camera.WorldToScreen(pos);
	const Vector2 rightBottom = camera.WorldToScreen({ pos.x + size.x, pos.y - size.y });

	Novice::DrawBox(
		static_cast<int>(leftTop.x), static_cast<int>(leftTop.y),
		static_cast<int>(rightBottom.x - leftTop.x), static_cast<int>(rightBottom.y - leftTop.y),
		0.0f, color, kFillModeSolid);

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
Camera* camera;
Player* player;

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, kWindowWidth, kWindowHeight);

	Random::Init();

	Enemy1* enemy1 = new Enemy1(Vector2(1200.0f, 100.0f), Vector2(100.0f, 100.0f), Vector2(0.0f, 0.0f), 100.0f);

	Weapon* weapon = new Weapon(Vector2(640.0f, 25.0f), Vector2(50.0f, 25.0f), Vector2(5.0f, 5.0f));

	Cooldown playerInvincible{ 60 };	// プレイヤーの被弾後の無敵時間(フレーム)
	bool weaponHitDone = false;			// 今回の攻撃でもう当てたか(多段ヒット防止)
	Cooldown weaponHitTimer{ 12 };		// 攻撃が連続で入るようにするための間隔のタイマ
	const int kWeaponDamage = 5;
	const float kContactDamage = 2.0f;
	float kWorldOriginY = 800.0f; // 従来の「ワールドのY=0が画面のどこか」(画面の見た目を変えないために使う)

	bool isPadAttackTrigger = false; //パッドで攻撃したか
	// カメラ: 今までと同じ見た目(Y=0が画面の800px)になるよう、カメラ中心のYを決める
	const float kCameraY = kWorldOriginY - kWindowHeight * 0.5f;

	camera = new Camera(
		Vector2(kWindowWidth * 0.5f, kCameraY),
		Vector2(static_cast<float>(kWindowWidth), static_cast<float>(kWindowHeight)),
		kWorldWidth,
		kCameraY,            // minY: 今の見た目(地面が画面の800px)より下には行かない
		kCameraY + 1000.0f); // maxY: 上方向にどこまでスクロールするか

	player = new Player(Vector2(640.0f, 50.0f), Vector2(5.0f, 5.0f), Vector2(50.0f, 50.0f), 100.0f, camera);
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
		enemy1->Move(playerCenter);

#pragma endregion

		if (player->Attack(weapon, mouseDirection))
		{
			player->attackTimer();
			if (!player->isSingleAttack())
			{
				weapon->Move(inputKeys, weapon->Velocity(), pad);
			}
		}
		else
		{
			if (!player->isSingleAttack())
			{
				weapon->PosReset(player->Pos());
			}

			player->Jump(inputKeys, inputPreKeys, *player->Velocity());
			if (!player->Defence()) {
				player->Move(inputKeys, *player->Velocity(), pad);

			}
			pad->Input();
		}

		if (player->isSingleAttack())
		{
			// パッド操作中の mouseDirection は純粋な方向なので、マウス用のスクロール補正は入れない
			const Vector2 scrollForAim = InputDevice::UsePad() ? Vector2{ 0.0f, 0.0f } : camera->ScrollPos();
			weapon->SingleAttack(weapon->Velocity(), mouseDirection, scrollForAim,isPadAttackTrigger/*, pad*/);
		}

		player->Defence();

		// 追加: プレイヤーがステージの外に出ないようにする(Player に ClampX を追加する必要あり)
		player->ClampX(0.0f, kWorldWidth);

		player->attackTimer();

		weapon->SetActive(player->Attack(weapon, mouseDirection) || player->isSingleAttack());

		weapon->IsActive();


#pragma region Collision

		playerInvincible.Update();

		const Rect playerRect = MakeRect(player->Pos(), player->Size());
		const Rect enemyRect = enemy1->GetRect();

		// 敵 → プレイヤー(接触ダメージ。敵の状態にかかわらず、重なっていれば入る)
		if (playerInvincible.IsReady() && IsHit(playerRect, enemyRect)) {
			player->Damage(kContactDamage);
			playerInvincible.Start();
		}

		// 武器 → 敵(1回の攻撃につき1ヒット)
		if (weapon->IsActive()) {
			const Rect weaponRect = MakeRect(weapon->Pos(), weapon->Size());
			if (!weaponHitDone && IsHit(weaponRect, enemyRect)) {
				enemy1->Damage(kWeaponDamage);
				weaponHitTimer.Start();
				weaponHitDone = true;

				// 一定確率でリジェネのアイテムが敵の位置に出る
				if (Random::Chance(kDropChance)) {
					drops.push_back(HealDrop(Vector2(
						enemy1->Pos().x + enemy1->Size().x * 0.5f,
						enemy1->Pos().y - enemy1->Size().y * 0.5f)));
				}
			}
			else if (weaponHitDone) {
				weaponHitTimer.Update();
				if (weaponHitTimer.IsReady()) {
					weaponHitDone = false;
				}

				player->SetSingleAttack(false);
			}
		}
		else {
			weaponHitDone = false;
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

		// 【仮】スクロールが分かりやすいよう、ステージに縦線を引く(不要になったら消してください)
		for (float x = 0.0f; x <= kWorldWidth; x += 240.0f) {
			DrawWorldLine(*camera, Vector2(x, 800.0f), Vector2(x, 0.0f), 0x404040ff);
		}

#endif // DEBUG	

#pragma region Enemy1

		DrawWorldBox(*camera, enemy1->Pos(), enemy1->Size(), 0x40C040ff);

#pragma endregion

		// 回復アイテム(ドロップ)
		for (const HealDrop& drop : drops) {
			if (drop.ShouldDraw()) {
				DrawWorldBox(*camera, drop.Pos(), drop.Size(), 0x60FF90ff);
			}
		}

		if (player->Attack(weapon, mouseDirection)) {//攻撃時
			DrawWorldBox(*camera, player->Pos(), player->Size(), 0xD06060ff);
		}
		else if (player->Defence()) {//ガード時
			DrawWorldBox(*camera, player->Pos(), player->Size(), 0x6060D0ff);
		}
		else {//攻撃、ガード無し
			DrawWorldBox(*camera, player->Pos(), player->Size(), WHITE);
		}

		// 武器の描画
		if (weapon->IsActive()) {
			DrawWorldBox(*camera, weapon->Pos(), weapon->Size(), BLACK);
		}

		// 地面
		DrawWorldLine(*camera, line->StartPos(), line->EndPos(), 0xffffffff);

#ifdef _DEBUG
		Novice::ScreenPrintf(0, 0, "JumpCount:%.1f", player->JumpCount());
		if (player->Attack(weapon, mouseDirection)) {//攻撃時
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
		Novice::ScreenPrintf(0, 80, "EnemyHP:%.1f", enemy1->Hp());
		Novice::ScreenPrintf(0, 120, "Camera:%.1f %.1f", camera->Pos().x, camera->Pos().y);
		Novice::ScreenPrintf(0, 140, "Flask:%s  Regen:x%d", flask.IsReady() ? "READY" : "COOLDOWN", regen.Stacks());
		Novice::ScreenPrintf(0, 160, "Input:%s  (K to change)", InputDevice::UsePad() ? "PAD" : "KEYBOARD+MOUSE");
		int debugLx = 0;
		int debugLy = 0;
		InputDevice::GetAnalogInputLeft(&debugLx, &debugLy);
		Novice::ScreenPrintf(0, 180, "LStick:%d %d", debugLx, debugLy);
		Novice::ScreenPrintf(300, 180, "AimDir:%.1f %.1f", mouseDirection.x, mouseDirection.y);

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