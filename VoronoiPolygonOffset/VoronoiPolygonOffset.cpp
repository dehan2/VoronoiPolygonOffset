#include "VoronoiPolygonOffset.h"
#include "rg_GeoFunc.h"
#include "rg_IntersectFunc.h"

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
			insiderVEdges.insert(*it++);

	for (auto& VVtx : m_mapFromPolygonVtxToVoroVtx)
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
	cout << "Trunks: " << trunkCounter << endl;
}



void VoronoiPolygonOffset::make_traverse_list()
{
	const VVertex2D* startVtx = m_mapFromPolygonVtxToVoroVtx.front();
	
	map<VEdge2D*, int> traverseCounter;
	pair<VEdge2D*, const VVertex2D*> startEdgePair = find_start_edge_pair();
	
	VEdge2D* currEdge = startEdgePair.first;
	const VVertex2D* lastVtx = startEdgePair.second;
	traverseCounter[currEdge] = 1;
	m_traverseList.push_back(currEdge);

	VEdge2D* nextEdge = nullptr;
	VVertex2D* nextVtx = nullptr;
	if (currEdge->getStartVertex() == lastVtx)
	{
		nextEdge = currEdge->getRightHand();
		nextVtx = currEdge->getEndVertex();
	}
	else
	{
		nextEdge = currEdge->getLeftLeg();
		nextVtx = currEdge->getStartVertex();
	}


	while (nextEdge != startEdgePair.first)
	{
		currEdge = nextEdge;
		lastVtx = nextVtx;

		if (m_mapForVEdgeType.at(currEdge) == TRUNK)
		{
			if (currEdge->getStartVertex() == lastVtx)
			{
				nextEdge = currEdge->getRightHand();
				nextVtx = currEdge->getEndVertex();
			}
			else
			{
				nextEdge = currEdge->getLeftLeg();
				nextVtx = currEdge->getStartVertex();
			}
		}
		else if(traverseCounter.count(currEdge) == 0)
		{
			nextEdge = currEdge;
			if (currEdge->getStartVertex() == lastVtx)
				nextVtx = currEdge->getEndVertex();
			else
				nextVtx = currEdge->getStartVertex();

			traverseCounter[nextEdge] = 1;
		}
		else // Returned case
		{
			if (currEdge->getStartVertex() == lastVtx)
			{
				nextEdge = currEdge->getRightHand();
				nextVtx = currEdge->getEndVertex();
			}
			else
			{
				nextEdge = currEdge->getLeftLeg();
				nextVtx = currEdge->getStartVertex();
			}
		}
		m_traverseList.push_back(currEdge);
		if (traverseCounter.count(currEdge) == 0)
			traverseCounter[currEdge] = 1;
		else
			traverseCounter.at(currEdge)++;
	}

	m_traverseList.push_back(startEdgePair.first);
}



pair<VEdge2D*, const VVertex2D*> VoronoiPolygonOffset::find_start_edge_pair()
{
	//Start from branch edge
	int currVtxIndex = 0;
	VEdge2D* startEdge = nullptr;
	const VVertex2D* startVtx = nullptr;
	while (startEdge == nullptr)
	{
		startVtx = m_mapFromPolygonVtxToVoroVtx.at(currVtxIndex);
		list<VEdge2D*> incEdges;
		startVtx->getIncident3VEdges(incEdges);
		auto& it = incEdges.begin();
		while (it != incEdges.end())
			if (m_VD.get_location_status_of_edge(*it)
				== PolygonVD2D::VDEntity_Location_Status::OUTSIDE_POLYGON)
				it = incEdges.erase(it);
			else
				it++;

		if (incEdges.size() == 1)
			startEdge = incEdges.front();
		else
			currVtxIndex++;
	}

	return make_pair(startEdge, startVtx);
}



