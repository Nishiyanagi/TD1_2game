#pragma once

// 乱数ユーティリティ
namespace Random {

	// 乱数の初期化 (ゲーム開始時に1回だけ呼ぶ)
	void Init();

	// min 以上 max 以下の整数 (両端を含む)
	int Range(int min, int max);

	// min 以上 max 以下の小数
	float RangeF(float min, float max);

	// percent% の確率で true (0〜100)
	bool Chance(int percent);

} // namespace Random