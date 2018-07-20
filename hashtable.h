#ifndef HASHTABLE
#define HASHTABLE
#include "point3f.h"
#include <vector>
using namespace std;
struct tableNode{
    long long ID;
    Point3f point;
    tableNode(const long long ID,Point3f &point):ID(ID),point(point){}
};

class HashTable{
private:
    int hash(long long ID);
public:
    size_t size;
    vector <tableNode *> vertices;
    HashTable();
    size_t addPoint(long long ID,Point3f point);
    void show();
};
#endif // HASHTABLE

