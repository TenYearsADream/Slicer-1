#ifndef SHAPEDIAMETERFUNCTION
#define SHAPEDIAMETERFUNCTION
#include <vector>
#include "point3f.h"
#include "hashtable.h"
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>
typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
typedef CGAL::Surface_mesh<Kernel::Point_3> Mesh;

using namespace std;
struct sdfValue
{
    double _sdfvalue,_facearea;
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
    vector<sdfValue> *charvalue;
    vector<vector<double>> calculateSDF(Mesh mesh);
private:
    vector<vector<double>> normalize(vector<vector<double>> charValue);
};

#endif // SHAPEDIAMETERFUNCTION

