#pragma once
#include "Vector2.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <assert.h>

// 3x3行列(行ベクトル方式: 座標 * 行列。平行移動は m[2][0], m[2][1])
struct Matrix3x3 {
	float m[3][3];
};

// 単位行列
Matrix3x3 MakeIdentityMatrix();

// 平行移動行列
Matrix3x3 MakeTranslateMatrix(Vector2 translate);

// 回転行列
Matrix3x3 MakeRotateMatrix(float theta);

// 拡縮行列
Matrix3x3 MakeScaleMatrix(Vector2 scale);

// 正射影行列
Matrix3x3 MakeOrthographicMatrix(float left, float top, float right, float bottom);

// ビューポート行列
Matrix3x3 MakeViewportMatrix(float left, float top, float width, float height);

// 逆行列(存在しない場合は全て0の行列)
Matrix3x3 Inverse(Matrix3x3 matrix);

// 行列の積(matrix1 を先に、matrix2 を後に適用する)
Matrix3x3 Multiply(Matrix3x3 matrix1, Matrix3x3 matrix2);

// 2次元ベクトルを同次座標系として変換する
Vector2 Transform(Vector2 vector, Matrix3x3 matrix);