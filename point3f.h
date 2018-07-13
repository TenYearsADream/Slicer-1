#pragma once
#include<d3d9.h>
#include<math.h>
#include<windows.h>

//-----------------------------顶点结构体----------------------------------

struct Vertex //顶点结构
{
    Vertex() {}
    Vertex(float x, float y, float z)
    {
        _x = x;  _y = y;  _z = z;
    }
    float _x, _y, _z;
    static const DWORD FVF;
};

class Point3f
{
public:
    Point3f();
    Point3f(float _x,float _y,float _z);
    int SetParam(float _x, float _y, float _z);
    inline Vertex IVertex()
    {
        return Vertex(x, y, z);
    }
private:
    float x, y, z;
};
