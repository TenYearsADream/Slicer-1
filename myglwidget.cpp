#include "myglwidget.h"
#include <QApplication>
#include <QTimer>
#include <QDebug>
#include <iostream>
#include <string.h>
#include <QKeyEvent>
//#include <gl/GLU.h>
#include <QPushButton>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
using namespace std;
static GLdouble colorMap[][3]={
    {0.7,0.7,0.7},
    {1.0,0.0,0.0},
    {0.0,1.0,0.0},
    {0.0,0.0,1.0},
    {1.0,1.0,0.0},
    {1.0,0.0,1.0},
    {0.667,0.667,1.0},
    {1.0,0.5,0.0},
    {0.5,0.0,0.0},
    {1.0,0.8,0.8},
    {0.0,0.0,0.5},
};
MyGLWidget::MyGLWidget(QWidget *parent) :QOpenGLWidget(parent)
  ,xtrans(0.0),ytrans(0.0),ztrans(0.0)
{
    layer=0;
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update())); //不停刷新窗口
    timer->start(3);
}

MyGLWidget::~MyGLWidget(){
    delete program;
}

void MyGLWidget::initializeGL()
{
    initializeOpenGLFunctions();				//初始化
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);

    // vertex shader
    QOpenGLShader *vshader = new QOpenGLShader(QOpenGLShader::Vertex, this);
    vshader->compileSourceFile("F:/QT/Slicer/vert.txt");
    //geometry shader
    QOpenGLShader *gshader = new QOpenGLShader(QOpenGLShader::Geometry, this);
    gshader->compileSourceFile("F:/QT/Slicer/geom.txt");
    // fragment shader
    QOpenGLShader *fshader = new QOpenGLShader(QOpenGLShader::Fragment, this);
    fshader->compileSourceFile("F:/QT/Slicer/frag.txt");
    QOpenGLShader *sliceshader = new QOpenGLShader(QOpenGLShader::Fragment, this);
    sliceshader->compileSourceFile("F:/QT/Slicer/sliceFrag.txt");

    sliceProgram = new QOpenGLShaderProgram;
    sliceProgram->addShader(vshader);
    sliceProgram->addShader(sliceshader);
    sliceProgram->link();
    //sliceProgram->bind();

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
    QVector4D worldLight = QVector4D(0.0f,150.0f,0.0f,1.0f);
    program->setUniformValue("Material.Kd", 0.9f, 0.5f, 0.3f);
    program->setUniformValue("Light.Ld", 1.0f, 1.0f, 1.0f);
    program->setUniformValue("Light.Position", view * worldLight );
    program->setUniformValue("Material.Ka", 0.9f, 0.5f, 0.3f);
    program->setUniformValue("Light.La", 0.4f, 0.4f, 0.4f);
    program->setUniformValue("Material.Ks", 0.8f, 0.8f, 0.8f);
    program->setUniformValue("Light.Ls", 1.0f, 1.0f, 1.0f);
    program->setUniformValue("Material.Shininess", 100.0f);
    program->setUniformValue("ViewportMatrix", view);


}


void MyGLWidget::resizeGL(int width, int height)
{
    if (height==0) {    // Prevent A Divide By Zero By
        height=1;    // Making Height Equal One
    }
    glViewport(0, 0, width, height);    // Reset The Current Viewport
    projection.setToIdentity();
    projection.perspective(30.0f, (GLfloat)width/(GLfloat)height, 0.001f,1000.0f);
    //projection.ortho(-100.0,100.0,-100.0/((GLfloat)width/(GLfloat)height),100.0/((GLfloat)width/(GLfloat)height),0.01,1000.0);
}


void MyGLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // transform the model
    model.setToIdentity();
    model.translate(xtrans, -ytrans, ztrans);
    model.rotate(rotation);
    QMatrix4x4 mv = view * model;
    if(!intrpoints.empty())
    {
        if(!intrpoints[layer].empty())
        {
            sliceProgram->setUniformValue("ModelViewMatrix", mv);
            sliceProgram->setUniformValue("NormalMatrix", mv.normalMatrix());
            sliceProgram->setUniformValue("MVP", projection * mv);
            sliceProgram->bind();
            //for(int i=0;i<layer;i++)
                paintSlice(layer);
        }
    }
    else{
        program->setUniformValue("ModelViewMatrix", mv);
        program->setUniformValue("NormalMatrix", mv.normalMatrix());
        program->setUniformValue("MVP", projection * mv);
        program->bind();
        paintModel();
    }
}

