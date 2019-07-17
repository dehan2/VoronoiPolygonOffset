#include "VoronoiPolygonOffsetDisplayer.h"
#include "OffsetEdge.h"
#include "OffsetVertex.h"
#include "rg_RQBzCurve2D.h"

VoronoiPolygonOffsetDisplayer::VoronoiPolygonOffsetDisplayer(QObject *parent)
	: QGraphicsScene(parent)
{
}



VoronoiPolygonOffsetDisplayer::~VoronoiPolygonOffsetDisplayer()
{
}



void VoronoiPolygonOffsetDisplayer::calculate_center()
{
	float xSum = 0;
	float ySum = 0;

	rg_Point2D* boundaryVertices = rg_NULL;
	int numVertices = 0;
	pPolygon->getBoundaryVertices(boundaryVertices, numVertices);
	for (int i = 0; i < numVertices; i++)
	{
		xSum += boundaryVertices[i].getX();
		ySum += boundaryVertices[i].getY();
	}

	center.setX(xSum / numVertices);
	center.setY(ySum / numVertices);
}



void VoronoiPolygonOffsetDisplayer::draw_polygon()
{
	/*rg_Point2D* boundaryVertices = rg_NULL;
	int numVertices = 0;
	pPolygon->getBoundaryVertices(boundaryVertices, numVertices);

	for (int i = 0; i < numVertices-1; i++)
	{
		draw_line(boundaryVertices[i], boundaryVertices[i + 1], 1, Qt::black);
	}

	draw_line(boundaryVertices[numVertices - 1], boundaryVertices[0], 1, Qt::black);

	if (boundaryVertices != rg_NULL)
		delete[] boundaryVertices;

	array<float, 2> lastPt = pLeftPolygon->back();
	rg_Point2D lastVtx(lastPt.at(0), lastPt.at(1));
	auto& it = pLeftPolygon->begin();
	while (it != pLeftPolygon->end())
	{
		rg_Point2D currVtx((*it).at(0), (*it).at(1));
		draw_line(lastVtx, currVtx, 1, Qt::black);
		lastVtx = currVtx;
		it++;
	}

	lastPt = pRightPolygon->back();
	lastVtx = rg_Point2D(lastPt.at(0), lastPt.at(1));
	it = pRightPolygon->begin();
	while (it != pRightPolygon->end())
	{
		rg_Point2D currVtx((*it).at(0), (*it).at(1));
		draw_line(lastVtx, currVtx, 1, Qt::black);
		lastVtx = currVtx;
		it++;
	}*/

	array<float, 2> lastPt = pTotalPolygon->back();
	rg_Point2D lastVtx(lastPt.at(0), lastPt.at(1));
	auto& it = pTotalPolygon->begin();
	while (it != pTotalPolygon->end())
	{
		rg_Point2D currVtx((*it).at(0), (*it).at(1));
		draw_line(lastVtx, currVtx, 1, Qt::black);
		lastVtx = currVtx;
		it++;
	}
}



void VoronoiPolygonOffsetDisplayer::draw_VD()
{
	list<const VEdge2D*> edges;
	pVD->getVoronoiEdges(edges);
	for (auto& edge : edges)
	{
		if (pVD->get_location_status_of_edge(edge)
			== PolygonVD2D::VDEntity_Location_Status::INSIDE_POLYGON)
		{
			Generator2D* leftGenerator = static_cast<Generator2D*>(edge->getLeftFace()->getGenerator()->user_data());
			Generator2D* rightGenerator = static_cast<Generator2D*>(edge->getRightFace()->getGenerator()->user_data());

			/*rg_Point2D startPt = edge->getStartVertex()->getLocation();
			rg_Point2D endPt = edge->getEndVertex()->getLocation();
			QPointF p1(startPt.getX(), startPt.getY());
			QPointF p2(endPt.getX(), endPt.getY());
			addLine(QLineF(p1, p2), QPen(Qt::blue));*/

			if ((leftGenerator->type() == Generator2D::EDGE_G && rightGenerator->type() == Generator2D::EDGE_G)
				|| (leftGenerator->type() == Generator2D::VERTEX_G && rightGenerator->type() == Generator2D::VERTEX_G))
			{
				rg_Point2D startPt = edge->getStartVertex()->getLocation();
				rg_Point2D endPt = edge->getEndVertex()->getLocation();

				draw_line(startPt, endPt, 1, Qt::blue);
			}
			else
			{
				if (pVD->get_location_status_of_vvertex(edge->getStartVertex()) == PolygonVD2D::VDEntity_Location_Status::ON_POLYGON_BOUNDARY
					|| pVD->get_location_status_of_vvertex(edge->getEndVertex()) == PolygonVD2D::VDEntity_Location_Status::ON_POLYGON_BOUNDARY)
				{
					rg_Point2D startPt = edge->getStartVertex()->getLocation();
					rg_Point2D endPt = edge->getEndVertex()->getLocation();
					draw_line(startPt, endPt, 1, Qt::blue);
				}
				else
					draw_parabolic_edge(edge);
			}			
		}		
	}
}



