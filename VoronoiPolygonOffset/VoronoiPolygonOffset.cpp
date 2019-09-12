#include "VoronoiPolygonOffset.h"
#include "rg_GeoFunc.h"
#include "rg_IntersectFunc.h"
#include "constForPolygonOffset.h"
#include <fstream>
#include <array>

#include <QFileDialog>
#include <QFileInfo>



static string translate_to_window_path(const QString& QfilePath)
{
	string filePath = QfilePath.toLocal8Bit();

	size_t i = filePath.find('/');
	while (i != string::npos)
	{
		string part1 = filePath.substr(0, i);
		string part2 = filePath.substr(i + 1);
		filePath = part1 + R"(\)" + part2; // Use "\\\\" instead of R"(\\)" if your compiler doesn't support C++11's raw string literals
		i = filePath.find('/', i + 1);
	}
	return filePath;
}



VoronoiPolygonOffset::VoronoiPolygonOffset(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	m_scene = new VoronoiPolygonOffsetDisplayer(parent);

	ui.graphicsView->setAcceptDrops(true);

	ui.graphicsView->setSceneRect(-400, -400, 800, 800);
	ui.graphicsView->setScene(m_scene);
	ui.graphicsView->scale(1, -1);

	showMaximized();
}



void VoronoiPolygonOffset::wheelEvent(QWheelEvent *event)
{
	if (event->delta() > 0)
	{
		m_scale = m_scale * 1.01;

		QMatrix matrix;
		matrix.scale(m_scale, -m_scale);

		QTransform transform(matrix);

		ui.graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
		ui.graphicsView->setTransform(transform);
		ui.graphicsView->update();
		//ui.graphicsView->scale(m_scale, -m_scale);
	}
	else
	{
		m_scale = m_scale / 1.01;

		QMatrix matrix;
		matrix.scale(m_scale, -m_scale);

		QTransform transform(matrix);

		ui.graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
		ui.graphicsView->setTransform(transform);
		ui.graphicsView->update();
	}
}



void VoronoiPolygonOffset::readData(const string& filename, Polygon2D& polygon, list<rg_Circle2D>& disks)
{
	ifstream fin;
	fin.open(filename);

	char* seps = " \t\n";
	char buffer[200];
	char* context = NULL;

	fin.getline(buffer, 200);
	rg_INT numVertices = atoi(strtok_s(buffer, seps, &context));

	rg_Point2D* boundaryVertices = new rg_Point2D[numVertices];
	rg_BoundingBox2D boundingBox;

	for (rg_INT i = 0; i < numVertices; i++)
	{
		fin.getline(buffer, 200);
		rg_REAL x = atof(strtok_s(buffer, seps, &context));
		rg_REAL y = atof(strtok_s(NULL, seps, &context));
		boundaryVertices[i].setPoint(x, y);

		rg_Point2D point(x, y);
		boundingBox.contain(point);
	}

	rg_Point2D minPt = boundingBox.getMinPt();
	rg_Point2D maxPt = boundingBox.getMaxPt();
	rg_REAL deltaX = minPt.getX();
	rg_REAL deltaY = minPt.getY();

	//for (rg_INT i = 0; i < numVertices; i++)
	//{
	//	boundaryVertices[i].setX(boundaryVertices[i].getX() - deltaX);
	//	boundaryVertices[i].setY(boundaryVertices[i].getY() - deltaY);
	//}

	//m_boundingBox.setMinPt(rg_Point2D(minPt.getX() - deltaX, minPt.getY() - deltaY));
	//m_boundingBox.setMaxPt(rg_Point2D(maxPt.getX() - deltaX, maxPt.getY() - deltaY));

	polygon.setBoundaryVertices(boundaryVertices, numVertices);

	fin.getline(buffer, 200);
	int numDisks = atoi(strtok(buffer, seps));

	for (int i = 0; i < numDisks; ++i) {
		fin.getline(buffer, 200);
		double x = atof(strtok(buffer, seps));
		double y = atof(strtok(NULL, seps));
		double r = atof(strtok(NULL, seps));

		rg_Circle2D circle(x, y, r);
		disks.push_back(circle);
	}
}



void VoronoiPolygonOffset::constructVD_of_disks_in_polygon(PolygonVD2D& vd_polygon, const Polygon2D& polygon, const list<rg_Circle2D>& disks)
{
	vd_polygon.constructVoronoiDiagram(polygon);

	for (list<rg_Circle2D>::const_iterator i_disk = disks.begin(); i_disk != disks.end(); ++i_disk) {
		const rg_Circle2D& disk = *i_disk;
		vd_polygon.insertThisDiskInPolygonVoronoiDiagram(disk);
	}
}



set<const VVertex2D*> VoronoiPolygonOffset::find_vertices_inside_offset(const double& offsetAmount)
{
	set<const VVertex2D*> insideVertices;

	for (auto edge : m_insiderVEdges)
	{
		VVertex2D* startVtx = edge->getStartVertex();
		VVertex2D* endVtx = edge->getEndVertex();

		if (startVtx->computeRadiusOfTangentCircle() > offsetAmount)
			insideVertices.insert(startVtx);
		
		if (endVtx->computeRadiusOfTangentCircle() > offsetAmount)
			insideVertices.insert(endVtx);
	}

	return insideVertices;
}



