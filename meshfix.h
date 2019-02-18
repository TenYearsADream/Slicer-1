#ifndef MESHFIX_H
#define MESHFIX_H
#include "dataset.h"

class MeshFix
{
public:
    MeshFix(Mesh *_mesh);
    ~MeshFix();
    void repair();

private:
    void fixConnectivity();
    void stitchBorders();
    void holeFill();
    void normalRepair();
    void selfIntersect();
private:
    Mesh *mesh;
};

#endif // MESHFIX_H
