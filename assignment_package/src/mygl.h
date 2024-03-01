#ifndef MYGL_H
#define MYGL_H

#include <openglcontext.h>
#include <utils.h>
#include <shaderprogram.h>
#include <scene/squareplane.h>
#include "camera.h"

#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>

#include <mesh.h>
#include <QFile>
#include "drawablecomponent.h"

#include "joint.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

class MyGL
    : public OpenGLContext
{
    Q_OBJECT
private:
    SquarePlane m_geomSquare;// The instance of a unit cylinder we can use to render any cylinder
    ShaderProgram m_progLambert;// A shader program that uses lambertian reflection
    ShaderProgram m_progFlat;// A shader program that uses "flat" reflection (no shadowing at all)
    ShaderProgram prog_skeleton; //added for hw7

    GLuint vao; // A handle for our vertex array object. This will store the VBOs created in our geometry classes.
                // Don't worry too much about this. Just know it is necessary in order to render geometry.

    Camera m_glCamera;

    Mesh mesh; // mesh added that will be populated through the file dialogue handler

    Vertex *mp_selectedVertex;
    HalfEdge *mp_selectedHE;
    Face *mp_selectedFace;
    Joint *mp_selectedJoint;

    bool vd_ready; //vertex display ready
    bool hed_ready; //half edge display ready
    bool fd_ready; //face display ready
    bool jd_ready; //joint display ready

    void makeJointTransformString();

public:
    explicit MyGL(QWidget *parent = nullptr);
    ~MyGL();

    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    Mesh readMesh(QFile file);

    VertexDisplay m_vertDisplay;
    HalfEdgeDisplay m_heDisplay;
    FaceDisplay m_fDisplay;
    Joint m_jointDisplay;

    void updateMesh();

    uPtr<Joint> readJoints(QFile file);
    uPtr<Joint> readJointsRec(QJsonObject joint, Joint* parent);

protected:
    void keyPressEvent(QKeyEvent *e);

signals:
    void sig_sendHeItems(std::vector<QListWidgetItem*>&);
    void sig_sendVertexItems(std::vector<QListWidgetItem*>&);
    void sig_sendFaceItems(std::vector<QListWidgetItem*>&);

    void sig_sendRootNode(QTreeWidgetItem*);

    void sig_updateJointTransform(const QString &text);

public slots:
    void slot_loadOBJ();

    void slot_setSelectedVertex(QListWidgetItem*);
    void slot_setSelectedHE(QListWidgetItem*);
    void slot_setSelectedFace(QListWidgetItem*);

    void slot_splitEdge();
    void slot_triangulateFace();

    void slot_newX(double);
    void slot_newY(double);
    void slot_newZ(double);

    void slot_newR(double);
    void slot_newG(double);
    void slot_newB(double);

    void slot_subdivide();
    void slot_extrude();

    void slot_loadJSON();
    void slot_skinMesh();
    void slot_setSelectedJoint(QTreeWidgetItem* item, int column);

    void slot_posXRot();
    void slot_posYRot();
    void slot_posZRot();
    void slot_negXRot();
    void slot_negYRot();
    void slot_negZRot();

    void slot_newJointX(double);
    void slot_newJointY(double);
    void slot_newJointZ(double);
};


#endif // MYGL_H
