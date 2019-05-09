#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_VoronoiPolygonOffset.h"
#include <QWheelEvent>

#include "Polygon2D.h"
#include "PolygonVD2D.h"
#include "VoronoiPolygonOffsetDisplayer.h"
#include "constForPolygonOffset.h"

#include "Offset.h"
#include "OffsetVertex.h"
#include "OffsetEdge.h"

#include <string>
#include <vector>

class VoronoiPolygonOffset : public QMainWindow
{
	Q_OBJECT

private:
	Polygon2D m_polygon;
	PolygonVD2D m_VD;
	vector<const VVertex2D*> m_mapFromPolygonVtxToVoroVtx;
	set<const VEdge2D*> insiderVEdges;

	//float m_threshold = 0;
	//vector<vector<rg_Point2D>> m_offsets;

	vector<Offset> m_offsets;

	list<VEdge2D*> m_traverseList;
	map<const VEdge2D*, EDGE_TYPE> m_mapForVEdgeType;
	map<const VEdge2D*, Offset*> m_mapFromEdgeToOffset;


	//For Draw
	VoronoiPolygonOffsetDisplayer* m_scene = nullptr;
	double m_scale = 1.0;

public:
	VoronoiPolygonOffset(QWidget *parent = Q_NULLPTR);
	void wheelEvent(QWheelEvent *event);

	void connect_VVertices_to_polygonVtx();
	void classify_insider_VEdges();

	void make_traverse_list();
	pair<VEdge2D*, const VVertex2D*> find_start_edge_pair();

	void make_offset_vertex_for_branch(VEdge2D* branch, const float& offsetAmount, Offset& offset,
		list<rg_Point2D>& TStack,	list<int>& CStack, set<VEdge2D*>& visitedEdges);
	pair<bool, rg_Point2D> find_offset_vertex_for_branch(VEdge2D* branch, const float& offsetAmount);

	void make_offset_vertex_for_fluff(VEdge2D* fluff, const float& offsetAmount, Offset& offset,
		list<rg_Point2D>& TStack, list<int>& CStack, set<VEdge2D*>& visitedEdges);
	pair<bool, rg_Point2D> find_offset_vertex_for_fluff(VEdge2D* fluff, const float& offsetAmount);

	void make_offset_vertex_for_trunk(VEdge2D* trunk, const float& offsetAmount, vector<Offset>& offsets,
		list<rg_Point2D>& TStack, list<int>& CStack, set<VEdge2D*>& visitedEdges, TRAVERSE_STATUS& status);
	pair<bool, rg_Point2D> find_offset_vertex_for_trunk(VEdge2D* trunk, const float& offsetAmount);

	list<rg_Point2D> calculate_points_for_trunk_edge(VEdge2D* trunk);
	list<rg_Point2D> calculate_offset_vertex_for_parabolic_edge(VEdge2D* trunk, const float& offsetAmount);

	pair<Generator2D*, Generator2D*> find_polygon_generators(VEdge2D* edge);

private:
	Ui::VoronoiPolygonOffsetClass ui;

public slots:
	void open_polygon_file();
	void compute_VD();
	void compute_offset();
};
