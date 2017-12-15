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
#define N 256
#define BIAS 1e-4f
#define MAX_DEPTH 3
#define IS_DEBUG false

unsigned char img[W * H * 3];

void drawLine(int x0, int y0, int x1, int y1) {
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
private:
	Shape* m_shape;
	Color m_emissive;
	float m_reflectivity;
public:
	Entity(Shape* s, Color e, float r = 0.0) :m_shape(s), m_emissive(e), m_reflectivity(r) {}
	Shape* GetShape() { return m_shape; }
	Color GetEmissive() { return m_emissive; }
	float GetReflectivity() { return m_reflectivity; }
};

class Scene
{
protected:
	vector<Entity*> m_entities;
public:
	Scene(vector<Entity*> entities) :m_entities(entities) {}
	vector<Entity*> GetEntities() { return m_entities; }
	Color Reflect(Entity* ent, Point inter, Vector d, int depth)
	{
		if (depth > MAX_DEPTH || ent->GetReflectivity() == 0.f) return{ 0.f, 0.f,0.f };
		Vector normal = ent->GetShape()->GetNormal(inter);
		if (normal * d > 0.f) return{ 0.f, 0.f,0.f };
		Vector reflect = d.reflect(normal);
		return GetColor(inter + reflect * BIAS, reflect, depth);
	}
	Color GetColor(Point p, Vector d, int depth = 0)	//获取p点从d方向收到的emissive
	{
		Color trace_emissive{ 0.0f, 0.0f, 0.0f };
		float distance = 10.0f;
		int ent_index = -1;
		Point inter;
		for (int j = 0; j < m_entities.size(); j++)
		{
			Point tmp_inter;
			if (m_entities[j]->GetShape()->Intersect(p, d, tmp_inter))
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
			if (IS_DEBUG)
				drawLine(p.x * 512, p.y * 512, inter.x * 512, inter.y * 512);
			Color reflect = Reflect(m_entities[ent_index], inter, d, depth + 1);
			return m_entities[ent_index]->GetEmissive() + reflect * m_entities[ent_index]->GetReflectivity();
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
			sum = sum + GetColor(p, { cosf(a), sinf(a) });
		}
		return sum / N;
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

Scene* GenerateScene2()
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


Scene* GenerateScene()
{
	Circle* c1 = new Circle({ 0.5f, 0.3f }, 0.1f);
	Line* l1 = new Line(0.0f, 1.0f, -0.7f);
	Entity* e1 = new Entity(c1, { 1.8f, 0.9f, 0.7f });			//葫芦形的黄光
	Entity* e2 = new Entity(l1, { 0.0f, 0.0f, 0.0f }, 0.8f);	//三角形的蓝光
	return new Scene({ e1,e2 });
}

int main() {
	Scene* s = GenerateScene2();
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
		s->sample({ 0.41f, 0.35f });	//debug
	svpng(fopen("reflect.png", "wb"), W, H, img, 0);
}
