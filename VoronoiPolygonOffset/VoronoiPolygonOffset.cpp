#include "VoronoiPolygonOffset.h"
#include "rg_GeoFunc.h"

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



void VoronoiPolygonOffset::make_offset_vertex_for_branch(VEdge2D* branch, const float& offsetAmount, Offset& offset, 
	list<rg_Point2D>& TStack, list<int>& CStack, set<VEdge2D*>& visitedEdges)
{
	if (visitedEdges.count(branch) == 0)
	{
		pair<bool, rg_Point2D> offsetVertex = find_offset_vertex_for_branch(branch, offsetAmount);
		if (offsetVertex.first == true)
			offset.add_offset_vertex(offsetVertex.second, branch);
		visitedEdges.insert(branch);
	}
}



pair<bool, rg_Point2D> VoronoiPolygonOffset::find_offset_vertex_for_branch(VEdge2D* branch, const float& offsetAmount)
{
	VVertex2D* convexVtx = nullptr;
	VVertex2D* oppositeVtx = nullptr;

	if (m_VD.get_location_status_of_vvertex(branch->getStartVertex())
		== PolygonVD2D::VDEntity_Location_Status::ON_POLYGON_BOUNDARY)
	{
		convexVtx = branch->getStartVertex();
		oppositeVtx = branch->getEndVertex();
	}
	else
	{
		convexVtx = branch->getEndVertex();
		oppositeVtx = branch->getStartVertex();
	}

	rg_Point2D polyEdgeDirectionVector;
	for (int i = 0; i < m_mapFromPolygonVtxToVoroVtx.size(); i++)
	{
		if (convexVtx == m_mapFromPolygonVtxToVoroVtx.at(i))
		{
			int indexForNextVtx = i+1;
			if (indexForNextVtx == m_mapFromPolygonVtxToVoroVtx.size())
				indexForNextVtx = 0;

			const VVertex2D* nextVtx = m_mapFromPolygonVtxToVoroVtx.at(indexForNextVtx);
			polyEdgeDirectionVector = nextVtx->getLocation() - convexVtx->getLocation();
			break;
		}
	}

	bool doesOffsetVtxExist = false;
	rg_Point2D offsetVtx;
	rg_Point2D VEdgeDirectionVector = oppositeVtx->getLocation() - convexVtx->getLocation();
	float angle = acos((polyEdgeDirectionVector%VEdgeDirectionVector) / (polyEdgeDirectionVector.magnitude() * VEdgeDirectionVector.magnitude()));
	float requiredDistance = offsetAmount / sin(angle);
	
	if (VEdgeDirectionVector.magnitude() > requiredDistance)
	{
		doesOffsetVtxExist = true;
		offsetVtx = convexVtx->getLocation() + VEdgeDirectionVector.getUnitVector()*requiredDistance;
	}
	return make_pair(doesOffsetVtxExist, offsetVtx);
}



void VoronoiPolygonOffset::make_offset_vertex_for_fluff(VEdge2D* fluff, const float& offsetAmount, Offset& offset, 
	list<rg_Point2D>& TStack, list<int>& CStack, set<VEdge2D*>& visitedEdges)
{
	if (visitedEdges.count(fluff) == 0)
	{
		pair<bool, rg_Point2D> offsetVertex = find_offset_vertex_for_fluff(fluff, offsetAmount);
		if (offsetVertex.first == true)
			offset.add_offset_vertex(offsetVertex.second, fluff);
		visitedEdges.insert(fluff);
	}
}



pair<bool, rg_Point2D> VoronoiPolygonOffset::find_offset_vertex_for_fluff(VEdge2D* fluff, const float& offsetAmount)
{
	VVertex2D* reflexVtx = nullptr;
	VVertex2D* oppositeVtx = nullptr;

	if (m_VD.get_location_status_of_vvertex(fluff->getStartVertex())
		== PolygonVD2D::VDEntity_Location_Status::ON_POLYGON_BOUNDARY)
	{
		reflexVtx = fluff->getStartVertex();
		oppositeVtx = fluff->getEndVertex();
	}
	else
	{
		reflexVtx = fluff->getEndVertex();
		oppositeVtx = fluff->getStartVertex();
	}
		
	bool doesOffsetVtxExist = false;
	rg_Point2D offsetVtx;
	rg_Point2D edgeDirectionVector = oppositeVtx->getLocation() - reflexVtx->getLocation();
	if (edgeDirectionVector.magnitude() > offsetAmount)
	{
		doesOffsetVtxExist = true;
		offsetVtx = reflexVtx->getLocation() + edgeDirectionVector.getUnitVector()*offsetAmount;
	}
	return make_pair(doesOffsetVtxExist, offsetVtx);
}



void VoronoiPolygonOffset::make_offset_vertex_for_trunk(VEdge2D* trunk, const float& offsetAmount, vector<Offset>& offsets,
	list<rg_Point2D>& TStack, list<int>& CStack, set<VEdge2D*>& visitedEdges, TRAVERSE_STATUS& status)
{
	pair<bool, rg_Point2D> offsetVertex = find_offset_vertex_for_trunk(trunk, offsetAmount);
	if (offsetVertex.first == true)
	{
		switch (status)
		{
		case START:
		case CONNECT:
		{
			if (!TStack.empty() && TStack.back().distance(offsetVertex.second) < 1.0E-3)
			{
				status = END;
				offsets.at(CStack.back()).close_offset();
				CStack.pop_back();
			}
			else
			{
				status = SPLIT;
				TStack.push_back(offsetVertex.second);
				offsets.at(CStack.back()).add_offset_vertex(offsetVertex.second, trunk);
				offsets.push_back(Offset(offsets.size(), offsetAmount));
				CStack.push_back(m_offsets.size() - 1);
				cout << "Split" << endl;
			}
		}
		break;
		case END:
		case SPLIT:
		{
			if (TStack.back().distance(offsetVertex.second) < 1.0E-3)
			{
				status = CONNECT;
				CStack.pop_back();
				cout << "Connect" << endl;
			}
			else
			{
				status = START;
				TStack.push_back(offsetVertex.second);
				offsets.at(CStack.back()).add_offset_vertex(offsetVertex.second, trunk);
				cout << "Start" << endl;
			}
		}
		break;
		default:
			cout << "Error case" << endl;
			break;
		}
	}
}



