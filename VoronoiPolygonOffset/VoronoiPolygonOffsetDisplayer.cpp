#include "VoronoiPolygonOffsetDisplayer.h"
#include "OffsetEdge.h"
#include "OffsetVertex.h"
#include "rg_RQBzCurve2D.h"
#include <array>

#define _USE_MATH_DEFINES 
#include <math.h>

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
	rg_Point2D* boundaryVertices = rg_NULL;
	int numVertices = 0;
	pPolygon->getBoundaryVertices(boundaryVertices, numVertices);

	for (int i = 0; i < numVertices-1; i++)
	{
		draw_line(boundaryVertices[i], boundaryVertices[i + 1], 1, Qt::black);
	}

	draw_line(boundaryVertices[numVertices - 1], boundaryVertices[0], 1, Qt::black);

	if (boundaryVertices != rg_NULL)
		delete[] boundaryVertices;
}



void VoronoiPolygonOffsetDisplayer::draw_disks()
{
	for (auto disk : *pDisks)
	{
		draw_circle(disk.getCenterPt(), disk.getRadius(), 1, Qt::black);
	}
}



void VoronoiPolygonOffsetDisplayer::draw_VD()
{
	for (auto& edge : *pInsiderEdges)
	{
		draw_edge(edge, 1, Qt::blue);
	}
}



void VoronoiPolygonOffsetDisplayer::draw_edge(const VEdge2D* edge, int width, QColor color)
{
	rg_RQBzCurve2D curve = pVD->get_geometry_of_edge(edge);
	for (int i = 0; i < 10; i++)
	{
		rg_Point2D startPt = curve.evaluatePt(0.1*i);
		rg_Point2D endPt = curve.evaluatePt(0.1*(i + 1));
		draw_line(startPt, endPt, width, color);
	}
}



void VoronoiPolygonOffsetDisplayer::draw_offsets(const int startID)
{
	for (int i=startID; i<pOffsets->size(); i++)
	{
		auto& offset = pOffsets->at(i);

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
				const Arc2D& arc = edge.get_arc();
				list<rg_Point2D> pointsOnArc;
				arc.evaluatePointsOnArcGivenResolution(100, pointsOnArc);

				rg_Point2D lastPoint;
				int counter = 0;
				for (auto pt : pointsOnArc)
				{
					if (counter > 0)
					{
						draw_line(lastPoint, pt, 1, Qt::red);
					}

					/*if (counter % 9 == 0 && edge.isAcute == false)
					{
						QColor color;
						switch (counter/18)
						{
						case 0:
							color = Qt::red;
							break;
						case 1:
							color = Qt::yellow;
							break;
						case 2:
							color = Qt::green;
							break;
						case 3:
							color = Qt::blue;
							break;
						case 4:
							color = Qt::darkBlue;
							break;
						case 5:
							color = Qt::cyan;
							break;
						default:
							break;
						}
						draw_circle(pt, 2, 2, color);
					}*/

					lastPoint = pt;
					counter++;
				}
			}
		}
	}
}



void VoronoiPolygonOffsetDisplayer::draw_point(float x, float y, QColor color)
{
	draw_circle(rg_Point2D(x, y), 3, 1, color);
	/*double rad = 10;
	double px = (x - rad - center.getX())*SCALE_FROM_REAL_TO_COORD;
	double py = (y - rad - center.getY())*SCALE_FROM_REAL_TO_COORD;
	addEllipse(px, py, rad*2.0, rad*2.0,	QPen(color), QBrush(Qt::SolidPattern));*/
}



void VoronoiPolygonOffsetDisplayer::draw_line(const rg_Point2D& pt1, const rg_Point2D& pt2, int width, QColor color)
{
	QPointF p1((pt1.getX()-center.getX())*SCALE_FROM_REAL_TO_COORD, (pt1.getY() - center.getY())*SCALE_FROM_REAL_TO_COORD);
	QPointF p2((pt2.getX() - center.getX())*SCALE_FROM_REAL_TO_COORD, (pt2.getY() - center.getY())*SCALE_FROM_REAL_TO_COORD);
	addLine(QLineF(p1, p2), QPen(color, width));
}



void VoronoiPolygonOffsetDisplayer::draw_circle(const rg_Point2D& center, const double& radius, int width, QColor color)
{
	for (int i = 0; i < 360; i++)
	{
		float startAngle = i / 360.0*2*M_PI;
		float endAngle = (i+1) / 360.0 * 2 * M_PI;
		rg_Point2D startPt = center + rg_Point2D(cos(startAngle), sin(startAngle))*radius;
		rg_Point2D endPt = center + rg_Point2D(cos(endAngle), sin(endAngle))*radius;
		draw_line(startPt, endPt, width, color);
	}
}

void VoronoiPolygonOffsetDisplayer::draw_horizontal_offsets()
{
	for (auto offsetLine : *pHorizontalOffsets)
	{
		draw_line(offsetLine.getSP(), offsetLine.getEP(), 1, Qt::red);
	}
}

