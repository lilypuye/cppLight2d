#include "svpng.inc"
#include <math.h> // fabsf(), fminf(), fmaxf(), sinf(), cosf(), sqrt()
#include <stdlib.h> // rand(), RAND_MAX
#include <initializer_list>
#include <vector>
#include <iostream>
#include "Shape.h"

using namespace std;
using std::vector;

#define SAFE_DELETE(p) do {delete (p); (p)  = NULL;} while (false)  
#define TWO_PI 6.28318530718f
#define W 512
#define H 512
#define N 64
#define BIAS 1e-4f
#define MAX_DEPTH 5
#define IS_DEBUG false

unsigned char img[W * H * 3];

void drawLine(Point p1, Point p2) {
	if (!p1.IsValid() || !p2.IsValid()) return;
	int x0 = p1.x * W;
	int x1 = p2.x * W;
	int y0 = p1.y * H;
	int y1 = p2.y * H;
	int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
	int dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
	int err = (dx > dy ? dx : -dy) / 2;
	while (*(img + (y0 * W + x0) * 3) = 255, x0 != x1 || y0 != y1) {
		int e2 = err;
		if (e2 > -dx) { err -= dy; x0 += sx; }
		if (e2 <  dy) { err += dx; y0 += sy; }
	}
}

class Entity
{
protected:
	Shape* m_shape;
	Color m_emissive;
	float m_reflectivity;		//不考虑漫反射
	float m_refractivity;		//一般的，要求reflectivity + refractivity <= 1
	float m_refract_index[3];	//认为每种颜色可以有不同的折射率
public:
	Entity(Shape* s, Color e, float re = 0.f, float ra = 0.f, float* ri = NULL) :
		m_shape(s), m_emissive(e), m_reflectivity(re), m_refractivity(ra)
	{
		if (ri)
			for (int i = 0; i < 3; i++)
				m_refract_index[i] = ri[i];
	}
	Shape* GetShape() { return m_shape; }
	Color GetEmissive() { return m_emissive; }
	float GetReflectivity() { return m_reflectivity; }
	float GetRefractivity() { return m_refractivity; }
	float GetRefractIndex(int index) { return m_refract_index[index];}
	virtual bool Intersect(Point p, Vector d, Point &inter)
	{
		return m_shape->Intersect(p, d, inter);
	}
};

//聚光灯
class SpotLight :public Entity
{
protected:
	Vector m_dir;		//聚光灯主方向
	float m_cosa;		//聚光灯角度范围的cos
public:	
	SpotLight(Shape* s, Color e, float re = 0.f, float ra = 0.f, float* ri = NULL,
		Vector dir = { 0.f, 1.f }, float a = 0.03f) :
		Entity(s, e, re, ra, ri), m_dir(dir)
	{
		m_cosa = cos(a);
	}
	bool Intersect(Point p, Vector d, Point &inter)
	{
		if (d*(-m_dir) < m_cosa)	//预过滤角度方向在照射范围外的光线
			return false;
		else
			return m_shape->Intersect(p, d, inter);
	}
};