pair<bool, rg_Point2D> VoronoiPolygonOffset::find_offset_vertex_for_trunk(VEdge2D* trunk, const float& offsetAmount)
{
	pair<Generator2D*, Generator2D*> polygonGenerators = find_polygon_generators(trunk);

	Generator2D* leftGenerator = polygonGenerators.first;
	Generator2D* rightGenerator = polygonGenerators.second;

	bool doesOffsetVtxExist = false;
	rg_Point2D offsetVtx;

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
			offsetVtx = intersectionPoint.second;
		}
	}
	else
	{
		//Parabolic edge
		list<rg_Point2D> intersectionPoints = calculate_offset_vertex_for_parabolic_edge(trunk, offsetAmount);
		if (intersectionPoints.size() == 1)
		{
			doesOffsetVtxExist = true;
			offsetVtx = intersectionPoints.front();
		}
		else if (intersectionPoints.size() > 1)
		{
			cout << "Many intersection Points!" << endl;
			doesOffsetVtxExist = true;
			offsetVtx = intersectionPoints.front();
		}
	}

	return make_pair(doesOffsetVtxExist, offsetVtx);
}



void VoronoiPolygonOffset::make_offset_edges(vector<Offset>& offsets)
{
	for (auto& offset : offsets)
	{
		int edgeSize = 0;
		for (int i=0; i<offset.get_vertices().size()-1; i++)
		{
			offset.get_edges().push_back(OffsetEdge(edgeSize++, &offset.get_vertices().at(i), &offset.get_vertices().at(i+1)));
		}
		offset.get_edges().push_back(OffsetEdge(edgeSize++, &offset.get_vertices().back(), &offset.get_vertices().front()));
	}
}



void VoronoiPolygonOffset::connect_offset_edge_to_vertex(vector<Offset>& offsets)
{
	for (auto& offset : offsets)
	{
		int edgeSize = 0;
		for (int i = 0; i < offset.get_vertices().size() - 1; i++)
		{
			offset.get_edges().push_back(OffsetEdge(edgeSize++, &offset.get_vertices().at(i), &offset.get_vertices().at(i + 1)));
		}
		offset.get_edges().push_back(OffsetEdge(edgeSize++, &offset.get_vertices().back(), &offset.get_vertices().front()));
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
	list<rg_Point2D> intersectionPts;
	int numIntersections = 0;
	numIntersections = rg_GeoFunc::compute_intersection_between_parabola_and_line(focusOfParabola, directrixOfParabola, offSetLine, intersectionPts);

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



void VoronoiPolygonOffset::compute_offset()
{
	float offsetAmount = ui.doubleSpinBox_offsetAmount->value();
	cout << "Start to find offset: " << offsetAmount << endl;

	list<rg_Point2D> TStack;
	list<int> CStack;
	set<VEdge2D*> visitedEdges;

	int IDOfInitialOffset = m_offsets.size();
	m_offsets.push_back(Offset(IDOfInitialOffset, offsetAmount));

	//m_offsets.push_back(vector<rg_Point2D>());
	CStack.push_back(IDOfInitialOffset);
	TRAVERSE_STATUS status = START;

	for (auto& insiderEdge : m_traverseList)
	{
		switch (m_mapForVEdgeType.at(insiderEdge))
		{
		case BRANCH:
			make_offset_vertex_for_branch(insiderEdge, offsetAmount, m_offsets.at(CStack.back()), TStack, CStack, visitedEdges);
			break;
		case FLUFF:
			make_offset_vertex_for_fluff(insiderEdge, offsetAmount, m_offsets.at(CStack.back()), TStack, CStack, visitedEdges);
			break;
		case TRUNK:
			make_offset_vertex_for_trunk(insiderEdge, offsetAmount, m_offsets, TStack, CStack, visitedEdges, status);
			break;
		default:
			break;
		}
	}

	m_offsets.at(IDOfInitialOffset).close_offset();

	auto& it = m_offsets.begin();
	while (it != m_offsets.end())
	{
		if ((*it).get_vertices().empty())
			it = m_offsets.erase(it);
		else
			it++;
	}

	cout << "Offset found: " << m_offsets.size() << endl;
	for (int i = 0; i < m_offsets.size(); i++)
	{
		cout << "Offset[" << i << "] : " << m_offsets.at(i).get_vertices().size() << endl;
	}

	m_scene->pOffsets = &m_offsets;
	m_scene->draw_offsets();

	ui.doubleSpinBox_offsetAmount->setValue(offsetAmount + 10);
}



void VoronoiPolygonOffset::open_polygon_file()
{
	QString QfilePath = QFileDialog::getOpenFileName(this, tr("Open Polygon File"), NULL, tr("Text file(*.ply)"));
	QFileInfo fileInfo(QfilePath);
	string filePath = translate_to_window_path(QfilePath);
	const char* c_filePath = filePath.c_str();
	cout << "Load LV - c_filePath: " << c_filePath << endl;

	m_polygon.readPolygonFile(c_filePath);
	m_scene->pPolygon = &m_polygon;
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