map<const VEdge2D*, int> VoronoiPolygonOffset::count_intersection_between_offset_and_edge(const set<const VEdge2D*>& insiderVEdges, const double& offsetAmount)
{
	map<const VEdge2D*, int> mapFromEdgeToOffsetIntersectionCounter;
	for (const auto edge : insiderVEdges)
	{
		pair<const Generator2D*, const Generator2D*> polygonGenerators = find_polygon_generators(edge);

		const Generator2D* leftGenerator = polygonGenerators.first;
		const Generator2D* rightGenerator = polygonGenerators.second;

		//Assume: convex polygon
		if (leftGenerator->type() == Generator2D::EDGE_G && rightGenerator->type() == Generator2D::EDGE_G)
		{
			//Line edge
			const EdgeGenerator2D* edgeGenerator = static_cast<const EdgeGenerator2D*>(leftGenerator);
			rg_Line2D directrixOfParabola(edgeGenerator->get_start_vertex_point(), edgeGenerator->get_end_vertex_point());
			rg_Line2D offSetLine = directrixOfParabola.make_parallel_line_to_normal_direction(offsetAmount);
			rg_Line2D edgeLine(edge->getStartVertex()->getLocation(), edge->getEndVertex()->getLocation());

			pair<bool, rg_Point2D> intersectionPoint = find_intersection_point_between_line_segments(edgeLine, offSetLine);
			if (intersectionPoint.first == true)
			{
				mapFromEdgeToOffsetIntersectionCounter[edge] = 1;
			}
			else
				mapFromEdgeToOffsetIntersectionCounter[edge] = 0;
		}
		else if (leftGenerator->type() == Generator2D::DISK_G && rightGenerator->type() == Generator2D::DISK_G)
		{
			//Hyperbola-circle intersection
			const DiskGenerator2D* leftDisk = static_cast<const DiskGenerator2D*>(leftGenerator);
			const DiskGenerator2D* rightDisk = static_cast<const DiskGenerator2D*>(rightGenerator);
			array<rg_Circle2D, 2> tangentCircles;
			mapFromEdgeToOffsetIntersectionCounter[edge] = computeCircleTangentTo2CirclesGivenRadius(leftDisk->getDisk(), rightDisk->getDisk(), offsetAmount, tangentCircles.data());
		}
		else
		{
			//Circle-parabola intersection
			list<rg_Point2D> candidates = calculate_offset_vertex_for_parabolic_edge(edge, offsetAmount);
			mapFromEdgeToOffsetIntersectionCounter[edge] = candidates.size();
		}
	}

	return mapFromEdgeToOffsetIntersectionCounter;
}



void VoronoiPolygonOffset::make_offset_components(const double& offsetAmount)
{
	int firstOffsetID = m_offsets.size();

	set<const VVertex2D*> insideVertices;
	insideVertices = find_vertices_inside_offset(offsetAmount);
	map<const VEdge2D*, int> mapFromEdgeToOffsetIntersectionCounter = count_intersection_between_offset_and_edge(m_insiderVEdges, offsetAmount);

	set<const VEdge2D*> unvisitedBranches = m_branches;

	while (!insideVertices.empty())
	{
		set<const VVertex2D*> insideVerticesForOffset = propagate_vertices_inside_offset(insideVertices, mapFromEdgeToOffsetIntersectionCounter);
		
		list<const VEdge2D*> edgeTraverseList = make_edge_traverse_list_for_offset_component(insideVerticesForOffset, mapFromEdgeToOffsetIntersectionCounter);
		if (!edgeTraverseList.empty())
		{
			list<pair<rg_Point2D, const VEdge2D*>> offsetVertices = find_offset_vertices_geometry(edgeTraverseList, offsetAmount);

			m_offsets.push_back(Offset(m_offsets.size(), offsetAmount));
			for (auto& offsetVertexPair : offsetVertices)
			{
				Offset& offset = m_offsets.back();
				offset.add_offset_vertex(offsetVertexPair.first, offsetVertexPair.second, V_EDGE);
			}

			for (auto visitedEdge : edgeTraverseList)
				if (unvisitedBranches.find(visitedEdge) != unvisitedBranches.end())
					unvisitedBranches.erase(visitedEdge);
		}
		
		set<DiskGenerator2D*> generatorsInOffsetLoop = find_generators_in_offset_loop(insideVerticesForOffset);
		for (auto generator : generatorsInOffsetLoop)
		{
			m_offsets.push_back(Offset(m_offsets.size(), offsetAmount));
			Offset& offset = m_offsets.back();

			rg_Point2D offsetPoint = generator->getDisk().getCenterPt() + rg_Point2D(generator->getDisk().getRadius() + offsetAmount, 0);
			offset.add_offset_vertex(offsetPoint, generator, DISK_G);
		}
	}

	//Traverse for branches
	while (!unvisitedBranches.empty())
	{
		auto branch = (*unvisitedBranches.begin());

		VVertex2D* startVtx = branch->getStartVertex();
		VVertex2D* endVtx = branch->getEndVertex();

		list<const VEdge2D*> edgeTraverseList;
		if (m_VD.get_location_status_of_vvertex(startVtx) == PolygonVD2D::INSIDE_POLYGON
			&& startVtx->computeRadiusOfTangentCircle() > offsetAmount)
		{
			edgeTraverseList = make_edge_traverse_list_for_offset_component_connected_to_branch(*unvisitedBranches.begin(), startVtx, mapFromEdgeToOffsetIntersectionCounter);
		}
		else if (m_VD.get_location_status_of_vvertex(endVtx) == PolygonVD2D::INSIDE_POLYGON
			&&endVtx->computeRadiusOfTangentCircle() > offsetAmount)
		{
			edgeTraverseList = make_edge_traverse_list_for_offset_component_connected_to_branch(*unvisitedBranches.begin(), endVtx, mapFromEdgeToOffsetIntersectionCounter);
		}

		if (!edgeTraverseList.empty())
		{
			list<pair<rg_Point2D, const VEdge2D*>> offsetVertices = find_offset_vertices_geometry(edgeTraverseList, offsetAmount);

			m_offsets.push_back(Offset(m_offsets.size(), offsetAmount));
			for (auto& offsetVertexPair : offsetVertices)
			{
				Offset& offset = m_offsets.back();
				offset.add_offset_vertex(offsetVertexPair.first, offsetVertexPair.second, V_EDGE);
			}

			for (auto visitedEdge : edgeTraverseList)
				if (unvisitedBranches.find(visitedEdge) != unvisitedBranches.end())
					unvisitedBranches.erase(visitedEdge);
		}
		else
		{
			unvisitedBranches.erase(branch);
		}
	}

	make_offset_edges(m_offsets, firstOffsetID);
}



