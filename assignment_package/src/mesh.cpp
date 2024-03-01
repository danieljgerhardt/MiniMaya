#include <iostream>
#include <mesh.h>
#include <unordered_map>

Mesh::Mesh() : Drawable(nullptr), hasBeenSkinned(false) {}

Mesh::Mesh(OpenGLContext *context) : Drawable(context), hasBeenSkinned(false) {}

Mesh::Mesh(OpenGLContext *context, std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals, std::vector<std::vector<int>>& faces)
    : Drawable(context), hasBeenSkinned(false) {
    std::unordered_map<std::pair<Vertex*, Vertex*>, HalfEdge*, pair_hash> twinEdgeMap;

    for (int i = 0; i < (int)vertices.size(); i++) {
        uPtr<Vertex> vertex = mkU<Vertex>();
        vertex->pos = vertices.at(i);
        vertex->nor = normals.at(i);
        this->addVertex(std::move(vertex));
    }

    //hes and faces
    for (const std::vector<int>& faceIndices : faces) {
        int numEdges = faceIndices.size();
        std::vector<HalfEdge*> faceHalfEdges;

        uPtr<Face> face = mkU<Face>();

        for (int i = 0; i < numEdges; ++i) {
            int startVertexIdx = faceIndices[i];
            int endVertexIdx = faceIndices[(i + 1) % numEdges];

            //make halfedge, set its vertex, set the vertex's he, add the edge to the face
            uPtr<HalfEdge> he = mkU<HalfEdge>();
            he->setVertex(this->vertices[endVertexIdx].get());
            this->vertices[endVertexIdx]->he = he.get();
            faceHalfEdges.push_back(he.get());

            //use hash map to set syms
            int minIdx = std::min(startVertexIdx, endVertexIdx);
            int maxIdx = std::max(startVertexIdx, endVertexIdx);
            std::pair<Vertex*, Vertex*> key;

            key.first = this->vertices[minIdx].get();
            key.second = this->vertices[maxIdx].get();
            //check if edge is in map
            if (twinEdgeMap.find(key) == twinEdgeMap.end()) {
                //edge not in map
                twinEdgeMap.insert({key, he.get()});
            } else {
                //edge in map
                he->setSym(twinEdgeMap.at(key));
            }

            face->setEdge(he.get());

            //add half edge to mesh
            this->addHE(std::move(he));
        }

        //set nexts of half edges
        for (int i = 0; i < numEdges; ++i) {
            int nextIdx = (i + 1) % numEdges;
            faceHalfEdges[i]->setNext(faceHalfEdges[nextIdx]);
        }

        float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        float g = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        float b = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        face->color = glm::vec3(r, g, b);
        this->addFace(std::move(face));
    }
}

int Mesh::calcTotalIndices() {
    int num = 0;

    for (uPtr<Face>& fPtr : this->faces) {
        HalfEdge* faceEdge = fPtr->getHE();
        HalfEdge* next = faceEdge->next;
        while(next != faceEdge){
            next = next->next;
            num++;
        }
    }
    return 3 * (num - faces.size());
}

