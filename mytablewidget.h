#ifndef MYTABLEWIDGET
#define MYTABLEWIDGET
#include <QTableWidget>
#include "point3f.h"
#include "hashtable.h"
using namespace std;

class MyTableWidget : public QTableWidget
{
    Q_OBJECT
public:
    explicit MyTableWidget(QWidget *parent = 0);
    void setData(vector <tableNode *> vertices, vector<vector<size_t>> faceList);
private slots:
    void slotItemClicked(QTableWidgetItem*);
signals:
    int rowClicked(int row);

};
#endif // MYTABLEWIDGET

