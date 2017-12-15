#pragma once
#include <math.h> // fabsf(), fminf(), fmaxf(), sinf(), cosf(), sqrt()

struct Vector
{
	float x;
	float y;
	Vector operator+(Vector v)
	{
		return{ x + v.x, y + v.y };
	}
	float operator*(Vector v) //点乘
	{
		return x*v.x + y*v.y;
	}
	Vector operator*(float f) //倍数
	{
		return{ x*f, y*f };
	}
	Vector operator/(float f)
	{
		return{ x / f, y / f };
	}
	float len()	//向量长度
	{
		return sqrt(x*x + y*y);
	}
	Vector reflect(const Vector normal)
	{
		float idotn2 = (normal.x * x + normal.y * y)*-2.f;
		return{ x + idotn2 * normal.x, y + idotn2 * normal.y };
	}
	Vector normalize()
	{
		float length = len();
		if (length > 0)
			return{ x / length, y / length };
		return{ 0.f, 0.f };
	}
};


struct Point
{
	float x;
	float y;
	Point operator+(Vector v)
	{
		return{ x + v.x, y + v.y };
	}
	Vector operator-(Point p)
	{
		return{ x - p.x, y - p.y };
	}
	Point operator-(Vector v)
	{
		return{ x - v.x, y - v.y };
	}
	bool IsValid()
	{
		return x >= 0.f && x <= 1.f && y > 0.f && y < 1.f;
	}
};

struct Color
{
	float r, g, b;
	Color operator+(Color c)
	{
		return{ r + c.r, g + c.g, b + c.b };
	}
	Color operator*(float f)
	{
		return{ r*f, g*f, b*f };
	}
	Color operator/(float f)
	{
		return{ r / f, g / f,b / f };
	}
	bool operator>(Color c)
	{
		return (r + g + b > c.r + c.g + c.b);
	}
	bool operator<(Color c)
	{
		return (r + g + b < c.r + c.g + c.b);
	}
};
