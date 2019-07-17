#include "PlanJsonReader.h"


PlanJsonReader::PlanJsonReader()
{
}


PlanJsonReader::PlanJsonReader(float homeLat, float homeLon)
	:m_homeLat(homeLat)
	, m_homeLon(homeLon)
{

}

PlanJsonReader::~PlanJsonReader()
{
}


void PlanJsonReader::read_plan_file(const char* filePath)
{
	ifstream json_dir(filePath);

	CharReaderBuilder builder;
	builder["collectComments"] = false;
	Value root;
	JSONCPP_STRING errs;
	bool ok = parseFromStream(builder, json_dir, &root, &errs);
	if (ok == true)
	{
		set_home_lat(root["mission"]["plannedHomePosition"][0].asFloat());
		set_home_lon(root["mission"]["plannedHomePosition"][1].asFloat());
		m_polygon.push_back(array<float, 2> {0.0f, 0.0f});

		for (int i = 0; i < root["mission"]["items"].size()-1; ++i)
		{
			array<float,2> temp_coord = translate_gps_to_coord(root["mission"]["items"][i]["params"][4].asFloat(), root["mission"]["items"][i]["params"][5].asFloat());
			m_polygon.push_back(temp_coord);
			cout << temp_coord.at(0) << " " << temp_coord.at(1) << endl;
		}
		
	}
	split_polygon(3);
}


void PlanJsonReader::split_polygon(int numofSplits)
{
	m_splittedPolygon.resize(numofSplits);

	m_maxXOfPolygon = find_right_point(m_polygon).at(0);
	m_minXOfPolygon = find_left_point(m_polygon).at(0);
	m_maxYOfPolygon = find_top_point(m_polygon).at(1);
	m_minYOfPolygon = find_bottom_point(m_polygon).at(1);
		
	vector<float> xVector;
	xVector.push_back(m_minXOfPolygon);
	for (int i = 1; i < numofSplits; ++i)
	{
		xVector.push_back((m_maxXOfPolygon - m_minXOfPolygon) / numofSplits * i + m_minXOfPolygon) ;
	}
	xVector.push_back(m_maxXOfPolygon);
	
	float nowIndex;
	stack<float> indexStack;
	stack<array<float, 2>> finishedStack;
	for (array<float, 2> it_data : m_polygon)
	{
		
		if (it_data.at(0) >= xVector[0] &&
			it_data.at(0) < xVector[1])
		{
			cout << xVector[0] << " " << it_data.at(0) << " " << xVector[1] << endl;
			//m_splittedPolygon[0].push_back(it_data);
			nowIndex = 0;
		}
		else if (it_data.at(0) >= xVector[1] &&
			it_data.at(0) < xVector[2])
		{
			cout << xVector[1] << " " << it_data.at(0) << " " << xVector[2] << endl;
			//m_splittedPolygon[1].push_back(it_data);
			nowIndex = 1;
		}
		else if (it_data.at(0) >= xVector[2])
		{
			cout << xVector[2] << " " << it_data.at(0) << " " << xVector[3] << endl;
			//m_splittedPolygon[2].push_back(it_data);
			nowIndex = 2;
		}


		if (indexStack.empty() == false)
		{
			if (indexStack.top() > nowIndex)
			{	
				cout << "2->1 new y: ";
				cout << (it_data.at(1) - finishedStack.top().at(1)) / (it_data.at(0) - finishedStack.top().at(0)) * (xVector[indexStack.top()] - finishedStack.top().at(0)) + finishedStack.top().at(1) << endl;
				m_splittedPolygon[nowIndex].push_back(
					array<float, 2> { 
						xVector[indexStack.top()] - INTERVAL_BETWEEN_SEARCH_AREA,
							(it_data.at(1) - finishedStack.top().at(1)) / (it_data.at(0) - finishedStack.top().at(0)) * (xVector[indexStack.top()] - finishedStack.top().at(0) - INTERVAL_BETWEEN_SEARCH_AREA) + finishedStack.top().at(1)
					}
				);

				m_splittedPolygon[indexStack.top()].push_back(
					array<float, 2> {
						xVector[indexStack.top()] + INTERVAL_BETWEEN_SEARCH_AREA,
							(it_data.at(1) - finishedStack.top().at(1)) / (it_data.at(0) - finishedStack.top().at(0)) * (xVector[indexStack.top()] - finishedStack.top().at(0) + INTERVAL_BETWEEN_SEARCH_AREA) + finishedStack.top().at(1)
				}
				);

			}
			else if (indexStack.top() < nowIndex)
			{
				cout << "1->2 new y: ";
				cout << (it_data.at(1) - finishedStack.top().at(1)) / (it_data.at(0) - finishedStack.top().at(0)) * (xVector[nowIndex] - finishedStack.top().at(0)) + finishedStack.top().at(1) << endl;
				
				m_splittedPolygon[nowIndex].push_back(
					array<float, 2> {
					xVector[nowIndex] + INTERVAL_BETWEEN_SEARCH_AREA,
						(it_data.at(1) - finishedStack.top().at(1)) / (it_data.at(0) - finishedStack.top().at(0)) * (xVector[nowIndex] - finishedStack.top().at(0) + INTERVAL_BETWEEN_SEARCH_AREA) + finishedStack.top().at(1)
				}
				);
				m_splittedPolygon[indexStack.top()].push_back(
					array<float, 2> {
						//(it_data.at(0) / 3.0f * 2.0f + finishedStack.top().at(0) / 3.0f * 1.0f),
						//(it_data.at(1) / 3.0f * 2.0f + finishedStack.top().at(1) / 3.0f * 1.0f)
						xVector[nowIndex] - INTERVAL_BETWEEN_SEARCH_AREA,
							(it_data.at(1) - finishedStack.top().at(1)) / (it_data.at(0) - finishedStack.top().at(0)) * (xVector[nowIndex] - finishedStack.top().at(0) - INTERVAL_BETWEEN_SEARCH_AREA) + finishedStack.top().at(1)
						
				}
				);
			}
		}

		if (it_data.at(0) >= xVector[0] &&
			it_data.at(0) < xVector[1])
		{
			m_splittedPolygon[0].push_back(it_data);
		}
		else if (it_data.at(0) >= xVector[1] &&
			it_data.at(0) < xVector[2])
		{
			m_splittedPolygon[1].push_back(it_data);
		}
		else if (it_data.at(0) >= xVector[2])
		{
			m_splittedPolygon[2].push_back(it_data);
		}


		finishedStack.push(it_data);
		indexStack.push(nowIndex);
	}




	int i = 1;
	for (list<array<float, 2>> polygon : m_splittedPolygon)
	{
		cout << i << "'s Polygon" << endl;
		for (array<float, 2> it_data : polygon)
		{
			cout << it_data.at(0) << " " << it_data.at(1) << endl;
		}
		++i;
	}
}

