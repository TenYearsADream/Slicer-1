#ifndef MYTABLEWIDGET
#define MYTABLEWIDGET
#include <QTableWidget>
#include "point3f.h"

using namespace std;
class MyTableWidget : public QTableWidget
{
    Q_OBJECT
public:
    explicit MyTableWidget(QWidget *parent = 0);
    void setData(vector<Point3f> pointList, int nFaceCount);
private slots:
    void slotItemClicked(QTableWidgetItem*);
signals:
    int rowClicked(int row);

};
#endif // MYTABLEWIDGET