set<const VVertex2D*> VoronoiPolygonOffset::propagate_vertices_inside_offset(set<const VVertex2D*>& vertexSet, const map<const VEdge2D*, int>& mapFromEdgeToOffsetIntersectionCounter)
{
	const VVertex2D* firstVtx = *vertexSet.begin();
	vertexSet.erase(firstVtx);

	set<const VVertex2D*> insideVertices;
	insideVertices.insert(firstVtx);

	list<const VVertex2D*> propagationQ;
	propagationQ.push_back(firstVtx);

	while (!propagationQ.empty())
	{
		const VVertex2D* currVtx = propagationQ.front();
		propagationQ.pop_front();

		list<VEdge2D*> incEdges;
		currVtx->getIncident3VEdges(incEdges);
		for (auto& incEdge : incEdges)
		{
			if (mapFromEdgeToOffsetIntersectionCounter.at(incEdge) == 0)
			{
				VVertex2D* adjVtx = incEdge->getOppositVVertex(currVtx);
				if (vertexSet.find(adjVtx) != vertexSet.end() &&
					insideVertices.find(adjVtx) == insideVertices.end())
				{
					vertexSet.erase(adjVtx);
					insideVertices.insert(adjVtx);
					propagationQ.push_back(adjVtx);
				}
			}
		}
	}

	return insideVertices;
}



list<const VEdge2D*> VoronoiPolygonOffset::make_edge_traverse_list_for_offset_component(const set<const VVertex2D*>& insideVertices, const map<const VEdge2D*, int>& mapFromEdgeToOffsetIntersectionCounter)
{
	list<const VEdge2D*> traverseList;

	const VEdge2D* initialEdge = nullptr;
	const VEdge2D* nextEdge = nullptr;
	VVertex2D* lastVtx = nullptr;

	//Find initial edge
	for (auto edge : m_insiderVEdges)
	{
		VVertex2D* startVtx = edge->getStartVertex();
		VVertex2D* endVtx = edge->getEndVertex();
		int counter = insideVertices.count(startVtx) + insideVertices.count(endVtx);

		pair<const Generator2D*, const Generator2D*> polygonGenerators = find_polygon_generators(edge);
		const Generator2D* leftGenerator = polygonGenerators.first;
		const Generator2D* rightGenerator = polygonGenerators.second;
		bool isBranch = (leftGenerator->type() == Generator2D::EDGE_G && rightGenerator->type() == Generator2D::EDGE_G);

		if (counter == 1 && !isBranch)
		{
			initialEdge = edge;

			auto it = insideVertices.find(edge->getStartVertex());
			if (it != insideVertices.end())
			{
				nextEdge = initialEdge->getLeftLeg();
				lastVtx = edge->getStartVertex();
			}
			else
			{
				nextEdge = initialEdge->getRightHand();
				lastVtx = edge->getEndVertex();
			}
			break;
		}
	}

	if (initialEdge != nullptr)
	{
		traverseList.push_back(initialEdge);

		//Traverse untili return to initial edge
		const VEdge2D* currEdge = nextEdge;
		while (currEdge != initialEdge)
		{
			VVertex2D* startVtx = currEdge->getStartVertex();
			VVertex2D* endVtx = currEdge->getEndVertex();
			traverseList.push_back(currEdge);

			int intersectionCounter = mapFromEdgeToOffsetIntersectionCounter.at(currEdge);
			switch (intersectionCounter)
			{
			case 1:	//Branch -> return
			case 2:
			{
				if (lastVtx == currEdge->getStartVertex())
				{
					nextEdge = currEdge->getLeftLeg();
				}
				else
				{
					nextEdge = currEdge->getRightHand();
				}
			}
			break;
			case 0:	//Trunk
			{
				if (lastVtx == currEdge->getStartVertex())
				{
					nextEdge = currEdge->getRightHand();
					lastVtx = currEdge->getEndVertex();
				}
				else
				{
					nextEdge = currEdge->getLeftLeg();
					lastVtx = currEdge->getStartVertex();
				}
			}
			break;
			default:
				cout << "Traverse Fail!" << endl;
				break;
			}
			currEdge = nextEdge;
		}
	}

	return traverseList;
}



