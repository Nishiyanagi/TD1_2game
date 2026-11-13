#include <Novice.h>
#include "Player.h"
#include "Weapon.h"
#include "Vector2.h"
#include "Line.h"
#include "Enemy1.h"
#include "Random.h"
#include "Collision.h"
#include "Cooldown.h"
#ifdef USE_IMGUI
#include <imgui.h>
#endif

Line* line = new Line(Vector2(0.0f, 0.0f), Vector2(1920.0f, 0.0f));

const char kWindowTitle[] = "LC1B_16_ニシヤナギ_リョウマ_タイトル";

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1920, 1080);

	Random::Init();

	Enemy1* enemy1 = new Enemy1(Vector2(1200.0f, 100.0f), Vector2(100.0f, 100.0f), Vector2(0.0f, 0.0f), 100);

	Player* player = new Player(Vector2(640.0f, 50.0f), Vector2(5.0f, 5.0f), Vector2(50.0f, 50.0f),100.0f);
	Weapon* weapon = new Weapon(Vector2(640.0f, 25.0f), Vector2(50.0f, 25.0f), Vector2(5.0f, 5.0f));

	Cooldown playerInvincible{ 60 };   // プレイヤーの被弾後の無敵時間(フレーム)
	bool weaponHitDone = false;        // 今回の攻撃でもう当てたか(多段ヒット防止)
	const int kWeaponDamage = 5;
	const float kContactDamage = 10.0f;

	float kWorldOriginY = 800.0f; // ワールド座標系の原点のY座標
	
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

		if (player->Attack())
		{
			weapon->Move(keys, weapon->Velocity());
		}
		else
		{
			weapon->Pos() = player->Pos();
			player->Jump(keys, preKeys, *player->Velocity());
			player->Move(keys, *player->Velocity());
		}

		player->Defence();

		player->attackTimer();

		weapon->SetActive(player->Attack());

		weapon->IsActive();

#pragma region Collision

		playerInvincible.Update();

		const Rect playerRect = MakeRect(player->Pos(), player->Size());
		const Rect enemyRect = enemy1->GetRect();

		// 敵 → プレイヤー(接触ダメージ)
		if (enemy1->IsAttacking() && playerInvincible.IsReady() && IsHit(playerRect, enemyRect)) {
			player->Damage(kContactDamage);
			playerInvincible.Start();
		}

		// 武器 → 敵(1回の攻撃につき1ヒット)
		if (weapon->IsActive()) {
			const Rect weaponRect = MakeRect(weapon->Pos(), weapon->Size());
			if (!weaponHitDone && IsHit(weaponRect, enemyRect)) {
				enemy1->Damage(kWeaponDamage);
				weaponHitDone = true;
			}
		}
		else {
			weaponHitDone = false;
		}

#pragma endregion

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

#pragma region Enemy1

		Novice::DrawBox(static_cast<int>(enemy1->Pos().x), static_cast<int>(enemy1->Pos().y * -1 + kWorldOriginY),
			static_cast<int>(enemy1->Size().x), static_cast<int>(enemy1->Size().y), 0.0f, 0x40C040ff, kFillModeSolid);

#pragma endregion

		if (player->Attack()) {//攻撃時
			Novice::DrawBox(static_cast<int>(player->Pos().x), static_cast<int>(player->Pos().y * -1 + kWorldOriginY), static_cast<int>(player->Size().x), static_cast<int>(player->Size().y), 0.0f, 0xD06060ff, kFillModeSolid);

		}
		else if (player->Defence()) {//ガード時
			Novice::DrawBox(static_cast<int>(player->Pos().x), static_cast<int>(player->Pos().y * -1 + kWorldOriginY), static_cast<int>(player->Size().x), static_cast<int>(player->Size().y), 0.0f, 0x6060D0ff, kFillModeSolid);

		}
		else {//攻撃、ガード無し
			Novice::DrawBox(static_cast<int>(player->Pos().x), static_cast<int>(player->Pos().y * -1 + kWorldOriginY), static_cast<int>(player->Size().x), static_cast<int>(player->Size().y), 0.0f, WHITE, kFillModeSolid);

		}

		if (weapon->IsActive()) {
			Novice::DrawBox(static_cast<int>(weapon->Pos().x), static_cast<int>(weapon->Pos().y * -1 + kWorldOriginY), static_cast<int>(weapon->Size().x), static_cast<int>(weapon->Size().y), 0.0f, BLACK, kFillModeSolid);
		}

		Novice::DrawLine(static_cast<int>(line->StartPos().x), static_cast<int>(line->StartPos().y * -1 + kWorldOriginY), static_cast<int>(line->EndPos().x), static_cast<int>(line->EndPos().y * -1 + kWorldOriginY), 0xffffffff);

#ifdef _DEBUG
		Novice::ScreenPrintf(0, 0, "JumpCount:%.1f", player->JumpCount());
		if (player->Attack()) {//攻撃時
			Novice::ScreenPrintf(0, 20, "Acction:Attack");

		}
		else if (player->Defence()) {//ガード時
			Novice::ScreenPrintf(0, 20, "Acction:Defence");

		}
		else {//攻撃、ガード無し
			Novice::ScreenPrintf(0, 20, "Acction:None");

		}

		Novice::ScreenPrintf(0, 60, "PlayerHP:%.1f", player->HP());
		Novice::ScreenPrintf(0, 80, "EnemyHP:%d", enemy1->Hp());

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
