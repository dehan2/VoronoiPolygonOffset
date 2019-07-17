#include "PlanJsonWriter.h"
#include <iostream>

void PlanJsonWriter::addTakeoff(Value &items)
{
	Value waypoint;

	waypoint["AMSLAltAboveTerrain"] = m_altitude;
	waypoint["Altitude"] = m_altitude;
	waypoint["AltitudeMode"] = 1;
	waypoint["autoContinue"] = true;
	waypoint["command"] = 22;
	waypoint["doJumpId"] = m_jumpID++;
	waypoint["frame"] = m_frame;
	waypoint["type"] = "SimpleItem";

	Value params;

	params.append(15);
	params.append(0);
	params.append(0);
	params.append(Value::null);
	params.append(m_launchLat);
	params.append(m_launchLon);
	params.append(m_altitude);

	waypoint["params"] = params;

	items.append(waypoint);
}

void PlanJsonWriter::addWayPoints(Value& items)
{
	for (int i = 0; i < m_polygons.size(); i++)
	{
		Value waypoint;

		waypoint["AMSLAltAboveTerrain"] = m_altitude;
		waypoint["Altitude"] = m_altitude;
		waypoint["AltitudeMode"] = 1;
		waypoint["autoContinue"] = true;
		waypoint["command"] = m_command;
		waypoint["doJumpId"] = m_jumpID++;
		waypoint["frame"] = m_frame;
		waypoint["type"] = "SimpleItem";

		Value params;
		params.append(m_params[0]);
		params.append(m_params[1]);
		params.append(m_params[2]);
		params.append(m_params[3]);
		params.append(m_polygons[i][0]);
		params.append(m_polygons[i][1]);
		params.append(m_altitude);

		waypoint["params"] = params;

		items.append(waypoint);
	}
}



void PlanJsonWriter::add_change_speed(Value& items)
{
	Value waypoint;

	waypoint["autoContinue"] = true;
	waypoint["command"] = 178;
	waypoint["doJumpId"] = m_jumpID++;
	waypoint["frame"] = 2;
	waypoint["type"] = "SimpleItem";

	Value params;
	params.append(1);
	params.append(m_hoverspeed);
	params.append(-1);
	params.append(0);
	params.append(0);
	params.append(0);
	params.append(0);

	waypoint["params"] = params;

	items.append(waypoint);
}



void PlanJsonWriter::addReturnToLaunch(Value& items)
{
	Value waypoint;

	waypoint["autoContinue"] = true;
	waypoint["command"] = 20;
	waypoint["doJumpId"] = m_jumpID++;
	waypoint["frame"] = 2;
	waypoint["type"] = "SimpleItem";

	Value params;
	params.append(0);
	params.append(0);
	params.append(0);
	params.append(0);
	params.append(0);
	params.append(0);
	params.append(0);

	waypoint["params"] = params;

	items.append(waypoint);
}

PlanJsonWriter::PlanJsonWriter()
{
}


PlanJsonWriter::PlanJsonWriter(float homeLat, float homeLon)
	:m_homeLat(homeLat)
	,m_homeLon(homeLon)
{

}

PlanJsonWriter::~PlanJsonWriter()
{
}


void PlanJsonWriter::writePlanFile(string filePath)
{
	ofstream planfile(filePath);

	Value Plan;
	Plan["fileType"] = "Plan";
	Plan["groundStation"] = "QGroundControl";
	Plan["version"] = 1;

	Value geoFence;

	Value circles;

	if (m_circleFenceCheck == 1)
	{
		Value fenceCenter;
		fenceCenter.append(m_center[0]);
		fenceCenter.append(m_center[1]);

		Value fenceCircle;
		fenceCircle["center"] = fenceCenter;
		fenceCircle["radius"] = m_radius;

		Value circle;
		circle["inclusion"] = true;
		circle["circle"] = fenceCircle;
		circle["version"] = 1;

		circles.append(circle);

		geoFence["circles"] = circles;
	}
	else
	{
		geoFence["circles"] = arrayValue;
	}

	Value polygons;

	if (m_polyFenceCheck == 1)
	{
		Value bottomleft;
		bottomleft.append(m_bottomleft[0]);
		bottomleft.append(m_bottomleft[1]);
		Value bottomright;
		bottomright.append(m_bottomleft[0]);
		bottomright.append(m_topright[1]);
		Value topright;
		topright.append(m_topright[0]);
		topright.append(m_topright[1]);
		Value topleft;
		topleft.append(m_topright[0]);
		topleft.append(m_bottomleft[1]);

		Value polygon1;
		polygon1.append(bottomleft);
		polygon1.append(bottomright);
		polygon1.append(topright);
		polygon1.append(topleft);

		polygons["inclusion"] = true;
		polygons["polygon"] = polygon1;
		polygons["version"] = 1;

		Value polygon;
		polygon.append(polygons);
		
		geoFence["polygons"] = polygon;
	}
	else
	{
		geoFence["polygons"] = arrayValue;
	}

	geoFence["version"] = 2;

	Plan["geoFence"] = geoFence;

	Value mission;

	mission["cruiseSpeed"] = 15;
	mission["firmwareType"] = 12;
	mission["hoverspeed"] = m_hoverspeed;

	Value items;
	
	add_change_speed(items);
	addTakeoff(items);
	addWayPoints(items);
	addReturnToLaunch(items);

	Value plannedhomeposition;
	plannedhomeposition.append(m_launchLat);
	plannedhomeposition.append(m_launchLon);
	plannedhomeposition.append(0);

	mission["plannedHomePosition"] = plannedhomeposition;
	mission["vehicleType"] = 2;
	mission["version"] = 2;

	mission["items"] = items;

	//Value point;
	//Value points;
	//points.append(point);

	Value rallypoints;
	//rallypoints["points"] = points;
	rallypoints["points"] = arrayValue;
	rallypoints["version"] = 1;

	Plan["rallyPoints"] = rallypoints;

	Plan["mission"] = mission;

	StreamWriterBuilder builder;
	builder["commentStyle"] = "None";
	builder["indentation"] = "\t";

	unique_ptr<Json::StreamWriter> writter(builder.newStreamWriter());

	writter->write(Plan, &planfile);

	planfile.close();
}


std::array<float, 2> PlanJsonWriter::translate_coord_to_gps(const float& x, const float& y) const
{
	float latitude = m_homeLat + (y / meter_per_latitude());
	float longitude = m_homeLon + (x / meter_per_longitude());
	return { latitude, longitude };
}

std::array<float, 2> PlanJsonWriter::translate_gps_to_coord(const float& latitude, const float& longitude) const
{
	float x = (longitude - m_homeLon)*meter_per_longitude();
	float y = (latitude - m_homeLat)*meter_per_latitude();
	return { x, y };
}

void PlanJsonWriter::set_polygons(const vector<array<float, 2>> polygons)
{
	m_polygons.resize(polygons.size());
	copy(polygons.begin(), polygons.end(), m_polygons.begin());

	for (array<float, 2>& polygon : m_polygons)
	{
		polygon = translate_coord_to_gps(polygon.at(0), polygon.at(1));
	}
}

void PlanJsonWriter::set_params(const array<float, 4>& params)
{
	for (int i = 0; i < 4; i++)
	{
		m_params[i] = params[i];
	}
}

void PlanJsonWriter::set_hoverspeed(const float & pathlength_meter, const float& covertime_sec)
{
	m_hoverspeed = pathlength_meter / covertime_sec;
}
