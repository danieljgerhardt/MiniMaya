#include "drawablecomponent.h"

VertexDisplay::VertexDisplay() : Drawable(nullptr), representedVertex(nullptr) {}
VertexDisplay::VertexDisplay(OpenGLContext *context, Vertex* v) : Drawable(context), representedVertex(v) {}
VertexDisplay::~VertexDisplay() {}

void VertexDisplay::create() {
    std::vector<glm::vec4> col;
    std::vector<glm::vec4> pos;
    std::vector<GLuint> idx;

    col.push_back(glm::vec4(1, 1, 1, 1));
    pos.push_back(glm::vec4(representedVertex->pos, 1));
    idx.push_back(0);

    count = idx.size();

    generateIdx();
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(GLuint), idx.data(), GL_STATIC_DRAW);

    generatePos();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufPos);
    mp_context->glBufferData(GL_ARRAY_BUFFER, pos.size() * sizeof(glm::vec4), pos.data(), GL_STATIC_DRAW);

    generateCol();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufCol);
    mp_context->glBufferData(GL_ARRAY_BUFFER, col.size() * sizeof(glm::vec4), col.data(), GL_STATIC_DRAW);
}

void VertexDisplay::updateVertex(Vertex* v) {
    this->representedVertex = v;
}

GLenum VertexDisplay::drawMode() {
    return GL_POINTS;
}

HalfEdge* VertexDisplay::getHE() const {
    return this->representedVertex->getHE();
}

HalfEdgeDisplay::HalfEdgeDisplay() : Drawable(nullptr), representedHE(nullptr) {}
HalfEdgeDisplay::HalfEdgeDisplay(OpenGLContext *context, HalfEdge* he) : Drawable(context), representedHE(he) {}
HalfEdgeDisplay::~HalfEdgeDisplay() {}

void HalfEdgeDisplay::create() {
    std::vector<glm::vec4> col;
    std::vector<glm::vec4> pos;
    std::vector<GLuint> idx;

    Vertex* prevVert;
    HalfEdge* iter = representedHE;
    do {
        prevVert = iter->getVertex();
        iter = iter->getNext();
    } while (iter != representedHE);

    col.push_back(glm::vec4(1, 0, 0, 1));
    col.push_back(glm::vec4(1, 1, 0, 1));
    pos.push_back(glm::vec4(prevVert->getPos(), 1));
    pos.push_back(glm::vec4(representedHE->getVertex()->getPos(), 1));
    idx.push_back(0);
    idx.push_back(1);

    count = idx.size();

    generateIdx();
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(GLuint), idx.data(), GL_STATIC_DRAW);

    generatePos();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufPos);
    mp_context->glBufferData(GL_ARRAY_BUFFER, pos.size() * sizeof(glm::vec4), pos.data(), GL_STATIC_DRAW);

    generateCol();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufCol);
    mp_context->glBufferData(GL_ARRAY_BUFFER, col.size() * sizeof(glm::vec4), col.data(), GL_STATIC_DRAW);
}

void HalfEdgeDisplay::updateHE(HalfEdge* he) {
    this->representedHE = he;
}

void HalfEdgeDisplay::makeNext() {
    this->representedHE = this->representedHE->getNext();
}

void HalfEdgeDisplay::makeSym() {
    this->representedHE = this->representedHE->getSym();
}

GLenum HalfEdgeDisplay::drawMode() {
    return GL_LINES;
}

Face* HalfEdgeDisplay::getFace() const {
    return this->representedHE->getFace();
}

Vertex* HalfEdgeDisplay::getVertex() const {
    return this->representedHE->getVertex();
}

FaceDisplay::FaceDisplay() : Drawable(nullptr), representedFace(nullptr) {}
FaceDisplay::FaceDisplay(OpenGLContext *context, Face* f) : Drawable(context), representedFace(f) {}
FaceDisplay::~FaceDisplay() {}

Vertex* getPrevVert(HalfEdge* he) {
    Vertex* prevVert;
    HalfEdge* iter = he;
    do {
        prevVert = iter->getVertex();
        iter = iter->getNext();
    } while (iter != he);
    return prevVert;
}

void FaceDisplay::create() {
    std::vector<glm::vec4> col;
    std::vector<glm::vec4> pos;
    std::vector<GLuint> idx;

    std::vector<HalfEdge*> hes;
    HalfEdge* iter = representedFace->getHE();
    do {
        hes.push_back(iter);
        iter = iter->getNext();
    } while (iter != representedFace->getHE());

    int counter = 0;
    for (HalfEdge* hePtr : hes) {
        col.push_back(glm::vec4(1, 0, 0, 1));
        col.push_back(glm::vec4(1, 1, 0, 1));
        Vertex* prev = getPrevVert(hePtr);
        Vertex* next = hePtr->getVertex();
        pos.push_back(glm::vec4(prev->getPos(), 1));
        pos.push_back(glm::vec4(next->getPos(), 1));
        idx.push_back(counter);
        idx.push_back(counter + 1);
        counter += 2;
    }

    count = idx.size();

    generateIdx();
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(GLuint), idx.data(), GL_STATIC_DRAW);

    generatePos();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufPos);
    mp_context->glBufferData(GL_ARRAY_BUFFER, pos.size() * sizeof(glm::vec4), pos.data(), GL_STATIC_DRAW);

    generateCol();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufCol);
    mp_context->glBufferData(GL_ARRAY_BUFFER, col.size() * sizeof(glm::vec4), col.data(), GL_STATIC_DRAW);
}

void FaceDisplay::updateFace(Face* f) {
    this->representedFace = f;
}

GLenum FaceDisplay::drawMode() {
    return GL_LINES;
}

HalfEdge* FaceDisplay::getHE() const {
    return this->representedFace->getHE();
}
