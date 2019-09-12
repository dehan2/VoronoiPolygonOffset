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
	list<rg_Circle2D> m_disks;

	PolygonVD2D m_VD;
	vector<const VVertex2D*> m_mapFromPolygonVtxToVoroVtx;
	set<const VEdge2D*> m_insiderVEdges;
	set<const VEdge2D*> m_branches;

	vector<Offset> m_offsets;

	list<rg_Line2D> m_horizontalOffsets;

	//For Draw
	VoronoiPolygonOffsetDisplayer* m_scene = nullptr;
	double m_scale = 1.0;

public:
	VoronoiPolygonOffset(QWidget *parent = Q_NULLPTR);
	void wheelEvent(QWheelEvent *event);

	void readData(const string& filename, Polygon2D& polygon, list<rg_Circle2D>& disks);
	void constructVD_of_disks_in_polygon(PolygonVD2D& vd_polygon, const Polygon2D& polygon, const list<rg_Circle2D>& disks);

	
	void make_offset_components(const double& offsetAmount);

		set<const VVertex2D*> find_vertices_inside_offset(const double& offsetValue);
		map<const VEdge2D*, int> count_intersection_between_offset_and_edge(const set<const VEdge2D*>& insiderVEdges, const double& offsetAmount);

		set<const VVertex2D*> propagate_vertices_inside_offset(set<const VVertex2D*>& vertexSet, const map<const VEdge2D*, int>& mapFromEdgeToOffsetIntersectionCounter);
		list<const VEdge2D*> make_edge_traverse_list_for_offset_component(const set<const VVertex2D*>& insideVertices, const map<const VEdge2D*, int>& mapFromEdgeToOffsetIntersectionCounter);
		list<const VEdge2D*> make_edge_traverse_list_for_offset_component_connected_to_branch(const VEdge2D* branch, const VVertex2D* insideVtx, const map<const VEdge2D*, int>& mapFromEdgeToOffsetIntersectionCounter);
		
		list<pair<rg_Point2D, const VEdge2D*>> find_offset_vertices_geometry(const list<const VEdge2D*>& traverseList, const double& offsetAmount);
		list<rg_Point2D> calculate_offset_vertex_for_parabolic_edge(const VEdge2D* parabolicEdge, const float& offsetAmount);

		pair<const Generator2D*, const Generator2D*> find_polygon_generators(const VEdge2D* edge);

		pair<bool, rg_Point2D> find_intersection_point_between_line_segments(const rg_Line2D& line1, const rg_Line2D& line2);

	void connect_VVertices_to_polygonVtx();
	void classify_insider_VEdges();

	pair<bool, rg_Point2D> find_offset_vertex_for_branch(VEdge2D* branch, const float& offsetAmount);
	pair<bool, list<rg_Point2D>> find_offset_vertex_for_trunk(VEdge2D* trunk, const float& offsetAmount);
	void manage_offset_vertices_order(list<rg_Point2D>& offsetVertices, VVertex2D* startVtxOfTraverse);
	
	void make_offset_edges(vector<Offset>& offsets, const int& IDOfInitialOffset);
	void connect_offset_edge_to_vertex(vector<Offset>& offsets, const int& IDOfInitialOffset);

	const Generator2D* find_generator_for_offset_edge(const OffsetEdge& edge);
	Arc2D make_arc_for_offset_edge(const DiskGenerator2D* diskGenerator, rg_Point2D startPt, rg_Point2D endPt, double offsetAmount);

	set<DiskGenerator2D*> find_generators_in_offset_loop(set<const VVertex2D*>& vertexSet);
	
	float calculate_offset_length();


	//Axis Parallel Offsets


private:
	Ui::VoronoiPolygonOffsetClass ui;

public slots:
	void open_polygon_file();
	void compute_VD();
	void compute_offset();
	void compute_horizontal_offsets();
};