class Scene
{
protected:
	vector<Entity*> m_entities;
public:
	Scene(vector<Entity*> entities) :m_entities(entities) {}
	vector<Entity*> GetEntities() { return m_entities; }
	Color Refract(Entity* ent, Point inter, Vector d, Vector normal, int color_index, int depth)
	{
		if (depth > MAX_DEPTH || ent->GetRefractivity() == 0.f ) return{ 0.f, 0.f,0.f };
		float idotn = d * normal;
		float ri = ent->GetRefractIndex(color_index);
		float k, a;
		if (idotn > 0.f)	//从内向外折射
		{
			k = 1.f - ri*ri*(1.f - idotn*idotn);
			if (k < 0.f) return{ 0.f, 0.f, 0.f };  //全反射
			a = ri * idotn - sqrtf(k);
		}
		else  //从外向内折射
		{
			ri = 1.f / ri;
			k = 1.f - ri*ri*(1.f - idotn*idotn);
			a = ri * idotn + sqrtf(k);
		}
		Vector refract = d*ri - normal*a;
		if (IS_DEBUG)
		{
			cout << "normal:" << normal.x << "  " << normal.y << endl;
			cout << "a" << a << endl;
			cout << "idotn" << idotn << endl;
			cout << "depth" << depth << endl;
			cout << "inter:" << inter.x << "  " << inter.y << endl;
			cout << "d:" << d.x << "  " << d.y << endl;
			cout << "refract:" << refract.x << "  " << refract.y << endl;
		}
		return GetColor(inter + refract * BIAS, refract, color_index, depth) * ent->GetRefractivity();
	}
	Color Reflect(Entity* ent, Point inter, Vector d, Vector normal, int color_index, int depth)
	{
		if (depth > MAX_DEPTH || ent->GetReflectivity() == 0.f) return{ 0.f, 0.f,0.f };
		//if (normal * d > 0.f) return{ 0.f, 0.f,0.f };
		Vector reflect = d.reflect(normal);
		return GetColor(inter + reflect * BIAS, reflect, color_index, depth) * ent->GetReflectivity();
	}
	Color GetColor(Point p, Vector d, int color_index, int depth = 0)	//获取p点从d方向收到的emissive
	{
		Color trace_emissive{ 0.0f, 0.0f, 0.0f };
		float distance = 10.0f;
		int ent_index = -1;
		Point inter;
		for (int j = 0; j < m_entities.size(); j++)
		{
			Point tmp_inter;
			if (m_entities[j]->Intersect(p, d, tmp_inter))
			{
				float new_dist = (tmp_inter - p).len();
				if (distance > new_dist)
				{
					ent_index = j;
					distance = new_dist;
					inter = tmp_inter;
				}
			}
		}
		if (ent_index >= 0)
		{
			Entity* ent = m_entities[ent_index];
			if (IS_DEBUG)
				drawLine(p, inter);
			Vector normal = ent->GetShape()->GetNormal(inter);
			Color reflect = Reflect(m_entities[ent_index], inter, d, normal, color_index, depth + 1);
			Color refract = { 0.f, 0.f, 0.f };
			if (color_index >= 3)
			{
				refract.r = Refract(ent, inter, d, normal, 0, depth + 1).r;
				refract.g = Refract(ent, inter, d, normal, 1, depth + 1).g;
				refract.b = Refract(ent, inter, d, normal, 2, depth + 1).b;
			}
			else
				refract = Refract(ent, inter, d, normal, color_index, depth + 1);
			return ent->GetEmissive() + reflect + refract;
		}
		else
			return{ 0.0f, 0.0f, 0.0f };
	}
	Color sample(Point p)
	{
		Color sum{ 0.0f, 0.0f, 0.0f };
		for (int i = 0; i < N; i++)
		{
			float a = TWO_PI * (i + (float)rand() / RAND_MAX) / N;
			//float a = TWO_PI * (i) / N;
			sum = sum + GetColor(p, { cosf(a), sinf(a) }, 3);
		}
		return sum / N;
		//return GetColor(p, { cosf(TWO_PI*-0.26f), sinf(TWO_PI*-0.26f) }, 3);
	}
	Color GetBaseColor(Point p)
	{
		for (auto ent : m_entities)
			if (ent->GetShape()->IsInside(p))
				return ent->GetEmissive();
		return{ 0.f, 0.f, 0.f };
	}
};


Shape* GeneratePolygon(std::initializer_list<Point> points)
{
	Point sum = { 0.f, 0.f };
	for (auto p : points)
	{
		sum.x += p.x;
		sum.y += p.y;
	}
	Point center = { sum.x / points.size(), sum.y / points.size() };
	Shape* si = NULL;
	for (auto i = points.begin(); i + 1 != points.end(); i++)
	{
		Point p1 = *i;
		Point p2 = *(i + 1);
		Line* l = new Line(p1, p2, center);
		if (si)
			si = new ShapeIntersect(si, l);
		else
			si = l;
	}
	si = new ShapeIntersect(si, new Line(*(points.begin()), *(points.end() - 1), center));
	return si;
}

