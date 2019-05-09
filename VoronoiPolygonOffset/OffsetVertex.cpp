#include "OffsetVertex.h"



OffsetVertex::OffsetVertex()
{
}



OffsetVertex::OffsetVertex(const int& ID, const rg_Point2D& coordinate)
{
	m_ID = ID;
	m_coordinate = coordinate;
}



OffsetVertex::OffsetVertex(const OffsetVertex& rhs)
{
	copy(rhs);
}



OffsetVertex::OffsetVertex(const int& ID, const rg_Point2D& coordinate, VEdge2D* corrVEdge)
{
	m_ID = ID;
	m_coordinate = coordinate;
	m_corrVEdge = corrVEdge;
}



OffsetVertex::OffsetVertex(const int& ID, const rg_Point2D& coordinate, VEdge2D* corrVEdge, OffsetEdge* prevEdge)
{
	m_ID = ID;
	m_coordinate = coordinate;
	m_corrVEdge = corrVEdge;
	m_prevEdge = prevEdge;
}



OffsetVertex& OffsetVertex::operator=(const OffsetVertex& rhs)
{
	if (this == &rhs)
		return *this;

	copy(rhs);
	return *this;
}


OffsetVertex::~OffsetVertex()
{
}



void OffsetVertex::copy(const OffsetVertex& rhs)
{
	m_ID = rhs.m_ID;
	m_coordinate = rhs.m_coordinate;

	m_prevEdge = rhs.m_prevEdge;
	m_nextEdge = rhs.m_nextEdge;
	m_corrVEdge = rhs.m_corrVEdge;
}
