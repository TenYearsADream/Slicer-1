﻿#ifndef MYGLWIDGET
#define MYGLWIDGET
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QMatrix4x4>
#include <QVector2D>
#include <string>
#include <fstream>
#include <sstream>
#include "dataset.h"

class QTimer;
class QOpenGLShaderProgram;
class MyGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    explicit MyGLWidget(QWidget *parent = 0);
    ~MyGLWidget();
    vector<uint> indices;
    vector<float> vertices;
    vector<float>vertexnormals;
    vector<Polylines> intrpoints;
    vector <vector<int>> clusterTable;
    GLfloat xtrans, ytrans, ztrans,scale; // translation on x,y,z-axis
    int layer;
private:
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

    void paintSlice(size_t layer);
    void paintModel();

public slots:
    void setLayer(int l);
};
#endif // MYGLWIDGET

