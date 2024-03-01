#include "mygl.h"
#include <la.h>

#include <iostream>
#include <QApplication>
#include <QKeyEvent>

#include <QFileDialog>

MyGL::MyGL(QWidget *parent)
    : OpenGLContext(parent),
      m_geomSquare(this),
      m_progLambert(this), m_progFlat(this), prog_skeleton(this), //added prog_skeleteon
      m_glCamera(),
      mp_selectedVertex(nullptr),
      mp_selectedHE(nullptr),
      mp_selectedFace(nullptr),
      mp_selectedJoint(nullptr),
      vd_ready(false),
      hed_ready(false),
      fd_ready(false),
      jd_ready(false)
{
    setFocusPolicy(Qt::StrongFocus);
}

MyGL::~MyGL()
{
    makeCurrent();
    glDeleteVertexArrays(1, &vao);
    m_geomSquare.destroy();
}

void MyGL::initializeGL()
{
    // Create an OpenGL context using Qt's QOpenGLFunctions_3_2_Core class
    // If you were programming in a non-Qt context you might use GLEW (GL Extension Wrangler)instead
    initializeOpenGLFunctions();
    // Print out some information about the current OpenGL context
    debugContextVersion();

    // Set a few settings/modes in OpenGL rendering
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    // Set the size with which points should be rendered
    glPointSize(5);
    // Set the color with which the screen is filled at the start of each render call.
    glClearColor(0.5, 0.5, 0.5, 1);

    printGLErrorLog();

    // Create a Vertex Attribute Object
    glGenVertexArrays(1, &vao);

    //Create the instances of Cylinder and Sphere.
    m_geomSquare.create();

    // Create and set up the diffuse shader
    m_progLambert.create(":/glsl/lambert.vert.glsl", ":/glsl/lambert.frag.glsl");
    // Create and set up the flat lighting shader
    m_progFlat.create(":/glsl/flat.vert.glsl", ":/glsl/flat.frag.glsl");
    //adding prog skelly
    prog_skeleton.create(":glsl/skeleton.vert.glsl", ":/glsl/skeleton.frag.glsl");

    // We have to have a VAO bound in OpenGL 3.2 Core. But if we're not
    // using multiple VAOs, we can just bind one once.
    glBindVertexArray(vao);
}

void MyGL::resizeGL(int w, int h)
{
    //This code sets the concatenated view and perspective projection matrices used for
    //our scene's camera view.
    m_glCamera = Camera(w, h);
    glm::mat4 viewproj = m_glCamera.getViewProj();

    // Upload the view-projection matrix to our shaders (i.e. onto the graphics card)

    m_progLambert.setViewProjMatrix(viewproj);
    m_progFlat.setViewProjMatrix(viewproj);
    //adding skelly for hw7
    prog_skeleton.setViewProjMatrix(viewproj);

    printGLErrorLog();
}

