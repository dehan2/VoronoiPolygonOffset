#pragma once
#include <array>
#include <vector>
#include <string>
#include <list>
#include <algorithm>
#include <stack>
#include <queue>
#include "rg_Point2D.h"
#include "rg_Line2D.h"

#include <iostream>
#include <fstream>
#include "json/json.h"
#include "constForPolygonOffset.h"

using namespace std;
using namespace Json;

class PlanJsonReader
{

private:
	float m_homeLat = 0.0f;
	float m_homeLon = 0.0f;
	float m_maxXOfPolygon = 0.0f;
	float m_minXOfPolygon = 0.0f;
	float m_minYOfPolygon = 0.0f;
	float m_maxYOfPolygon = 0.0f;

	list<array<float, 2>> m_polygon;
	vector<list<array<float, 2>>> m_splittedPolygon;

public:

	PlanJsonReader();
	PlanJsonReader(float homeLat, float homeLon);
	~PlanJsonReader();

	void read_plan_file(const char* filePath);
	void split_polygon(int numofSplits);	
	list<rg_Point2D> make_horizental_zigzag_path(list < array<float, 2>> polygon, int numSlice);
	list<rg_Point2D> make_vertical_zigzag_path(list < array<float, 2>> polygon, int numSlice);
	pair<bool, rg_Point2D> find_intersection_point_between_line_segments(const rg_Line2D& line1, const rg_Line2D& line2);

	array<float, 2 > find_bottom_point(list<array<float, 2>> polygon);
	array<float, 2 > find_top_point(list<array<float, 2>> polygon);
	array<float, 2 > find_left_point(list<array<float, 2>> polygon);
	array<float, 2 > find_right_point(list<array<float, 2>> polygon);

	vector<rg_Line2D> polygon_point_to_line(list<array<float, 2>> polygon);

	array<float, 2> translate_coord_to_gps(const float& x, const float& y) const;
	array<float, 2> translate_gps_to_coord(const float& latitude, const float& longitude) const;

	//the formulation belows are based on WGS84 and referred from https://en.wikipedia.org/wiki/Geographic_coordinate_system
	inline double meter_per_latitude() const { return 111132.92 - 559.82*cos(2 * m_homeLat) + 1.175*cos(4 * m_homeLat) - 0.0023*cos(6 * m_homeLat); }
	inline double meter_per_longitude() const { return 111412.84*cos(m_homeLat) - 93.5*cos(3 * m_homeLat) + 0.118*cos(5 * m_homeLat); }

	inline float get_home_lat() const { return m_homeLat; }
	inline float get_home_lon() const { return m_homeLon; }
	inline float get_max_x() const { return m_maxXOfPolygon; }
	inline float get_min_x() const { return m_minXOfPolygon; }
	inline float get_max_y() const { return m_maxYOfPolygon; }
	inline float get_min_y() const { return m_minYOfPolygon; }
	inline list<array<float, 2>>& get_polygon() { return m_polygon; }
	inline vector<list<array<float, 2>>>& get_splittedPolygon() { return m_splittedPolygon; }

	inline void set_home_lat(const float homeLat) { m_homeLat = homeLat; }
	inline void set_home_lon(const float homeLon) { m_homeLon = homeLon; }
	inline void set_polygons(const list<array<float, 2>>& polygon) { m_polygon = polygon; }
};

