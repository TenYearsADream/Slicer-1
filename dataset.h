#ifndef DATASET_H
#define DATASET_H
#include <vector>
#include "point3f.h"
#include "hashtable.h"
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>
typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
typedef CGAL::Surface_mesh<Kernel::Point_3> Mesh;
class dataSet
{
public:
        dataSet(vector <tableNode *> vertices,vector<vector<size_t>> faceList);
        ~dataSet();
        Mesh mesh;
        float surroundBox[6];
        vector<unsigned short> indices;
        vector<float> vertices;
private:
        void constructMesh(vector <tableNode *> vertices,vector<vector<size_t>> faceList);
        int getIndex(vector<vector<int> > index, int ID);
};

#endif // DATASET_H
