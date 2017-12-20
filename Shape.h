#pragma once

#include "basic.h"

#define EPSILON 1e-5f

class Shape
{
public:
	Shape() {}
	~Shape() {}
	//判断是否相交
	virtual bool Intersect(Point p, Vector d)
	{
		return false;
	}
	//判断是否相交并求交点
	virtual bool Intersect(Point p, Vector d, Point& inter)
	{
		return false;
	}
	//判断是否在shape内部
	virtual bool IsInside(Point p)
	{
		return false;
	}
	//获取边界上的点所在位置的法向
	virtual Vector GetNormal(Point p)
	{
		return{ 0.f, 1.f };
	}
	//是否在边界上
	virtual bool IsOnBoundary(Point p)
	{
		return false;
	}
};

class Line :public Shape
{
	//半平面表示形式：m_a*x + m_b*y + m_c > 0， 法线方向m_normal
protected:
	float m_a;
	float m_b;
	float m_c;
	Vector m_normal;
	void CalcIntersect(Point p, Vector d, Point &inter)
	{
		inter.x = (m_b*d.y*p.x - m_b*d.x*p.y - m_c*d.x) / (m_a*d.x + m_b*d.y);
		if (d.x) inter.y = (inter.x - p.x)*d.y / d.x + p.y;
		else inter.y = (-m_c - m_a*inter.x) / m_b;
	}
public:
	Line(float a, float b, float c) :m_a(a), m_b(b), m_c(c)
	{
		m_normal = { -a, -b };
		m_normal = m_normal.normalize();
	}
	Line(Point p1, Point p2, Point in)
	{
		m_a = p2.y - p1.y;
		m_b = p1.x - p2.x;
		m_c = p2.x*p1.y - p1.x*p2.y;
		if (!IsInside(in))
		{
			m_a = -m_a;
			m_b = -m_b;
			m_c = -m_c;
		}
		m_normal = { -m_a, -m_b };
		m_normal = m_normal.normalize();
	}
	Vector GetNormal(Point p)
	{
		return m_normal;
	}
	bool IsInside(Point p)
	{
		return p.x * m_a + p.y * m_b + m_c >= 0.f;
	}
	bool IsOnBoundary(Point p)
	{
		return fabs(p.x * m_a + p.y * m_b + m_c) <= EPSILON;
	}
	//求p出发的d方向的直线与Line的交点
	bool Intersect(Point p, Vector d)
	{
		if (IsInside(p)) return true;			//光源内部
		if (d*m_normal < 0)
		{
			Point inter;
			CalcIntersect(p, d, inter);
			//if (inter.IsValid()) 
			return true;
		}
		return false;
	}

	bool Intersect(Point p, Vector d, Point &inter)
	{
		if (IsInside(p))
		{
			//inter = p;
			CalcIntersect(p, d, inter);
			if ((inter - p)*d < 0)	//射线的反向与直线相交
				inter = { -1.f, -1.f };
			return true;
		}
		if (d*m_normal < 0)
		{
			CalcIntersect(p, d, inter);
			//if (inter.IsValid()) 
			return true;
		}
		return false;
	}
};

class Circle :public Shape
{
protected:
	Point m_o;	//圆心
	float m_r;	//半径
public:
	Circle(Point o, float r) :m_o(o), m_r(r) {}
	bool IsInside(Point p)
	{
		return (m_o - p).len() <= m_r;
	}
	Vector GetNormal(Point p)
	{
		return (p - m_o).normalize();
	}
	bool IsOnBoundary(Point p)
	{
		return fabs((m_o - p).len() - m_r) <= EPSILON;
	}
	bool Intersect(Point p, Vector d)				//求p出发的d方向的射线与Circle是否相交
	{
		if (IsInside(p)) return true;				//在圆内则必然相交
		float proj = (m_o - p)*d;					//向量po在射线上垂足的距离
		if (proj <= 0) return false;				//反向射线
		Point foot = p + d * proj;					//垂足位置
		float dis = (m_o - foot).len();				//圆心到垂足的距离
		return dis < m_r;
	}
	bool Intersect(Point p, Vector d, Point& inter)
	{
		if (IsInside(p))
		{
			float proj = (m_o - p)*d;
			Point foot = p + d * proj;
			float dis1 = (m_o - foot).len();
			float dis2 = sqrt(m_r*m_r - dis1*dis1);
			inter = foot + d * dis2;
			//inter = p;
			return true;
		}
		float proj = (m_o - p)*d;					//向量po在射线上垂足的距离
		if (proj <= 0) return false;				//反向射线
		Point foot = p + d * proj;					//垂足位置
		float dis1 = (m_o - foot).len();			//圆心到垂足的距离
		if (dis1 > m_r)	return false;
		float dis2 = sqrt(m_r*m_r - dis1*dis1);
		inter = foot - d * dis2;
		return true;
	}
};