//This function is called by Qt any time your GL window is supposed to update
//For example, when the function update() is called, paintGL is called implicitly.
void MyGL::paintGL()
{
    // Clear the screen so that we only see newly drawn images
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_progFlat.setViewProjMatrix(m_glCamera.getViewProj());
    m_progLambert.setViewProjMatrix(m_glCamera.getViewProj());
    m_progLambert.setCamPos(m_glCamera.eye);
    m_progFlat.setModelMatrix(glm::mat4(1.f));

    //set prog skelly values
    prog_skeleton.setViewProjMatrix(m_glCamera.getViewProj());
    prog_skeleton.setCamPos(m_glCamera.eye);
    prog_skeleton.setModelMatrix(glm::mat4());

    if (mesh.hasVertices() && !mesh.skinned()) {
        glm::mat4 model = glm::mat4();
        m_progLambert.setModelMatrix(model);
        if (!mesh.skinned()) {
            m_progLambert.draw(mesh);
        }
    }

    if (mesh.hasJoints()) {
        int mCount = 0;
        std::array<glm::mat4, 100> binds, transforms;
        m_progFlat.setModelMatrix(glm::mat4());

        std::vector<Joint*> queue, toDraw;
        queue.push_back(mesh.getRoot());
        //data structures moment -- do while for level order traversal
        do {
            //get joint and draw it
            Joint* curr = queue[0];
            curr->create();
            toDraw.push_back(curr);

            //update binds and transforms mat for prog skelly
            binds[curr->getId() - 1] = curr->getBind();
            transforms[curr->getId() - 1] = curr->getOverallTransformation();
            mCount++;

            //add children to queue
            for (uPtr<Joint>& child : curr->getChildren()) {
                queue.push_back(child.get());
            }
            queue.erase(queue.begin());
        } while (!queue.empty());

        if (mesh.skinned()) {
            prog_skeleton.setBinds(binds, mCount);
            prog_skeleton.setTransforms(transforms, mCount);
            prog_skeleton.draw(mesh);
        }

        glDisable(GL_DEPTH_TEST);
        for (Joint* j : toDraw) {
            m_progFlat.draw(*j);
        }
        glEnable(GL_DEPTH_TEST);
    }

    if (vd_ready) {
        hed_ready = false;
        fd_ready = false;
        glDisable(GL_DEPTH_TEST);
        m_vertDisplay.create();
        m_progFlat.setModelMatrix(glm::mat4());
        m_progFlat.draw(m_vertDisplay);
        glEnable(GL_DEPTH_TEST);
    }

    if (hed_ready) {
        vd_ready = false;
        fd_ready = false;
        glDisable(GL_DEPTH_TEST);
        m_heDisplay.create();
        m_progFlat.setModelMatrix(glm::mat4());
        m_progFlat.draw(m_heDisplay);
        glEnable(GL_DEPTH_TEST);
    }

    if (fd_ready) {
        hed_ready = false;
        vd_ready = false;
        glDisable(GL_DEPTH_TEST);
        m_fDisplay.create();
        m_progFlat.setModelMatrix(glm::mat4());
        m_progFlat.draw(m_fDisplay);
        glEnable(GL_DEPTH_TEST);
    }

    if (jd_ready) {
        glDisable(GL_DEPTH_TEST);
        mp_selectedJoint->create();
        m_progFlat.setModelMatrix(glm::mat4());
        m_progFlat.draw(*mp_selectedJoint);
        glEnable(GL_DEPTH_TEST);
    }
}


