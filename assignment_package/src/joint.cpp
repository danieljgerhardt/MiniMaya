#include "joint.h"

#define PI 3.1415926535f

Joint::Joint() :
    Drawable(nullptr), name(""), parent(nullptr), pos_relative(glm::vec3()), rot(glm::quat()), bindMat(glm::mat4()), selected(false), id(topId + 1)
{
    topId++;
}

Joint::Joint(OpenGLContext *context) :
    Drawable(context), name(""), parent(nullptr), pos_relative(glm::vec3()), rot(glm::quat()), bindMat(glm::mat4()), selected(false), id(topId + 1)
{
    topId++;
}


Joint::Joint(OpenGLContext* context, glm::vec3 pos) :
    Drawable(context), name(""), parent(nullptr), pos_relative(pos), rot(glm::quat()), bindMat(glm::mat4()), selected(false), id(topId + 1)
{
    topId++;
}

Joint::Joint(OpenGLContext *context, QString name, glm::vec3 pos, glm::quat rot) :
    Drawable(context), name(name), parent(nullptr), pos_relative(pos), rot(rot), selected(false), id(topId + 1)
{
    topId++;
}

Joint::~Joint() {}

void Joint::addChild(uPtr<Joint> newChild) {
    ((QTreeWidgetItem*) this)->addChild(newChild.get());
    this->children.push_back(std::move(newChild));
}

void Joint::setParent(Joint* p) {
    this->parent = p;
}

void Joint::updateBindMat() {
    this->bindMat = glm::inverse(this->getOverallTransformation());
    for (auto& child : children) {
        child->updateBindMat();
    }
}

glm::mat4 Joint::getBind() {
    return this->bindMat;
}

glm::mat4 Joint::getLocalTransformation() const {
    return glm::translate(glm::mat4(), pos_relative) * glm::toMat4(rot);
}

glm::mat4 Joint::getOverallTransformation() const {
    if (!parent) {
        return this->getLocalTransformation();
    }
    return parent->getOverallTransformation() * this->getLocalTransformation();
}

QString Joint::getName() const {
    return this->name;
}

int Joint::getId() const {
    return this->id;
}

std::vector<uPtr<Joint>>& Joint::getChildren() {
    return children;
}

void Joint::select() {
    this->selected = true;
}

void Joint::unselect() {
    this->selected = false;
}

void Joint::create() {
    //draw sphere
    std::vector<glm::vec4> col;
    std::vector<glm::vec4> pos;
    std::vector<GLuint> idx;

    //y ring
    glm::vec4 rotAbtY = glm::vec4(0.5, 0, 0, 1);
    glm::mat4 rotateMat = glm::rotate(glm::mat4(), PI / 6.f, glm::vec3(0, 1, 0));

    glm::vec4 drawColor = selected ? glm::vec4(0.8, 0, 0.8, 1) : glm::vec4(1, 1, 1, 1);

    for (int i = 0; i < 12; i++) {
        pos.push_back(rotAbtY);
        col.push_back(drawColor);
        rotAbtY = rotateMat * rotAbtY;
    }

    //x ring
    glm::vec4 rotAbtX = glm::vec4(0, 0, 0.5, 1);
    rotateMat = glm::rotate(glm::mat4(), PI / 6.f, glm::vec3(1, 0, 0));

    for (int i = 0; i < 12; i++) {
        pos.push_back(rotAbtX);
        col.push_back(drawColor);
        rotAbtX = rotateMat * rotAbtX;
    }

    //z ring
    glm::vec4 rotAbtZ = glm::vec4(0, 0.5, 0, 1);
    rotateMat = glm::rotate(glm::mat4(), PI / 6.f, glm::vec3(0, 0, 1));

    for (int i = 0; i < 12; i++) {
        pos.push_back(rotAbtZ);
        col.push_back(drawColor);
        rotAbtZ = rotateMat * rotAbtZ;
    }

    for (int i = 0; i < 11; i++) {
        idx.push_back(i);
        idx.push_back(i + 1);
    }
    idx.push_back(11);
    idx.push_back(0);

    for (int i = 12; i < 23; i++) {
        idx.push_back(i);
        idx.push_back(i + 1);
    }
    idx.push_back(23);
    idx.push_back(12);

    for (int i = 24; i < 35; i++) {
        idx.push_back(i);
        idx.push_back(i + 1);
    }
    idx.push_back(35);
    idx.push_back(24);

    for (int i = 0; i < (int) pos.size(); i++) {
        pos[i] = this->getOverallTransformation() * pos[i];
    }

    int selfIdx = pos.size();
    if (parent != nullptr) {
        pos.push_back(parent->getOverallTransformation() * glm::vec4(0, 0, 0, 1));
        pos.push_back(this->getOverallTransformation() * glm::vec4(0, 0, 0, 1));
        col.push_back(glm::vec4(1, 0, 1, 1));
        col.push_back(glm::vec4(1, 1, 0, 1));
        idx.push_back(selfIdx);
        idx.push_back(selfIdx + 1);
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

GLenum Joint::drawMode() {
    return GL_LINES;
}

void Joint::posXRot() {
    this->rot = glm::angleAxis(glm::radians(5.f), glm::vec3(1, 0, 0)) * this->rot;
}

void Joint::posYRot() {
    this->rot = glm::angleAxis(glm::radians(5.f), glm::vec3(0, 1, 0)) * this->rot;
}

void Joint::posZRot() {
    this->rot = glm::angleAxis(glm::radians(5.f), glm::vec3(0, 0, 1)) * this->rot;
}

void Joint::negXRot() {
    this->rot = glm::angleAxis(glm::radians(-5.f), glm::vec3(1, 0, 0)) * this->rot;
}

void Joint::negYRot() {
    this->rot = glm::angleAxis(glm::radians(-5.f), glm::vec3(0, 1, 0)) * this->rot;
}

void Joint::negZRot() {
    this->rot = glm::angleAxis(glm::radians(-5.f), glm::vec3(0, 0, 1)) * this->rot;
}

void Joint::setX(float x) {
    this->pos_relative.x = x;
}

void Joint::setY(float y) {
    this->pos_relative.y = y;
}

void Joint::setZ(float z) {
    this->pos_relative.z = z;
}

int Joint::topId = -1;
