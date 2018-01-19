#include "svpng.inc"
#include <math.h> // fabsf(), fminf(), fmaxf(), sinf(), cosf(), sqrt()
#include <stdlib.h> // rand(), RAND_MAX
#include <iostream>
#include <fstream>
#include "basic.h"
#include "time.h"
#include "Example.h"
#include <initializer_list>
using std::initializer_list;

#define W 512
#define H 512

unsigned char img[W * H * 3];

//用于截断画布外的线段，使p1和p2均处在画布内
void validate(Point& p1, Point& p2)
{
	if (p1.x < 0.f)
		p1 = { 0.f, (p1.y*p2.x - p2.y*p1.x) / (p2.x - p1.x) };
	if (p1.x > 1.f)
		p1 = { 1.f, (p1.y - p2.y + p1.x*p2.y - p2.x*p1.y) / (p1.x - p2.x) };
	if (p1.y < 0.f)
		p1 = { (p1.x*p2.y - p2.x*p1.y) / (p2.y - p1.y), 0.f };
	if (p1.y > 1.f)
		p1 = { (p1.x - p2.x + p1.y*p2.x - p2.y*p1.x) / (p1.y - p2.y),1.f };
	if (!p2.IsValid())
		validate(p2, p1);
}

//调试用画线函数
void drawLine(Point p1, Point p2) {
	validate(p1, p2);
	//if (!p1.IsValid() || !p2.IsValid()) return;
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


//生成多边形
Shape* GeneratePolygon(initializer_list<Point> points)
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

void main_drawrainbow()
{
	int rainbows[9][3] = {
		{ 0,0,0 },
		{ 255,0,0 },
		{ 255,165,0 },
		{ 255,255,0 },
		{ 0,255,0 },
		{ 0,127,255 },
		{ 0,0,255 },
		{ 139,0,255 },
		{ 0,0,0 } };
	ifstream file("ciexyz31.csv");
	char str[300];
	float a[95][4];
	for (int i = 0; i < 95; i++)
	{
		file >> str;
		sscanf(str, "%f,%f,%f,%f", &a[i][0], &a[i][1], &a[i][2], &a[i][3]);
		cout <<a[i][0]<<" "<< a[i][1] << " " << a[i][2] << " " << a[i][3] << endl;
	}
	getchar();
	unsigned char* p = img;
	for (int y = 0; y < H; y++)
		for (int x = 0; x < W; x++, p += 3)
		{
			int idx = y / 7+3;
			float sum = a[idx][1] + a[idx][2] + a[idx][3] + 0.001;
			Color color = { a[idx][1]*0.4185+a[idx][2]*-0.1587+a[idx][3]*-0.0828, 
				a[idx][1] * -0.0912 + a[idx][2] * 0.2524 + a[idx][3] * 0.0157,
				a[idx][1] * 0.0009 + a[idx][2] * -0.0025 + a[idx][3] * 0.1786, };
			//Color color = { a[idx][1], a[idx][2], a[idx][3] };
			//float idxf = y * 8.f/ 511.f;
			//float idx1 = floor(idxf);
			//float idx2 = ceil(idxf);
			//float gap1 = idxf - idx1;
			//float gap2 = idx2 - idxf;
			//Color color;
			//if (idx1 == idx2)
			//	color = { rainbows[(int)idx2][0] / 255.f,rainbows[(int)idx2][1] / 255.f, rainbows[(int)idx2][2] / 255.f };
			//else
			//	color = { rainbows[(int)idx1][0] / 255.f * gap2 + rainbows[(int)idx2][0] / 255.f * gap1,
			//			rainbows[(int)idx1][1] / 255.f * gap2 + rainbows[(int)idx2][1] / 255.f * gap1,
			//			rainbows[(int)idx1][2] / 255.f * gap2 + rainbows[(int)idx2][2] / 255.f * gap1,
			//};
			color.r = color.r > 0.0f ? color.r : 0.0f;
			color.g = color.g > 0.0f ? color.g : 0.0f;
			color.b = color.b > 0.0f ? color.b : 0.0f;
			p[0] = (int)fminf(color.r *255.0f, 255.0f);
			p[1] = (int)fminf(color.g *255.0f, 255.0f);
			p[2] = (int)fminf(color.b *255.0f, 255.0f);
		}
	svpng(fopen("rainbow.png", "wb"), W, H, img, 0);
}

void main() {
	time_t a = time(NULL);
	int star_num = 1;
	Scene* s = GenerateSceneDiamond();
	unsigned char* p = img;
	if (!IS_DEBUG)
		for (int y = 0; y < H; y++)
			for (int x = 0; x < W; x++, p += 3)
			{
				Color color = s->Sample({ (float)x / W, (float)y / H });
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
		s->Sample({ 0.76f, 0.16f });
	}
	svpng(fopen("reflect.png", "wb"), W, H, img, 0);
	if (IS_DEBUG)
	{
		cout << "done!" << endl;
		getchar();
	}
	else
	{
		//记录运行时间
		time_t b = time(NULL);
		ofstream SaveFile("time_record.csv", ios::app);
		struct tm * timeinfo = localtime(&b);
		SaveFile << asctime(timeinfo) <<","<<N << "," << MAX_DEPTH << "," << TREE_DEPTH << "," << star_num << "," << (b - a) <<",diamond" <<endl;
		SaveFile.close();
	}
}