Scene* GenerateScene2()		//包含葫芦形的黄光，三角形的蓝光，两个正方形的反射块
{
	Circle* c1 = new Circle({ 0.5f, 0.6f }, 0.1f);
	Circle* c2 = new Circle({ 0.5f, 0.7f }, 0.12f);
	Shape* su1 = new ShapeUnion(c1, c2);					//两个圆的并
	Shape* triangle = GeneratePolygon({ { 0.9f,0.5f },{ 0.7f, 0.5f },{ 0.8f, 0.4f } });
	Shape* quad1 = GeneratePolygon({ { 0.3f, 0.4f },{ 0.4f, 0.4f },{ 0.4f, 0.3f },{ 0.3f, 0.3f } });
	Shape* quad2 = GeneratePolygon({ { 0.2f, 0.5f },{ 0.3f, 0.5f },{ 0.3f, 0.6f },{ 0.2f, 0.6f } });
	Entity* e1 = new Entity(su1, { 1.8f, 0.9f, 0.7f }, 0.0f);			//葫芦形的黄光
	Entity* e2 = new Entity(triangle, { 0.2f, 0.9f, 1.1f });	//三角形的蓝光
	Entity* e3 = new Entity(quad1, { 0.05f, 0.05f, 0.2f }, 0.8f);
	Entity* e4 = new Entity(quad2, { 0.05f, 0.05f, 0.2f }, 0.8f);
	//Entity* e3 = new Entity(quad, { 0.05f, 0.05f, 0.2f }, 1.0f);		//正方形的深蓝色
	Shape* s = GeneratePolygon({ { 1.f, 2.f },{ 3.f, 4.f } });
	return new Scene({ e1,e2, e3, e4 });
}


Scene* GenerateScene()	//半圆形凸透镜折射红色光和蓝色光
{
	Circle* c1 = new Circle({ 1.0f, -0.5f }, 0.05f);
	Circle* c3 = new Circle({ 0.0f, -0.5f }, 0.05f);
	Shape* triangle1 = GeneratePolygon({ { 0.42f,0.0f },{ 0.30f, 0.25f },{ 0.35f, 0.25f } });
	Shape* triangle2 = GeneratePolygon({ { 0.58f,0.0f },{ 0.70f, 0.25f },{ 0.65f, 0.25f } });
	Line* l1 = new Line(0.f, 1.f, -0.3f);
	Line* l2 = new Line(0.f, -1.f, 0.32f);
	Circle* c2 = new Circle({ 0.5f, 0.3f }, 0.3f);
	Shape* triangle = GeneratePolygon({ { 0.3f,0.3f },{ 0.70f, 0.3f },{ 0.5f, 0.5f } });
	Shape* si1 = new ShapeIntersect(l1, c2);
	Entity* e1 = new Entity(c1, { 2.f, 9.f, 11.f });
	//Entity* e2 = new Entity(triangle1, { 0.05f, 0.05f, 0.2f }, 0.8f);
	//Entity* e3 = new Entity(triangle2, { 0.05f, 0.05f, 0.2f }, 0.8f);
	Entity* e2 = new Entity(c3, { 11.f, 2.f, 9.f });
	float refract[3] = { 1.5f, 1.5f, 1.5f };
	Entity* e4 = new Entity(si1, { 0.0f, 0.0f, 0.0f }, 0.2f, 1.f, refract);
	return new Scene({ e1, e2, e4});
}

Scene* GenerateScene3() //三棱镜
{
	Circle* c1 = new Circle({ 0.5f, -0.5f }, 0.05f);
	//Shape* triangle = GeneratePolygon({ { 0.3f,0.3f },{ 0.7f, 0.3f },{ 0.5f, 0.5f } });
	Shape* triangle = GeneratePolygon({ { 0.3f,0.4f },{ 0.7f, 0.5f },{ 0.7f, 0.3f } });
	Entity* e1 = new Entity(c1, { 10.f, 10.f, 10.f });
	float refract[3] = { 1.2f, 1.4f, 1.6f };
	Entity* e2 = new Entity(triangle, { 0.01f, 0.12f, 0.17f }, 0.2f, 1.f, refract);
	return new Scene({ e1, e2});
}

