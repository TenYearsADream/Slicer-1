#ifndef SHAPEDIAMETERFUNCTION
#define SHAPEDIAMETERFUNCTION
#include <vector>
#include "point3f.h"
#include "hashtable.h"

using namespace std;
struct sdfValue
{
    double _sdfvalue,_facearea;
    sdfValue(){};
    sdfValue(double sdfvalue,double facearea)
    {
        _sdfvalue=sdfvalue;
        _facearea=facearea;
    }
};

class ShapeDiameterFunction{
public:
    ShapeDiameterFunction();
    ~ShapeDiameterFunction();
    vector<sdfValue> charvalue;
    vector<vector<double>> calculateSDF(vector <tableNode *> vertices, vector<vector<size_t>> faceList);
    void constructMesh(vector <tableNode *> vertices,vector<vector<size_t>> faceList);
private:
    int getIndex(vector<vector<int>> index,int ID);
    vector<vector<double>> normalize(vector<vector<double>> charValue);
};

#endif // SHAPEDIAMETERFUNCTION