list<const VEdge2D*> VoronoiPolygonOffset::make_edge_traverse_list_for_offset_component_connected_to_branch(const VEdge2D* branch, const VVertex2D* insideVtx, const map<const VEdge2D*, int>& mapFromEdgeToOffsetIntersectionCounter)
{
	list<const VEdge2D*> traverseList;

	const VEdge2D* nextEdge = nullptr;
	VVertex2D* lastVtx = nullptr;

	if (branch->getStartVertex() == insideVtx)
	{
		nextEdge = branch->getLeftLeg();
		lastVtx = branch->getStartVertex();
	}
	else
	{
		nextEdge = branch->getRightHand();
		lastVtx = branch->getEndVertex();
	}

	traverseList.push_back(branch);

	//Traverse untili return to initial edge
	const VEdge2D* currEdge = nextEdge;
	while (currEdge != branch)
	{
		VVertex2D* startVtx = currEdge->getStartVertex();
		VVertex2D* endVtx = currEdge->getEndVertex();
		traverseList.push_back(currEdge);

		int intersectionCounter = mapFromEdgeToOffsetIntersectionCounter.at(currEdge);
		switch (intersectionCounter)
		{
		case 1:	//Branch -> return
		case 2:
		{
			if (lastVtx == currEdge->getStartVertex())
			{
				nextEdge = currEdge->getLeftLeg();
			}
			else
			{
				nextEdge = currEdge->getRightHand();
			}
		}
		break;
		case 0:	//Trunk
		{
			if (lastVtx == currEdge->getStartVertex())
			{
				nextEdge = currEdge->getRightHand();
				lastVtx = currEdge->getEndVertex();
			}
			else
			{
				nextEdge = currEdge->getLeftLeg();
				lastVtx = currEdge->getStartVertex();
			}
		}
		break;
		default:
			cout << "Traverse Fail!" << endl;
			break;
		}
		currEdge = nextEdge;
	}

	return traverseList;
}



list<pair<rg_Point2D, const VEdge2D*>> VoronoiPolygonOffset::find_offset_vertices_geometry(const list<const VEdge2D*>& traverseList, const double& offsetAmount)
{
	list<pair<rg_Point2D, const VEdge2D*>> offsetVertices;

	const VEdge2D* lastEdge = traverseList.back();
	for (auto edge : traverseList)
	{
		pair<const Generator2D*, const Generator2D*> polygonGenerators = find_polygon_generators(edge);

		const Generator2D* leftGenerator = polygonGenerators.first;
		const Generator2D* rightGenerator = polygonGenerators.second;

		//Assume: convex polygon
		if (leftGenerator->type() == Generator2D::EDGE_G && rightGenerator->type() == Generator2D::EDGE_G)
		{
			//Line edge
			const EdgeGenerator2D* edgeGenerator = static_cast<const EdgeGenerator2D*>(leftGenerator);
			rg_Line2D directrixOfParabola(edgeGenerator->get_start_vertex_point(), edgeGenerator->get_end_vertex_point());
			rg_Line2D offSetLine = directrixOfParabola.make_parallel_line_to_normal_direction(offsetAmount);
			rg_Line2D edgeLine(edge->getStartVertex()->getLocation(), edge->getEndVertex()->getLocation());

			pair<bool, rg_Point2D> intersectionPoint = find_intersection_point_between_line_segments(edgeLine, offSetLine);
			if (intersectionPoint.first == true)
			{
				rg_Point2D offsetVtx = intersectionPoint.second;
				offsetVertices.push_back({ offsetVtx, edge });
			}
		}
		else if (leftGenerator->type() == Generator2D::DISK_G && rightGenerator->type() == Generator2D::DISK_G)
		{
			//Hyperbola-circle intersection
			const DiskGenerator2D* leftDisk = static_cast<const DiskGenerator2D*>(leftGenerator);
			const DiskGenerator2D* rightDisk = static_cast<const DiskGenerator2D*>(rightGenerator);
			array<rg_Circle2D, 2> tangentCircles;
			int result = computeCircleTangentTo2CirclesGivenRadius(leftDisk->getDisk(), rightDisk->getDisk(), offsetAmount, tangentCircles.data());

			if (result == 2)
			{
				VVertex2D* commonVtx = nullptr;
				if (edge->getStartVertex() == lastEdge->getStartVertex()
					|| edge->getStartVertex() == lastEdge->getEndVertex())
					commonVtx = edge->getStartVertex();
				else
					commonVtx = edge->getEndVertex();

				rg_Circle2D closestTangentCircle;
				double distance1 = tangentCircles.at(0).getCenterPt().distance(commonVtx->getLocation());
				double distance2 = tangentCircles.at(1).getCenterPt().distance(commonVtx->getLocation());
				if (distance1 < distance2)
					closestTangentCircle = tangentCircles.at(0);
				else
					closestTangentCircle = tangentCircles.at(1);

				rg_Point2D offsetVtx = closestTangentCircle.getCenterPt();
				offsetVertices.push_back({ offsetVtx, edge });
			}	
		}
		else
		{
			//Circle-parabola intersection
			list<rg_Point2D> candidates = calculate_offset_vertex_for_parabolic_edge(edge, offsetAmount);

			if (!candidates.empty())
			{
				VVertex2D* commonVtx = nullptr;
				if (edge->getStartVertex() == lastEdge->getStartVertex()
					|| edge->getStartVertex() == lastEdge->getEndVertex())
					commonVtx = edge->getStartVertex();
				else
					commonVtx = edge->getEndVertex();

				rg_Point2D closestOffsetVertex;
				double closestDistance = DBL_MAX;
				for (auto offsetVtx : candidates)
				{
					double distance = offsetVtx.distance(commonVtx->getLocation());
					if (distance < closestDistance)
					{
						closestDistance = distance;
						closestOffsetVertex = offsetVtx;
					}
				}
				offsetVertices.push_back({ closestOffsetVertex, edge });
			}
		}

		lastEdge = edge;
	}

	return offsetVertices;
}



