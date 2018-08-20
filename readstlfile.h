#pragma once
#include<vector>
#include"point3f.h"
#include "hashtable.h"

using namespace std;

class ReadSTLFile
{
public:
    bool ReadStlFile(const char *cfilename);
    int NumTri();
    vector<Point3f> normalList;
    vector<vector<size_t>> faceList;
    float surroundBox[6];
    HashTable *hashtable;
private:
    unsigned int unTriangles;
    bool ReadASCII(const char *cfilename);
    bool ReadBinary(const char *cfilename);

    char* memwriter;
    int cpyint(const char*& p);
    float cpyfloat(const char*& p);
};
