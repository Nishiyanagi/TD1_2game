#pragma once
#define _USE_MATH_DEFINES
#include <math.h>
#include "Vector2.h"

float theta = (1.0f / 16.0f) * static_cast<float>(M_PI);
float rotateTheta = 0.0f;

Vector2 thetaMove = { sinf(theta), cosf(theta) };