void MyGL::keyPressEvent(QKeyEvent *e)
{
    float amount = 2.0f;
    if(e->modifiers() & Qt::ShiftModifier){
        amount = 10.0f;
    }
    // http://doc.qt.io/qt-5/qt.html#Key-enum
    // This could all be much more efficient if a switch
    // statement were used
    if (e->key() == Qt::Key_Escape) {
        QApplication::quit();
    } else if (e->key() == Qt::Key_Right) {
        m_glCamera.RotateAboutUp(-amount);
    } else if (e->key() == Qt::Key_Left) {
        m_glCamera.RotateAboutUp(amount);
    } else if (e->key() == Qt::Key_Up) {
        m_glCamera.RotateAboutRight(-amount);
    } else if (e->key() == Qt::Key_Down) {
        m_glCamera.RotateAboutRight(amount);
    } else if (e->key() == Qt::Key_1) {
        m_glCamera.fovy += amount;
    } else if (e->key() == Qt::Key_2) {
        m_glCamera.fovy -= amount;
    } else if (e->key() == Qt::Key_W) {
        m_glCamera.TranslateAlongLook(amount);
    } else if (e->key() == Qt::Key_S) {
        m_glCamera.TranslateAlongLook(-amount);
    } else if (e->key() == Qt::Key_D) {
        m_glCamera.TranslateAlongRight(amount);
    } else if (e->key() == Qt::Key_A) {
        m_glCamera.TranslateAlongRight(-amount);
    } else if (e->key() == Qt::Key_Q) {
        m_glCamera.TranslateAlongUp(-amount);
    } else if (e->key() == Qt::Key_E) {
        m_glCamera.TranslateAlongUp(amount);
    } else if (e->key() == Qt::Key_R) {
        m_glCamera = Camera(this->width(), this->height());
    } else if (e->key() == Qt::Key_N) {
        //half edge displayed = half edge displayed's next
        this->m_heDisplay.makeNext();
    } else if (e->key() == Qt::Key_M) {
        //half edge displayed = half edge displayed's sym
        this->m_heDisplay.makeSym();
    } else if (e->key() == Qt::Key_F) {
        hed_ready = false;
        fd_ready = true;
        this->m_fDisplay.updateFace(m_heDisplay.getFace());
        slot_setSelectedFace(m_heDisplay.getFace());
    } else if (e->key() == Qt::Key_V) {
        if (hed_ready) {
            hed_ready = false;
            vd_ready = true;
            this->m_vertDisplay.updateVertex(m_heDisplay.getVertex());
            slot_setSelectedVertex(m_heDisplay.getVertex());
        }
    } else if (e->key() == Qt::Key_H) {
        bool skip = false;
        if (e->modifiers() == Qt::ShiftModifier && fd_ready) {
            //shift + h
            //he of face
            std::cout << " shfi";
            fd_ready = false;
            hed_ready = true;
            this->m_heDisplay.updateHE(m_fDisplay.getHE());
            slot_setSelectedHE(m_fDisplay.getHE());
            skip = true;
        }
        if (!skip && vd_ready) {
            hed_ready = true;
            vd_ready = false;
            this->m_heDisplay.updateHE(m_vertDisplay.getHE());
            slot_setSelectedHE(m_vertDisplay.getHE());
        }
    }
    m_glCamera.RecomputeAttributes();
    update();  // Calls paintGL, among other things
}

Mesh MyGL::readMesh(QFile file) {
    //https://forum.qt.io/topic/59249/read-textfile-and-display-on-qlabel-solved
    //help log 6:30 10/11
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        std::cout << "could not read" << std::endl;
        return Mesh();
    }
    QString string;
    QTextStream in(&file);
    QTextStream stream(&file);

    //get rid of header data
    for (int i = 0; i < 4; i++) {
        stream.readLine();
    }

    //vcetors to contain data that is read in
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<std::vector<int>> faces;

    while (!stream.atEnd()){
        string = stream.readLine();

        if (string.length() < 2) continue;

        if (string[0] == 'v' && string[1] == ' ') {
            //line is a vertex
            std::string temp = string.toStdString().substr(2); //skip the "v " prefix
            std::stringstream ss(temp);
            std::vector<float> v;
            float value;

            while (ss >> value) {
                v.push_back(value);
            }
            vertices.push_back(glm::vec3(v[0], v[1], v[2]));
        } else if (string[0] == 'v' && string[1] == 'n') {
            //line is a normal
            std::string temp = string.toStdString().substr(3); //skip the "vn " prefix
            std::stringstream ss(temp);
            std::vector<float> v;
            float value;

            while (ss >> value) {
                v.push_back(value);
            }
            normals.push_back(glm::vec3(v[0], v[1], v[2]));
        } else if (string[0] == 'f') {
            //line is a face
            std::string temp = string.toStdString().substr(2); //skip the "f " prefix
            std::stringstream ss(temp);
            std::vector<int> vertexIndices;
            std::string s;
            int num1, num2, num3;
            char slash1, slash2;
            //man i love string steams :)
            while (ss >> num1 >> slash1 >> num2 >> slash2 >> num3) {
                //subtract 1 because obj isn't 0 indexed
                vertexIndices.push_back(num1 - 1);
            }

            faces.push_back(vertexIndices);
        }
    }

    //use mesh constructor to turn this data into a mesh
    Mesh mesh = Mesh(this, vertices, normals, faces);

    file.close();

    return mesh;
}

