#pragma once

#include <vector>
#include "OffsetVertex.h"
#include "OffsetEdge.h"

using namespace std;

class Offset
{
	int m_ID = -1;
	float m_offsetAmount;
	list<OffsetVertex> m_vertices;
	list<OffsetEdge> m_edges;

public:
	Offset();
	Offset(const int& ID, const float& offsetAmount);

	~Offset();

	int get_ID() const { return m_ID; }
	list<OffsetVertex>& get_vertices() { return m_vertices; }
	list<OffsetEdge>& get_edges() { return m_edges; }

	void set_ID(const int& ID) { m_ID = ID; }
	void add_offset_vertex(const rg_Point2D& coord, VEdge2D* corrVEdge);
	void close_offset();
};

