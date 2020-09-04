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

	list<rg_Point2D> boundaryVertices;
	pPolygon->get_boundary_vertices(boundaryVertices);
	for (auto& boundaryVtx : boundaryVertices)
	{
		xSum += boundaryVtx.getX();
		ySum += boundaryVtx.getY();
	}

	center.setX(xSum / boundaryVertices.size());
	center.setY(ySum / boundaryVertices.size());
}



void VoronoiPolygonOffsetDisplayer::draw_polygon()
{
	list<rg_Point2D> boundaryVertices;
	pPolygon->get_boundary_vertices(boundaryVertices);

	rg_Point2D lastVtx = boundaryVertices.back();

	for (auto& boundaryVtx : boundaryVertices)
	{
		draw_line(lastVtx, boundaryVtx, 1, Qt::black);
		lastVtx = boundaryVtx;
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
			Generator2D* leftGenerator = static_cast<Generator2D*>(edge->getLeftFace()->getGenerator()->getUserData());
			Generator2D* rightGenerator = static_cast<Generator2D*>(edge->getRightFace()->getGenerator()->getUserData());

			/*rg_Point2D startPt = edge->getStartVertex()->getLocation();
			rg_Point2D endPt = edge->getEndVertex()->getLocation();
			QPointF p1(startPt.getX(), startPt.getY());
			QPointF p2(endPt.getX(), endPt.getY());
			addLine(QLineF(p1, p2), QPen(Qt::blue));*/

			if ((leftGenerator->getType() == Generator2D::EDGE_G && rightGenerator->getType() == Generator2D::EDGE_G)
				|| (leftGenerator->getType() == Generator2D::VERTEX_G && rightGenerator->getType() == Generator2D::VERTEX_G))
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
	for (auto& offset : *pOffsets)
	{
		if (offset.get_ID() >= startID)
		{
			vector<OffsetEdge>& edges = offset.get_edges();
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
					const rg_RQBzCurve2D& curve = edge.get_curve();

					int numPtsOnCurve = 10;
					if (numPtsOnCurve < 1)
						numPtsOnCurve = 2;

					list<rg_Point2D> samplingPts;
					int divisor = numPtsOnCurve - 1;
					for (int i = 0; i < numPtsOnCurve; i++) 
					{
						double parameter = ((double)i) / divisor;

						rg_Point2D pt = curve.evaluatePt(parameter);
						samplingPts.push_back(pt);
					}

					draw_curves(samplingPts, 1, Qt::red);
				}
			}
		}		
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



void VoronoiPolygonOffsetDisplayer::draw_curves(const list<rg_Point2D>& curvePts, int width, QColor color)
{
	const rg_Point2D* lastPt = nullptr;
	for (auto& pt : curvePts)
	{
		if (lastPt != nullptr)
		{
			draw_line(*lastPt, pt, width, color);
		}			

		lastPt = &pt;
	}
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
