﻿#ifndef MYGLWIDGET
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
#include "point3f.h"
#include "hashtable.h"
#include <GL/GLU.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

class QTimer;
using namespace std;
class QOpenGLShaderProgram;
class MyGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    explicit MyGLWidget(QWidget *parent = 0);
    ~MyGLWidget();
    vector<GLushort> faceList;
    vector<GLfloat> vertices;
    vector <int> colorFlag;
    int nFaceCount;
    GLfloat m_xMove;
    GLfloat m_yMove;
    GLfloat zoom;
private:
    HDC hdc;
    QTimer *timer;
    GLfloat colorMap[8][3];
    QOpenGLShaderProgram *program;
    QOpenGLBuffer vbo;
    QOpenGLVertexArrayObject vao;
private:
    GLfloat angle;
    //变换矩阵
    QMatrix4x4 mvpMatrix;
    QMatrix4x4 model;
    QMatrix4x4 view;
    QMatrix4x4 projection;

    GLfloat xtrans, ytrans, ztrans; // translation on x,y,z-axis
    QVector2D mousePos;
    QQuaternion rotation;
protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int w, int h);

    void mouseMoveEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);
    void wheelEvent(QWheelEvent *);

};
#endif // MYGLWIDGET

