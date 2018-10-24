#ifndef MYGLWIDGET
#define MYGLWIDGET
#include <QGLWidget>
#include <qopenglwidget.h>
#include <qopenglfunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QMatrix4x4>
#include <string>
#include <fstream>
#include <sstream>
#include "hashtable.h"
#include "Slice.h"

class QTimer;
using namespace std;
class QOpenGLShaderProgram;
class MyGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    explicit MyGLWidget(QWidget *parent = 0);
    ~MyGLWidget();
    vector<GLushort> indices;
    vector<GLfloat> vertices;
    vector<Polylines> intrpoints;
    vector <vector<int>> clusterTable;
    GLfloat xtrans, ytrans, ztrans; // translation on x,y,z-axis
    int layer;
private:
    HDC hdc;
    QTimer *timer;
    QOpenGLShaderProgram *program,*sliceProgram;
private:
    //变换矩阵
    QMatrix4x4 mvpMatrix;
    QMatrix4x4 model;
    QMatrix4x4 view;
    QMatrix4x4 projection;

    QVector2D mousePos;
    QQuaternion rotation;
protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int w, int h);

    void mouseMoveEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);
    void wheelEvent(QWheelEvent *);

    void paintSlice(int layer);
    void paintModel();

public slots:
    void setLayer(int l);
};
#endif // MYGLWIDGET

