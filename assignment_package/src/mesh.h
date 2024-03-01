#ifndef MESH_H
#define MESH_H

#include <la.h>
#include <smartpointerhelp.h>
#include <component.h>
#include <drawable.h>

class Mesh : public Drawable {
private:
    std::vector<uPtr<Vertex>> vertices;
    std::vector<uPtr<Face>> faces;
    std::vector<uPtr<HalfEdge>> hes;
    //quadrangulates a face using a face pointer, a centroid of that faces, and the hes pointing to midpoints on that face
    void quadrangulateFace(Face* f, Vertex* centroid, std::vector<HalfEdge*>& hes);

    //added for hw 7
    uPtr<Joint> joint;
    bool hasBeenSkinned;
public:
    Mesh();
    Mesh(OpenGLContext *context);
    Mesh(OpenGLContext *context, std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals, std::vector<std::vector<int>>& faces);

    void create() override;
    GLenum drawMode() override;

    void addFace(uPtr<Face> fPtr);
    void addVertex(uPtr<Vertex> vPtr);
    void addHE(uPtr<HalfEdge> hePtr);

    bool hasVertices() const;
    int calcTotalIndices();

    std::vector<uPtr<Vertex>>& getVertices();
    std::vector<uPtr<Face>>& getFaces();
    std::vector<uPtr<HalfEdge>>& getHes();

    Vertex &splitEdge(HalfEdge* he1);

    void triangulateFace(Face* f);
    void catmullclarkSubdivision();
    void extrudeFace(Face* toExtrude);

    //added for hw7
    void setRootJoint(uPtr<Joint> newRoot);
    void findVertJointsRec(Vertex* v, Joint* curr, std::array<Joint*, 2>& jointArr, std::array<float, 2>& dists);
    void skinMesh();
    bool hasJoints() const;
    Joint* getRoot() const;
    bool skinned() const;
};

//hash function for a pair
struct pair_hash {
    template <class T1, class T2>
    std::size_t operator () (const std::pair<T1, T2>& p) const {
        auto h1 = std::hash<T1>{}(p.first);
        auto h2 = std::hash<T2>{}(p.second);

        return h1 ^ h2;
    }
};

//hash function for a single object
struct single_hash {
    template <typename T>
    std::size_t operator () (const T& input) const {
        return std::hash<T>{}(input);
    }
};

#endif // MESH_H
