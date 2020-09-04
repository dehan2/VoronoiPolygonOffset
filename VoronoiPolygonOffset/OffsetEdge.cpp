#include "OffsetEdge.h"
#include "OffsetVertex.h"


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
	m_isArcEdge = rhs.m_isArcEdge;
	m_reflexVtx = rhs.m_reflexVtx;
}



void OffsetEdge::calculate_curve(const double& offsetAmount)
{
	rg_Point2D vectorFromRVToSV = m_startVtx->get_coordinate() - m_reflexVtx->get_point();
	rg_Point2D vectorFromRVToEV = m_endVtx->get_coordinate() - m_reflexVtx->get_point();
	rg_Point2D edgeCenterCoord = (m_startVtx->get_coordinate() + m_endVtx->get_coordinate()) / 2;

	rg_Point2D tangentVectorForStartVtx(vectorFromRVToSV.getY(), -vectorFromRVToSV.getX());
	rg_Point2D tangentVectorForEndVtx(vectorFromRVToEV.getY(), -vectorFromRVToEV.getX());

	rg_Point2D passingPoint = edgeCenterCoord - m_reflexVtx->get_point();
	passingPoint = m_reflexVtx->get_point() + passingPoint.getUnitVector() * offsetAmount;

	m_curve.makeRQBezier(m_startVtx->get_coordinate(), tangentVectorForStartVtx.getUnitVector(), m_endVtx->get_coordinate(), tangentVectorForEndVtx.getUnitVector(), passingPoint);
}
