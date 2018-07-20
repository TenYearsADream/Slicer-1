#ifndef SHAPEDIAMETERFUNCTION
#define SHAPEDIAMETERFUNCTION
#include <vector>
#include "point3f.h"

using namespace std;

class ShapeDiameterFunction{
public:
    ShapeDiameterFunction();
    ~ShapeDiameterFunction();
    void ConstructMesh(vector<Point3f> pointList, int nFaceCount);
    void show();

private:
};

#endif // SHAPEDIAMETERFUNCTION

