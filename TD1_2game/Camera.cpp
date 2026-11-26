#include "Camera.h"
#include "Player.h"
#include <Novice.h>

Camera::Camera(Vector2 position, Vector2 screenSize, float worldWidth, float minY, float maxY)
	: pos_{ position },
	scrollPos_{ 0.0f,0.0f },
	screenSize_{ screenSize },
	worldWidth_{ worldWidth },
	minY_{ minY },
	maxY_{ maxY }
{
	ClampToWorld();
	UpdateMatrix();
}

void Camera::Update(const Vector2& targetPos)
{
	// 追いかける目標を、先にカメラが映せる範囲に収めておく。
	// (範囲の外の目標を追いかけて、最後に ClampToWorld で急に止めると、
	//  画面の中のプレイヤーの動きが、その瞬間だけ急に速く見えてしまう)
	const Vector2 target = ClampTarget(targetPos);

	// ガクッとならないよう、少しずつ近づける(縦は横よりゆっくり)
	pos_.x += (target.x - pos_.x) * kFollow;
	pos_.y += (target.y - pos_.y) * kFollowY;

	ClampToWorld();
	UpdateMatrix();
}

Vector2 Camera::ClampTarget(const Vector2& target) const
{
	Vector2 result = target;

	// 横: ステージが画面より狭いときは中央、広いときはステージの端まで
	const float halfWidth = screenSize_.x * 0.5f;
	if (worldWidth_ <= screenSize_.x) {
		result.x = worldWidth_ * 0.5f;
	}
	else if (result.x < halfWidth) {
		result.x = halfWidth;
	}
	else if (result.x > worldWidth_ - halfWidth) {
		result.x = worldWidth_ - halfWidth;
	}

	// 縦
	if (result.y < minY_) { result.y = minY_; }
	else if (result.y > maxY_) { result.y = maxY_; }

	return result;
}

Vector2 Camera::ScrollPos() {
	return scrollPos_;
}
Vector2 Camera::WorldToScreen(const Vector2& worldPos) const
{
	return Transform(worldPos, vpMatrix_);
}

Vector2 Camera::ScreenToWorld(const Vector2& screenPos) const
{
	return Transform(screenPos, Inverse(vpMatrix_));
}

void Camera::ClampToWorld()
{
	const float halfWidth = screenSize_.x * 0.5f;
	// ステージが画面より狭いときは中央に固定
	if (worldWidth_ <= screenSize_.x) {
		pos_.x = worldWidth_ * 0.5f;
		return;
	}

	if (pos_.x < halfWidth) {
		pos_.x = halfWidth;
	}
	else if (pos_.x > worldWidth_ - halfWidth) {
		pos_.x = worldWidth_ - halfWidth;
	}
	Novice::ScreenPrintf(500, 0, "worldWidth_:%f", worldWidth_);

	// 縦の範囲を制限
	if (pos_.y < minY_) { pos_.y = minY_; }
	else if (pos_.y > maxY_) { pos_.y = maxY_; }
}

void Camera::UpdateMatrix()
{
	const float halfWidth = screenSize_.x * 0.5f;
	const float halfHeight = screenSize_.y * 0.5f;

	// ビュー行列: カメラのワールド行列の逆行列
	const Matrix3x3 viewMatrix = Inverse(MakeTranslateMatrix(pos_));

	// 正射影行列: カメラが映す範囲(カメラ中心から上下左右に画面の半分)を -1〜1 に収める
	const Matrix3x3 orthoMatrix = MakeOrthographicMatrix(-halfWidth, halfHeight, halfWidth, -halfHeight);

	// ビューポート行列: -1〜1 を画面のピクセル座標に広げる
	const Matrix3x3 viewportMatrix = MakeViewportMatrix(0.0f, 0.0f, screenSize_.x, screenSize_.y);

	// ワールド座標のオブジェクトにはワールド行列が要らないので、ビュー → 正射影 → ビューポートの順
	vpMatrix_ = Multiply(Multiply(viewMatrix, orthoMatrix), viewportMatrix);
}