class ShapeUnion :public Shape
{
private:
	Shape* m_shape1;
	Shape* m_shape2;
public:
	ShapeUnion(Shape* shape1, Shape* shape2)
	{
		m_shape1 = shape1, m_shape2 = shape2;
	}
	bool IsInside(Point p)
	{
		return m_shape1->IsInside(p) || m_shape2->IsInside(p);
	}
	bool IsOnBoundary(Point p)
	{
		return m_shape1->IsOnBoundary(p) || m_shape2->IsOnBoundary(p);
	}
	Vector GetNormal(Point p)
	{
		if (m_shape1->IsOnBoundary(p) && m_shape2->IsOnBoundary(p))
			return (m_shape1->GetNormal(p) + m_shape2->GetNormal(p)) / 2.f;
		if (m_shape1->IsOnBoundary(p))
			return m_shape1->GetNormal(p);
		if (m_shape2->IsOnBoundary(p))
			return m_shape2->GetNormal(p);
		return{ 0.f, 1.f };
	}
	bool Intersect(Point p, Vector d)
	{
		Point inter1, inter2;
		if (!(m_shape1->Intersect(p, d, inter1) || m_shape2->Intersect(p, d, inter2)))
			return false;
		return true;
	}

	bool Intersect(Point p, Vector d, Point &inter)
	{
		Point inter1, inter2;
		bool res1 = m_shape1->Intersect(p, d, inter1);
		bool res2 = m_shape2->Intersect(p, d, inter2);
		if (!res1 && !res2)
			return false;
		if (!res1)
			inter = inter2;
		else if (!res2)
			inter = inter1;
		else
			inter = (inter1 - p).len() > (inter2 - p).len() ? inter2 : inter1;
		return true;
	}
};

class ShapeIntersect : public Shape
{
private:
	Shape* m_shape1;
	Shape* m_shape2;
public:
	ShapeIntersect(Shape* shape1, Shape* shape2)
	{
		m_shape1 = shape1, m_shape2 = shape2;
	}

	bool IsInside(Point p)
	{
		return m_shape1->IsInside(p) && m_shape2->IsInside(p);
	}

	bool IsOnBoundary(Point p)
	{
		return m_shape1->IsOnBoundary(p) || m_shape2->IsOnBoundary(p);
	}
	Vector GetNormal(Point p)
	{
		if (m_shape1->IsOnBoundary(p) && m_shape2->IsOnBoundary(p))
			return ((m_shape1->GetNormal(p)) + (m_shape2->GetNormal(p))) / 2.f;
		if (m_shape1->IsOnBoundary(p))
			return m_shape1->GetNormal(p);
		if (m_shape2->IsOnBoundary(p))
			return m_shape2->GetNormal(p);
		return{ 0.f, 1.f };
	}

	bool Intersect(Point p, Vector d)
	{
		Point inter1, inter2;
		if (!(m_shape1->Intersect(p, d, inter1) && m_shape2->Intersect(p, d, inter2)))
			return false;
		return m_shape2->IsInside(inter1) || m_shape1->IsInside(inter2);
	}

	bool Intersect(Point p, Vector d, Point &inter)
	{
		Point inter1, inter2;
		if (!(m_shape1->Intersect(p, d, inter1) && m_shape2->Intersect(p, d, inter2)))
			return false;
		if (m_shape2->IsInside(inter1) && m_shape1->IsInside(inter2))	//两个交点都合法，则取最近的交点
			inter = (inter1 - p).len() > (inter2 - p).len() ? inter2 : inter1;
		else if (m_shape2->IsInside(inter1))
			inter = inter1;
		else if (m_shape1->IsInside(inter2))
			inter = inter2;
		else
			return false;
		return true;
	}
};


//交点计算未完成，暂不可用
class ShapeSubstract : public Shape
{
private:
	Shape* m_shape1;
	Shape* m_shape2;
public:
	ShapeSubstract(Shape* shape1, Shape* shape2)
	{
		m_shape1 = shape1, m_shape2 = shape2;
	}

	bool IsInside(Point p)
	{
		return m_shape1->IsInside(p) && (!m_shape2->IsInside(p));
	}

	bool IsOnBoundary(Point p)
	{
		return m_shape1->IsOnBoundary(p) || m_shape2->IsOnBoundary(p);
	}
	Vector GetNormal(Point p)
	{
		if (m_shape1->IsOnBoundary(p) && m_shape2->IsOnBoundary(p))
			return ((m_shape1->GetNormal(p)) - (m_shape2->GetNormal(p))) / 2.f;
		if (m_shape1->IsOnBoundary(p))
			return m_shape1->GetNormal(p);
		if (m_shape2->IsOnBoundary(p))
			return -(m_shape2->GetNormal(p));
		return{ 0.f, 1.f };
	}

	bool Intersect(Point p, Vector d)
	{
		Point inter1, inter2;
		if (!(m_shape1->Intersect(p, d, inter1) && m_shape2->Intersect(p, d, inter2)))
			return false;
		return m_shape2->IsInside(inter1) || m_shape1->IsInside(inter2);
	}

	bool Intersect(Point p, Vector d, Point &inter)
	{
		Point inter1, inter2;
		if (!(m_shape1->Intersect(p, d, inter1) && m_shape2->Intersect(p, d, inter2)))
			return false;
		if (m_shape2->IsInside(inter1) && m_shape1->IsInside(inter2))	//两个交点都合法，则取最近的交点
			inter = (inter1 - p).len() > (inter2 - p).len() ? inter2 : inter1;
		else if (m_shape2->IsInside(inter1))
			inter = inter1;
		else if (m_shape1->IsInside(inter2))
			inter = inter2;
		else
			return false;
		return true;
	}
};