#include "Line.h"
#include <Novice.h>

Line::Line(Vector2 StartPosition, Vector2 EndPosition)
	:startPos_ {StartPosition},
	endPos_{EndPosition}
{}

Vector2 Line::StartPos() {
	return startPos_;
}

Vector2 Line::EndPos() {
	return endPos_;
}

