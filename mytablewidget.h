#ifndef MYTABLEWIDGET
#define MYTABLEWIDGET
#include <QTableWidget>

class MyTableWidget : public QTableWidget
{
    Q_OBJECT
public:
    explicit MyTableWidget(QWidget *parent = 0);
    void setData(float faceArray[][9],int nFaceCount);
private slots:
    void slotItemClicked(QTableWidgetItem*);
signals:
    int rowClicked(int row);

};
#endif // MYTABLEWIDGET

