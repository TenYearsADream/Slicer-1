#ifndef MYGLWIDGET
#define MYGLWIDGET
#include <QGLWidget>
#include "point3f.h"
#include "hashtable.h"
class QTimer;
using namespace std;
class MyGLWidget : public QGLWidget
{
    Q_OBJECT
public:
    explicit MyGLWidget(QWidget *parent = 0);
    void resizeGL(int w, int h);
    vector<vector<size_t>> faceList;
    vector <tableNode *> vertices;
    vector <int> colorFlag;
    int nFaceCount;
    GLfloat m_xMove;
    GLfloat m_yMove;
    GLfloat zoom;
private:
    HDC hdc;
    QTimer *timer;
    GLfloat colorMap[8][3];

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