Scene* GenerateScene5() //三棱镜和激光灯
{
	Circle* c1 = new Circle({ 0.5f, -0.5f }, 0.05f);
	//Shape* triangle = GeneratePolygon({ { 0.3f,0.3f },{ 0.7f, 0.3f },{ 0.5f, 0.5f } });
	Shape* triangle = GeneratePolygon({ { 0.3f,0.3f },{ 0.7f, 0.4f },{ 0.7f, 0.2f } });
	Entity* e1 = new SpotLight(c1, { 75.f, 75.f, 75.f });
	float refract[3] = { 1.2f, 1.4f, 1.6f };
	Entity* e2 = new Entity(triangle, { 0.01f, 0.12f, 0.17f }, 0.2f, 1.f, refract);
	return new Scene({ e1, e2});
}

Scene* GenerateScene4() //不均匀玻璃片
{
	Circle* c1 = new Circle({ 0.5f, -0.5f }, 0.05f);
	Entity* e1 = new Entity(c1, { 10.f, 10.f, 10.f });
	Line *l1 = new Line(0.f, 1.f, 0.3f);
	Line *l2 = new Line(1.f, -12.5f, 4.f);
	ShapeIntersect *si = new ShapeIntersect(l1, l2);
	float refract[3] = { 1.2f, 1.4f, 1.6f };
	Entity* e2 = new Entity(si, { 0.01f, 0.12f, 0.17f }, 0.2f, 1.f, refract);
	return new Scene({ e1, e2 });
}

Scene* GenerateScene6()	//半圆形凸透镜折射白光
{
	Circle* c1 = new Circle({ 0.5f, -0.5f }, 0.05f);
	Line* l1 = new Line(0.f, 1.f, -0.3f);
	Circle* c2 = new Circle({ 0.5f, 0.3f }, 0.3f);
	Shape* si1 = new ShapeIntersect(l1, c2);
	Entity* e1 = new Entity(c1, { 20.f, 20.f, 20.f });
	float refract[3] = { 1.4f, 1.5f, 1.6f };
	Entity* e4 = new Entity(si1, { 0.0f, 0.0f, 0.0f }, 0.2f, 1.f, refract);
	return new Scene({ e1, e4 });
}

Scene* GenerateScene7()	//聚光灯
{
	Circle* c1 = new Circle({ 0.1f, 0.1f }, 0.05f);
	Entity* e1 = new SpotLight(c1, { 20.f, 20.f, 20.f }, 0.f, 0.f, NULL, { 0.707f, 0.707f }, 0.1f);
	return new Scene({ e1});
}

int main() {
	Scene* s = GenerateScene5();
	unsigned char* p = img;
	if (!IS_DEBUG)
		for (int y = 0; y < H; y++)
			for (int x = 0; x < W; x++, p += 3)
			{
				Color color = s->sample({ (float)x / W, (float)y / H });
				p[0] = (int)fminf(color.r *255.0f, 255.0f);
				p[1] = (int)fminf(color.g *255.0f, 255.0f);
				p[2] = (int)fminf(color.b *255.0f, 255.0f);
			}
	else
	{
		for (int y = 0; y < H; y++)
			for (int x = 0; x < W; x++, p += 3)
			{
				Color color = s->GetBaseColor({ (float)x / W, (float)y / H });
				p[0] = (int)fminf(color.r *255.0f, 255.0f);
				p[1] = (int)fminf(color.g *255.0f, 255.0f);
				p[2] = (int)fminf(color.b *255.0f, 255.0f);
			}
		s->sample({ 0.5f, 0.9f });	//debug
	}
	svpng(fopen("reflect.png", "wb"), W, H, img, 0);
	if (IS_DEBUG)
		getchar();
}
