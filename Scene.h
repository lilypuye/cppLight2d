#pragma once
#include <iostream>
#include <stdlib.h> // rand(), RAND_MAX
#include <vector>
using std::vector;
#include "Shape.h"
#include "QuadTree.h"

using namespace std;


#define MAX_DEPTH 3
#define IS_DEBUG false
#define N 64
#define TWO_PI 6.28318530718f
#define BIAS 1e-4f

void drawLine(Point p1, Point p2);

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
	float GetRefractIndex(int index) { return m_refract_index[index]; }
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
		if (depth > MAX_DEPTH || ent->GetRefractivity() == 0.f) return{ 0.f, 0.f,0.f };
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