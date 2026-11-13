#pragma once
#include "Vector2.h"

class Line {
public:
	Line(Vector2 StartPosition,Vector2 EndPosition);

	Vector2 StartPos();
	Vector2 EndPos();
private:
	Vector2 startPos_{};
	Vector2 endPos_{};
};

extern Line* line;