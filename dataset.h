#ifndef DATASET_H
#define DATASET_H
#include <vector>
#include <QObject>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>
using namespace std;
typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
typedef CGAL::Surface_mesh<Kernel::Point_3> Mesh;
typedef Kernel::Point_3 Point;
class dataSet
{
public:
        dataSet();
        ~dataSet();
        Mesh mesh;
        float surroundBox[6];
        vector<unsigned short> indices;
        vector<float> vertices;
        void rotateModel(int x,int y,int z);
        void getIndices();
private:
        void constructMesh(vector <Point> vertices,vector<vector<int>> faceList);


};

#endif // DATASET_H
