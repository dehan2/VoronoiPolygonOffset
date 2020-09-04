/********************************************************************************
** Form generated from reading UI file 'VoronoiPolygonOffset.ui'
**
** Created by: Qt User Interface Compiler version 5.15.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_VORONOIPOLYGONOFFSET_H
#define UI_VORONOIPOLYGONOFFSET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QHBoxLayout>
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
    QPushButton *pushButton_compute_search_path;
    QPushButton *pushButton_generate_horizontal_search_path;
    QPushButton *pushButton_generate_verticall_search_path;
    QPushButton *pushButton_print_plan;
    QSpacerItem *verticalSpacer;
    QWidget *tab_2;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *VoronoiPolygonOffsetClass)
    {
        if (VoronoiPolygonOffsetClass->objectName().isEmpty())
            VoronoiPolygonOffsetClass->setObjectName(QString::fromUtf8("VoronoiPolygonOffsetClass"));
        VoronoiPolygonOffsetClass->resize(1113, 650);
        centralWidget = new QWidget(VoronoiPolygonOffsetClass);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        horizontalLayout = new QHBoxLayout(centralWidget);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        graphicsView = new QGraphicsView(centralWidget);
        graphicsView->setObjectName(QString::fromUtf8("graphicsView"));

        horizontalLayout->addWidget(graphicsView);

        tabWidget = new QTabWidget(centralWidget);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
        tabWidget->setMaximumSize(QSize(250, 16777215));
        tab = new QWidget();
        tab->setObjectName(QString::fromUtf8("tab"));
        verticalLayout = new QVBoxLayout(tab);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        pushButton_open_polygon = new QPushButton(tab);
        pushButton_open_polygon->setObjectName(QString::fromUtf8("pushButton_open_polygon"));

        verticalLayout->addWidget(pushButton_open_polygon);

        pushButton_compute_VD = new QPushButton(tab);
        pushButton_compute_VD->setObjectName(QString::fromUtf8("pushButton_compute_VD"));

        verticalLayout->addWidget(pushButton_compute_VD);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        doubleSpinBox_offsetAmount = new QDoubleSpinBox(tab);
        doubleSpinBox_offsetAmount->setObjectName(QString::fromUtf8("doubleSpinBox_offsetAmount"));
        doubleSpinBox_offsetAmount->setDecimals(1);
        doubleSpinBox_offsetAmount->setMaximum(1000.000000000000000);
        doubleSpinBox_offsetAmount->setValue(3.000000000000000);

        horizontalLayout_2->addWidget(doubleSpinBox_offsetAmount);

        doubleSpinBox_offsetIncrement = new QDoubleSpinBox(tab);
        doubleSpinBox_offsetIncrement->setObjectName(QString::fromUtf8("doubleSpinBox_offsetIncrement"));
        doubleSpinBox_offsetIncrement->setDecimals(1);
        doubleSpinBox_offsetIncrement->setMinimum(1.000000000000000);
        doubleSpinBox_offsetIncrement->setValue(3.000000000000000);

        horizontalLayout_2->addWidget(doubleSpinBox_offsetIncrement);

        pushButton_compute_offset = new QPushButton(tab);
        pushButton_compute_offset->setObjectName(QString::fromUtf8("pushButton_compute_offset"));

        horizontalLayout_2->addWidget(pushButton_compute_offset);


        verticalLayout->addLayout(horizontalLayout_2);

        pushButton_compute_search_path = new QPushButton(tab);
        pushButton_compute_search_path->setObjectName(QString::fromUtf8("pushButton_compute_search_path"));

        verticalLayout->addWidget(pushButton_compute_search_path);

        pushButton_generate_horizontal_search_path = new QPushButton(tab);
        pushButton_generate_horizontal_search_path->setObjectName(QString::fromUtf8("pushButton_generate_horizontal_search_path"));

        verticalLayout->addWidget(pushButton_generate_horizontal_search_path);

        pushButton_generate_verticall_search_path = new QPushButton(tab);
        pushButton_generate_verticall_search_path->setObjectName(QString::fromUtf8("pushButton_generate_verticall_search_path"));

        verticalLayout->addWidget(pushButton_generate_verticall_search_path);

        pushButton_print_plan = new QPushButton(tab);
        pushButton_print_plan->setObjectName(QString::fromUtf8("pushButton_print_plan"));

        verticalLayout->addWidget(pushButton_print_plan);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        tabWidget->addTab(tab, QString());
        tab_2 = new QWidget();
        tab_2->setObjectName(QString::fromUtf8("tab_2"));
        tabWidget->addTab(tab_2, QString());

        horizontalLayout->addWidget(tabWidget);

        VoronoiPolygonOffsetClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(VoronoiPolygonOffsetClass);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 1113, 21));
        VoronoiPolygonOffsetClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(VoronoiPolygonOffsetClass);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        VoronoiPolygonOffsetClass->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(VoronoiPolygonOffsetClass);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        VoronoiPolygonOffsetClass->setStatusBar(statusBar);

        retranslateUi(VoronoiPolygonOffsetClass);
        QObject::connect(pushButton_open_polygon, SIGNAL(clicked()), VoronoiPolygonOffsetClass, SLOT(open_polygon_file()));
        QObject::connect(pushButton_compute_VD, SIGNAL(clicked()), VoronoiPolygonOffsetClass, SLOT(compute_VD()));
        QObject::connect(pushButton_compute_offset, SIGNAL(clicked()), VoronoiPolygonOffsetClass, SLOT(compute_offset()));
        QObject::connect(pushButton_compute_search_path, SIGNAL(clicked()), VoronoiPolygonOffsetClass, SLOT(compute_search_path()));
        QObject::connect(pushButton_generate_horizontal_search_path, SIGNAL(clicked()), VoronoiPolygonOffsetClass, SLOT(compute_horizontal_search_path()));
        QObject::connect(pushButton_generate_verticall_search_path, SIGNAL(clicked()), VoronoiPolygonOffsetClass, SLOT(compute_vertical_search_path()));
        QObject::connect(pushButton_print_plan, SIGNAL(clicked()), VoronoiPolygonOffsetClass, SLOT(output_plans()));

        QMetaObject::connectSlotsByName(VoronoiPolygonOffsetClass);
    } // setupUi

    void retranslateUi(QMainWindow *VoronoiPolygonOffsetClass)
    {
        VoronoiPolygonOffsetClass->setWindowTitle(QCoreApplication::translate("VoronoiPolygonOffsetClass", "VoronoiPolygonOffset", nullptr));
        pushButton_open_polygon->setText(QCoreApplication::translate("VoronoiPolygonOffsetClass", "Input Polygon", nullptr));
        pushButton_compute_VD->setText(QCoreApplication::translate("VoronoiPolygonOffsetClass", "Compute VD", nullptr));
        pushButton_compute_offset->setText(QCoreApplication::translate("VoronoiPolygonOffsetClass", "Compute Offset", nullptr));
        pushButton_compute_search_path->setText(QCoreApplication::translate("VoronoiPolygonOffsetClass", "Generate Search Path", nullptr));
        pushButton_generate_horizontal_search_path->setText(QCoreApplication::translate("VoronoiPolygonOffsetClass", "Generate Horizontal Search Path", nullptr));
        pushButton_generate_verticall_search_path->setText(QCoreApplication::translate("VoronoiPolygonOffsetClass", "Generate Vertical Search Path", nullptr));
        pushButton_print_plan->setText(QCoreApplication::translate("VoronoiPolygonOffsetClass", "Output Plans", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab), QCoreApplication::translate("VoronoiPolygonOffsetClass", "Tab 1", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_2), QCoreApplication::translate("VoronoiPolygonOffsetClass", "Tab 2", nullptr));
    } // retranslateUi

};

namespace Ui {
    class VoronoiPolygonOffsetClass: public Ui_VoronoiPolygonOffsetClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_VORONOIPOLYGONOFFSET_H