void Mesh::create() {

    int elems = hes.size(), idx_count = calcTotalIndices();
    std::vector<glm::vec4> col(elems);
    std::vector<glm::vec4> pos(elems);
    std::vector<glm::vec4> nor(elems);
    std::vector<GLuint> idx(idx_count);

    std::vector<glm::ivec2> ids(elems);
    std::vector<glm::vec2> weights(elems);

    //index is used for the first part of the loop, setting up pos col and nor
    int index = 0;
    //baseIdx and vertex_count are used for the second part of the loop, setting up idx
    int baseIdx = 0, vertex_count = 0;
    for (uPtr<Face>& facePtr : this->faces) {
        HalfEdge* faceHE = facePtr->getHE();

        Vertex *v1 = faceHE->v, *v2 = faceHE->next->v, *v3 = faceHE->next->next->v;
        glm::vec3 normal = glm::normalize(glm::cross(v1->pos - v3->pos, v2->pos - v3->pos));

        HalfEdge* loopHE = faceHE;
        do {
            pos[index] = glm::vec4(loopHE->v->pos, 1);
            col[index] = glm::vec4(facePtr->color, 1);
            nor[index] = glm::vec4(normal, 1);

            if (this->skinned()) {
                std::vector<std::pair<Joint*, float>>& currInfluences = loopHE->v->getInfluences();
                glm::ivec2 newId = glm::ivec2();
                glm::vec2 newWeight = glm::vec2();
                newId[0] = currInfluences[0].first->getId() - 1;
                newId[1] = currInfluences[1].first->getId() - 1;
                newWeight[0] = currInfluences[0].second;
                newWeight[1] = currInfluences[1].second;
                ids[index] = newId;
                weights[index] = newWeight;
            }

            loopHE = loopHE->next;
            index++;
        } while (loopHE != faceHE);

        //loop part 2! time for idx
        //step 1 - count da verts
        int numVerts = 0;
        loopHE = faceHE;
        do {
            numVerts++;
            loopHE = loopHE->getNext();
        } while (loopHE != faceHE);

        //step 2 - use vert count to place verts in the right spot in idx
        for (int j = 2; j < numVerts; j++) {
            int startIdx = baseIdx * 3;
            idx[startIdx] = vertex_count;
            idx[startIdx + 1] = vertex_count + j - 1;
            idx[startIdx + 2] = vertex_count + j;
            baseIdx++;
        }
        //accumulate the total vertex count so far to start in the right place next time
        vertex_count += numVerts;
    }

    count = calcTotalIndices();

    if (this->skinned()) {
        generateIds();
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufIds);
        mp_context->glBufferData(GL_ARRAY_BUFFER, ids.size() * sizeof(glm::ivec2), ids.data(), GL_STATIC_DRAW);

        generateWeights();
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufWeights);
        mp_context->glBufferData(GL_ARRAY_BUFFER, weights.size() * sizeof(glm::vec2), weights.data(), GL_STATIC_DRAW);
    }

    generateIdx();
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(GLuint), idx.data(), GL_STATIC_DRAW);

    generatePos();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufPos);
    mp_context->glBufferData(GL_ARRAY_BUFFER, pos.size() * sizeof(glm::vec4), pos.data(), GL_STATIC_DRAW);

    generateNor();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufNor);
    mp_context->glBufferData(GL_ARRAY_BUFFER, nor.size() * sizeof(glm::vec4), nor.data(), GL_STATIC_DRAW);

    generateCol();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufCol);
    mp_context->glBufferData(GL_ARRAY_BUFFER, col.size() * sizeof(glm::vec4), col.data(), GL_STATIC_DRAW);
}

GLenum Mesh::drawMode() {
    return GL_TRIANGLES;
}

void Mesh::addFace(uPtr<Face> fPtr) {
    this->faces.push_back(std::move(fPtr));
}

void Mesh::addVertex(uPtr<Vertex> vPtr) {
    this->vertices.push_back(std::move(vPtr));
}

void Mesh::addHE(uPtr<HalfEdge> hePtr) {
    this->hes.push_back(std::move(hePtr));
}

bool Mesh::hasVertices() const {
    return !this->vertices.empty();
}

std::vector<uPtr<Vertex>>& Mesh::getVertices() {
    return this->vertices;
}

std::vector<uPtr<Face>>& Mesh::getFaces() {
    return this->faces;
}

std::vector<uPtr<HalfEdge>>& Mesh::getHes() {
    return this->hes;
}

Vertex& Mesh::splitEdge(HalfEdge* he1) {
    HalfEdge* he2 = he1->sym;
    Vertex* v1 = he1->v;
    Vertex* v2 = he2->v;
    uPtr<Vertex> v3 = mkU<Vertex>();
    v3->pos = (v2->pos + v1->pos) / 2.f;
    //v3 has average pos of vert before and after he
    uPtr<HalfEdge> he1b = mkU<HalfEdge>(), he2b = mkU<HalfEdge>();

    he1b->v = v1;
    he2b->v = v2;

    v1->he = he1b.get();
    v2->he = he2b.get();

    he1b->setFace(he1->face);
    he2b->setFace(he2->face);

    he1b->next = he1->next;
    he2b->next = he2->next;

    he1->next = he1b.get();
    he2->next = he2b.get();

    he1->v = v3.get();
    he2->v = v3.get();

    v3->he = he1;

    he1->setSym(he2b.get());
    he2->setSym(he1b.get());

    Vertex &ret = *v3;

    this->addVertex(std::move(v3));
    this->addHE(std::move(he1b));
    this->addHE(std::move(he2b));

    return ret;
}

