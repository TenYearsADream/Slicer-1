#include "myglwidget.h"
#include <QApplication>
#include <QTimer>
#include <QDebug>
#include <QKeyEvent>
#include <gl/GLU.h>
#include <QPushButton>
using namespace std;

MyGLWidget::MyGLWidget(QWidget *parent) :
    QGLWidget(parent)
{
    zoom=-10;
    rQuad = 0.0;
    colorMap[0][0]=0.7;colorMap[0][1]=0.7;colorMap[0][2]= 0.7;
    colorMap[1][0]=0.0;colorMap[1][1]=1.0;colorMap[1][2]= 0.0;
    colorMap[2][0]=1.0;colorMap[2][1]=0.0;colorMap[2][2]= 0.0;
    colorMap[3][0]=0.0;colorMap[3][1]=0.0;colorMap[3][2]= 1.0;
    colorMap[4][0]=1.0;colorMap[4][1]=1.0;colorMap[4][2]= 0.0;
    colorMap[5][0]=0.0;colorMap[5][1]=1.0;colorMap[5][2]= 1.0;
    colorMap[6][0]=1.0;colorMap[6][1]=0.0;colorMap[6][2]= 0.5;
    colorMap[7][0]=0.0;colorMap[7][1]=0.5;colorMap[7][2]= 0.5;

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateGL())); //不停刷新窗口
    timer->start(5);
}

void MyGLWidget::initializeGL()
{
    glEnable( GL_TEXTURE_2D );//启用纹理

    glShadeModel(GL_SMOOTH);   // Enables Smooth Shading
    glClearColor(1.0f, 1.0f, 0.6f, 1.0f);  // White Background
    glClearDepth(1.0f);             // Depth Buffer Setup
    glEnable(GL_DEPTH_TEST);        // Enables Depth Testing
    glDepthFunc(GL_LEQUAL);        // The Type Of Depth Test To Do
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); // Really Nice Perspective Calculations

}

void MyGLWidget::resizeGL(int width, int height)
{
    if (height==0) {    // Prevent A Divide By Zero By
        height=1;    // Making Height Equal One
    }
    glViewport(0, 0, width, height);    // Reset The Current Viewport
    glMatrixMode(GL_PROJECTION);       // Select The Projection Matrix
    glLoadIdentity();                  // Reset The Projection Matrix

    // Calculate The Aspect Ratio Of The Window
    gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,1.0f,zoom);

    glMatrixMode(GL_MODELVIEW);      // Select The Modelview Matrix
    glLoadIdentity();                // Reset The Modelview Matrix
}

void MyGLWidget::paintGL()
{

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  // Clear The Screen And The Depth Buffer
    glLoadIdentity();       // Reset The Current Modelview Matrix
    glTranslatef(  m_xMove,  -m_yMove, zoom );
    //qDebug()<<zoom<<endl;
    glRotatef(m_xRot / 16.0f, 1, 0, 0);
    glRotatef(m_yRot / 16.0f, 0, 1, 0);
    glRotatef(m_zRot / 16.0f, 0, 0, 1);
    glRotatef( rQuad,  1.0,  1.0,  1.0 );
    glBegin( GL_TRIANGLES );
        for (int i=0;i<faceList.size();i++)
        {
           tableNode *v1 = vertices[faceList[i][0]];
           tableNode *v2 = vertices[faceList[i][1]];
           tableNode *v3 = vertices[faceList[i][2]];
           {
               glColor3f( colorMap[colorFlag[i]][0], colorMap[colorFlag[i]][1], colorMap[colorFlag[i]][2] );
               glVertex3f(v1->point.x,v1->point.y,v1->point.z);
               glVertex3f(v2->point.x,v2->point.y,v2->point.z);
               glVertex3f(v3->point.x,v3->point.y,v3->point.z);
           }

        }
    glEnd();
    //rQuad -= 0.15;
}

void MyGLWidget::wheelEvent(QWheelEvent *e)
{
    QPoint qpMag = e->angleDelta();
    int iMag = qpMag.y();
    bool bUpdate = false;
    if(iMag > 0)
    {
        zoom += 10;
        bUpdate = true;
    }

    if(iMag < 0)
    {
        zoom -= 10;
        bUpdate = true;

    }

    if(bUpdate)
    {
        updateGL();
    }
}

void MyGLWidget::mouseMoveEvent(QMouseEvent *event)
{
    float dx = event->x() - m_lastPos.x();
    float dy = event->y() - m_lastPos.y();
    if (event->buttons() & Qt::LeftButton)
        {
           m_xMove+=dx/5;
           m_yMove+=dy/5;
           update();
        }
        if (event->buttons() & Qt::RightButton)
        {
            setXRotation(m_xRot + 8 * dy);
            setZRotation(m_zRot + 8 * dx);
        }
        m_lastPos = event->pos();
}

void MyGLWidget::mousePressEvent(QMouseEvent *event)
{
     m_lastPos = event->pos();
}
void MyGLWidget::setXRotation(int angle)
{
    qNormalizeAngle(angle);
    if (angle != m_xRot) {
        m_xRot = angle;
        update();
    }
}
void MyGLWidget::setYRotation(int angle)
{
    qNormalizeAngle(angle);
    if (angle != m_yRot) {
        m_yRot = angle;
        update();
    }
}
void MyGLWidget::setZRotation(int angle)
{
    qNormalizeAngle(angle);
    if (angle != m_zRot) {
        m_zRot = angle;
        update();
    }
}
 void MyGLWidget::qNormalizeAngle(int &angle)
{
    while (angle < 0)
        angle += 360 * 16;
    while (angle > 360 * 16)
        angle -= 360 * 16;
}