void::MyGLWidget::paintSlice(int l)
{
    //cout<<intrpoints[l].Points.empty()<<endl;
    vector<GLushort> sliceindices;
    vector<GLfloat> slicevertices;
    for(list<Lines>::const_iterator iter = intrpoints[l].begin();iter != intrpoints[l].end();iter++)
    {
        for(uint j=0;j<(*iter).size();j++)
        {
            slicevertices.push_back((*iter).at(j).x());
            slicevertices.push_back((*iter).at(j).y());
            slicevertices.push_back((*iter).at(j).z());
        }
    }
    for(list<Lines>::const_iterator iter = intrpoints[l].begin();iter != intrpoints[l].end();iter++)
    {
        size_t n;
        if(iter!=intrpoints[l].begin())
            n=(*iter).size();
        else
            n=0;
        for(int j=0;j<(*iter).size();j++)
        {
            sliceindices.push_back(n+j);
            if(j+1==intrpoints[l].front().size())
            {
                sliceindices.push_back(n);
            }
            else
                sliceindices.push_back(n+j+1);
        }

    }

    unsigned int slicehandle[3];
    glGenBuffers(3, slicehandle);

    glEnableVertexAttribArray(0);  // Vertex position
    glBindBuffer(GL_ARRAY_BUFFER, slicehandle[0]);
    glVertexAttribPointer( (GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, ((GLubyte *)NULL + (0)) );

    glEnableVertexAttribArray(1);  // Vertex normal
    glBindBuffer(GL_ARRAY_BUFFER, slicehandle[1]);
    glVertexAttribPointer( (GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, ((GLubyte *)NULL + (0)) );

    glBindBuffer(GL_ARRAY_BUFFER, slicehandle[0]);
    glBufferData(GL_ARRAY_BUFFER, slicevertices.size()*sizeof(GLfloat),slicevertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, slicehandle[1]);
    glBufferData(GL_ARRAY_BUFFER, slicevertices.size()*sizeof(GLfloat),slicevertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, slicehandle[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,sliceindices.size()*sizeof(GLfloat),sliceindices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, slicehandle[2]);
    //开始绘制
    glDrawElements(GL_LINES,(GLsizei)sliceindices.size(), GL_UNSIGNED_SHORT,0);
}

void MyGLWidget::paintModel()
{
    unsigned int handle[3];
    glGenBuffers(3, handle);

    glEnableVertexAttribArray(0);  // Vertex position
    glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
    glVertexAttribPointer( (GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, ((GLubyte *)NULL + (0)) );

    glEnableVertexAttribArray(1);  // Vertex normal
    glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
    glVertexAttribPointer( (GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, ((GLubyte *)NULL + (0)) );

    glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
    glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(GLfloat),vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
    glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(GLfloat),vertices.data(), GL_STATIC_DRAW);
    if(!indices.empty())
    {
        if(clusterTable.empty())
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle[2]);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER,indices.size()*sizeof(GLushort),indices.data(), GL_STATIC_DRAW);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle[2]);
            program->setUniformValue("SegmentColor",QVector3D(colorMap[0][0],colorMap[0][1],colorMap[0][2]));
            program->setUniformValue("isSlice",false);
            //开始绘制
            glDrawElements(GL_TRIANGLES,(GLsizei)indices.size(), GL_UNSIGNED_SHORT, 0);
        }
        else
        {
            vector<GLushort> index;
            for(int i=0;i<clusterTable.size();i++)
            {
                vector<GLushort>().swap(index);
                for(int j=0;j<clusterTable[i].size();j++)
                {
                    index.push_back(indices[3*clusterTable[i][j]+0]);
                    index.push_back(indices[3*clusterTable[i][j]+1]);
                    index.push_back(indices[3*clusterTable[i][j]+2]);
                }
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle[2]);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER,index.size()*sizeof(GLushort),index.data(), GL_STATIC_DRAW);

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle[2]);
                program->setUniformValue("SegmentColor",QVector3D(colorMap[i][0],colorMap[i][1],colorMap[i][2]));
                program->setUniformValue("isSlice",false);
                //开始绘制
                glDrawElements(GL_TRIANGLES,(GLsizei)index.size(), GL_UNSIGNED_SHORT, 0);
            }
        }
    }
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
        xtrans +=diff.x()/10;
        ytrans +=diff.y()/10;
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
        //rotation = QQuaternion::fromAxisAndAngle(QVector3D(1.0f,0.0f,0.0f), 90) * rotation;
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

void MyGLWidget::setLayer(int l)
{
    layer=l-1;
}
