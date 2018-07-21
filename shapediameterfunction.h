#ifndef SHAPEDIAMETERFUNCTION
#define SHAPEDIAMETERFUNCTION
#include <vector>
#include "point3f.h"
#include "hashtable.h"
using namespace std;

class ShapeDiameterFunction{
public:
    ShapeDiameterFunction();
    ~ShapeDiameterFunction();
    vector<vector<double>> calculateSDF(vector <tableNode *> vertices, vector<vector<size_t>> faceList);
};

#endif // SHAPEDIAMETERFUNCTION