list<rg_Point2D> VoronoiPolygonOffset::calculate_offset_vertex_for_parabolic_edge(const VEdge2D* parabolicEdge, const float& offsetAmount)
{
	pair<const Generator2D*, const Generator2D*> polygonGenerators = find_polygon_generators(parabolicEdge);

	const Generator2D* leftGenerator = polygonGenerators.first;
	const Generator2D* rightGenerator = polygonGenerators.second;

	//left G -> disk, right G -> edge
	if (leftGenerator->type() == Generator2D::EDGE_G)
		swap(leftGenerator, rightGenerator);

	// Testing intersection between parabola and line

	const EdgeGenerator2D* edgeGenerator = static_cast<const EdgeGenerator2D*>(rightGenerator);
	rg_Line2D directrixOfParabola(edgeGenerator->get_start_vertex_point(), edgeGenerator->get_end_vertex_point());
	//DiskGenerator2D* diskGenerator = static_cast<DiskGenerator2D*>(leftGenerator);

	rg_Line2D offSetLine = directrixOfParabola.make_parallel_line_to_normal_direction(offsetAmount);
	rg_RQBzCurve2D parabola = m_VD.get_geometry_of_edge(parabolicEdge);
	list<rg_Point2D> intersectionPts;
	int numIntersections = 0;
	numIntersections = rg_IntersectFunc::intersectRQBzCurveVsLine(parabola, offSetLine, intersectionPts);

	return intersectionPts;
}



pair<const Generator2D*, const  Generator2D*> VoronoiPolygonOffset::find_polygon_generators(const VEdge2D* edge)
{
	const Generator2D* leftGenerator = static_cast<const Generator2D*>(edge->getLeftFace()->getGenerator()->user_data());
	const Generator2D* rightGenerator = static_cast<const Generator2D*>(edge->getRightFace()->getGenerator()->user_data());
	return make_pair(leftGenerator, rightGenerator);
}



pair<bool, rg_Point2D> VoronoiPolygonOffset::find_intersection_point_between_line_segments(const rg_Line2D& line1, const rg_Line2D& line2)
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







void VoronoiPolygonOffset::connect_VVertices_to_polygonVtx()
{
	rg_Point2D* polyVertices = rg_NULL;
	int numVertices = 0;
	m_polygon.getBoundaryVertices(polyVertices, numVertices);

	list<const VVertex2D*> voroVertices;
	m_VD.getVoronoiVertices(voroVertices);

	//Filter vertices not on boundary
	auto& it = voroVertices.begin();
	while (it != voroVertices.end())
	{
		if (m_VD.get_location_status_of_vvertex(*it) != PolygonVD2D::VDEntity_Location_Status::ON_POLYGON_BOUNDARY)
			it = voroVertices.erase(it);
		else
			it++;
	}
	
	cout << "Map PV to VV" << endl;
	for (int i = 0; i < numVertices; i++)
	{
		rg_Point2D polyVtx = polyVertices[i];
		cout << "PV[" << i << "]: [" << polyVtx.getX() << ", " << polyVtx.getY() << "] - ";

		const VVertex2D* closestVoroVtx = nullptr;
		float minDistance = FLT_MAX;
		for (auto& voroVtx : voroVertices)
		{
			float distance = polyVtx.distance(voroVtx->getLocation());
			if (distance < minDistance)
			{
				minDistance = distance;
				closestVoroVtx = voroVtx;
			}
		}

		cout << "closest VV: [" << closestVoroVtx->getLocation().getX() << ", " << closestVoroVtx->getLocation().getY() << "] "<<endl;
		m_mapFromPolygonVtxToVoroVtx.push_back(closestVoroVtx);
	}

	if (polyVertices != rg_NULL)
		delete[] polyVertices;
}



void VoronoiPolygonOffset::classify_insider_VEdges()
{
	list<const VEdge2D*> edges;

	m_VD.getVoronoiEdges(edges);
	auto& it = edges.begin();
	while (it != edges.end())
		if (m_VD.get_location_status_of_edge(*it)
			!= PolygonVD2D::VDEntity_Location_Status::INSIDE_POLYGON)
			it = edges.erase(it);
		else
		{
			if (m_VD.get_location_status_of_vvertex((*it)->getStartVertex()) == PolygonVD2D::ON_POLYGON_BOUNDARY
				|| m_VD.get_location_status_of_vvertex((*it)->getEndVertex()) == PolygonVD2D::ON_POLYGON_BOUNDARY)
			{
				m_branches.insert(*it);
			}
			m_insiderVEdges.insert(*it++);
		}
			

	/*for (auto& VVtx : m_mapFromPolygonVtxToVoroVtx)
	{
		list<VEdge2D*> incidentEdges;
		VVtx->getIncident3VEdges(incidentEdges);
		int insiderEdgeCounter = 0;
		for (auto& incEdge : incidentEdges)
			if (insiderVEdges.find(incEdge) != insiderVEdges.end())
				insiderEdgeCounter++;

		EDGE_TYPE type;
		switch (insiderEdgeCounter)
		{
		case 1:
			type = BRANCH;
			break;
		case 2:
			type = FLUFF;
			break;
		default:
			cout << "No insider edge found" << endl;
		}

		for (auto& incEdge : incidentEdges)
		{
			if (insiderVEdges.find(incEdge) != insiderVEdges.end())
				m_mapForVEdgeType[incEdge] = type;
		}	
	}

	int trunkCounter = 0;
	for (auto& insiderEdge : insiderVEdges)
		if (m_mapForVEdgeType.count(insiderEdge) == 0)
		{
			m_mapForVEdgeType[insiderEdge] = TRUNK;
			trunkCounter++;
		}
	cout << "Trunks: " << trunkCounter << endl;*/
}