void VoronoiPolygonOffset::make_offset_vertices(const float& offsetAmount)
{
	list<rg_Point2D> TStack;
	list<int> CStack;
	set<VEdge2D*> visitedEdges;

	int IDOfInitialOffset = m_offsets.size();
	m_offsets.push_back(Offset(IDOfInitialOffset, offsetAmount));

	//m_offsets.push_back(vector<rg_Point2D>());
	CStack.push_back(IDOfInitialOffset);
	TRAVERSE_STATUS status = INITIALIZE;

	VEdge2D* lastEdge = m_traverseList.back();
	for (auto& insiderEdge : m_traverseList)
	{
		VVertex2D* startVtxOfTraverse = find_start_vertex_of_traverse(insiderEdge, lastEdge);
		switch (m_mapForVEdgeType.at(insiderEdge))
		{
		case BRANCH:
			make_offset_vertex_for_branch(insiderEdge, offsetAmount, m_offsets.at(CStack.back()), TStack, CStack, visitedEdges, status);
			break;
		case FLUFF:
			make_offset_vertex_for_fluff(insiderEdge, offsetAmount, m_offsets.at(CStack.back()), TStack, CStack, visitedEdges, status);
			break;
		case TRUNK:
			make_offset_vertex_for_trunk(insiderEdge, offsetAmount, m_offsets, TStack, CStack, visitedEdges, status, startVtxOfTraverse);
			break;
		default:
			break;
		}
		lastEdge = insiderEdge;

		if (CStack.empty())
		{
			int IDOfNewChain = m_offsets.size();
			m_offsets.push_back(Offset(IDOfNewChain, offsetAmount));
			CStack.push_back(IDOfNewChain);
			status = INITIALIZE;
		}
	}
}



VVertex2D* VoronoiPolygonOffset::find_start_vertex_of_traverse(VEdge2D* currEdge, VEdge2D* lastEdge)
{
	VVertex2D* commonVtx = nullptr;
	if (currEdge->getStartVertex() == lastEdge->getStartVertex())
		commonVtx = currEdge->getStartVertex();
	else if (currEdge->getStartVertex() == lastEdge->getEndVertex())
		commonVtx = currEdge->getStartVertex();
	else if (currEdge->getEndVertex() == lastEdge->getStartVertex())
		commonVtx = currEdge->getEndVertex();
	else if (currEdge->getEndVertex() == lastEdge->getEndVertex())
		commonVtx = currEdge->getEndVertex();

	return commonVtx;
}



void VoronoiPolygonOffset::make_offset_vertex_for_branch(VEdge2D* branch, const float& offsetAmount, Offset& offset, 
	list<rg_Point2D>& TStack, list<int>& CStack, set<VEdge2D*>& visitedEdges, TRAVERSE_STATUS& status)
{
	if (visitedEdges.count(branch) == 0)
	{
		pair<bool, rg_Point2D> offsetVertex = find_offset_vertex_for_branch(branch, offsetAmount);
		if (offsetVertex.first == true)
		{
			offset.add_offset_vertex(offsetVertex.second, branch);
			if (status == INITIALIZE)
				status = START;
		}
			
		visitedEdges.insert(branch);
	}
}



