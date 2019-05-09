#include "VoronoiPolygonOffset.h"

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



pair<bool, rg_Point2D> VoronoiPolygonOffset::find_offset_vertex_for_trunk(VEdge2D* trunk, const float& offsetAmount)
{
	pair<Generator2D*, Generator2D*> polygonGenerators = find_polygon_generators(trunk);

	Generator2D* leftGenerator = polygonGenerators.first;
	Generator2D* rightGenerator = polygonGenerators.second;

	rg_Point2D startPt = trunk->getStartVertex()->getLocation();
	rg_Point2D endPt = trunk->getEndVertex()->getLocation();

	bool doesOffsetVtxExist = false;
	rg_Point2D offsetVtx;

	if (leftGenerator->type() == Generator2D::EDGE_G)
	{
		EdgeGenerator2D* edgeGenerator = static_cast<EdgeGenerator2D*>(leftGenerator);
		rg_Line2D polyEdge(edgeGenerator->get_start_vertex_point(), edgeGenerator->get_end_vertex_point());
		float distanceToStartPt = polyEdge.getDistance(startPt);
		float distanceToEndPt = polyEdge.getDistance(endPt);
		if ((distanceToStartPt - offsetAmount)*(distanceToEndPt - offsetAmount) < 0)
		{
			doesOffsetVtxExist = true;
			offsetVtx = (startPt + endPt) / 2;
		}
	}
	else if(leftGenerator->type() == Generator2D::VERTEX_G)
	{
		VertexGenerator2D* vtxGenerator = static_cast<VertexGenerator2D*>(leftGenerator);
		rg_Point2D vtxPt = vtxGenerator->get_point();
		
		float distanceToStartPt = vtxPt.distance(startPt);
		float distanceToEndPt = vtxPt.distance(endPt);
		if ((distanceToStartPt - offsetAmount)*(distanceToEndPt - offsetAmount) < 0)
		{
			doesOffsetVtxExist = true;
			offsetVtx = (startPt + endPt) / 2;
		}
	}

	return make_pair(doesOffsetVtxExist, offsetVtx);
}



list<rg_Point2D> VoronoiPolygonOffset::calculate_points_for_trunk_edge(VEdge2D* trunk)
{
	list<rg_Point2D> trunkPoints;

	Generator2D* leftGenerator = trunk->getLeftFace()->getGenerator();
	Generator2D* rightGenerator = trunk->getRightFace()->getGenerator();

	if (leftGenerator->type() == Generator2D::VERTEX_G
		&& rightGenerator->type() == Generator2D::VERTEX_G)
	{
		trunkPoints.push_back(trunk->getStartVertex()->getLocation());
	}

	return list<rg_Point2D>();
}



list<rg_Point2D> VoronoiPolygonOffset::calculate_offset_vertex_for_parabolic_edge(VEdge2D* trunk, const float& offsetAmount)
{
	pair<Generator2D*, Generator2D*> polygonGenerators = find_polygon_generators(trunk);

	Generator2D* leftGenerator = polygonGenerators.first;
	Generator2D* rightGenerator = polygonGenerators.second;


}



pair<Generator2D*, Generator2D*> VoronoiPolygonOffset::find_polygon_generators(VEdge2D* edge)
{
	Generator2D* leftGenerator = static_cast<Generator2D*>(edge->getLeftFace()->getGenerator()->user_data());
	Generator2D* rightGenerator = static_cast<Generator2D*>(edge->getRightFace()->getGenerator()->user_data());
	return make_pair(leftGenerator, rightGenerator);
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

	int traverseCounter = 0;
	for (auto& insiderEdge : m_traverseList)
	{
		switch (m_mapForVEdgeType.at(insiderEdge))
		{
		case BRANCH:
		{
			if (visitedEdges.count(insiderEdge) == 0)
			{
				pair<bool, rg_Point2D> offsetVertex = find_offset_vertex_for_branch(insiderEdge, offsetAmount);
				if (offsetVertex.first == true)
					m_offsets.at(CStack.back()).add_offset_vertex(offsetVertex.second, insiderEdge);
				visitedEdges.insert(insiderEdge);
			}
		}
			break;
		case FLUFF:
		{
			if (visitedEdges.count(insiderEdge) == 0)
			{
				pair<bool, rg_Point2D> offsetVertex = find_offset_vertex_for_fluff(insiderEdge, offsetAmount);
				if (offsetVertex.first == true)
					m_offsets.at(CStack.back()).add_offset_vertex(offsetVertex.second, insiderEdge);
				visitedEdges.insert(insiderEdge);
			}
		}
			break;
		case TRUNK:
		{
			pair<bool, rg_Point2D> offsetVertex = find_offset_vertex_for_trunk(insiderEdge, offsetAmount);
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
						m_offsets.at(CStack.back()).close_offset();
						CStack.pop_back();
					}
					else
					{
						status = SPLIT;	
						TStack.push_back(offsetVertex.second);
						m_offsets.at(CStack.back()).add_offset_vertex(offsetVertex.second, insiderEdge);
						m_offsets.push_back(Offset(m_offsets.size(), offsetAmount));
						CStack.push_back(m_offsets.size()-1);
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
						m_offsets.at(CStack.back()).add_offset_vertex(offsetVertex.second, insiderEdge);
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
			break;
		default:
			break;
		}
		traverseCounter++;
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