void MyGL::slot_loadOBJ() {
    QString fileName = QFileDialog::getOpenFileName(this,
                                            tr("Open OBJ"), "/home/", tr("OBJ Files (*.obj)"));

    mesh = readMesh(QFile(fileName));

    mesh.create();

    std::vector<QListWidgetItem*> listItems;
    for (uPtr<Face>& fPtr : mesh.getFaces()) {
        listItems.push_back(fPtr.get());
    }
    emit sig_sendFaceItems(listItems);

    listItems.clear();
    for (uPtr<Vertex>& vPtr : mesh.getVertices()) {
        listItems.push_back(vPtr.get());
    }
    emit sig_sendVertexItems(listItems);

    listItems.clear();
    for (uPtr<HalfEdge>& hePtr : mesh.getHes()) {
        listItems.push_back(hePtr.get());
    }
    emit sig_sendHeItems(listItems);
}

void MyGL::slot_setSelectedVertex(QListWidgetItem *i) {
    mp_selectedVertex = static_cast<Vertex*>(i);
    m_vertDisplay = VertexDisplay(this, mp_selectedVertex);
    vd_ready = true;

    update();
}

void MyGL::slot_setSelectedHE(QListWidgetItem *i) {
    mp_selectedHE = static_cast<HalfEdge*>(i);
    m_heDisplay = HalfEdgeDisplay(this, mp_selectedHE);
    hed_ready = true;

    update();
}

void MyGL::slot_setSelectedFace(QListWidgetItem *i) {
    mp_selectedFace = static_cast<Face*>(i);
    m_fDisplay = FaceDisplay(this, mp_selectedFace);
    fd_ready = true;

    update();
}

void MyGL::slot_splitEdge() {
    if (!hed_ready) {
        return;
    }
    HalfEdge* toSplit = mp_selectedHE;
    mesh.splitEdge(toSplit);
    updateMesh();
}

void MyGL::slot_triangulateFace() {
    if (!fd_ready) {
        return;
    }
    Face* toTriangulate = mp_selectedFace;
    mesh.triangulateFace(toTriangulate);
    updateMesh();
}

void MyGL::slot_newX(double x) {
    if (!vd_ready) {
        return;
    }
    mp_selectedVertex->setX(float(x));
    updateMesh();
}

void MyGL::slot_newY(double y) {
    if (!vd_ready) {
        return;
    }
    mp_selectedVertex->setY(float(y));
    updateMesh();
}

void MyGL::slot_newZ(double z) {
    if (!vd_ready) {
        return;
    }
    mp_selectedVertex->setZ(float(z));
    updateMesh();
}

void MyGL::slot_newR(double r) {
    if (!fd_ready) {
        return;
    }
    mp_selectedFace->setR(float(r));
    updateMesh();
}

void MyGL::slot_newG(double g) {
    if (!fd_ready) {
        return;
    }
    mp_selectedFace->setG(float(g));
    updateMesh();
}

void MyGL::slot_newB(double b) {
    if (!fd_ready) {
        return;
    }
    mp_selectedFace->setB(float(b));
    updateMesh();
}

void MyGL::updateMesh() {
    if (mesh.hasVertices()) mesh.create();

    std::vector<QListWidgetItem*> listItems;
    for (uPtr<Face>& fPtr : mesh.getFaces()) {
        listItems.push_back(fPtr.get());
    }
    emit sig_sendFaceItems(listItems);

    listItems.clear();
    for (uPtr<Vertex>& vPtr : mesh.getVertices()) {
        listItems.push_back(vPtr.get());
    }
    emit sig_sendVertexItems(listItems);

    listItems.clear();
    for (uPtr<HalfEdge>& hePtr : mesh.getHes()) {
        listItems.push_back(hePtr.get());
    }
    emit sig_sendHeItems(listItems);

    update();
}

void MyGL::slot_subdivide() {
    this->mesh.catmullclarkSubdivision();
    updateMesh();
}

void MyGL::slot_extrude() {
    if (!fd_ready) {
        return;
    }
    Face* toExtrude = mp_selectedFace;
    mesh.extrudeFace(toExtrude);
    updateMesh();
}

