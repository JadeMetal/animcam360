#pragma once
#include <math.h>

const float M_PI = 3.14159265f;

inline RE::NiPoint2 Rotate(const RE::NiPoint2& src, float angle) {
	RE::NiPoint2 dst;
	dst.x = src.x * cos(angle) - src.y * sin(angle);
	dst.y = src.x * sin(angle) + src.y * cos(angle);
	return dst;
}

inline float NormalAbsoluteAngle(float angle)
{
	while (angle < 0)
		angle += 2 * M_PI;
	while (angle > 2 * M_PI)
		angle -= 2 * M_PI;
	return angle;
}

inline float VectorMagnitude(const RE::NiPoint2& src)
{
	return std::sqrt(src.x * src.x + src.y * src.y);
}

inline RE::NiPoint2 VectorNormalize(const RE::NiPoint2& src)
{
	RE::NiPoint2 dst;
	float invlen = 1.0f / VectorMagnitude(src);
	dst.x = src.x * invlen;
	dst.y = src.y * invlen;
	return dst;
}

inline float DotProduct(const RE::NiPoint2& l, const RE::NiPoint2& r)
{
	return l.x * r.x + l.y * r.y;
}

inline float CrossProduct(const RE::NiPoint2& l, const RE::NiPoint2& r)
{
	return l.x * r.y - l.y * r.x;
}

#define INIPATH ".\\Data\\SKSE\\Plugins\\Animcam360.ini"

