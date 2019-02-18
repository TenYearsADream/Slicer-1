#ifndef HIERARCHICALCLUSTERING
#define HIERARCHICALCLUSTERING
#include <vector>

using namespace std;
class HierarchicalClustering{
public:
    HierarchicalClustering();
    ~HierarchicalClustering();
    vector<vector<int>>  Cluster(vector<vector<double>> dataset,double eps);
private:
    float squareDistance(double x1,double y1,double x2,double y2);
};
#endif // HIERARCHICALCLUSTERING

