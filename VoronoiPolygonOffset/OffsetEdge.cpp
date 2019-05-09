#include "OffsetEdge.h"



OffsetEdge::OffsetEdge()
{
}



OffsetEdge::OffsetEdge(const int& ID)
{
	m_ID = ID;
}



OffsetEdge::OffsetEdge(const OffsetEdge& rhs)
{
	copy(rhs);
}



OffsetEdge::OffsetEdge(const int& ID, OffsetVertex* startVtx)
{
	m_ID = ID;
	m_startVtx = startVtx;
}



OffsetEdge& OffsetEdge::operator=(const OffsetEdge& rhs)
{
	if (this == &rhs)
		return *this;

	copy(rhs);
	return *this;
}



OffsetEdge::~OffsetEdge()
{
}



void OffsetEdge::copy(const OffsetEdge& rhs)
{
	m_ID = rhs.m_ID;
	m_startVtx = rhs.m_startVtx;
	m_endVtx = rhs.m_endVtx;
	m_isArcEdge = rhs.m_isArcEdge;
	m_reflexVtx = rhs.m_reflexVtx;
}
