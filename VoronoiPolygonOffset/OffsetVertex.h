#pragma once

#include "rg_Point2D.h"
#include "VEdge2D.h"
#include "constForPolygonOffset.h"

class OffsetEdge;
class Offset;

class OffsetVertex
{
	int m_ID = -1;
	rg_Point2D m_coordinate;

	OffsetEdge* m_prevEdge = nullptr;
	OffsetEdge* m_nextEdge = nullptr;
	
	const void* m_corrEntity = nullptr;
	ENTITY_TYPE m_entityType;

	Offset* m_offset = nullptr;

public:
	OffsetVertex();
	OffsetVertex(const int& ID, const rg_Point2D& coordinate, Offset* offset);
	OffsetVertex(const int& ID, const rg_Point2D& coordinate, const void* corrEntity, const ENTITY_TYPE& entityType, Offset* offset);
	OffsetVertex(const int& ID, const rg_Point2D& coordinate, const void* corrEntity, const ENTITY_TYPE& entityType, OffsetEdge* prevEdge, Offset* offset);
	OffsetVertex(const OffsetVertex& rhs);

	OffsetVertex& operator = (const OffsetVertex& rhs);

	~OffsetVertex();

	void copy(const OffsetVertex& rhs);

	int get_ID() const { return m_ID; }
	OffsetEdge* get_prev_edge() const { return m_prevEdge; }
	OffsetEdge* get_next_edge() const { return m_nextEdge; }
	const void* get_corr_entity() const { return m_corrEntity; }
	ENTITY_TYPE get_entity_type() const { return m_entityType; }
	rg_Point2D get_coordinate() const { return m_coordinate; }

	void set_ID(const int& ID) { m_ID = ID; }
	void set_prev_edge(OffsetEdge* prevEdge) { m_prevEdge = prevEdge; }
	void set_next_edge(OffsetEdge* nextEdge) { m_nextEdge = nextEdge; }
	void set_corr_entity(const void* corrEntity) { m_corrEntity = corrEntity; }
	void set_entity_type(ENTITY_TYPE entityType) { m_entityType = entityType; }
	void set_coordinate(const rg_Point2D& coordinate) { m_coordinate = coordinate; }
};

