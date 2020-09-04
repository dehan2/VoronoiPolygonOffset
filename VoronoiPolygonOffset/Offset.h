#pragma once

#include <vector>
#include "OffsetVertex.h"
#include "OffsetEdge.h"

using namespace std;

class Offset
{
	int m_ID = -1;
	float m_offsetAmount;
	vector<OffsetVertex> m_vertices;
	vector<OffsetEdge> m_edges;

public:
	Offset();
	Offset(const int& ID, const float& offsetAmount);

	~Offset();

	int get_ID() const { return m_ID; }
	float get_offset_amount() { return m_offsetAmount; }
	vector<OffsetVertex>& get_vertices() { return m_vertices; }
	vector<OffsetEdge>& get_edges() { return m_edges; }
	
	void set_ID(const int& ID) { m_ID = ID; }
	void add_offset_vertex(const rg_Point2D& coord, VEdge2D* corrVEdge);
	void close_offset();
};

