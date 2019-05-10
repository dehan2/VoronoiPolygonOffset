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



void VoronoiPolygonOffsetDisplayer::draw_polygon()
{
	rg_Point2D* boundaryVertices = rg_NULL;
	int numVertices = 0;
	pPolygon->getBoundaryVertices(boundaryVertices, numVertices);

	for (int i = 0; i < numVertices-1; i++)
	{
		QPointF p1(boundaryVertices[i].getX(), boundaryVertices[i].getY());
		QPointF p2(boundaryVertices[i+1].getX(), boundaryVertices[i+1].getY());
		addLine(QLineF(p1, p2), QPen(Qt::black));
	}

	QPointF p1(boundaryVertices[numVertices - 1].getX(), boundaryVertices[numVertices - 1].getY());
	QPointF p2(boundaryVertices[0].getX(), boundaryVertices[0].getY());
	addLine(QLineF(p1, p2), QPen(Qt::black));

	if (boundaryVertices != rg_NULL)
		delete[] boundaryVertices;
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

			if (leftGenerator->type() == Generator2D::EDGE_G && rightGenerator->type() == Generator2D::EDGE_G)
			{
				rg_Point2D startPt = edge->getStartVertex()->getLocation();
				rg_Point2D endPt = edge->getEndVertex()->getLocation();
				QPointF p1(startPt.getX(), startPt.getY());
				QPointF p2(endPt.getX(), endPt.getY());
				addLine(QLineF(p1, p2), QPen(Qt::blue));
			}
			else
			{
				if (pVD->get_location_status_of_vvertex(edge->getStartVertex()) == PolygonVD2D::VDEntity_Location_Status::ON_POLYGON_BOUNDARY
					|| pVD->get_location_status_of_vvertex(edge->getEndVertex()) == PolygonVD2D::VDEntity_Location_Status::ON_POLYGON_BOUNDARY)
				{
					rg_Point2D startPt = edge->getStartVertex()->getLocation();
					rg_Point2D endPt = edge->getEndVertex()->getLocation();
					QPointF p1(startPt.getX(), startPt.getY());
					QPointF p2(endPt.getX(), endPt.getY());
					addLine(QLineF(p1, p2), QPen(Qt::blue));
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
		QPointF p1(startPt.getX(), startPt.getY());
		QPointF p2(endPt.getX(), endPt.getY());
		addLine(QLineF(p1, p2), QPen(Qt::blue));
	}
}



void VoronoiPolygonOffsetDisplayer::draw_offsets()
{
	for (auto& offset : (*pOffsets))
	{
		vector<OffsetVertex>& vertices = offset.get_vertices();
		OffsetVertex* lastVtx = &vertices.back();

		for (auto& vtx : vertices)
		{
			rg_Point2D startPt = lastVtx->get_coordinate();
			rg_Point2D endPt = vtx.get_coordinate();

			QPointF p1(startPt.getX(), startPt.getY());
			QPointF p2(endPt.getX(), endPt.getY());
			addLine(QLineF(p1, p2), QPen(Qt::red));
			//draw_point(startPt.getX(), startPt.getY(), Qt::red);

			lastVtx = &vtx;
		}
	}
}



void VoronoiPolygonOffsetDisplayer::draw_point(float x, float y, QColor color)
{
	double rad = 10;
	addEllipse(x - rad, y - rad, rad*2.0, rad*2.0,	QPen(color), QBrush(Qt::SolidPattern));
}
