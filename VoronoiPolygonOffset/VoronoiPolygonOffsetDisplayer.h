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
	list<Offset>* pOffsets;
	vector<rg_Point2D>* pSearchPath;
	vector<rg_Point2D>* pHorizontalSearchPath;
	vector<rg_Point2D>* pVerticalSearchPath;
	
	rg_Point2D center;

	list<array<float, 2>>* pTotalPolygon;

	list<array<float, 2>>* pLeftPolygon;
	list<array<float, 2>>* pRightPolygon;

public:
	VoronoiPolygonOffsetDisplayer(QObject*parent);
	~VoronoiPolygonOffsetDisplayer();

	void calculate_center();
	void draw_polygon();
	void draw_VD();
	void draw_parabolic_edge(const VEdge2D* edge);
	void draw_offsets(const int startID = 0);
	void draw_point(float x, float y, QColor color);
	void draw_line(const rg_Point2D& pt1, const rg_Point2D& pt2, int width, QColor color);
	void draw_curves(const list<rg_Point2D>& curvePts, int width, QColor color);
	void draw_search_path();
	void draw_horizontal_search_path();
	void draw_vertical_search_path();
};
