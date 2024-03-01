#include "mainwindow.h"
#include <iostream>
#include <ui_mainwindow.h>
#include "cameracontrolshelp.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->mygl->setFocus();

    //makin a load OBJ button
    connect(ui->loadOBJButton, SIGNAL(clicked()),
            ui->mygl, SLOT(slot_loadOBJ()));

    connect(ui->mygl,
            // Signal name
            SIGNAL(sig_sendHeItems(std::vector<QListWidgetItem*>&)),
            // Widget with the slot that receives the signal
            this,
            // Slot name
            SLOT(slot_addHeItems(std::vector<QListWidgetItem*>&)));

    connect(ui->mygl,
            // Signal name
            SIGNAL(sig_sendVertexItems(std::vector<QListWidgetItem*>&)),
            // Widget with the slot that receives the signal
            this,
            // Slot name
            SLOT(slot_addVertexItems(std::vector<QListWidgetItem*>&)));

    connect(ui->mygl,
            // Signal name
            SIGNAL(sig_sendFaceItems(std::vector<QListWidgetItem*>&)),
            // Widget with the slot that receives the signal
            this,
            // Slot name
            SLOT(slot_addFaceItems(std::vector<QListWidgetItem*>&)));

    connect(ui->vertsListWidget, SIGNAL(itemClicked(QListWidgetItem*)),
            ui->mygl, SLOT(slot_setSelectedVertex(QListWidgetItem*)));

    connect(ui->halfEdgesListWidget, SIGNAL(itemClicked(QListWidgetItem*)),
            ui->mygl, SLOT(slot_setSelectedHE(QListWidgetItem*)));

    connect(ui->facesListWidget, SIGNAL(itemClicked(QListWidgetItem*)),
            ui->mygl, SLOT(slot_setSelectedFace(QListWidgetItem*)));

    connect(ui->splitEdgeButton, SIGNAL(clicked()),
            ui->mygl, SLOT(slot_splitEdge()));

    connect(ui->triangulateFaceButton, SIGNAL(clicked()),
            ui->mygl, SLOT(slot_triangulateFace()));

    connect(ui->vertPosXSpinBox, SIGNAL(valueChanged(double)),
            ui->mygl, SLOT(slot_newX(double)));

    connect(ui->vertPosYSpinBox, SIGNAL(valueChanged(double)),
            ui->mygl, SLOT(slot_newY(double)));

    connect(ui->vertPosZSpinBox, SIGNAL(valueChanged(double)),
            ui->mygl, SLOT(slot_newZ(double)));

    connect(ui->faceRedSpinBox, SIGNAL(valueChanged(double)),
            ui->mygl, SLOT(slot_newR(double)));

    connect(ui->faceGreenSpinBox, SIGNAL(valueChanged(double)),
            ui->mygl, SLOT(slot_newG(double)));

    connect(ui->faceBlueSpinBox, SIGNAL(valueChanged(double)),
            ui->mygl, SLOT(slot_newB(double)));

    connect(ui->subdivideButton, SIGNAL(clicked()),
            ui->mygl, SLOT(slot_subdivide()));

    connect(ui->extrudeButton, SIGNAL(clicked()),
            ui->mygl, SLOT(slot_extrude()));

    connect(ui->loadJSONButton, SIGNAL(clicked()),
            ui->mygl, SLOT(slot_loadJSON()));

    connect(ui->skinMeshButton, SIGNAL(clicked()),
            ui->mygl, SLOT(slot_skinMesh()));

    connect(ui->mygl,
            SIGNAL(sig_sendRootNode(QTreeWidgetItem*)),
            this,
            SLOT(slot_addRootToTreeWidget(QTreeWidgetItem*)));

    connect(ui->jointsTreeWidget, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
            ui->mygl, SLOT(slot_setSelectedJoint(QTreeWidgetItem*,int)));

    connect(ui->plusXRotationButton, SIGNAL(clicked()),
            ui->mygl, SLOT(slot_posXRot()));

    connect(ui->plusYRotationButton, SIGNAL(clicked()),
            ui->mygl, SLOT(slot_posYRot()));

    connect(ui->plusZRotationButton, SIGNAL(clicked()),
            ui->mygl, SLOT(slot_posZRot()));

    connect(ui->minusXRotationButton, SIGNAL(clicked()),
            ui->mygl, SLOT(slot_negXRot()));

    connect(ui->minusYRotationButton, SIGNAL(clicked()),
            ui->mygl, SLOT(slot_negYRot()));

    connect(ui->minusZRotationButton, SIGNAL(clicked()),
            ui->mygl, SLOT(slot_negZRot()));

    connect(ui->jointPosXSpinBox, SIGNAL(valueChanged(double)),
           ui->mygl, SLOT(slot_newJointX(double)));

    connect(ui->jointPosYSpinBox, SIGNAL(valueChanged(double)),
            ui->mygl, SLOT(slot_newJointY(double)));

    connect(ui->jointPosZSpinBox, SIGNAL(valueChanged(double)),
            ui->mygl, SLOT(slot_newJointZ(double)));

    connect(ui->mygl, SIGNAL(sig_updateJointTransform(QString)),
            ui->jointTransform, SLOT(setText(QString)));

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionQuit_triggered()
{
    QApplication::exit();
}

void MainWindow::on_actionCamera_Controls_triggered()
{
    CameraControlsHelp* c = new CameraControlsHelp();
    c->show();
}

void MainWindow::slot_addFaceItems(std::vector<QListWidgetItem*>& faces) {
    int counter = 0;
    for (QListWidgetItem* face : faces) {
        QString text = "Face - " + QString::number(counter + 1);
        face->setText(text);
        ui->facesListWidget->addItem(face);
        counter++;
    }
}

void MainWindow::slot_addVertexItems(std::vector<QListWidgetItem*>& vertices) {
    int counter = 0;
    for (QListWidgetItem* vertex : vertices) {
        QString text = "Vertex - " + QString::number(counter + 1);
        vertex->setText(text);
        ui->vertsListWidget->addItem(vertex);
        counter++;
    }
}

void MainWindow::slot_addHeItems(std::vector<QListWidgetItem*>& hes) {
    int counter = 0;
    for (QListWidgetItem* he : hes) {
        QString text = "HalfEdge - " + QString::number(counter + 1);
        he->setText(text);
        ui->halfEdgesListWidget->addItem(he);
        counter++;
    }
}

void MainWindow::slot_addRootToTreeWidget(QTreeWidgetItem *i) {
    ui->jointsTreeWidget->addTopLevelItem(i);
}
