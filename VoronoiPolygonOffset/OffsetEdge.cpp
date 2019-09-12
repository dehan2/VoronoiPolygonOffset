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



OffsetEdge::OffsetEdge(const int& ID, OffsetVertex* startVtx, OffsetVertex* endVtx)
{
	m_ID = ID;
	m_startVtx = startVtx;
	m_endVtx = endVtx;
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
	m_isArc = rhs.m_isArc;
	m_arc = rhs.m_arc;
}
