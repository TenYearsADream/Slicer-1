#include "hashtable.h"
#include <stdint.h>
#include <string.h>
#include <iostream>
using namespace std;
HashTable::HashTable(){
    size=0;
    vertices.resize(1000000);
}
int HashTable::hash(long long ID){
    int hash=0;
    hash=ID % 999983;
    return hash;
}

size_t HashTable::addPoint(long long ID,Point3f point){
    tableNode *tmp=new tableNode(ID,point);
    size_t index= hash(ID);
    tableNode *cur=vertices[index];
    if(cur==NULL)
    {
        vertices[index]=tmp;
        size++;
        return index;
    }
    else
    {
        return index;
    }
}
void HashTable::show()
{
    for(int i=0;i<vertices.size();i++)
    {
        tableNode *cur = vertices[i];
        if(cur!=NULL)
            cout<<cur->point.x<<" "<<cur->point.y<<" "<<cur->point.z<<endl;
    }
}
