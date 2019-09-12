/********************************************************************************
** Form generated from reading UI file 'VoronoiPolygonOffset.ui'
**
** Created by: Qt User Interface Compiler version 5.10.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_VORONOIPOLYGONOFFSET_H
#define UI_VORONOIPOLYGONOFFSET_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_VoronoiPolygonOffsetClass
{
public:
    QWidget *centralWidget;
    QHBoxLayout *horizontalLayout;
    QGraphicsView *graphicsView;
    QTabWidget *tabWidget;
    QWidget *tab;
    QVBoxLayout *verticalLayout;
    QPushButton *pushButton_open_polygon;
    QPushButton *pushButton_compute_VD;
    QHBoxLayout *horizontalLayout_2;
    QDoubleSpinBox *doubleSpinBox_offsetAmount;
    QDoubleSpinBox *doubleSpinBox_offsetIncrement;
    QPushButton *pushButton_compute_offset;
    QPushButton *pushButton_compute_horizontal_offsets;
    QSpacerItem *verticalSpacer;
    QWidget *tab_2;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *VoronoiPolygonOffsetClass)
    {
        if (VoronoiPolygonOffsetClass->objectName().isEmpty())
            VoronoiPolygonOffsetClass->setObjectName(QStringLiteral("VoronoiPolygonOffsetClass"));
        VoronoiPolygonOffsetClass->resize(1113, 650);
        centralWidget = new QWidget(VoronoiPolygonOffsetClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        horizontalLayout = new QHBoxLayout(centralWidget);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        graphicsView = new QGraphicsView(centralWidget);
        graphicsView->setObjectName(QStringLiteral("graphicsView"));

        horizontalLayout->addWidget(graphicsView);

        tabWidget = new QTabWidget(centralWidget);
        tabWidget->setObjectName(QStringLiteral("tabWidget"));
        tabWidget->setMaximumSize(QSize(250, 16777215));
        tab = new QWidget();
        tab->setObjectName(QStringLiteral("tab"));
        verticalLayout = new QVBoxLayout(tab);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        pushButton_open_polygon = new QPushButton(tab);
        pushButton_open_polygon->setObjectName(QStringLiteral("pushButton_open_polygon"));

        verticalLayout->addWidget(pushButton_open_polygon);

        pushButton_compute_VD = new QPushButton(tab);
        pushButton_compute_VD->setObjectName(QStringLiteral("pushButton_compute_VD"));

        verticalLayout->addWidget(pushButton_compute_VD);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        doubleSpinBox_offsetAmount = new QDoubleSpinBox(tab);
        doubleSpinBox_offsetAmount->setObjectName(QStringLiteral("doubleSpinBox_offsetAmount"));
        doubleSpinBox_offsetAmount->setDecimals(1);
        doubleSpinBox_offsetAmount->setMaximum(1000);
        doubleSpinBox_offsetAmount->setValue(4);

        horizontalLayout_2->addWidget(doubleSpinBox_offsetAmount);

        doubleSpinBox_offsetIncrement = new QDoubleSpinBox(tab);
        doubleSpinBox_offsetIncrement->setObjectName(QStringLiteral("doubleSpinBox_offsetIncrement"));
        doubleSpinBox_offsetIncrement->setDecimals(1);
        doubleSpinBox_offsetIncrement->setValue(4);

        horizontalLayout_2->addWidget(doubleSpinBox_offsetIncrement);

        pushButton_compute_offset = new QPushButton(tab);
        pushButton_compute_offset->setObjectName(QStringLiteral("pushButton_compute_offset"));

        horizontalLayout_2->addWidget(pushButton_compute_offset);


        verticalLayout->addLayout(horizontalLayout_2);

        pushButton_compute_horizontal_offsets = new QPushButton(tab);
        pushButton_compute_horizontal_offsets->setObjectName(QStringLiteral("pushButton_compute_horizontal_offsets"));

        verticalLayout->addWidget(pushButton_compute_horizontal_offsets);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        tabWidget->addTab(tab, QString());
        tab_2 = new QWidget();
        tab_2->setObjectName(QStringLiteral("tab_2"));
        tabWidget->addTab(tab_2, QString());

        horizontalLayout->addWidget(tabWidget);

        VoronoiPolygonOffsetClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(VoronoiPolygonOffsetClass);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 1113, 21));
        VoronoiPolygonOffsetClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(VoronoiPolygonOffsetClass);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        VoronoiPolygonOffsetClass->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(VoronoiPolygonOffsetClass);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        VoronoiPolygonOffsetClass->setStatusBar(statusBar);

        retranslateUi(VoronoiPolygonOffsetClass);
        QObject::connect(pushButton_open_polygon, SIGNAL(clicked()), VoronoiPolygonOffsetClass, SLOT(open_polygon_file()));
        QObject::connect(pushButton_compute_VD, SIGNAL(clicked()), VoronoiPolygonOffsetClass, SLOT(compute_VD()));
        QObject::connect(pushButton_compute_offset, SIGNAL(clicked()), VoronoiPolygonOffsetClass, SLOT(compute_offset()));
        QObject::connect(pushButton_compute_horizontal_offsets, SIGNAL(clicked()), VoronoiPolygonOffsetClass, SLOT(compute_horizontal_offsets()));

        QMetaObject::connectSlotsByName(VoronoiPolygonOffsetClass);
    } // setupUi

    void retranslateUi(QMainWindow *VoronoiPolygonOffsetClass)
    {
        VoronoiPolygonOffsetClass->setWindowTitle(QApplication::translate("VoronoiPolygonOffsetClass", "VoronoiPolygonOffset", nullptr));
        pushButton_open_polygon->setText(QApplication::translate("VoronoiPolygonOffsetClass", "Input Polygon", nullptr));
        pushButton_compute_VD->setText(QApplication::translate("VoronoiPolygonOffsetClass", "Compute VD", nullptr));
        pushButton_compute_offset->setText(QApplication::translate("VoronoiPolygonOffsetClass", "Compute Offset", nullptr));
        pushButton_compute_horizontal_offsets->setText(QApplication::translate("VoronoiPolygonOffsetClass", "Compute Horizontal Offsets", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab), QApplication::translate("VoronoiPolygonOffsetClass", "Tab 1", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_2), QApplication::translate("VoronoiPolygonOffsetClass", "Tab 2", nullptr));
    } // retranslateUi

};

namespace Ui {
    class VoronoiPolygonOffsetClass: public Ui_VoronoiPolygonOffsetClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_VORONOIPOLYGONOFFSET_H
