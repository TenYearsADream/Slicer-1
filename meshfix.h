#ifndef MESHFIX_H
#define MESHFIX_H
#include "dataset.h"
#include <QObject>
class MeshFix: public QObject
{
    Q_OBJECT
public:
    explicit MeshFix(QObject *parent=0);
    ~MeshFix();
    void repair(Mesh &mesh);
signals:
    void outputMsg(QString);
private:
    void fixConnectivity(Mesh *mesh);
    void stitchBorders(Mesh *mesh);
    void holeFill(Mesh *mesh);
    void normalRepair(Mesh *mesh);
    void selfIntersect(Mesh *mesh);
};

#endif // MESHFIX_H
