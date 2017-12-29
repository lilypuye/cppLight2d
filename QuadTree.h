//http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.447.992&rep=rep1&type=pdf
#pragma once
#include <list>
using namespace std;

#define TREE_DEPTH 3
#define MIN_NODE_SIZE 0.1

template<typename T> class QuadNode
{
protected:
	float m_left, m_right, m_up, m_down;
	list<T> m_dataList;
	QuadNode* m_child[4] = { NULL, NULL, NULL, NULL }; //0:LeftUp, 1:LeftDown, 2:RightUp, 3:RightDown
public:
	QuadNode(float left, float right, float up, float down) :
		m_left(left), m_right(right), m_up(up), m_down(down) {}
	void GenerateNode(list<T*> data)
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
				for (list<T*>::iterator iter = data.begin(); iter != data.end();)
				{
					if (*iter->contained(m_left, m_right, m_up, m_down))
					{
						child_list.push_back(*iter);
						m_dataList.erase(iter++);
					}
					else
						iter++;
				}
				m_child[i]->GenerateNode();
			}
		}
	}
};

template<typename T> class QuadTree
{
protected:
	QuadNode<T>* m_root;	//¸ù½Úµã
public:
	QuadTree(list<T*> data)
	{
		m_root = new QuadNode<T>(0.f, 1.f, 0.f, 1.f);
		m_root->GenerateNode(data);
	}
};


