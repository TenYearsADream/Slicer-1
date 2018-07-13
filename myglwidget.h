#ifndef MYGLWIDGET
#define MYGLWIDGET
#include <QGLWidget>
class QTimer;

class MyGLWidget : public QGLWidget
{
    Q_OBJECT
public:
    explicit MyGLWidget(QWidget *parent = 0);
    void resizeGL(int w, int h);
    float faceArray[4][9];
    size_t nFaceCount;
    bool ColorFlag[4];
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
    GLfloat m_xMove;
    GLfloat m_yMove;
    QPoint m_lastPos;
    GLfloat zoom;
    GLfloat rQuad;
    GLfloat m_xRot;
    GLfloat m_yRot;
    GLfloat m_zRot;

};
#endif // MYGLWIDGET

