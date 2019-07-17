#pragma once

#include "rg_Point2D.h"
#include "VEdge2D.h"

class OffsetEdge;
class Offset;

class OffsetVertex
{
	int m_ID = -1;
	rg_Point2D m_coordinate;

	OffsetEdge* m_prevEdge = nullptr;
	OffsetEdge* m_nextEdge = nullptr;
	VEdge2D* m_corrVEdge = nullptr;

	Offset* m_offset = nullptr;

public:
	OffsetVertex();
	OffsetVertex(const int& ID, const rg_Point2D& coordinate, Offset* offset);
	OffsetVertex(const int& ID, const rg_Point2D& coordinate, VEdge2D* corrVEdge, Offset* offset);
	OffsetVertex(const int& ID, const rg_Point2D& coordinate, VEdge2D* corrVEdge, OffsetEdge* prevEdge, Offset* offset);
	OffsetVertex(const OffsetVertex& rhs);

	OffsetVertex& operator = (const OffsetVertex& rhs);

	~OffsetVertex();

	void copy(const OffsetVertex& rhs);

	int get_ID() const { return m_ID; }
	OffsetEdge* get_prev_edge() const { return m_prevEdge; }
	OffsetEdge* get_next_edge() const { return m_nextEdge; }
	VEdge2D* get_corr_VEdge() const { return m_corrVEdge; }
	rg_Point2D get_coordinate() const { return m_coordinate; }

	void set_ID(const int& ID) { m_ID = ID; }
	void set_prev_edge(OffsetEdge* prevEdge) { m_prevEdge = prevEdge; }
	void set_next_edge(OffsetEdge* nextEdge) { m_nextEdge = nextEdge; }
	void set_corr_VEdge(VEdge2D* corrVEdge) { m_corrVEdge = corrVEdge; }
	void set_coordinate(const rg_Point2D& coordinate) { m_coordinate = coordinate; }
};

