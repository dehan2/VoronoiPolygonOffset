#include "OffsetVertex.h"



OffsetVertex::OffsetVertex()
{
}



OffsetVertex::OffsetVertex(const int& ID, const rg_Point2D& coordinate, Offset* offset)
{
	m_ID = ID;
	m_coordinate = coordinate;
	m_offset = offset;
}



OffsetVertex::OffsetVertex(const OffsetVertex& rhs)
{
	copy(rhs);
}



OffsetVertex::OffsetVertex(const int& ID, const rg_Point2D& coordinate, const void* corrEntity, const ENTITY_TYPE& entityType, Offset* offset)
{
	m_ID = ID;
	m_coordinate = coordinate;
	m_corrEntity = corrEntity;
	m_entityType = entityType;
	m_offset = offset;
}



OffsetVertex::OffsetVertex(const int& ID, const rg_Point2D& coordinate, const void* corrEntity, const ENTITY_TYPE& entityType, OffsetEdge* prevEdge, Offset* offset)
{
	m_ID = ID;
	m_coordinate = coordinate;
	m_corrEntity = corrEntity;
	m_entityType = entityType;
	m_prevEdge = prevEdge;
	m_offset = offset;
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
	m_corrEntity = rhs.m_corrEntity;
	m_entityType = rhs.m_entityType;
	m_offset = rhs.m_offset;
}
