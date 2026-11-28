#include "Matrix3x3.h"
#include <assert.h>

Matrix3x3 MakeIdentityMatrix()
{
	Matrix3x3 result = {};
	result.m[0][0] = 1.0f;
	result.m[1][1] = 1.0f;
	result.m[2][2] = 1.0f;
	return result;
}

Matrix3x3 MakeTranslateMatrix(Vector2 translate)
{
	Matrix3x3 matrix = MakeIdentityMatrix();
	matrix.m[2][0] = translate.x;
	matrix.m[2][1] = translate.y;
	return matrix;
}

Matrix3x3 MakeRotateMatrix(float theta)
{
	Matrix3x3 rotationMatrix{};
	rotationMatrix.m[0][0] = cosf(theta);
	rotationMatrix.m[0][1] = sinf(theta);
	rotationMatrix.m[0][2] = 0.0f;
	rotationMatrix.m[1][0] = -sinf(theta);
	rotationMatrix.m[1][1] = cosf(theta);
	rotationMatrix.m[1][2] = 0.0f;
	rotationMatrix.m[2][0] = 0.0f;
	rotationMatrix.m[2][1] = 0.0f;
	rotationMatrix.m[2][2] = 1.0f;
	return rotationMatrix;
}

Matrix3x3 MakeScaleMatrix(Vector2 scale)
{
	Matrix3x3 scaleMatrix;
	scaleMatrix.m[0][0] = scale.x;
	scaleMatrix.m[0][1] = 0.0f;
	scaleMatrix.m[0][2] = 0.0f;
	scaleMatrix.m[1][0] = 0.0f;
	scaleMatrix.m[1][1] = scale.y;
	scaleMatrix.m[1][2] = 0.0f;
	scaleMatrix.m[2][0] = 0.0f;
	scaleMatrix.m[2][1] = 0.0f;
	scaleMatrix.m[2][2] = 1.0f;
	return scaleMatrix;
}

Matrix3x3 MakeOrthographicMatrix(float left, float top, float right, float bottom)
{
	Matrix3x3 result = MakeIdentityMatrix();
	result.m[0][0] = 2.0f / (right - left);
	result.m[1][1] = 2.0f / (top - bottom);
	result.m[2][0] = (left + right) / (left - right);
	result.m[2][1] = (top + bottom) / (bottom - top);
	return result;
}

Matrix3x3 MakeViewportMatrix(float left, float top, float width, float height)
{
	Matrix3x3 result = MakeIdentityMatrix();
	result.m[0][0] = width / 2.0f;
	result.m[1][1] = -height / 2.0f;   // 画面はYが下向きなので反転
	result.m[2][0] = left + width / 2.0f;
	result.m[2][1] = top + height / 2.0f;
	return result;
}

Matrix3x3 Inverse(Matrix3x3 matrix)
{
	Matrix3x3 result = {};

	const float a = matrix.m[0][0], b = matrix.m[0][1], c = matrix.m[0][2];
	const float d = matrix.m[1][0], e = matrix.m[1][1], f = matrix.m[1][2];
	const float g = matrix.m[2][0], h = matrix.m[2][1], i = matrix.m[2][2];

	const float det = (a * e * i) + (b * f * g) + (c * d * h) - (c * e * g) - (b * d * i) - (a * f * h);
	if (det == 0.0f) {
		return result;  // 逆行列なし
	}
	const float invDet = 1.0f / det;

	result.m[0][0] = (e * i - f * h) * invDet;
	result.m[0][1] = -(b * i - c * h) * invDet;
	result.m[0][2] = (b * f - c * e) * invDet;

	result.m[1][0] = -(d * i - f * g) * invDet;
	result.m[1][1] = (a * i - c * g) * invDet;
	result.m[1][2] = -(a * f - c * d) * invDet;

	result.m[2][0] = (d * h - e * g) * invDet;
	result.m[2][1] = -(a * h - b * g) * invDet;
	result.m[2][2] = (a * e - b * d) * invDet;
	return result;
}

Matrix3x3 Multiply(Matrix3x3 matrix1, Matrix3x3 matrix2)
{
	Matrix3x3 result = {};
	for (int row = 0; row < 3; ++row) {
		for (int col = 0; col < 3; ++col) {
			result.m[row][col] =
				matrix1.m[row][0] * matrix2.m[0][col] +
				matrix1.m[row][1] * matrix2.m[1][col] +
				matrix1.m[row][2] * matrix2.m[2][col];
		}
	}
	return result;
}

Vector2 Transform(Vector2 vector, Matrix3x3 matrix)
{
	Vector2 result;
	result.x = vector.x * matrix.m[0][0] + vector.y * matrix.m[1][0] + 1.0f * matrix.m[2][0];
	result.y = vector.x * matrix.m[0][1] + vector.y * matrix.m[1][1] + 1.0f * matrix.m[2][1];
	const float w = vector.x * matrix.m[0][2] + vector.y * matrix.m[1][2] + 1.0f * matrix.m[2][2];
	assert(w != 0.0f);
	result.x /= w;
	result.y /= w;
	return result;
}