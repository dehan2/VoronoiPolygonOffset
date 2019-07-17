#pragma once
#include <array>
#include <vector>

#include <fstream>
#include "json/json.h"

using namespace std;
using namespace Json;

class PlanJsonWriter
{

private:
	float m_homeLat = 0.0f;
	float m_homeLon = 0.0f;
	float m_altitude = 0.0f;

	float m_launchLat = 0.0f;
	float m_launchLon = 0.0f;

	int m_command = 16;
	int m_frame = 3;
	int m_jumpID = 1;

	array<float, 2> m_center = {0.0,};//latitude,longitude
	float m_radius = 0.0;

	array<float, 2> m_bottomleft = { 0.0f, };
	array<float, 2> m_topright = { 0.0f, };

	array<float, 4> m_params = { 0.0f, };

	float m_hoverspeed = 0.0f;
	float m_unitspeed = 0.1f;

	vector<array<float, 2>> m_polygons;//latitude, longitude

	bool m_circleFenceCheck = 0;
	bool m_polyFenceCheck = 0;

	void addTakeoff(Value& items);
	void addWayPoints(Value& items);
	void add_change_speed(Value& items);
	void addReturnToLaunch(Value& items);

public:
	PlanJsonWriter();
	PlanJsonWriter(float homeLat, float homeLon);
	~PlanJsonWriter();

	void writePlanFile(string filePath);

	array<float, 2> translate_coord_to_gps(const float& x, const float& y) const;
	array<float, 2> translate_gps_to_coord(const float& latitude, const float& longitude) const;

	inline double meter_per_latitude() const { return 111132.92 - 559.82*cos(2 * m_homeLat) + 1.175*cos(4 * m_homeLat) - 0.0023*cos(6 * m_homeLat); }
	inline double meter_per_longitude() const { return 111412.84*cos(m_homeLat) - 93.5*cos(3 * m_homeLat) + 0.118*cos(5 * m_homeLat); }

	inline void set_home(const float& homeLat, const float& homeLon) { m_homeLat = homeLat;  m_homeLon = homeLon; }
	inline void set_launchPoint(const float& launchLat, const float& launchLon) { m_launchLat = launchLat; m_launchLon = launchLon; }

	void set_polygons(const vector<array<float, 2>> polygons);
	inline void set_altitude(const float& altitude) { m_altitude = altitude; }

	inline void set_command(const int& command) { m_command = command; }
	inline void set_frame(const int& frame) { m_frame = frame; }
	void set_params(const array<float, 4>& params);
	inline void set_hoverspeed(const float& speed) { m_hoverspeed = speed; }
	void set_hoverspeed(const float& pathlength, const float& covertime_sec);

	inline void set_geoFence(const array<float, 2>& center, const float& radius) { m_center[0] = center[0]; m_center[1] = center[1]; m_radius = radius; m_circleFenceCheck = 1; }
	inline void set_geoFence(const array<float, 2>& bottomleft, const array<float, 2> topright) { m_bottomleft[0] = bottomleft[0]; m_bottomleft[1] = bottomleft[1]; m_topright[0] = topright[0]; m_topright[1] = topright[1]; m_polyFenceCheck = 1; }
};

