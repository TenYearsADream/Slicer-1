#ifndef MYGLWIDGET
#define MYGLWIDGET
#include <QGLWidget>
#include "point3f.h"
class QTimer;
using namespace std;
class MyGLWidget : public QGLWidget
{
    Q_OBJECT
public:
    explicit MyGLWidget(QWidget *parent = 0);
    void resizeGL(int w, int h);
    vector<Point3f> pointList;
    int nFaceCount;
    bool ColorFlag[1000000];
    GLfloat m_xMove;
    GLfloat m_yMove;
    GLfloat zoom;
public slots:
    void changeColorFlag(int row);
private:
    HDC hdc;
    QTimer *timer;


protected:
    void initializeGL();
    void paintGL();
    void mouseMoveEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);
    void wheelEvent(QWheelEvent *);
    void setXRotation(int angle);
    void setYRotation(int angle);
    void setZRotation(int angle);
    void qNormalizeAngle(int &angle);
protected:
    QPoint m_lastPos;
    GLfloat rQuad;
    GLfloat m_xRot;
    GLfloat m_yRot;
    GLfloat m_zRot;

};
#endif // MYGLWIDGET

