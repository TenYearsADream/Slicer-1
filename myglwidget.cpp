#include "myglwidget.h"
#include <QApplication>
#include <QTimer>
#include <QDebug>
#include <iostream>
#include <string.h>
#include <QKeyEvent>
#include <gl/GLU.h>
#include <QPushButton>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <glm/gtc/type_ptr.hpp>
using namespace std;

MyGLWidget::MyGLWidget(QWidget *parent) :QOpenGLWidget(parent)
  ,xtrans(0.0),ytrans(0.0),ztrans(0.0)
{
    colorMap[0][0]=0.7;colorMap[0][1]=0.7;colorMap[0][2]= 0.7;
    colorMap[1][0]=0.0;colorMap[1][1]=1.0;colorMap[1][2]= 0.0;
    colorMap[2][0]=1.0;colorMap[2][1]=0.0;colorMap[2][2]= 0.0;
    colorMap[3][0]=0.0;colorMap[3][1]=0.0;colorMap[3][2]= 1.0;
    colorMap[4][0]=1.0;colorMap[4][1]=1.0;colorMap[4][2]= 0.0;
    colorMap[5][0]=0.0;colorMap[5][1]=1.0;colorMap[5][2]= 1.0;
    colorMap[6][0]=1.0;colorMap[6][1]=0.0;colorMap[6][2]= 0.5;
    colorMap[7][0]=0.0;colorMap[7][1]=0.5;colorMap[7][2]= 0.5;

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update())); //不停刷新窗口
    timer->start(2);
}

MyGLWidget::~MyGLWidget(){
    delete program;
}

void MyGLWidget::initializeGL()
{
    initializeOpenGLFunctions();				//初始化
    // vertex shader
    QOpenGLShader *vshader = new QOpenGLShader(QOpenGLShader::Vertex, this);
    vshader->compileSourceFile("F:/QT/Slicer/vert.txt");
    //geometry shader
    QOpenGLShader *gshader = new QOpenGLShader(QOpenGLShader::Geometry, this);
    gshader->compileSourceFile("F:/QT/Slicer/geom.txt");
    // fragment shader
    QOpenGLShader *fshader = new QOpenGLShader(QOpenGLShader::Fragment, this);
    fshader->compileSourceFile("F:/QT/Slicer/frag.txt");
    program = new QOpenGLShaderProgram;
    program->addShader(vshader);
    program->addShader(gshader);
    program->addShader(fshader);
    program->link();
    program->bind();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);					//开启深度缓存
    //glDepthFunc(GL_LEQUAL);//设置深度测试类型

    // view matrix
    view.setToIdentity();
    view.lookAt(QVector3D(0.0f, 0.0f,300.0f), QVector3D(0.0f,0.0f,0.0f), QVector3D(0.0f,1.0f,0.0f));

    // uniform light/material property
    program->setUniformValue("Kd", QVector3D(0.9f, 0.5f, 0.3f));
    program->setUniformValue("Ld", QVector3D(1.0f, 1.0f, 1.0f));
    program->setUniformValue("LightPosition", view * QVector4D(0.0f,0.0f,150.0f,1.0f) );
    program->setUniformValue("ViewportMatrix", view);
}


void MyGLWidget::resizeGL(int width, int height)
{
    if (height==0) {    // Prevent A Divide By Zero By
        height=1;    // Making Height Equal One
    }
    glViewport(0, 0, width, height);    // Reset The Current Viewport
    projection.setToIdentity();
    projection.perspective(45.0f, (GLfloat)width/(GLfloat)height, 0.001f,1000.0f);
}


void MyGLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 指定顶点属性数据 顶点位置 颜色 纹理
    GLfloat position[] = {
        0.0f, 0.0f, 0.0f,
        100.0f, 0.0f, 0.0f,
        100.0f, 100.0f, 0.0f,
        0.0f, 100.0f, 0.0f,

        0.0f, 0.0f, 100.0f,
        100.0f, 0.0f,100.0f,
        100.0f, 100.0f, 100.0f,
        0.0f, 100.0f, 100.0f,
    };
    GLfloat normal[] = {
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f,1.0f,1.0f,
        1.0f,1.0f,1.0f,
        1.0f,0.0f,0.0f,
        1.0f,1.0f,0.0f,
        1.0f,0.0f,1.0f,
    };
    GLushort indices[]={
        0,1,2,0,2,3,
        0,4,7,0,3,7,
        1,2,6,1,5,6,
        0,1,4,1,4,5,
        4,5,6,4,6,7,
        3,2,7,2,6,7,
    };

    unsigned int handle[3];
    glGenBuffers(3, handle);

    glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(position),position, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(normal), normal, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices),indices, GL_STATIC_DRAW);

    // Create the VAO
    vao.create();
    vao.bind();

    glEnableVertexAttribArray(0);  // Vertex position
    glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
    glVertexAttribPointer( (GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, ((GLubyte *)NULL + (0)) );

    glEnableVertexAttribArray(1);  // Vertex normal
    glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
    glVertexAttribPointer( (GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, ((GLubyte *)NULL + (0)) );

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle[2]);
    // transform the model
    model.setToIdentity();
    model.translate(xtrans, -ytrans, ztrans);
    model.rotate(rotation);
    QMatrix4x4 mv = view * model;
    program->setUniformValue("ModelViewMatrix", mv);
    program->setUniformValue("NormalMatrix", mv.normalMatrix());
    program->setUniformValue("MVP", projection * mv);
    program->bind();
    //开始绘制
    glDrawElements(GL_TRIANGLES,sizeof(indices)/sizeof(indices[0]), GL_UNSIGNED_SHORT, 0);
    //glDrawArrays(GL_TRIANGLES,0,3);

}

void MyGLWidget::wheelEvent(QWheelEvent *event)
{
    QPoint numDegrees = event->angleDelta() / 8;

     if (numDegrees.y() > 0) {
         ztrans += 5.0f;
     } else if (numDegrees.y() < 0) {
         ztrans -= 5.0f;
     }
     this->update();
     event->accept();
}

void MyGLWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        QVector2D newPos = (QVector2D)event->pos();
        QVector2D diff = newPos - mousePos;
        xtrans +=diff.x()/20;
        ytrans +=diff.y()/20;
        mousePos = newPos;
        update();
    }
    if(event->buttons() == Qt::RightButton)
    {
        QVector2D newPos = (QVector2D)event->pos();
        QVector2D diff = newPos - mousePos;
        qreal angle = (diff.length())/3.6;
        // Rotation axis is perpendicular to the mouse position difference
        // vector
        QVector3D rotationAxis = QVector3D(diff.y(), diff.x(), 0.0).normalized();
        rotation = QQuaternion::fromAxisAndAngle(rotationAxis, angle) * rotation;
        mousePos = newPos;
        this->update();
    }
   event->accept();
}

void MyGLWidget::mousePressEvent(QMouseEvent *event)
{
    mousePos = QVector2D(event->pos());
    event->accept();
}
