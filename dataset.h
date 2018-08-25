#ifndef DATASET_H
#define DATASET_H
#include <vector>
#include <QObject>
#include "hashtable.h"
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>
typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
typedef CGAL::Surface_mesh<Kernel::Point_3> Mesh;
class dataSet
{
public:
        dataSet(vector <Point> vertices,vector<vector<int>> faceList);
        ~dataSet();
        Mesh mesh;
        float surroundBox[6];
        vector<unsigned short> indices;
        vector<float> vertices;
        void rotateModel(int x,int y,int z);
private:
        void constructMesh(vector <Point> vertices,vector<vector<int>> faceList);
        void getIndices();

};

#endif // DATASET_H