list<rg_Point2D> PlanJsonReader::make_horizental_zigzag_path(list<array<float, 2>> polygon, int numSlice)
{
	list<rg_Point2D> path;

	array<float, 2> bottomPoint = find_bottom_point(polygon);
	array<float, 2> topPoint = find_top_point(polygon);
	array<float, 2> leftPoint = find_left_point(polygon);
	array<float, 2> rightPoint = find_right_point(polygon);
	//find bottom point
	//find top point

	float maxY = topPoint.at(1);
	float minY = bottomPoint.at(1);
	float maxX = rightPoint.at(0);
	float minX = leftPoint.at(0);

	vector<rg_Line2D> ySlice;
	for (int i = 1; i < numSlice; ++i)
	{
		float tmpY = (maxY - minY) / numSlice * i + minY;
		ySlice.push_back(rg_Line2D(rg_Point2D(minX, tmpY), rg_Point2D(maxX, tmpY)));
	}

	vector<rg_Line2D> linesOfPolygon = polygon_point_to_line(polygon);


	path.push_back(rg_Point2D(bottomPoint.at(0), bottomPoint.at(1)));
	queue<rg_Point2D> leftQueue;
	queue<rg_Point2D> rightQueue;
	vector<pair<bool, rg_Point2D>> tmpVector;
	vector<rg_Point2D> compareVector;
	for (rg_Line2D lined2D : ySlice)
	{
		for (rg_Line2D lineOfPolygon : linesOfPolygon)
		{
			tmpVector.push_back(find_intersection_point_between_line_segments(lineOfPolygon, lined2D));
		}

		for (pair<bool, rg_Point2D> result : tmpVector)
		{
			if (result.first == false)
			{
				continue;
			}
			else
			{
				compareVector.push_back(result.second);
			}
		}
		double leftX = min(compareVector[0].getX(), compareVector[1].getX());
		double rightX = max(compareVector[0].getX(), compareVector[1].getX());

		leftQueue.push(rg_Point2D(leftX, lined2D.getEP().getY()));
		rightQueue.push(rg_Point2D(rightX, lined2D.getEP().getY()));
		tmpVector.clear();
		compareVector.clear();
	}
	bool lastIsLeft = true;
	while (leftQueue.empty() != true)
	{
		if (lastIsLeft == true)
		{
			rg_Point2D leftOne = leftQueue.front();
			rg_Point2D rightOne = rightQueue.front();
			path.push_back(leftOne);
			path.push_back(rightOne);
			lastIsLeft = false;
		}
		else if (lastIsLeft == false)
		{
			rg_Point2D leftOne = leftQueue.front();
			rg_Point2D rightOne = rightQueue.front();
			path.push_back(rightOne);
			path.push_back(leftOne);
			lastIsLeft = true;
		}
		leftQueue.pop();
		rightQueue.pop();
	}
	path.push_back(rg_Point2D(topPoint.at(0), topPoint.at(1)));

	cout << " path " << endl;
	for (auto it_auto : path)
	{
		cout << it_auto.getX() << " " << it_auto.getY() << endl;
	}

	return path;
}