void Mesh::triangulateFace(Face* f) {
    int numVerts = 0;
    HalfEdge* loopHE = f->he;
    do {
        numVerts++;
        loopHE = loopHE->getNext();
    } while (loopHE != f->he);

    if (numVerts <= 3) {
        return;
    }

    HalfEdge* he_0 = f->he;

    for (int i = 0; i < numVerts - 3; i++) {
        uPtr<HalfEdge> he_a = mkU<HalfEdge>(), he_b = mkU<HalfEdge>();

        he_a->v = he_0 ->v;
        he_b->v = he_0->next->next->v;
        he_a->setSym(he_b.get());
        uPtr<Face> f2 = mkU<Face>();
        he_a->setFace(f2.get());
        he_0->next->setFace(f2.get());
        he_0->next->next->setFace(f2.get());
        he_b->setFace(f);
        he_b->next = he_0->next->next->next;
        he_0->next->next->next = he_a.get();
        he_a->next = he_0->next;
        he_0->next = he_b.get();

        f2->color = f->color;

        this->addHE(std::move(he_a));
        this->addHE(std::move(he_b));
        this->addFace(std::move(f2));

        he_0 = he_0->next;
    }
}

void Mesh::catmullclarkSubdivision() {

    std::vector<Vertex*> originalVertices;
    for(auto& v : this->vertices) {
        originalVertices.push_back(v.get());
    }

    //step 1: calculate centroids
    std::unordered_map<Face*, Vertex*, single_hash> faceCentroidMap;
    for (uPtr<Face>& fPtr : faces) {
        float numVerts = 0.f;
        glm::vec3 avgPos;
        HalfEdge* loopHE = fPtr->he;
        do {
            //counts number of vertices, gets the pos to add to avgPog, and goes to next he
            numVerts++;
            avgPos += loopHE->getVertex()->pos;
            loopHE = loopHE->getNext();
        } while (loopHE != fPtr->he);
        avgPos /= numVerts;
        uPtr<Vertex> vPtr = mkU<Vertex>();
        vPtr->pos = avgPos;
        faceCentroidMap.insert({fPtr.get(), vPtr.get()});
        this->addVertex(std::move(vPtr));
    }

    //step 2: calculate smoothed midpoints
    //e = (v1 + v2 + f1 + f2) / 4
    //midpoints should be pushed back
    //map with edges and vertex so i don't split the same edge twice
    //add the edge and its sym to the map when its split
    std::unordered_map<HalfEdge*, Vertex*, single_hash> heMidptMap;
    std::vector<HalfEdge*> originalHEs;
    for (auto& hePtr : hes) {
        originalHEs.push_back(hePtr.get());
    }
    for (HalfEdge* hePtr : originalHEs) {
        //if haven't already split this HE
        if (heMidptMap.find(hePtr) == heMidptMap.end()) {
            Vertex* v1 = hePtr->v;
            Vertex* v2 = hePtr->sym->v;
            Vertex* f1 = faceCentroidMap.at(hePtr->face);
            Vertex* f2 = faceCentroidMap.at(hePtr->sym->face);
            heMidptMap.insert({hePtr, nullptr});
            heMidptMap.insert({hePtr->sym, nullptr});
            Vertex &newVert = this->splitEdge(hePtr);
            newVert.pos = (v1->pos + v2->pos + f1->pos + f2->pos) / 4.f;
        }
    }

    //step 3: smooth the original vertices
    //v' = (n-2)v/n +
    //     sum(e)/n^2 +
    //     sum(f)/n^2
    for (Vertex* vPtr : originalVertices) {
        glm::vec3 sumCentroids = glm::vec3();
        glm::vec3 sumMidpts = glm::vec3();

        HalfEdge *loopHE = vPtr->he;

        //loopHE should now be the he pointing into the vertex

        float n = 0.f;
        do {
            Vertex* c = faceCentroidMap.at(loopHE->face);
            sumCentroids += c->pos;

            Vertex* midpt = loopHE->sym->v;
            sumMidpts += midpt->pos;

            loopHE = loopHE->next;
            loopHE = loopHE->sym;

            n++;
        } while (loopHE != vPtr->he);
        //smooth vertex using equation from slides
        vPtr->pos = (n - 2.f) * vPtr->pos / n +
                     sumMidpts / (n * n) +
                     sumCentroids / (n * n);
    }

    std::vector<Face*> originalFaces;
    for(auto& f : this->faces) {
        originalFaces.push_back(f.get());
    }

    //step 4: quadrangulate faces
    //face's he should be pointing to an og vertex, not a midpt
    for (Face* fPtr : originalFaces) {
        std::vector<HalfEdge*> hesToMidpts;
        HalfEdge* loopHE = fPtr->he->next;
        do {
            hesToMidpts.push_back(loopHE);
            loopHE = loopHE->getNext()->getNext();
        } while (loopHE != fPtr->he->next);
        //use method idea from slides
        quadrangulateFace(fPtr, faceCentroidMap.at(fPtr), hesToMidpts);
    }
}


