#pragma once
#include<vector>
#include <QString>
#include "hashtable.h"
using namespace std;
class ReadSTLFile
{
public:
    bool ReadStlFile(const QString filename);
    int NumTri();
    vector<Point> normalList;
    vector<vector<int>> faceList;
    HashTable *hashtable;
private:
    unsigned int unTriangles;
    bool ReadASCII(const char *cfilename);
    bool ReadBinary(const char *cfilename);

    char* memwriter;
    int cpyint(const char*& p);
    float cpyfloat(const char*& p);
};