std::list<rg_Point2D> PlanJsonReader::make_vertical_zigzag_path(list < array<float, 2>> polygon, int numSlice)
{
	list<rg_Point2D> path;

	array<float, 2> bottomPoint = find_bottom_point(polygon);
	array<float, 2> topPoint = find_top_point(polygon);
	array<float, 2> leftPoint = find_left_point(polygon);
	array<float, 2> rightPoint = find_right_point(polygon);
	//find bottom point
	//find top point

	float maxY = topPoint.at(1);
	float minY = bottomPoint.at(1);
	float maxX = rightPoint.at(0);
	float minX = leftPoint.at(0);

	vector<rg_Line2D> xSlice;
	for (int i = 1; i < numSlice; ++i)
	{
		float tmpX = (maxX - minX) / numSlice * i + minX;
		xSlice.push_back(rg_Line2D(rg_Point2D(tmpX, minY), rg_Point2D(tmpX, maxY)));
	}

	list<array<float, 2>>::iterator iter = polygon.begin();

	vector<rg_Line2D> linesOfPolygon = polygon_point_to_line(polygon);



	path.push_back(rg_Point2D(leftPoint.at(0), leftPoint.at(1)));

	queue<rg_Point2D> bottomQueue;
	queue<rg_Point2D> topQueue;
	vector<pair<bool, rg_Point2D>> tmpVector;
	vector<rg_Point2D> compareVector;

	for (rg_Line2D lined2D : xSlice)
	{
		for (rg_Line2D lineOfPolygon : linesOfPolygon)
		{
			tmpVector.push_back(find_intersection_point_between_line_segments(lineOfPolygon, lined2D));
		}

		for (pair<bool, rg_Point2D> result : tmpVector)
		{
			if (result.first == false)
			{
				continue;
			}
			else
			{
				compareVector.push_back(result.second);
			}
		}
		double bottomY = min(compareVector[0].getY(), compareVector[1].getY());
		double topY = max(compareVector[0].getY(), compareVector[1].getY());

		bottomQueue.push(rg_Point2D(lined2D.getEP().getX(),bottomY));
		topQueue.push(rg_Point2D(lined2D.getEP().getX(), topY));
		tmpVector.clear();
		compareVector.clear();
	}

	bool latsIsBottom = true;
	while (bottomQueue.empty() != true)
	{
		if (latsIsBottom == true)
		{
			rg_Point2D bottomOne = bottomQueue.front();
			rg_Point2D topOne = topQueue.front();
			path.push_back(bottomOne);
			path.push_back(topOne);
			latsIsBottom = false;
		}
		else if (latsIsBottom == false)
		{
			rg_Point2D bottomOne = bottomQueue.front();
			rg_Point2D topOne = topQueue.front();
			path.push_back(topOne);
			path.push_back(bottomOne);
			latsIsBottom = true;
		}
		bottomQueue.pop();
		topQueue.pop();
	}

	path.push_back(rg_Point2D(rightPoint.at(0), rightPoint.at(1)));

	cout << " path " << endl;
	for (auto it_auto : path)
	{
		cout << it_auto.getX() << " " << it_auto.getY() << endl;
	}

	return path;
}

