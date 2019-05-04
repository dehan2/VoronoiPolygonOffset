#include "VoronoiPolygonOffset.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	VoronoiPolygonOffset w;
	w.show();
	return a.exec();
}
