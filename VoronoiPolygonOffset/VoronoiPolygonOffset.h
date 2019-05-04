#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_VoronoiPolygonOffset.h"

class VoronoiPolygonOffset : public QMainWindow
{
	Q_OBJECT

public:
	VoronoiPolygonOffset(QWidget *parent = Q_NULLPTR);

private:
	Ui::VoronoiPolygonOffsetClass ui;
};
