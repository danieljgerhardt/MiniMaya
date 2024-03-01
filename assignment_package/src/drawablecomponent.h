#ifndef DRAWABLECOMPONENT_H
#define DRAWABLECOMPONENT_H

#include <drawable.h>
#include "component.h"

class VertexDisplay : public Drawable {
protected:
    Vertex *representedVertex;

public:
    VertexDisplay();
    VertexDisplay(OpenGLContext *context, Vertex*);
    ~VertexDisplay();
    // Creates VBO data to make a visual
    // representation of the currently selected Vertex
    void create() override;
    // Change which Vertex representedVertex points to
    void updateVertex(Vertex*);
    GLenum drawMode() override;

    HalfEdge* getHE() const;

    friend class Vertex;
};

class HalfEdgeDisplay : public Drawable {
protected:
    HalfEdge *representedHE;

public:
    HalfEdgeDisplay();
    HalfEdgeDisplay(OpenGLContext *context, HalfEdge*);
    ~HalfEdgeDisplay();
    // Creates VBO data to make a visual
    // representation of the currently selected HalfEdge
    void create() override;
    // Change which HalfEdge representedHE points to
    void updateHE(HalfEdge*);
    void makeNext();
    void makeSym();
    GLenum drawMode() override;

    Face* getFace() const;
    Vertex* getVertex() const;

    friend class HalfEdge;
};

class FaceDisplay : public Drawable {
protected:
    Face *representedFace;

public:
    FaceDisplay();
    FaceDisplay(OpenGLContext *context, Face*);
    ~FaceDisplay();
    // Creates VBO data to make a visual
    // representation of the currently selected Face
    void create() override;
    // Change which Face representedFace points to
    void updateFace(Face*);
    GLenum drawMode() override;

    HalfEdge* getHE() const;

    friend class Face;
};

#endif // DRAWABLECOMPONENT_H