void MyGL::slot_loadJSON() {
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open OBJ"), "/home/", tr("JSON Files (*.json)"));

    uPtr<Joint> root = readJoints(QFile(fileName));
    emit sig_sendRootNode(root.get());
    mesh.setRootJoint(std::move(root));
}

uPtr<Joint> MyGL::readJoints(QFile file) {
    //https://stackoverflow.com/questions/15893040/how-to-create-read-write-json-files-in-qt5
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        std::cout << "could not read" << std::endl;
        return nullptr;
    }
    QString string = file.readAll();
    file.close();
    QJsonDocument json = QJsonDocument::fromJson(string.toUtf8());
    QJsonObject object = json.object();
    QJsonObject rootJoint = object["root"].toObject();
    return readJointsRec(rootJoint, nullptr);
}

uPtr<Joint> MyGL::readJointsRec(QJsonObject joint, Joint* parent) {
    QJsonArray currPos = joint["pos"].toArray(), currRot = joint["rot"].toArray(), currChildren = joint["children"].toArray();

    QString name = joint["name"].toString();

    glm::vec3 position = glm::vec3(currPos[0].toDouble(), currPos[1].toDouble(), currPos[2].toDouble());
    glm::vec3 axis = glm::vec3(currRot[1].toDouble(), currRot[2].toDouble(), currRot[3].toDouble());
    glm::quat rotation = glm::quat(glm::angleAxis(float(currRot[0].toDouble()), axis));

    uPtr<Joint> curr = mkU<Joint>(this, name, position, rotation);
    curr->setText(0, QString::number(curr->getId()) + ": " + name);

    for (int i = 0; i < currChildren.size(); i++) {
        readJointsRec(currChildren[i].toObject(), curr.get());
    }

    if (parent != nullptr) {
        curr->setParent(parent);
        parent->addChild(std::move(curr));
        return NULL;
    } else {
        return curr;
    }
}

void MyGL::slot_skinMesh() {
    mesh.skinMesh();
    updateMesh();
}

void MyGL::slot_setSelectedJoint(QTreeWidgetItem* item, int column) {
    if (mp_selectedJoint) {
        mp_selectedJoint->unselect();
    }
    mp_selectedJoint = static_cast<Joint*>(item);
    mp_selectedJoint->select();
    jd_ready = true;

    this->makeJointTransformString();

    update();
}

void MyGL::slot_posXRot() {
    mp_selectedJoint->posXRot();
    updateMesh();
    this->makeJointTransformString();
}

void MyGL::slot_posYRot() {
    mp_selectedJoint->posYRot();
    updateMesh();
    this->makeJointTransformString();
}

void MyGL::slot_posZRot() {
    mp_selectedJoint->posZRot();
    updateMesh();
    this->makeJointTransformString();
}

void MyGL::slot_negXRot() {
    mp_selectedJoint->negXRot();
    updateMesh();
    this->makeJointTransformString();
}

void MyGL::slot_negYRot() {
    mp_selectedJoint->negYRot();
    updateMesh();
    this->makeJointTransformString();
}

void MyGL::slot_negZRot() {
    mp_selectedJoint->negZRot();
    updateMesh();
    this->makeJointTransformString();
}

void MyGL::slot_newJointX(double x) {
    mp_selectedJoint->setX(x);
    updateMesh();
    this->makeJointTransformString();
}

void MyGL::slot_newJointY(double y) {
    mp_selectedJoint->setY(y);
    updateMesh();
    this->makeJointTransformString();
}

void MyGL::slot_newJointZ(double z) {
    mp_selectedJoint->setZ(z);
    updateMesh();
    this->makeJointTransformString();
}

void MyGL::makeJointTransformString() {
    QString transform = "";
    glm::mat4 t = mp_selectedJoint->getOverallTransformation();
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            transform += QString::number(t[j][i]);
            transform += " ";
        }
        if (i != 3) transform += "\n";
    }
    emit sig_updateJointTransform(transform);
}
