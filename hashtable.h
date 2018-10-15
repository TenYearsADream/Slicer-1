#ifndef HASHTABLE
#define HASHTABLE
#include <vector>
#include <QHash>
#include <QString>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
typedef Kernel::Point_3 Point;
using namespace std;
class HashTable{
private:
    QHash<QString,int> verticesmap;
public:
    int size;
    vector <Point> vertices;
    HashTable();
    int addPoint(QString key,Point point);
    void show();
};
#endif // HASHTABLE