void VoronoiPolygonOffsetDisplayer::draw_parabolic_edge(const VEdge2D* edge)
{
	rg_RQBzCurve2D curve = pVD->get_geometry_of_edge(edge);
	for (int i = 0; i < 10; i++)
	{
		rg_Point2D startPt = curve.evaluatePt(0.1*i);
		rg_Point2D endPt = curve.evaluatePt(0.1*(i + 1));
		draw_line(startPt, endPt, 1, Qt::blue);
	}
}



void VoronoiPolygonOffsetDisplayer::draw_offsets(const int startID)
{
	for (int i=startID; i<pOffsets->size(); i++)
	{
		auto& offset = pOffsets->at(i);
		vector<OffsetVertex>& vertices = offset.get_vertices();
		rg_Point2D lastPt = vertices.back().get_coordinate();
		for (auto& vtx : vertices)
		{
			rg_Point2D currPt = vtx.get_coordinate();
			draw_line(lastPt, currPt, 1, Qt::red);
			lastPt = currPt;
		}

		/*vector<OffsetEdge>& edges = offset.get_edges();
		for (auto& edge : edges)
		{
			if (edge.get_is_arc_edge() == false)
			{
				rg_Point2D startPt = edge.get_start_vertex()->get_coordinate();
				rg_Point2D endPt = edge.get_end_vertex()->get_coordinate();
				draw_line(startPt, endPt, 1, Qt::red);
			}
			else
			{
				VertexGenerator2D* reflexVtx = edge.get_reflex_vertex();
				float distance = edge.get_start_vertex()->get_coordinate().distance(reflexVtx->get_point());
				rg_Point2D lastPoint = edge.get_start_vertex()->get_coordinate();
				for (int i = 1; i <= 10; i++)
				{
					rg_Point2D midPoint = 0.1*i*edge.get_start_vertex()->get_coordinate() + (1 - 0.1*i)*edge.get_end_vertex()->get_coordinate();
					rg_Point2D direction = midPoint - reflexVtx->get_point();
					rg_Point2D arcPoint = reflexVtx->get_point() + direction.getUnitVector()*distance;

					draw_line(lastPoint, arcPoint, 1, Qt::red);
					lastPoint = arcPoint;
				}
			}
		}*/
	}
}



void VoronoiPolygonOffsetDisplayer::draw_point(float x, float y, QColor color)
{
	double rad = 10;
	addEllipse(x - rad, y - rad, rad*2.0, rad*2.0,	QPen(color), QBrush(Qt::SolidPattern));
}



void VoronoiPolygonOffsetDisplayer::draw_line(const rg_Point2D& pt1, const rg_Point2D& pt2, int width, QColor color)
{
	QPointF p1((pt1.getX()-center.getX())*SCALE_FROM_REAL_TO_COORD, (pt1.getY() - center.getY())*SCALE_FROM_REAL_TO_COORD);
	QPointF p2((pt2.getX() - center.getX())*SCALE_FROM_REAL_TO_COORD, (pt2.getY() - center.getY())*SCALE_FROM_REAL_TO_COORD);
	addLine(QLineF(p1, p2), QPen(color, width));
}



void VoronoiPolygonOffsetDisplayer::draw_search_path()
{
	for (int i = 0; i < pSearchPath->size()-1; i++)
	{
		rg_Point2D startPt = pSearchPath->at(i);
		rg_Point2D endPt = pSearchPath->at(i+1);
		draw_line(startPt, endPt, 3, Qt::green);
	}
}



void VoronoiPolygonOffsetDisplayer::draw_horizontal_search_path()
{
	for (int i = 0; i < pHorizontalSearchPath->size() - 1; i++)
	{
		rg_Point2D startPt = pHorizontalSearchPath->at(i);
		rg_Point2D endPt = pHorizontalSearchPath->at(i + 1);
		draw_line(startPt, endPt, 3, Qt::green);
	}
}



void VoronoiPolygonOffsetDisplayer::draw_vertical_search_path()
{
	for (int i = 0; i < pVerticalSearchPath->size() - 1; i++)
	{
		rg_Point2D startPt = pVerticalSearchPath->at(i);
		rg_Point2D endPt = pVerticalSearchPath->at(i + 1);
		draw_line(startPt, endPt, 3, Qt::green);
	}
}
