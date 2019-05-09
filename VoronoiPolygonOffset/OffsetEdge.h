#pragma once

#include "constForPolygonOffset.h"
#include "VertexGenerator2D.h"

class OffsetVertex;

class OffsetEdge
{
	int m_ID = -1;
	OffsetVertex* m_startVtx = nullptr;
	OffsetVertex* m_endVtx = nullptr;
	bool m_isArcEdge = false;
	VertexGenerator2D* m_reflexVtx = nullptr;

public:
	OffsetEdge();
	OffsetEdge(const int& ID);
	OffsetEdge(const int& ID, OffsetVertex* startVtx);
	OffsetEdge(const OffsetEdge& rhs);

	OffsetEdge& operator=(const OffsetEdge& rhs);

	~OffsetEdge();

	void copy(const OffsetEdge& rhs);

	int get_ID() const { return m_ID; }
	OffsetVertex* get_start_vertex() const { return m_startVtx; }
	OffsetVertex* get_end_vertex() const { return m_endVtx; }

	bool get_is_arc_edge() const { return m_isArcEdge; }
	VertexGenerator2D* get_reflex_vertex() const { return m_reflexVtx; }

	void set_ID(const int& ID) { m_ID = ID; }
	void set_start_vertex(OffsetVertex* startVtx) { m_startVtx = startVtx; }
	void set_end_vertex(OffsetVertex* endVtx) { m_endVtx = endVtx; }

	void set_is_arc_edge(const bool& isArcEdge) { m_isArcEdge = isArcEdge; }
	void set_reflex_vertex(VertexGenerator2D* reflexVtx) { m_reflexVtx = reflexVtx; }
};

