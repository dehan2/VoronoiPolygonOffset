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
#include "PlanJsonReader.h"
#include "PlanJsonWriter.h"

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

	PlanJsonReader m_reader;
	PlanJsonWriter m_totalWriter;
	PlanJsonWriter m_leftWriter;
	PlanJsonWriter m_centerWriter;
	PlanJsonWriter m_rightWriter;

	//float m_threshold = 0;
	//vector<vector<rg_Point2D>> m_offsets;

	list<Offset> m_offsets;

	list<VEdge2D*> m_traverseList;
	map<const VEdge2D*, EDGE_TYPE> m_mapForVEdgeType;

	vector<rg_Point2D> m_searchPath;
	vector<rg_Point2D> m_horizontalSearchPath;
	vector<rg_Point2D> m_verticalSearchPath;

	//For Draw
	VoronoiPolygonOffsetDisplayer* m_scene = nullptr;
	double m_scale = 1.0;

	//For Debug
	VEdge2D* firstTrunk = nullptr;

public:
	VoronoiPolygonOffset(QWidget *parent = Q_NULLPTR);
	void wheelEvent(QWheelEvent *event);

	void connect_VVertices_to_polygonVtx();
	void classify_insider_VEdges();

	void make_traverse_list();
	pair<VEdge2D*, const VVertex2D*> find_start_edge_pair();

	void make_offset_vertices(const float& offsetAmount);
	VVertex2D* find_start_vertex_of_traverse(VEdge2D* currEdge, VEdge2D* lastEdge);

	void make_offset_vertex_for_branch(VEdge2D* branch, const float& offsetAmount, Offset& offset,
		list<rg_Point2D>& TStack,	list<Offset*>& CStack, set<VEdge2D*>& visitedEdges, TRAVERSE_STATUS& status);
	pair<bool, rg_Point2D> find_offset_vertex_for_branch(VEdge2D* branch, const float& offsetAmount);

	void make_offset_vertex_for_fluff(VEdge2D* fluff, const float& offsetAmount, Offset& offset,
		list<rg_Point2D>& TStack, list<Offset*>& CStack, set<VEdge2D*>& visitedEdges, TRAVERSE_STATUS& status);
	pair<bool, rg_Point2D> find_offset_vertex_for_fluff(VEdge2D* fluff, const float& offsetAmount);

	void make_offset_vertex_for_trunk(VEdge2D* trunk, const float& offsetAmount, list<Offset>& offsets,
		list<rg_Point2D>& TStack, list<Offset*>& CStack, set<VEdge2D*>& visitedEdges, TRAVERSE_STATUS& status, VVertex2D* startVtxOfTraverse);
	pair<bool, list<rg_Point2D>> find_offset_vertex_for_trunk(VEdge2D* trunk, const float& offsetAmount);
	void manage_offset_vertices_order(list<rg_Point2D>& offsetVertices, VVertex2D* startVtxOfTraverse);
	void process_offset_vertex_for_trunk(const rg_Point2D& offsetVtx, VEdge2D* trunk, const float& offsetAmount, list<Offset>& offsets,
		list<rg_Point2D>& TStack, list<Offset*>& CStack, set<VEdge2D*>& visitedEdges, TRAVERSE_STATUS& status);

	double calculate_edge_length(const VEdge2D* edge);

	void remove_empty_offsets();
	void make_offset_edges(list<Offset>& offsets, const int& IDOfInitialOffset);
	void connect_offset_edge_to_vertex(list<Offset>& offsets, const int& IDOfInitialOffset);
	void find_arc_edges(list<Offset>& offsets, const int& IDOfInitialOffset);

	list<rg_Point2D> calculate_offset_vertex_for_parabolic_edge(VEdge2D* trunk, const float& offsetAmount);

	pair<Generator2D*, Generator2D*> find_polygon_generators(VEdge2D* edge);

	pair<bool, rg_Point2D> find_intersection_point_between_line_segments(const rg_Line2D& line1, const rg_Line2D& line2);


	void make_map_from_edge_to_offset(list<Offset>& offsets, map<VEdge2D*, list<Offset*>>& mapFromEdgeToOffset);

	VEdge2D* find_edge_to_start_search(const rg_Point2D& currPosition, set<Offset*>& offsetToVisit);

	rg_Point2D calculate_drone_start_position();
	rg_BoundingBox2D calculate_bounding_box();

	array<float, 3> calculate_flight_speeds();
	void write_total_plan(const string& planFileName);
	void write_individual_plan(PlanJsonWriter& writer, vector<rg_Point2D>& searchPath, const float& altitude, const float& speed, const string& planFileName);

	

private:
	Ui::VoronoiPolygonOffsetClass ui;

public slots:
	void open_polygon_file();
	void compute_VD();
	void compute_offset();
	void compute_search_path();
	void compute_horizontal_search_path();
	void compute_vertical_search_path();
	void output_plans();
};
