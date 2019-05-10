#pragma once

#include <QGraphicsScene>

#include "Polygon2D.h"
#include "PolygonVD2D.h"
#include "Offset.h"

class VoronoiPolygonOffsetDisplayer : public QGraphicsScene
{
	Q_OBJECT

public:
	Polygon2D* pPolygon;
	PolygonVD2D* pVD;
	vector<Offset>* pOffsets;

public:
	VoronoiPolygonOffsetDisplayer(QObject*parent);
	~VoronoiPolygonOffsetDisplayer();

	void draw_polygon();
	void draw_VD();
	void draw_parabolic_edge(const VEdge2D* edge);
	void draw_offsets();
	void draw_point(float x, float y, QColor color);
};
