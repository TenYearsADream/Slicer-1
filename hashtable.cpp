#include "hashtable.h"
#include <stdint.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <qmath.h>
using namespace std;
HashTable::HashTable(){
    size=0;
}

int HashTable::addPoint(QString key,Point point){
    int index;
    auto it = verticesmap.find(key);
    if(it != verticesmap.end())
    {
        index=it.value();
        //cout<<"索引："<<it.value()<<endl;
    }
    else
    {
        vertices.push_back(point);
        verticesmap.insert(key,size);
        index=size;
        size++;
    }
    return index;
}
void HashTable::show()
{
    //cout<<verticesmap.size()<<endl;
    for(int i=0;i<vertices.size();i++)
    {
        cout<<vertices[i].x()<<" "<<vertices[i].y()<<" "<<vertices[i].z()<<endl;
    }
}