void Mesh::quadrangulateFace(Face* f, Vertex* centroid, std::vector<HalfEdge*>& hesToMidpts) {
    HalfEdge *prevTo, *firstFrom;
    bool firstIter = true;
    for (HalfEdge* hePtr : hesToMidpts) {
        HalfEdge* first = hePtr;

        //sym next until the next of the one we are on is the original
        HalfEdge* prev = hePtr;
        do {
            prev = prev->sym;
            if (prev->next != hePtr) {
                prev = prev->next;
            }
        } while (prev->next != hePtr);

        uPtr<HalfEdge> toCentroid = mkU<HalfEdge>(), fromCentroid = mkU<HalfEdge>();
        uPtr<Face> newFace = mkU<Face>();
        Face* faceToMake = firstIter ? f : newFace.get();

        //reset first's next
        first->next = toCentroid.get();

        //set to edge's next and v and the centroid's he
        toCentroid->v = centroid;
        toCentroid->next = fromCentroid.get();
        centroid->he = toCentroid.get();

        //set from edge's next and v
        fromCentroid->next = prev;
        fromCentroid->v = prev->sym->v;

        //faces
        toCentroid->setFace(faceToMake);
        fromCentroid->setFace(faceToMake);
        first->setFace(faceToMake);
        prev->setFace(faceToMake);

        //color
        faceToMake->color = f->color;

        if (!firstIter) {
            //set syms of to and from centroid edges
            fromCentroid->setSym(prevTo);
        } else {
            //it is the first iteration, set the first from edge for setting its sym
            //at the end and set firstIter to false
            firstFrom = fromCentroid.get();
        }

        prevTo = toCentroid.get();

        //add stuff to mesh
        if (!firstIter) {
            this->addFace(std::move(newFace));
        } else {
            firstIter = false;
        }
        this->addHE(std::move(toCentroid));
        this->addHE(std::move(fromCentroid));
    }
    firstFrom->setSym(prevTo);
}

