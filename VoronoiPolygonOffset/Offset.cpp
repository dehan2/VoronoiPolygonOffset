#include "Offset.h"



Offset::Offset()
{
}


Offset::Offset(const int& ID, const float& offsetAmount)
{
	m_ID = ID;
	m_offsetAmount = offsetAmount;
}


Offset::~Offset()
{
}



void Offset::add_offset_vertex(const rg_Point2D& coord, VEdge2D* corrVEdge)
{
	m_vertices.push_back(OffsetVertex(m_vertices.size(), coord, corrVEdge, this));
}



void Offset::close_offset()
{
	//m_edges.back().set_end_vertex(&(m_vertices.front()));
	//m_vertices.front().set_prev_edge(&(m_edges.back()));
}
