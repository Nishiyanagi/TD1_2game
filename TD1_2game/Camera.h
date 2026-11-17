#pragma once
#include "Vector2.h"
#include "Matrix3x3.h"

// 横スクロールのカメラ
// ワールド座標(Y上向き)を、行列でスクリーン座標(Y下向き)に変換する
class Camera
{
public:
	// position: カメラの中心にあたるワールド座標
	// screenSize: 画面のサイズ(px)
	// worldWidth: ステージ全体の幅
	Camera(Vector2 position, Vector2 screenSize, float worldWidth, float minY, float maxY);

	// targetPos(プレイヤーの中心など)を追いかける。毎フレーム、描画の前に呼ぶ
	void Update(const Vector2& targetPos);

	// ワールド座標 → スクリーン座標
	Vector2 WorldToScreen(const Vector2& worldPos) const;

	// スクリーン座標 → ワールド座標(マウス位置をワールドで使いたいとき用)
	Vector2 ScreenToWorld(const Vector2& screenPos) const;

	const Vector2& Pos() const { return pos_; }

private:
	void ClampToWorld();   // カメラがステージの外を映さないようにする
	void UpdateMatrix();   // ビュー × 正射影 × ビューポート を作り直す

	Vector2 pos_{};          // カメラの中心(ワールド座標)
	Vector2 screenSize_{};
	float worldWidth_ = 0.0f;
	Matrix3x3 vpMatrix_{};   // ワールド → スクリーンの合成行列
	float minY_ = 0.0f;   // カメラ中心Yの下限
	float maxY_ = 0.0f;   // カメラ中心Yの上限

	static constexpr float kFollow = 0.1f;  // 追従のなめらかさ(0〜1。小さいほどゆっくり)
};