#pragma once
#include "Vector2.h"

class Player {
public:

	void Jump();
	void Move();

	Vector2 Pos()const;

private:
	Vector2 pos_{};
	Vector2 size_{};
	Vector2 velocity_{};
	Vector2 acceleration_{};

};