pair<bool, rg_Point2D> VoronoiPolygonOffset::find_offset_vertex_for_branch(VEdge2D* branch, const float& offsetAmount)
{
	pair<Generator2D*, Generator2D*> polygonGenerators = find_polygon_generators(branch);

	Generator2D* leftGenerator = polygonGenerators.first;
	Generator2D* rightGenerator = polygonGenerators.second;

	bool doesOffsetVtxExist = false;
	rg_Point2D offsetVtx;

	//Line edge
	EdgeGenerator2D* edgeGenerator = static_cast<EdgeGenerator2D*>(leftGenerator);
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



void VoronoiPolygonOffset::make_offset_vertex_for_fluff(VEdge2D* fluff, const float& offsetAmount, Offset& offset, 
	list<rg_Point2D>& TStack, list<int>& CStack, set<VEdge2D*>& visitedEdges, TRAVERSE_STATUS& status)
{
	if (visitedEdges.count(fluff) == 0)
	{
		pair<bool, rg_Point2D> offsetVertex = find_offset_vertex_for_fluff(fluff, offsetAmount);
		if (offsetVertex.first == true)
		{
			offset.add_offset_vertex(offsetVertex.second, fluff);
			if (status == INITIALIZE)
				status = START;
		}
		visitedEdges.insert(fluff);
	}
}



pair<bool, rg_Point2D> VoronoiPolygonOffset::find_offset_vertex_for_fluff(VEdge2D* fluff, const float& offsetAmount)
{
	VVertex2D* reflexVtx = nullptr;

	if (m_VD.get_location_status_of_vvertex(fluff->getStartVertex())
		== PolygonVD2D::VDEntity_Location_Status::ON_POLYGON_BOUNDARY)
	{
		reflexVtx = fluff->getStartVertex();
	}
	else
	{
		reflexVtx = fluff->getEndVertex();
	}
		
	bool doesOffsetVtxExist = false;
	rg_Point2D offsetVtx;

	//Line-circle intersection
	rg_Circle2D circle(reflexVtx->getLocation(), offsetAmount);
	rg_Line2D fluffLine(fluff->getStartVertex()->getLocation(), fluff->getEndVertex()->getLocation());

	rg_Point2D* intersectionPoints = new rg_Point2D[2];
	int numIntersection = rg_GeoFunc::compute_intersection_between_circle_and_line_segment(circle, fluffLine, intersectionPoints);
	if (numIntersection > 0)
	{
		doesOffsetVtxExist = true;
		offsetVtx = intersectionPoints[0];
	}

	delete[] intersectionPoints;
	return make_pair(doesOffsetVtxExist, offsetVtx);
}



void VoronoiPolygonOffset::make_offset_vertex_for_trunk(VEdge2D* trunk, const float& offsetAmount, vector<Offset>& offsets,
	list<rg_Point2D>& TStack, list<int>& CStack, set<VEdge2D*>& visitedEdges, TRAVERSE_STATUS& status, VVertex2D* startVtxOfTraverse)
{
	pair<bool, list<rg_Point2D>> offsetVerticesInfo = find_offset_vertex_for_trunk(trunk, offsetAmount);
	if (offsetVerticesInfo.first == true)
	{
/*
		cout << "Curr trunk: " << trunk->getID() << " - vtx: "<<offsetVerticesInfo.second.size()<<endl;

		if (trunk == firstTrunk)
			cout << "Trunk again" << endl;
		if (firstTrunk == nullptr)
			firstTrunk = trunk;*/

		list<rg_Point2D>& offsetVertices = offsetVerticesInfo.second;
		manage_offset_vertices_order(offsetVertices, startVtxOfTraverse);
		for (auto& offsetVtx : offsetVertices)
		{
			process_offset_vertex_for_trunk(offsetVtx, trunk, offsetAmount, offsets, TStack, CStack, visitedEdges, status);
		}
	}
}



pair<bool, list<rg_Point2D>> VoronoiPolygonOffset::find_offset_vertex_for_trunk(VEdge2D* trunk, const float& offsetAmount)
{
	pair<Generator2D*, Generator2D*> polygonGenerators = find_polygon_generators(trunk);

	Generator2D* leftGenerator = polygonGenerators.first;
	Generator2D* rightGenerator = polygonGenerators.second;

	bool doesOffsetVtxExist = false;
	list<rg_Point2D> offsetVertices;

	if (leftGenerator->type() == Generator2D::EDGE_G && rightGenerator->type() == Generator2D::EDGE_G)
	{
		//Line edge
		EdgeGenerator2D* edgeGenerator = static_cast<EdgeGenerator2D*>(leftGenerator);
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
		VertexGenerator2D* vtxGenerator = static_cast<VertexGenerator2D*>(leftGenerator);
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



void VoronoiPolygonOffset::process_offset_vertex_for_trunk(const rg_Point2D& offsetVtx, VEdge2D* trunk, const float& offsetAmount, vector<Offset>& offsets, 
	list<rg_Point2D>& TStack, list<int>& CStack, set<VEdge2D*>& visitedEdges, TRAVERSE_STATUS& status)
{
	if (status != INITIALIZE)
	{
		if (!TStack.empty())
		{
			float distanceToTop = TStack.back().distance(offsetVtx);
			cout << "TstackSize: "<<TStack.size()<<", distance: " << distanceToTop << endl;
		}

		switch (status)
		{
		case START:
		case CONNECT:
		{
			if (!TStack.empty() && TStack.back().distance(offsetVtx) < 1.0)
			{
				status = END;
				offsets.at(CStack.back()).close_offset();
				CStack.pop_back();
				TStack.pop_back();
				cout << "End" << endl;
			}
			else
			{
				status = SPLIT;
				TStack.push_back(offsetVtx);
				offsets.at(CStack.back()).add_offset_vertex(offsetVtx, trunk);
				cout << "Split" << endl;
			}
		}
		break;
		case END:
		case SPLIT:
		{
			if (TStack.back().distance(offsetVtx) < 1.0)
			{
				status = CONNECT;
				//CStack.pop_back();
				TStack.pop_back();
				cout << "Connect" << endl;
			}
			else
			{
				status = START;
				TStack.push_back(offsetVtx);
				offsets.push_back(Offset(offsets.size(), offsetAmount));
				CStack.push_back(m_offsets.size() - 1);
				offsets.at(CStack.back()).add_offset_vertex(offsetVtx, trunk);
				cout << "Start" << endl;
			}
		}
		break;
		default:
			cout << "Error case" << endl;
			break;
		}
	}
	else
	{
		status = START;
		TStack.push_back(offsetVtx);
		offsets.at(CStack.back()).add_offset_vertex(offsetVtx, trunk);
		cout << "Start" << endl;
	}
}



void VoronoiPolygonOffset::remove_empty_offsets()
{
	auto& it = m_offsets.begin();
	while (it != m_offsets.end())
	{
		if ((*it).get_vertices().empty())
			it = m_offsets.erase(it);
		else
			it++;
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
	}
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



void VoronoiPolygonOffset::find_arc_edges(vector<Offset>& offsets, const int& IDOfInitialOffset)
{
	for (int i = IDOfInitialOffset; i < offsets.size(); i++)
	{
		Offset& offset = offsets.at(i);
		for (int j = 0; j < offset.get_edges().size(); j++)
		{
			OffsetEdge* currEdge = &offset.get_edges().at(j);
			
			//Case 1: edge between fluffs
			//Case 2: 
			
			
			
			
			
			
			
			pair<Generator2D*, Generator2D*> polygonGenerators = find_polygon_generators(currEdge->get_start_vertex()->get_corr_VEdge());
			VertexGenerator2D* vertexGenerator = nullptr;
			if (polygonGenerators.first->type() == Generator2D::VERTEX_G)
				vertexGenerator = static_cast<VertexGenerator2D*>(polygonGenerators.first);
			else if(polygonGenerators.second->type() == Generator2D::VERTEX_G)
				vertexGenerator = static_cast<VertexGenerator2D*>(polygonGenerators.second);
			
			if (vertexGenerator != nullptr)
			{
				currEdge->set_is_arc_edge(true);
				currEdge->set_reflex_vertex(vertexGenerator);
			}
		}
	}
}



list<rg_Point2D> VoronoiPolygonOffset::calculate_offset_vertex_for_parabolic_edge(VEdge2D* trunk, const float& offsetAmount)
{
	pair<Generator2D*, Generator2D*> polygonGenerators = find_polygon_generators(trunk);

	Generator2D* leftGenerator = polygonGenerators.first;
	Generator2D* rightGenerator = polygonGenerators.second;

	//list<rg_Point2D> intersectionPts;
	//if (leftGenerator->type() != Generator2D::VERTEX_G && leftGenerator->type() == Generator2D::EDGE_G)
	//	return intersectionPts;

	//left G -> vtx, right G -> edge
	if (leftGenerator->type() == Generator2D::EDGE_G)
		swap(leftGenerator, rightGenerator);

	// Testing intersection between parabola and line

	EdgeGenerator2D* edgeGenerator = static_cast<EdgeGenerator2D*>(rightGenerator);
	rg_Line2D directrixOfParabola(edgeGenerator->get_start_vertex_point(), edgeGenerator->get_end_vertex_point());
	VertexGenerator2D* vertexGenerator = static_cast<VertexGenerator2D*>(leftGenerator);
	rg_Point2D focusOfParabola = vertexGenerator->get_point();

	rg_Line2D offSetLine = directrixOfParabola.make_parallel_line_to_normal_direction(offsetAmount);
	rg_RQBzCurve2D parabola = m_VD.get_geometry_of_edge(trunk);
	list<rg_Point2D> intersectionPts;
	int numIntersections = 0;
	//numIntersections = rg_GeoFunc::compute_intersection_between_parabola_and_line(focusOfParabola, directrixOfParabola, offSetLine, intersectionPts);
	numIntersections = rg_IntersectFunc::intersectRQBzCurveVsLine(parabola, offSetLine, intersectionPts);

	return intersectionPts;
}



pair<Generator2D*, Generator2D*> VoronoiPolygonOffset::find_polygon_generators(VEdge2D* edge)
{
	Generator2D* leftGenerator = static_cast<Generator2D*>(edge->getLeftFace()->getGenerator()->user_data());
	Generator2D* rightGenerator = static_cast<Generator2D*>(edge->getRightFace()->getGenerator()->user_data());
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



void VoronoiPolygonOffset::make_map_from_edge_to_offset(vector<Offset>& offsets, map<VEdge2D*, vector<Offset*>>& mapFromEdgeToOffset)
{
	for (auto& offset : m_offsets)
	{
		for (auto& vtx : offset.get_vertices())
		{
			VEdge2D* corrEdge = vtx.get_corr_VEdge();
			if (mapFromEdgeToOffset.count(corrEdge) == 0)
				mapFromEdgeToOffset[corrEdge] = vector<Offset*>();

			mapFromEdgeToOffset.at(corrEdge).push_back(&offset);
		}
	}
}




VEdge2D* VoronoiPolygonOffset::find_edge_to_start_search(const rg_Point2D& currPosition, set<Offset*>& offsetToVisit)
{
	VEdge2D* edgeToStartSearch = nullptr;
	float minDistance = FLT_MAX;
	for (auto& offset : offsetToVisit)
	{
		for (auto& vtx : offset->get_vertices())
		{
			float distance = currPosition.distance(vtx.get_coordinate());
			if (distance < minDistance)
			{
				minDistance = distance;
				edgeToStartSearch = vtx.get_corr_VEdge();
			}
		}
	}

	return edgeToStartSearch;
}



rg_Point2D VoronoiPolygonOffset::calculate_drone_start_position()
{
	rg_BoundingBox2D boundingBox = calculate_bounding_box();

	float droneX = boundingBox.getCenterPt().getX();
	float droneY = boundingBox.getMinPt().getY();
	rg_Point2D startPosition(droneX, droneY);
	return startPosition;
}

rg_BoundingBox2D VoronoiPolygonOffset::calculate_bounding_box()
{
	rg_BoundingBox2D boundingBox;
	auto& polygonVertices = m_reader.get_polygon();
	for (auto& vtx : polygonVertices)
		boundingBox.contain(rg_Point2D(vtx.at(0), vtx.at(1)));

	return boundingBox;
}


array<float, 3> VoronoiPolygonOffset::calculate_flight_speeds()
{
	float leftTravelDistance = 0;
	float centerTravelDistance = 0;
	float rightTravelDistance = 0;

	rg_Point2D lastPoint = m_horizontalSearchPath.back();
	for (auto& vtx : m_horizontalSearchPath)
	{
		leftTravelDistance += lastPoint.distance(vtx);
		lastPoint = vtx;
	}

	lastPoint = m_searchPath.back();
	for (auto& vtx : m_searchPath)
	{
		centerTravelDistance += lastPoint.distance(vtx);
		lastPoint = vtx;
	}

	lastPoint = m_verticalSearchPath.back();
	for (auto& vtx : m_verticalSearchPath)
	{
		rightTravelDistance += lastPoint.distance(vtx);
		lastPoint = vtx;
	}

	array<float, 3> flightSpeeds = { 0, 0, 0 };
	flightSpeeds.at(0) = leftTravelDistance / TARGET_FLIGHT_TIME;
	flightSpeeds.at(1) = centerTravelDistance / TARGET_FLIGHT_TIME;
	flightSpeeds.at(2) = rightTravelDistance / TARGET_FLIGHT_TIME;

	cout << "Left distance: " << leftTravelDistance << ", speed: " << flightSpeeds.at(0) << endl;
	cout << "Center distance: " << centerTravelDistance << ", speed: " << flightSpeeds.at(1) << endl;
	cout << "Right distance: " << rightTravelDistance << ", speed: " << flightSpeeds.at(2) << endl;

	return flightSpeeds;
}



void VoronoiPolygonOffset::write_total_plan(const string& planFileName)
{
	m_totalWriter.set_home(m_reader.get_home_lat(), m_reader.get_home_lon());

	rg_BoundingBox2D boundingBox = calculate_bounding_box();
	float margin = 5;
	rg_Point2D geoFenceDLVtx(boundingBox.getMinPt().getX() - margin, boundingBox.getMinPt().getY() - margin);
	rg_Point2D geoFenceURVtx(boundingBox.getMaxPt().getX() + margin, boundingBox.getMaxPt().getY() + margin);

	array<float, 4> params = { 0,0,0,0 };
	array<float, 2> geoFenceDLGps = m_totalWriter.translate_coord_to_gps(geoFenceDLVtx.getX(), geoFenceDLVtx.getY());
	array<float, 2> geoFenceURGps = m_totalWriter.translate_coord_to_gps(geoFenceURVtx.getX(), geoFenceURVtx.getY());
	m_totalWriter.set_geoFence(geoFenceDLGps, geoFenceURGps);

	rg_Point2D launchCoord = m_horizontalSearchPath.front();
	array<float, 2> launchGps = m_totalWriter.translate_coord_to_gps(launchCoord.getX(), launchCoord.getY());

	m_totalWriter.set_launchPoint(launchGps.at(0), launchGps.at(1));//drone start point

	m_totalWriter.set_frame(3);
	m_totalWriter.set_altitude(5);
	m_totalWriter.set_command(16);
	m_totalWriter.set_params(params);
	m_totalWriter.set_hoverspeed(1);
	//plan1.set_hoverspeed(5);

	vector<array<float, 2>> wayPoints;
	{
		for (auto& vtx : m_horizontalSearchPath)
			wayPoints.push_back({ (float)vtx.getX(), (float)vtx.getY() });
		wayPoints.push_back({ (float)m_horizontalSearchPath.front().getX(), (float)m_horizontalSearchPath.front().getY() });

		for (auto& vtx : m_searchPath)
			wayPoints.push_back({ (float)vtx.getX(), (float)vtx.getY() });
		wayPoints.push_back({ (float)m_searchPath.front().getX(), (float)m_searchPath.front().getY() });

		for (auto& vtx : m_verticalSearchPath)
			wayPoints.push_back({ (float)vtx.getX(), (float)vtx.getY() });
		wayPoints.push_back({ (float)m_verticalSearchPath.front().getX(), (float)m_verticalSearchPath.front().getY() });
	}

	m_totalWriter.set_polygons(wayPoints);
	m_totalWriter.writePlanFile(planFileName);
}



void VoronoiPolygonOffset::write_individual_plan(PlanJsonWriter& writer, vector<rg_Point2D>& searchPath, const float& altitude, const float& speed, const string& planFileName)
{
	writer.set_home(m_reader.get_home_lat(), m_reader.get_home_lon());

	rg_BoundingBox2D boundingBox = calculate_bounding_box();
	float margin = 5;
	rg_Point2D geoFenceDLVtx(boundingBox.getMinPt().getX() - margin, boundingBox.getMinPt().getY() - margin);
	rg_Point2D geoFenceURVtx(boundingBox.getMaxPt().getX() + margin, boundingBox.getMaxPt().getY() + margin);

	array<float, 4> params = { 0,0,0,0 };
	array<float, 2> geoFenceDLGps = writer.translate_coord_to_gps(geoFenceDLVtx.getX(), geoFenceDLVtx.getY());
	array<float, 2> geoFenceURGps = writer.translate_coord_to_gps(geoFenceURVtx.getX(), geoFenceURVtx.getY());
	writer.set_geoFence(geoFenceDLGps, geoFenceURGps);

	rg_Point2D launchCoord = searchPath.front();
	array<float, 2> launchGps = writer.translate_coord_to_gps(launchCoord.getX(), launchCoord.getY());

	writer.set_launchPoint(launchGps.at(0), launchGps.at(1));//drone start point

	writer.set_frame(3);
	writer.set_altitude(altitude);
	writer.set_command(16);
	writer.set_params(params);
	writer.set_hoverspeed(speed);

	vector<array<float, 2>> wayPoints;
	{
		for (auto& vtx : searchPath)
			wayPoints.push_back({ (float)vtx.getX(), (float)vtx.getY() });
		wayPoints.push_back({ (float)searchPath.front().getX(), (float)searchPath.front().getY() });
	}

	writer.set_polygons(wayPoints);
	writer.writePlanFile(planFileName);
}



void VoronoiPolygonOffset::compute_offset()
{
	float offsetAmount = ui.doubleSpinBox_offsetAmount->value();
	float offsetIncrement = ui.doubleSpinBox_offsetIncrement->value();

	bool doesNewOffsetFound = true;
	int IDOfInitialOffset = 0;
	while (doesNewOffsetFound)
	{
		cout << "Start to find offset: " << offsetAmount << endl;

		IDOfInitialOffset = m_offsets.size();
		make_offset_vertices(offsetAmount);
		remove_empty_offsets();

		if (m_offsets.size() > IDOfInitialOffset)
		{
			make_offset_edges(m_offsets, IDOfInitialOffset);
			connect_offset_edge_to_vertex(m_offsets, IDOfInitialOffset);
			offsetAmount += offsetIncrement;
		}
		else
		{
			doesNewOffsetFound = false;
		}
	}

	
	//find_arc_edges(m_offsets, IDOfInitialOffset);

	cout << "Offset found: " << m_offsets.size() << endl;
	for (int i = 0; i < m_offsets.size(); i++)
	{
		cout << "Offset[" << i << "] : " << m_offsets.at(i).get_vertices().size() << endl;
	}

	m_scene->pOffsets = &m_offsets;
	m_scene->draw_offsets(0);

	//ui.doubleSpinBox_offsetAmount->setValue(offsetAmount + 10);
}



void VoronoiPolygonOffset::compute_search_path()
{
	rg_BoundingBox2D boundingBox;
	auto& polygonVertices = m_reader.get_polygon();
	for (auto& vtx : polygonVertices)
		boundingBox.contain(rg_Point2D(vtx.at(0), vtx.at(1)));

	float droneX = boundingBox.getCenterPt().getX();
	float droneY = boundingBox.getMinPt().getY();
	rg_Point2D startPosition(droneX, droneY);
	m_searchPath.push_back(startPosition);

	map<VEdge2D*, vector<Offset*>> mapFromEdgeToOffset;
	make_map_from_edge_to_offset(m_offsets, mapFromEdgeToOffset);

	set<Offset*> offsetToVisit;
	for (auto& offset : m_offsets)
		offsetToVisit.insert(&offset);

	while (!offsetToVisit.empty())
	{
		VEdge2D* edgeToStartSearch = find_edge_to_start_search(m_searchPath.back(), offsetToVisit);

		vector<Offset*>& connectedOffsets = mapFromEdgeToOffset.at(edgeToStartSearch);

		for (auto& offset : connectedOffsets)
		{
			int indexOfClosestVtx = -1;
			vector<OffsetVertex>& vertices = offset->get_vertices();
			for (int i = 0; i < vertices.size(); i++)
			{
				if(vertices.at(i).get_corr_VEdge() == edgeToStartSearch)
					indexOfClosestVtx = i;
			}

			for (int i = 0; i < vertices.size(); i++)
			{
				int indexOfVtx = i + indexOfClosestVtx;
				if (indexOfVtx >= vertices.size())
					indexOfVtx -= vertices.size();

				OffsetVertex& vtx = vertices.at(indexOfVtx);
				m_searchPath.push_back(vtx.get_coordinate());
			}
			m_searchPath.push_back(vertices.at(indexOfClosestVtx).get_coordinate());
			offsetToVisit.erase(offset);
		}
	}

	m_scene->pSearchPath = &m_searchPath;
	m_scene->draw_search_path();
}


bool comparePointsByXIncrement(const rg_Point2D& lhs, const rg_Point2D& rhs)
{
	return lhs.getX() < rhs.getX();
}

bool comparePointsByYIncrement(const rg_Point2D& lhs, const rg_Point2D& rhs)
{
	return lhs.getY() < rhs.getY();
}


void VoronoiPolygonOffset::compute_horizontal_search_path()
{
	rg_BoundingBox2D boundingBox;
	auto& polygonVertices = m_reader.get_splittedPolygon().at(0);
	for (auto& vtx : polygonVertices)
		boundingBox.contain(rg_Point2D(vtx.at(0), vtx.at(1)));

	list<rg_Line2D> polygonLines;
	auto& lastPt = m_reader.get_splittedPolygon().at(0).back();
	rg_Point2D lastVtx(lastPt.at(0), lastPt.at(1));
	for (auto& pt : m_reader.get_splittedPolygon().at(0))
	{
		rg_Point2D currVtx(pt.at(0), pt.at(1));
		polygonLines.push_back(rg_Line2D(lastVtx, currVtx));
		lastVtx = currVtx;
	}
	
	rg_Point2D startPosition = calculate_drone_start_position();
	startPosition.setX(startPosition.getX() - 5);

	m_horizontalSearchPath.push_back(startPosition);
	bool goLeft = true;
	float amount = boundingBox.getMinPt().getY() + ui.doubleSpinBox_offsetAmount->value();
	float increment = ui.doubleSpinBox_offsetIncrement->value();
	float maxAmount = boundingBox.getMaxPt().getY();

	while(amount < maxAmount)
	{
		rg_Point2D searchLineLeftPt(boundingBox.getMinPt().getX() - 10, amount);
		rg_Point2D searchLineRightPt(boundingBox.getMaxPt().getX() + 10, amount);
		rg_Line2D searchLine(searchLineLeftPt, searchLineRightPt);
		
		list<rg_Point2D> intersectionPoints;
		for (auto& polygonLine : polygonLines)
		{
			rg_Point2D intersectionPoint; 
			bool doesIntersect = rg_GeoFunc::compute_intersection_between_two_line_segments(searchLine, polygonLine, intersectionPoint);
			if (doesIntersect)
				intersectionPoints.push_back(intersectionPoint);
		}

		intersectionPoints.sort(comparePointsByXIncrement);
		if (goLeft)
			intersectionPoints.reverse();

		for (auto& intersectionPoint : intersectionPoints)
		{
			m_horizontalSearchPath.push_back(intersectionPoint);
		}

		goLeft = !goLeft;
		amount += increment;
	}

	m_scene->pHorizontalSearchPath = &m_horizontalSearchPath;
	m_scene->draw_horizontal_search_path();
}



void VoronoiPolygonOffset::compute_vertical_search_path()
{
	rg_BoundingBox2D boundingBox;
	auto& polygonVertices = m_reader.get_splittedPolygon().at(2);
	for (auto& vtx : polygonVertices)
		boundingBox.contain(rg_Point2D(vtx.at(0), vtx.at(1)));

	list<rg_Line2D> polygonLines;
	auto& lastPt = m_reader.get_splittedPolygon().at(2).back();
	rg_Point2D lastVtx(lastPt.at(0), lastPt.at(1));
	for (auto& pt : m_reader.get_splittedPolygon().at(2))
	{
		rg_Point2D currVtx(pt.at(0), pt.at(1));
		polygonLines.push_back(rg_Line2D(lastVtx, currVtx));
		lastVtx = currVtx;
	}

	rg_Point2D startPosition = calculate_drone_start_position();
	startPosition.setX(startPosition.getX() + 5);

	m_verticalSearchPath.push_back(startPosition);
	bool goDown = false;
	float amount = boundingBox.getMinPt().getX() + ui.doubleSpinBox_offsetAmount->value();
	float increment = ui.doubleSpinBox_offsetIncrement->value();
	float maxAmount = boundingBox.getMaxPt().getX();

	while (amount < maxAmount)
	{
		rg_Point2D searchLineBottomPt(amount, boundingBox.getMinPt().getY() - 10);
		rg_Point2D searchLineCeilingPt(amount, boundingBox.getMaxPt().getY() + 10);
		rg_Line2D searchLine(searchLineBottomPt, searchLineCeilingPt);

		list<rg_Point2D> intersectionPoints;
		for (auto& polygonLine : polygonLines)
		{
			rg_Point2D intersectionPoint;
			bool doesIntersect = rg_GeoFunc::compute_intersection_between_two_line_segments(searchLine, polygonLine, intersectionPoint);
			if (doesIntersect)
				intersectionPoints.push_back(intersectionPoint);
		}

		intersectionPoints.sort(comparePointsByYIncrement);
		if (goDown)
			intersectionPoints.reverse();

		for (auto& intersectionPoint : intersectionPoints)
		{
			m_verticalSearchPath.push_back(intersectionPoint);
		}

		goDown = !goDown;
		amount += increment;
	}

	m_scene->pVerticalSearchPath = &m_verticalSearchPath;
	m_scene->draw_vertical_search_path();
}



void VoronoiPolygonOffset::output_plans()
{
	array<float, 3> flightSpeeds = calculate_flight_speeds();
	write_total_plan("flightPlan_total.plan");
	write_individual_plan(m_leftWriter, m_horizontalSearchPath, 5, flightSpeeds.at(0), "flightPlan_left.plan");
	write_individual_plan(m_centerWriter, m_searchPath, 5, flightSpeeds.at(1), "flightPlan_center.plan");
	write_individual_plan(m_rightWriter, m_verticalSearchPath, 5, flightSpeeds.at(2), "flightPlan_right.plan");
}



void VoronoiPolygonOffset::open_polygon_file()
{
	QString QfilePath = QFileDialog::getOpenFileName(this, tr("Open Polygon File"), NULL, tr("Text file(*.plan)"));
	QFileInfo fileInfo(QfilePath);
	string filePath = translate_to_window_path(QfilePath);
	const char* c_filePath = filePath.c_str();
	cout << "Load Plan - c_filePath: " << c_filePath << endl;

	m_reader.read_plan_file(c_filePath);
	m_polygon.load_polygon_vertices(m_reader.get_splittedPolygon().at(1));

	m_scene->pPolygon = &m_polygon;
	m_scene->pTotalPolygon = &m_reader.get_polygon();
	m_scene->pLeftPolygon = &m_reader.get_splittedPolygon().at(0);
	m_scene->pRightPolygon = &m_reader.get_splittedPolygon().at(2);

	m_scene->calculate_center();
	m_scene->draw_polygon();
}



void VoronoiPolygonOffset::compute_VD()
{
	m_VD.constructVoronoiDiagram(m_polygon);
	connect_VVertices_to_polygonVtx();
	classify_insider_VEdges();
	make_traverse_list();

	m_scene->pVD = &m_VD;
	m_scene->draw_VD();
}