void Mesh::extrudeFace(Face* f) {
    std::vector<HalfEdge*> faceHes;
    HalfEdge* loopHE = f->he;
    std::vector<Vertex*> ogVerts;

    //loop gets verts for normal calculation and stores the face's hes in faceHes
    do {
        loopHE = loopHE->next;
        faceHes.push_back(loopHE);
    } while (loopHE != f->he);

    //calculate normal
    glm::vec3 normal = glm::normalize(glm::cross(f->he->next->v->pos - f->he->v->pos,
                                                 f->he->next->next->v->pos - f->he->next->v->pos));

    //make one new vertex for each he
    std::vector<Vertex*> newVerts;
    for (int i = 0; i < (int)faceHes.size(); i++) {
        uPtr<Vertex> newVert = mkU<Vertex>();
        newVerts.push_back(newVert.get());
        this->addVertex(std::move(newVert));
    }

    //vector for edges that will be in the top face
    std::vector<HalfEdge*> topEdges;

    bool firstIter = true;
    HalfEdge *prevHE2, *firstHE1;

    //make a face for each edge in faceHes
    for (int i = 0; i < (int)faceHes.size(); i++) {
        HalfEdge* ogHe = faceHes[i];
        Vertex* ogV1 = ogHe->sym->v;
        Vertex* ogV2 = ogHe->v;
        uPtr<Face> newFace = mkU<Face>();
        Vertex *v1 = newVerts[i], *v2 = newVerts[(i + 1) % newVerts.size()];

        //he1's sym will have to be set to the prev he2
        //he1 points into ogHe base vert
        //he2 points out of ogHe vert
        //he3 points into base of he1(top edge
        uPtr<HalfEdge> he1 = mkU<HalfEdge>(), he2 = mkU<HalfEdge>(), he3 = mkU<HalfEdge>();

        he1->v = ogV1;
        he2->v = v2;
        he3->v = v1;

        v1->pos = ogV1->pos + normal * 0.5f;
        v2->pos = ogV2->pos + normal * 0.5f;

        v1->he = he3.get();
        v2->he = he2.get();

        //set nexts of edges
        ogHe->setNext(he2.get());
        he2->setNext(he3.get());
        he3->setNext(he1.get());
        he1->setNext(ogHe);

        //set face pointers
        ogHe->setFace(newFace.get());
        he1->setFace(newFace.get());
        he2->setFace(newFace.get());
        he3->setFace(newFace.get());

        //he3 will need to be kept track of for making the top face
        topEdges.push_back(he3.get());

        if (firstIter) {
            firstIter = false;
            firstHE1 = he1.get();
        } else {
            prevHE2->setSym(he1.get());
        }

        prevHE2 = he2.get();

        newFace->color = f->color;

        //add stuff to mesh
        this->addFace(std::move(newFace));
        this->addHE(std::move(he1));
        this->addHE(std::move(he2));
        this->addHE(std::move(he3));
    }

    firstHE1->setSym(prevHE2);

    //handle top face
    std::vector<HalfEdge*> newTopEdges;
    for (int i = 0; i < (int)topEdges.size(); i++) {
        uPtr<HalfEdge> newTopEdge = mkU<HalfEdge>();
        newTopEdge->setFace(f);
        newTopEdge->setSym(topEdges[i]);
        newTopEdge->v = topEdges[i]->next->next->next->v;
        newTopEdges.push_back(newTopEdge.get());
        this->addHE(std::move(newTopEdge));
    }

    //set nexts for top face
    for (int i = 0; i < (int)newTopEdges.size(); i++) {
        newTopEdges[i]->next = newTopEdges[(i + 1) % newTopEdges.size()];
    }
}

void Mesh::setRootJoint(uPtr<Joint> newRoot) {
    this->joint = std::move(newRoot);
}

void Mesh::findVertJointsRec(Vertex* v, Joint* curr, std::array<Joint*, 2>& jointArr, std::array<float, 2>& dists) {
    curr->updateBindMat();

    float currDist = glm::distance(glm::vec4(v->getPos(), 1), curr->getOverallTransformation() * glm::vec4(0,0,0,1));
    if (currDist < std::max(dists[0], dists[1])) {
        if (dists[0] > dists[1]) {
            dists[0] = currDist;
            jointArr[0] = curr;
        } else {
            dists[1] = currDist;
            jointArr[1] = curr;
        }
    }
    for (uPtr<Joint>& child : curr->getChildren()) {
        findVertJointsRec(v, child.get(), jointArr, dists);
    }
}

void Mesh::skinMesh() {
    for (int i = 0; i < (int) vertices.size();i++) {
        std::array<Joint*, 2> joints;
        std::array<float, 2> dists = {FLT_MAX, FLT_MAX};

        findVertJointsRec(vertices[i].get(), this->joint.get(), joints, dists);

        float total = dists[0] + dists[1];
        dists[0] /= total;
        dists[1] /= total;

        vertices[i]->addInfluence(joints[0], dists[0]);
        vertices[i]->addInfluence(joints[1], dists[1]);
    }
    this->hasBeenSkinned = true;
}

bool Mesh::hasJoints() const {
    return !(this->joint == nullptr);
}

Joint* Mesh::getRoot() const {
    return this->joint.get();
}

bool Mesh::skinned() const {
    return this->hasBeenSkinned;
}