pair<bool, rg_Point2D> VoronoiPolygonOffset::find_offset_vertex_for_branch(VEdge2D* branch, const float& offsetAmount)
{
	pair<const Generator2D*, const Generator2D*> polygonGenerators = find_polygon_generators(branch);

	const Generator2D* leftGenerator = polygonGenerators.first;
	const Generator2D* rightGenerator = polygonGenerators.second;

	bool doesOffsetVtxExist = false;
	rg_Point2D offsetVtx;

	//Line edge
	const EdgeGenerator2D* edgeGenerator = static_cast<const EdgeGenerator2D*>(leftGenerator);
	rg_Line2D directrixOfParabola(edgeGenerator->get_start_vertex_point(), edgeGenerator->get_end_vertex_point());
	rg_Line2D offSetLine = directrixOfParabola.make_parallel_line_to_normal_direction(offsetAmount);
	rg_Line2D branchLine(branch->getStartVertex()->getLocation(), branch->getEndVertex()->getLocation());

	pair<bool, rg_Point2D> intersectionPoint = find_intersection_point_between_line_segments(branchLine, offSetLine);
	if (intersectionPoint.first == true)
	{
		doesOffsetVtxExist = true;
		offsetVtx = intersectionPoint.second;
	}

	return make_pair(doesOffsetVtxExist, offsetVtx);
}




pair<bool, list<rg_Point2D>> VoronoiPolygonOffset::find_offset_vertex_for_trunk(VEdge2D* trunk, const float& offsetAmount)
{
	pair<const Generator2D*, const Generator2D*> polygonGenerators = find_polygon_generators(trunk);

	const Generator2D* leftGenerator = polygonGenerators.first;
	const Generator2D* rightGenerator = polygonGenerators.second;

	bool doesOffsetVtxExist = false;
	list<rg_Point2D> offsetVertices;

	if (leftGenerator->type() == Generator2D::EDGE_G && rightGenerator->type() == Generator2D::EDGE_G)
	{
		//Line edge
		const EdgeGenerator2D* edgeGenerator = static_cast<const EdgeGenerator2D*>(leftGenerator);
		rg_Line2D directrixOfParabola(edgeGenerator->get_start_vertex_point(), edgeGenerator->get_end_vertex_point());
		rg_Line2D offSetLine = directrixOfParabola.make_parallel_line_to_normal_direction(offsetAmount);
		rg_Line2D trunkLine(trunk->getStartVertex()->getLocation(), trunk->getEndVertex()->getLocation());
		
		pair<bool, rg_Point2D> intersectionPoint = find_intersection_point_between_line_segments(trunkLine, offSetLine);
		if (intersectionPoint.first == true)
		{
			doesOffsetVtxExist = true;
			offsetVertices.push_back(intersectionPoint.second);
		}
	}
	else if (leftGenerator->type() == Generator2D::VERTEX_G && rightGenerator->type() == Generator2D::VERTEX_G)
	{
		//Line-circle intersection
		const VertexGenerator2D* vtxGenerator = static_cast<const VertexGenerator2D*>(leftGenerator);
		rg_Circle2D circle(vtxGenerator->get_point(), offsetAmount);
		rg_Line2D trunkLine(trunk->getStartVertex()->getLocation(), trunk->getEndVertex()->getLocation());

		rg_Point2D* intersectionPoints = new rg_Point2D[2];
		int numIntersection = rg_GeoFunc::compute_intersection_between_circle_and_line_segment(circle, trunkLine, intersectionPoints);
		if (numIntersection > 0)
		{
			doesOffsetVtxExist = true;
			for(int i=0; i<numIntersection; i++)
				offsetVertices.push_back(intersectionPoints[i]);
		}

		delete[] intersectionPoints;
	}
	else
	{
		//Parabolic edge
		offsetVertices = calculate_offset_vertex_for_parabolic_edge(trunk, offsetAmount);
		doesOffsetVtxExist = !offsetVertices.empty();
	}

	return make_pair(doesOffsetVtxExist, offsetVertices);
}



void VoronoiPolygonOffset::manage_offset_vertices_order(list<rg_Point2D>& offsetVertices, VVertex2D* startVtxOfTraverse)
{
	if (offsetVertices.size() > 1)
	{
		if (offsetVertices.size() > 2)
			cout << "Too many offset vertices: " << offsetVertices.size() << endl;

		float distance1 = offsetVertices.front().distance(startVtxOfTraverse->getLocation());
		float distance2 = offsetVertices.back().distance(startVtxOfTraverse->getLocation());
		if (distance2 < distance1)
			offsetVertices.reverse();
	}
}




void VoronoiPolygonOffset::make_offset_edges(vector<Offset>& offsets, const int& IDOfInitialOffset)
{
	for (int i = IDOfInitialOffset; i < offsets.size(); i++)
	{
		Offset& offset = offsets.at(i);
		int edgeSize = 0;
		for (int j = 0; j < offset.get_vertices().size() - 1; j++)
		{
			offset.get_edges().push_back(OffsetEdge(edgeSize++, &offset.get_vertices().at(j), &offset.get_vertices().at(j+1)));
		}
		offset.get_edges().push_back(OffsetEdge(edgeSize++, &offset.get_vertices().at(offset.get_vertices().size()-1), &offset.get_vertices().at(0)));
		

		for (auto& edge : offset.get_edges())
		{
			const Generator2D* generatorForOffsetEdge = find_generator_for_offset_edge(edge);

			if (generatorForOffsetEdge->type() == Generator2D::DISK_G)
			{
				const DiskGenerator2D* diskGenerator = static_cast<const DiskGenerator2D*>(generatorForOffsetEdge);
				Arc2D edgeArc = make_arc_for_offset_edge(diskGenerator, edge.get_start_vertex()->get_coordinate(), edge.get_end_vertex()->get_coordinate(), offset.get_offset_amount());
				edge.set_is_arc_edge(true);
				edge.set_arc(edgeArc);
			}
		}
	}

	connect_offset_edge_to_vertex(offsets, IDOfInitialOffset);
}