pair<bool, rg_Point2D> PlanJsonReader::find_intersection_point_between_line_segments(const rg_Line2D& line1, const rg_Line2D& line2)
{
	float p0_x = line1.getSP().getX();
	float p1_x = line1.getEP().getX();
	float p0_y = line1.getSP().getY();
	float p1_y = line1.getEP().getY();

	float p2_x = line2.getSP().getX();
	float p3_x = line2.getEP().getX();
	float p2_y = line2.getSP().getY();
	float p3_y = line2.getEP().getY();

	float s1_x = p1_x - p0_x;
	float s1_y = p1_y - p0_y;
	float s2_x = p3_x - p2_x;
	float s2_y = p3_y - p2_y;

	float s, t;
	s = (-s1_y * (p0_x - p2_x) + s1_x * (p0_y - p2_y)) / (-s2_x * s1_y + s1_x * s2_y);
	t = (s2_x * (p0_y - p2_y) - s2_y * (p0_x - p2_x)) / (-s2_x * s1_y + s1_x * s2_y);

	bool doesIntersectionFound = false;
	rg_Point2D intersectionPoint;
	if (s >= 0 && s <= 1 && t >= 0 && t <= 1)
	{
		doesIntersectionFound = true;
		intersectionPoint.setX(p0_x + t * s1_x);
		intersectionPoint.setY(p0_y + t * s1_y);
	}

	return make_pair(doesIntersectionFound, intersectionPoint);
}

std::array<float, 2 > PlanJsonReader::find_bottom_point(list<array<float, 2>> polygon)
{
	array<float, 2> bottomPoint = polygon.front();
	for (array<float, 2> tmpPoint : polygon)
	{
		if (tmpPoint.at(1) <= bottomPoint.at(1))
		{
			bottomPoint = tmpPoint;
		}
	}

	return bottomPoint;
}

std::array<float, 2 > PlanJsonReader::find_top_point(list<array<float, 2>> polygon)
{
	array<float, 2> topPoint = polygon.front();
	for (array<float, 2> tmpPoint : polygon)
	{
		if (tmpPoint.at(1) >= topPoint.at(1))
		{
			topPoint = tmpPoint;
		}
	}

	return topPoint;
}

std::array<float, 2 > PlanJsonReader::find_left_point(list<array<float, 2>> polygon)
{
	array<float, 2> leftPoint = polygon.front();
	for (array<float, 2> tmpPoint : polygon)
	{
		if (tmpPoint.at(0) <= leftPoint.at(0))
		{
			leftPoint = tmpPoint;
		}
	}

	return leftPoint;
}

std::array<float, 2 > PlanJsonReader::find_right_point(list<array<float, 2>> polygon)
{
	array<float, 2> rightPoint = polygon.front();
	for (array<float, 2> tmpPoint : polygon)
	{
		if (tmpPoint.at(0) >= rightPoint.at(0))
		{
			rightPoint = tmpPoint;
		}
	}

	return rightPoint;
}

vector<rg_Line2D> PlanJsonReader::polygon_point_to_line(list<array<float, 2>> polygon)
{
	list<array<float, 2>>::iterator iter = polygon.begin();
	vector<rg_Line2D> LinesOfPolygon;
	for (int i = 0; i < polygon.size(); ++i)
	{
		rg_Point2D firstPoint(iter->at(0), iter->at(1));
		if (i == polygon.size() - 1)
		{
			iter = polygon.begin();
		}
		else
		{
			++iter;
		}
		rg_Point2D secondPoint(iter->at(0), iter->at(1));
		LinesOfPolygon.push_back(rg_Line2D(firstPoint, secondPoint));
	}

	return LinesOfPolygon;
}

std::array<float, 2> PlanJsonReader::translate_coord_to_gps(const float& x, const float& y) const
{
	float latitude = m_homeLat + (y / meter_per_latitude());
	float longitude = m_homeLon + (x / meter_per_longitude());
	return { latitude, longitude };
}

std::array<float, 2> PlanJsonReader::translate_gps_to_coord(const float& latitude, const float& longitude) const
{
	float x = (longitude - m_homeLon)*meter_per_longitude();
	float y = (latitude - m_homeLat)*meter_per_latitude();
	return { x, y };
}
