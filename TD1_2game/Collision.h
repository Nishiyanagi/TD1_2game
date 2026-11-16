#pragma once
#include "Vector2.h"

// 矩形(Y上向き座標なので top > bottom)
struct Rect {
	float left;
	float right;
	float top;
	float bottom;
};

// pos(左上)と size から矩形を作る
inline Rect MakeRect(const Vector2& pos, const Vector2& size)
{
	return { pos.x, pos.x + size.x, pos.y, pos.y - size.y };
}

// 矩形同士の当たり判定(辺が触れただけでは当たらない)
inline bool IsHit(const Rect& a, const Rect& b)
{
	return a.left < b.right && a.right > b.left &&
		a.bottom < b.top && a.top > b.bottom;
}