#ifndef JOINT_H
#define JOINT_H

#include <QString>
#include "drawable.h"
#include "smartpointerhelp.h"
#include "la.h"
#include <QTreeWidgetItem>
#include <cmath>
#include <iostream>

class Joint : public Drawable, public QTreeWidgetItem {
private:
    QString name;
    Joint* parent;
    std::vector<uPtr<Joint>> children;
    glm::vec3 pos_relative;
    glm::quat rot;
    glm::mat4 bindMat;
    bool selected;
    int id;
public:
    Joint();
    Joint(OpenGLContext *context);
    Joint(OpenGLContext *context, glm::vec3 pos);
    Joint(OpenGLContext *context, QString name, glm::vec3 pos, glm::quat rot);
    ~Joint();

    void addChild(uPtr<Joint> newChild);
    void setParent(Joint* p);

    void updateBindMat();
    glm::mat4 getBind();

    glm::mat4 getLocalTransformation() const;
    glm::mat4 getOverallTransformation() const;

    QString getName() const;
    int getId() const;

    void select();
    void unselect();

    std::vector<std::unique_ptr<Joint>>& getChildren();

    void create() override;
    GLenum drawMode() override;

    void posXRot();
    void posYRot();
    void posZRot();
    void negXRot();
    void negYRot();
    void negZRot();

    void setX(float x);
    void setY(float y);
    void setZ(float z);

    static int topId;
};

#endif // JOINT_H
