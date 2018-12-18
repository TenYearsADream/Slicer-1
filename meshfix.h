#ifndef MESHFIX_H
#define MESHFIX_H
#include "dataset.h"

class MeshFix
{
public:
    MeshFix(Mesh *_mesh);
    ~MeshFix();
    bool fixConnectivity();

private:
    int removeVertices();
    void stitchBorders();
    int holeFill();
    void normalRepair();
private:
    Mesh *mesh;
};

#endif // MESHFIX_H