void VoronoiPolygonOffset::connect_offset_edge_to_vertex(vector<Offset>& offsets, const int& IDOfInitialOffset)
{
	for (int i = IDOfInitialOffset; i < offsets.size(); i++)
	{
		Offset& offset = offsets.at(i);
		OffsetEdge* lastEdge = &offset.get_edges().at(offset.get_edges().size()-1);
		for (int j = 0; j < offset.get_vertices().size(); j++)
		{
			offset.get_vertices().at(j).set_prev_edge(lastEdge);
			OffsetEdge* currEdge = &offset.get_edges().at(j);
			offset.get_vertices().at(j).set_next_edge(currEdge);
			lastEdge = currEdge;
		}
	}
}




const Generator2D* VoronoiPolygonOffset::find_generator_for_offset_edge(const OffsetEdge& edge)
{
	OffsetVertex* startVtx = edge.get_start_vertex();
	OffsetVertex* endVtx = edge.get_end_vertex();

	if (startVtx != endVtx)
	{
		const Generator2D* commonGenerator = nullptr;
		if (startVtx->get_corr_entity() == endVtx->get_corr_entity())
		{
			const VEdge2D* corrEdge = static_cast<const VEdge2D*>(startVtx->get_corr_entity());
			pair<const Generator2D*, const Generator2D*> generators = find_polygon_generators(corrEdge);
			if (generators.first->type() == Generator2D::DISK_G)
				commonGenerator = generators.first;
			else
				commonGenerator = generators.second;
		}
		else
		{
			const VEdge2D* corrStartEdge = static_cast<const VEdge2D*>(startVtx->get_corr_entity());
			const VEdge2D* corrEndEdge = static_cast<const VEdge2D*>(endVtx->get_corr_entity());

			pair<const Generator2D*, const Generator2D*> generatorsForStartVtx = find_polygon_generators(corrStartEdge);
			pair<const Generator2D*, const Generator2D*> generatorsForEndVtx = find_polygon_generators(corrEndEdge);

			if (generatorsForStartVtx.first == generatorsForEndVtx.first
				|| generatorsForStartVtx.first == generatorsForEndVtx.second)
				commonGenerator = generatorsForStartVtx.first;
			else
				commonGenerator = generatorsForStartVtx.second;
		}
		return commonGenerator;
	}
	else
	{
		//Disk Generator
		const DiskGenerator2D* generator = static_cast<const DiskGenerator2D*>(startVtx->get_corr_entity());
		return generator;
	}	
}




Arc2D VoronoiPolygonOffset::make_arc_for_offset_edge(const DiskGenerator2D* diskGenerator, rg_Point2D startPt, rg_Point2D endPt, double offsetAmount)
{
	rg_Circle2D bigCircle = diskGenerator->getDisk();
	bigCircle.setRadius(bigCircle.getRadius() + offsetAmount);

	Arc2D edgeArc;
	edgeArc.setCircle(bigCircle);

	rg_Point2D center = bigCircle.getCenterPt();
	rg_REAL    radius = bigCircle.getRadius();

	edgeArc.setStartPoint(endPt);
	edgeArc.setEndPoint(startPt);

	return edgeArc;
}



set<DiskGenerator2D*> VoronoiPolygonOffset::find_generators_in_offset_loop(set<const VVertex2D*>& vertexSet)
{
	set<DiskGenerator2D*> generatorsInOffsetLoop;

	set<DiskGenerator2D*> generatorsAdjacentToVertices;
	for (auto vtx : vertexSet)
	{
		list<VFace2D*> incFaces;
		vtx->getIncident3VFaces(incFaces);
		for (auto incFace : incFaces)
		{
			Generator2D* generator = incFace->getGenerator();
			if (generator->type() == Generator2D::DISK_G)
				generatorsAdjacentToVertices.insert(static_cast<DiskGenerator2D*>(generator));
		}
	}

	for (auto generator : generatorsAdjacentToVertices)
	{
		list<VVertex2D*> boundingVertices;
		generator->assigned_face()->getBoundaryVVertices(boundingVertices);
		bool isSurroundedByOffset = true;
		for (auto vtx : boundingVertices)
			if (vertexSet.find(vtx) == vertexSet.end())
			{
				isSurroundedByOffset = false;
				break;
			}

		if (isSurroundedByOffset)
			generatorsInOffsetLoop.insert(generator);
	}

	return generatorsInOffsetLoop;
}



float VoronoiPolygonOffset::calculate_offset_length()
{
	float offsetLength = 0.0f;
	for (auto offset : m_offsets)
	{
		vector<OffsetEdge>& edges = offset.get_edges();
		for (auto& edge : edges)
		{
			if (edge.get_is_arc_edge() == false)
			{
				rg_Point2D startPt = edge.get_start_vertex()->get_coordinate();
				rg_Point2D endPt = edge.get_end_vertex()->get_coordinate();
				offsetLength += startPt.distance(endPt);
			}
			else
			{
				const Arc2D& arc = edge.get_arc();
				float radius = arc.getRadius();
				float angle = arc.angle();
				float length = abs(radius * angle);
				offsetLength += length;
			}
		}
	}
	return offsetLength;
}



