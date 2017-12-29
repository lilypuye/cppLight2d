// 部分场景生成实例
#pragma once
#include <stdlib.h> // rand(), RAND_MAX
#include "Scene.h"

Shape* GeneratePolygon(initializer_list<Point> points);

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
	return new Scene({ e1, e2, e4 });
}

Scene* GenerateScene3() //三棱镜
{
	Circle* c1 = new Circle({ 0.5f, -0.5f }, 0.05f);
	//Shape* triangle = GeneratePolygon({ { 0.3f,0.3f },{ 0.7f, 0.3f },{ 0.5f, 0.5f } });
	Shape* triangle = GeneratePolygon({ { 0.3f,0.4f },{ 0.7f, 0.5f },{ 0.7f, 0.3f } });
	Entity* e1 = new Entity(c1, { 10.f, 10.f, 10.f });
	float refract[3] = { 1.2f, 1.4f, 1.6f };
	Entity* e2 = new Entity(triangle, { 0.01f, 0.12f, 0.17f }, 0.2f, 1.f, refract);
	return new Scene({ e1, e2 });
}

Scene* GenerateScene5() //三棱镜和激光灯
{
	Circle* c1 = new Circle({ 0.5f, 0.1f }, 0.05f);
	//Shape* triangle = GeneratePolygon({ { 0.3f,0.3f },{ 0.7f, 0.3f },{ 0.5f, 0.5f } });
	Shape* triangle = GeneratePolygon({ { 0.3f,0.3f },{ 0.7f, 0.4f },{ 0.7f, 0.2f } });
	Entity* e1 = new SpotLight(c1, { 75.f, 75.f, 75.f });
	float refract[3] = { 1.2f, 1.4f, 1.6f };
	Entity* e2 = new Entity(triangle, { 0.01f, 0.12f, 0.17f }, 0.2f, 1.f, refract);
	return new Scene({ e1, e2 });
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
	return new Scene({ e1 });
}

Scene* GenerateScene8() //很多星星
{
	Circle* c1 = new Circle({ 0.5f, -0.5f }, 0.05f);
	Entity* e1 = new Entity(c1, { 20.f, 20.f, 20.f });
	vector<Entity*> stars = { e1 };
	float refract[3] = { 1.4f, 1.5f, 1.6f };
	for (int i = 0; i < 6; i++)
	{
		Circle* c = new Circle({ (float)rand() / RAND_MAX, (float)rand() / RAND_MAX }, (float)rand() / RAND_MAX / 10.f);
		stars.push_back(new Entity(c, { 0.f, 0.f, 0.f }, 0.1f, 0.8f, refract));
	}
	return new Scene(stars);
}
