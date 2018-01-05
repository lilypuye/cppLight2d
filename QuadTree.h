#pragma once
#include <list>
using std::list;

#define TREE_DEPTH 3
#define MIN_NODE_SIZE 0.1
#define SAFE_DELETE(p) do {delete (p); (p)  = NULL;} while (false)  

template<typename T> class QuadNode
{
protected:
	float m_left, m_right, m_up, m_down;
	list<T*> m_dataList;
	QuadNode* m_child[4] = { NULL, NULL, NULL, NULL }; //0:LeftUp, 1:LeftDown, 2:RightUp, 3:RightDown
public:
	QuadNode(float left, float right, float up, float down) :
		m_left(left), m_right(right), m_up(up), m_down(down) {}
	~QuadNode()
	{
		for (auto child : m_child)
			SAFE_DELETE(child);
	}
	void SetDataList(list<T*> data) { m_dataList = data; }
	//递归生成四叉树节点
	void GenerateNode()
	{
		if (m_right - m_left > MIN_NODE_SIZE)
		{
			m_child[0] = new QuadNode<T>(m_left, (m_left + m_right) / 2.f, m_up, (m_up + m_down) / 2.f);		//LU
			m_child[1] = new QuadNode<T>(m_left, (m_left + m_right) / 2.f, (m_up + m_down) / 2.f, m_down);		//LD
			m_child[2] = new QuadNode<T>((m_left + m_right) / 2.f, m_right, m_up, (m_up + m_down) / 2.f);		//RU
			m_child[3] = new QuadNode<T>((m_left + m_right) / 2.f, m_right, (m_up + m_down) / 2.f, m_down);		//RD
			for (int i = 0; i < 4; i++)
			{
				list<T*> child_list = {};
				for (list<T*>::iterator iter = m_dataList.begin(); iter != m_dataList.end();)
				{
					if ((*iter)->Contained(m_child[i]->m_left, m_child[i]->m_right, m_child[i]->m_up, m_child[i]->m_down))
					{
						child_list.push_back(*iter);
						m_dataList.erase(iter++);
					}
					else
						iter++;
				}
				if (child_list.empty())
				{
					delete m_child[i];
					m_child[i] = NULL;
				}
				else
				{	
					m_child[i]->SetDataList(child_list);
					m_child[i]->GenerateNode();
				}
			}
		}
	}
	float CalcLineFx(Point p, Vector d, Point s)
	{
		return d.y*s.x - d.x*s.y - p.x*d.y + p.y*d.x;
	}
	//判断射线与四叉树节点本身包围盒是否相交，用于剪枝
	bool IntersectBound(Point p, Vector d)
	{
		//剪枝1
		int flag = 0;
		if(p.x < m_right && p.x > m_left && p.y < m_down && p.y > m_up)		//p在节点内
			return true;
		if (p.x < m_left) flag |= 0x01;
		else if (p.x < m_right) flag |= 0x0A;
		else flag |= 0x0F;
		if (p.y < m_up) flag |= 0x10;
		else if (p.y < m_down) flag |= 0xA0;
		else flag |= 0xF0;
		switch (flag)
		{
		case 0x11: if (d.x <= 0.f || d.y <= 0.f)	return false;	break;
		case 0x1A: if (d.y <= 0.f)					return false;	break;
		case 0x1F: if (d.x > 0.f || d.y <= 0.f)		return false;	break;
		case 0xA1: if (d.x <= 0.f)					return false;	break;
		case 0xAA:									return true; 
		case 0xAF: if (d.x > 0.f)					return false;	break;
		case 0xF1: if (d.x <= 0.f || d.y > 0.f)		return false;	break;
		case 0xFA: if (d.y > 0.f)					return false;	break;
		case 0xFF: if (d.x > 0.f || d.y > 0.f)		return false;	break;
		}
		//剪枝2
		if (CalcLineFx(p, d, { m_left, m_up })*CalcLineFx(p, d, { m_right, m_down }) < 0) return true;
		if (CalcLineFx(p, d, { m_left, m_down })*CalcLineFx(p, d, { m_right, m_up }) < 0) return true;
		return false;
	}
	//返回该射线相交的最近entity和距离
	bool Intersect(Point p, Vector d, T* &ent_near, Point& inter, float& dist)
	{
		if (!IntersectBound(p, d)) return false;
		for (auto ent : m_dataList)		//首先获取该节点存储的entity中最近的交点
		{
			Point tmp_inter;
			if (ent->Intersect(p, d, tmp_inter))
			{
				float tmp_dist = (tmp_inter - p).len();
				if (dist > tmp_dist)
				{
					ent_near = ent;
					dist = tmp_dist;
					inter = tmp_inter;
				}
			}
		}
		for (int i = 0; i < 4; i++)		//再递归比较子节点中的最近交点
		{
			QuadNode* node = m_child[i];
			if (node)		//由于在建树时已经进行了一次剪枝，因此一部分节点为空，不需要计算
			{
				T *tmp_ent;
				Point tmp_inter;
				float tmp_dist = 10.f;
				if (node->Intersect(p, d, tmp_ent, tmp_inter, tmp_dist) && tmp_dist < dist)
				{
					ent_near = tmp_ent;
					dist = tmp_dist;
					inter = tmp_inter;
				}

			}
		}
		if (ent_near) return true;
		else return false;
	}
};

template<typename T> class QuadTree
{
protected:
	QuadNode<T>* m_root;	//根节点
public:
	QuadTree(list<T*> data)
	{
		m_root = new QuadNode<T>(0.f, 1.f, 0.f, 1.f);
		m_root->SetDataList(data);
		m_root->GenerateNode();
	}
	void Intersect(Point p, Vector d, T* &ent, Point &inter)
	{
		float dist = 10.f;
		m_root->Intersect(p, d, ent, inter, dist);
	}
};


