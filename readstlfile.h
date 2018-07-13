#pragma once
#include<vector>
#include"point3f.h"
using namespace std;

class ReadSTLFile
{
public:
    bool ReadStlFile(const char *cfilename);
    int NumTri();
    vector<Point3f>& PointList();
private:
    vector<Point3f> pointList;
    unsigned int unTriangles;
    bool ReadASCII(const char *cfilename);
    bool ReadBinary(const char *cfilename);

    char* memwriter;
    int cpyint(const char*& p);
    float cpyfloat(const char*& p);
};
