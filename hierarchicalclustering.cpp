#include "hierarchicalclustering.h"
#include <qmath.h>
#include <limits>
#include <iostream>
using namespace std;
HierarchicalClustering::HierarchicalClustering(){

}
HierarchicalClustering::~HierarchicalClustering()
{

}

vector<vector<int>> HierarchicalClustering::Cluster(vector<vector<double> > dataset, double eps)
{
    vector<vector<int>> clusterTable(dataset.size());
    for(int i=0;i<dataset.size();i++)
    {
        vector<int> tmp;
        tmp.push_back(i);
        clusterTable[i]=(tmp);//聚类表保存索引
    }
    vector<vector<float> > dTable;
//    //计算两点之间距离
    for(int i=0;i<dataset.size();i++){
        vector<float> temp;
        for(int j=0;j<dataset.size();j++){
            if(j>i)
                temp.push_back(squareDistance(dataset[i][0],dataset[i][1],dataset[j][0],dataset[j][1]));
            else if(j<i)
                temp.push_back(dTable[j][i]);
            else
                temp.push_back(0);
        }
        dTable.push_back(temp);
    }
    float maxDt;
    do{
        //Merge two closest clusters
        int mi,mj;
        float minDt =FLT_MAX;
        maxDt=0;
        //寻找最小距离的两个点i和j
        for(int i=0;i<dTable.size();i++){
            for(int j=i+1;j<dTable[i].size();j++){
                 if(dTable[i][j]<minDt){
                    minDt = dTable[i][j];
                    mi = i;
                    mj = j;
                 }
                 if(dTable[i][j]>maxDt){
                    maxDt = dTable[i][j];
                 }
            }
        }
        for(int i=0;i<clusterTable[mj].size();i++)
        {
            clusterTable[mi].push_back(clusterTable[mj][i]);
        }
        clusterTable.erase(clusterTable.begin()+mj);
//        for(int i=0;i<dTable.size();i++)
//        {
//            for(int j=0;j<dTable[i].size();j++)
//            {
//                cout<<dTable[i][j]<<" ";
//            }
//            cout<<endl;
//        }
//        //Update the dTable
        for(int j=0;j<dTable.size();j++){
            if(j==mi||j==mj) continue;
            if(dTable[mi][j]>dTable[mj][j]){
                dTable[mi][j] = dTable[mj][j];
                dTable[j][mi] = dTable[mi][j];
            }
        }
        dTable.erase(dTable.begin()+mj);
        for(int i=0;i<dTable.size();i++){
            dTable[i].erase(dTable[i].begin()+mj);
        }
        if(clusterTable.size()<=1)
        {
            cout<<"one cluster"<<endl;
            break;
        }
        //cout<<"maxDt:"<<maxDt<<" "<<dTable.size()<<endl;
    }while( maxDt>eps);
//    for(int i=0;i<clusterTable.size();i++)
//    {
//        for(int j=0;j<clusterTable[i].size();j++)
//        {
//            cout<<clusterTable[i][j]<<" ";
//        }
//        cout<<endl;
//    }
    cout<<"number of cluster:"<<clusterTable.size()<<endl;
    return clusterTable;
}

float HierarchicalClustering::squareDistance(double x1, double y1,double x2,double y2)
{
    return qSqrt((x1-x2)*(x1-x2)+(y1-y1)*(y1-y2));
}