void VoronoiPolygonOffset::open_polygon_file()
{
	QString QfilePath = QFileDialog::getOpenFileName(this, tr("Open Polygon File"), NULL, tr("Text file(*.ply)"));
	QFileInfo fileInfo(QfilePath);
	string filePath = translate_to_window_path(QfilePath);
	const char* c_filePath = filePath.c_str();
	cout << "Load Plan - c_filePath: " << c_filePath << endl;

	readData(filePath, m_polygon, m_disks);

	m_scene->pPolygon = &m_polygon;
	m_scene->pDisks = &m_disks;

	m_scene->calculate_center();
	m_scene->draw_polygon();
	m_scene->draw_disks();
}



void VoronoiPolygonOffset::compute_VD()
{
	constructVD_of_disks_in_polygon(m_VD, m_polygon, m_disks);

	cout << "Compute VD complete!" << endl;

	//m_VD.constructVoronoiDiagram(m_polygon);
	connect_VVertices_to_polygonVtx();
	classify_insider_VEdges();

	m_scene->pVD = &m_VD;
	m_scene->pInsiderEdges = &m_insiderVEdges;

	//m_scene->draw_VD();
}



void VoronoiPolygonOffset::compute_offset()
{
	int firstOffsetID = m_offsets.size();
	float offsetAmount = ui.doubleSpinBox_offsetAmount->value();

	make_offset_components(offsetAmount);

	int numOffsetEdges = 0;
	for (auto& offset : m_offsets)
	{
		numOffsetEdges += offset.get_edges().size();
	}

	float offsetLength = calculate_offset_length();

	cout << "Offset amount: " << offsetAmount << ", num OS: " << m_offsets.size() << ", n(OS): " << numOffsetEdges << ", l(OS): " << offsetLength << endl;

	m_scene->pOffsets = &m_offsets;
	m_scene->draw_offsets(firstOffsetID);

	float offsetIncrement = ui.doubleSpinBox_offsetIncrement->value();
	ui.doubleSpinBox_offsetAmount->setValue(offsetAmount + offsetIncrement);
}




bool compare_points_by_x_coord_in_ascending_order(const pair<rg_Point2D, rg_Circle2D*>& lhs, const pair<rg_Point2D, rg_Circle2D*>& rhs)
{
	return lhs.first.getX() < rhs.first.getX();
}



void VoronoiPolygonOffset::compute_horizontal_offsets()
{
	float offsetAmount = ui.doubleSpinBox_offsetAmount->value();
	
	rg_BoundingBox2D box = m_polygon.getBoundingBox();
	float minX = box.getMinPt().getX()+offsetAmount;
	float maxX = box.getMaxPt().getX() - offsetAmount;

	float minY = box.getMinPt().getY() + offsetAmount;
	float maxY = box.getMaxPt().getY() - offsetAmount;
	
	float currY = minY;

	while(currY <= maxY)
	{
		rg_Line2D line(rg_Point2D(minX, currY), rg_Point2D(maxX, currY));

		list<pair<rg_Point2D, rg_Circle2D*>> intersectionPoints;
		set<rg_Circle2D*> disksWithIntersection;

		for (auto& circle : m_disks)
		{
			array<rg_Point2D, 2> intersectionWithCircle;
			rg_Circle2D bigCircle(circle.getCenterPt(), circle.getRadius() + offsetAmount);
			int numIntersection = rg_GeoFunc::compute_intersection_between_circle_and_line_segment(bigCircle, line, intersectionWithCircle.data());
			for (int j = 0; j < numIntersection; j++)
				intersectionPoints.push_back({ intersectionWithCircle.at(j), &circle });

			disksWithIntersection.insert(&circle);
		}
		intersectionPoints.push_back({ rg_Point2D(minX, currY), nullptr });
		intersectionPoints.push_back({ rg_Point2D(maxX, currY), nullptr });

		intersectionPoints.sort(compare_points_by_x_coord_in_ascending_order);

		list<rg_Point2D> offsetVertices;
		for (auto& intersectionPoint : intersectionPoints)
		{
			bool isIncluded = false;
			for (auto& circle : m_disks)
			{
				if (&circle != intersectionPoint.second)
				{
					float distance = intersectionPoint.first.distance(circle.getCenterPt());
					if (distance < circle.getRadius() + offsetAmount)
					{
						isIncluded = true;
						break;
					}
				}
			}

			if (!isIncluded)
				offsetVertices.push_back(intersectionPoint.first);
		}

		//boundary point test
		bool onOffset = false;
		rg_Point2D lastVtx;
		for (auto offsetVtx : offsetVertices)
		{
			if (onOffset)
			{
				m_horizontalOffsets.push_back(rg_Line2D(offsetVtx, lastVtx));
			}
			onOffset = !onOffset;
			lastVtx = offsetVtx;
		}

		currY += offsetAmount;
	}

	m_scene->pHorizontalOffsets = &m_horizontalOffsets;
	m_scene->draw_horizontal_offsets();

	int numLS = m_horizontalOffsets.size();
	float lengthLS = 0.0f;

	for (auto segment : m_horizontalOffsets)
	{
		lengthLS += segment.getLength();
	}
	cout << "Num LS: " << numLS << ", length: " << lengthLS << endl;
}
