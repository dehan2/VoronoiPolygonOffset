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
	list<rg_Circle2D>* pDisks;
	set<const VEdge2D*>* pInsiderEdges;
	PolygonVD2D* pVD;
	vector<Offset>* pOffsets;
	list<rg_Line2D>* pHorizontalOffsets;
	
	rg_Point2D center;

public:
	VoronoiPolygonOffsetDisplayer(QObject*parent);
	~VoronoiPolygonOffsetDisplayer();

	void calculate_center();
	void draw_polygon();
	void draw_disks();
	void draw_VD();
	void draw_edge(const VEdge2D* edge, int width, QColor color);
	void draw_offsets(const int startID = 0);
	void draw_point(float x, float y, QColor color);
	void draw_line(const rg_Point2D& pt1, const rg_Point2D& pt2, int width, QColor color);
	void draw_circle(const rg_Point2D& center, const double& radius, int width, QColor color);
	void draw_horizontal_offsets();
};
