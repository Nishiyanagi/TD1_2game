#include <Novice.h>
#include "Player.h"
#include "Weapon.h"
#include "Vector2.h"
#include "Line.h"
#ifdef USE_IMGUI
#include <imgui.h>
#endif

Line* line = new Line(Vector2(0.0f, 0.0f), Vector2(1280.0f, 0.0f));

const char kWindowTitle[] = "LC1B_16_ニシヤナギ_リョウマ_タイトル";

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1920, 1080);

	Player* player = new Player(Vector2(640.0f, 50.0f), Vector2(5.0f, 5.0f), Vector2(50.0f, 50.0f));
	Weapon* weapon = new Weapon(Vector2(640.0f, 50.0f), Vector2(50.0f, 25.0f));

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

		player->Move(keys, *player->Velocity());
		player->Jump(keys, preKeys, *player->Velocity());

		player->Defence();

		if (player->attackTimer() >= 90)
		{
			if (player->Attack())
			{
				weapon->SetActive(true);
			}
			else
			{
				weapon->SetActive(false);
			}
		}
		else
		{
			weapon->SetActive(false);
		}

		weapon->IsActive();

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		if (player->Attack()) {//攻撃時
			Novice::DrawBox(static_cast<int>(player->Pos().x), static_cast<int>(player->Pos().y * -1 + 600), static_cast<int>(player->Size().x), static_cast<int>(player->Size().y), 0.0f, 0xD06060ff, kFillModeSolid);

		}
		else if (player->Defence()) {//ガード時
			Novice::DrawBox(static_cast<int>(player->Pos().x), static_cast<int>(player->Pos().y * -1 + 600), static_cast<int>(player->Size().x), static_cast<int>(player->Size().y), 0.0f, 0x6060D0ff, kFillModeSolid);

		}
		else {//攻撃、ガード無し
			Novice::DrawBox(static_cast<int>(player->Pos().x), static_cast<int>(player->Pos().y * -1 + 600), static_cast<int>(player->Size().x), static_cast<int>(player->Size().y), 0.0f, WHITE, kFillModeSolid);

		}

		if (weapon->IsActive()) {
			Novice::DrawBox(static_cast<int>(weapon->Pos().x), static_cast<int>(weapon->Pos().y * -1 + 600), static_cast<int>(weapon->Size().x), static_cast<int>(weapon->Size().y), 0.0f, BLACK, kFillModeSolid);
		}

		Novice::DrawLine(static_cast<int>(line->StartPos().x), static_cast<int>(line->StartPos().y * -1 + 600), static_cast<int>(line->EndPos().x), static_cast<int>(line->EndPos().y * -1 + 600), 0xffffffff);

#ifdef _DEBUG
		Novice::ScreenPrintf(0, 0, "JumpCount:%f", player->JumpCounte());
		if (player->Attack()) {//攻撃時
			Novice::ScreenPrintf(0, 20, "Acction:Attack");

		}
		else if (player->Defence()) {//ガード時
			Novice::ScreenPrintf(0, 20, "Acction:Defence");

		}
		else {//攻撃、ガード無し
			Novice::ScreenPrintf(0, 20, "Acction:None");

		}

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
