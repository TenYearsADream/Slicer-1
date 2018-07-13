#include <QHeaderView>
#include <QDebug>
#include "mytablewidget.h"
using namespace std;
MyTableWidget::MyTableWidget(QWidget *parent) :
    QTableWidget(parent)
{
    this->setColumnCount(12);
    QStringList headers;
    this->setEditTriggers(QAbstractItemView::NoEditTriggers);
    headers << "nx" << "ny" << "nz"<< "X1" << "Y1" << "Z1"<< "X2" << "Y2" << "Z2"<< "X3" << "Y3" << "Z3";
    this->setHorizontalHeaderLabels(headers);
    this->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    this->setSelectionBehavior(QAbstractItemView::SelectRows); //整行选中的方式

    connect(this,SIGNAL(itemPressed(QTableWidgetItem*)),this,SLOT(slotItemClicked(QTableWidgetItem *)));
}

void MyTableWidget::slotItemClicked(QTableWidgetItem *item)
{
    int row=item->row();
    emit rowClicked(row);
    //qDebug()<<"You clicked!"<<row<<endl;
}

void MyTableWidget::setData(vector<Point3f> pointList, int nFaceCount)
{
    for(int i=0; i< nFaceCount; i++)
    {
        for (int j=0;j<4;j++)
        {
            qDebug()<<pointList.at(j).x<<endl;
//            this->setItem(i,3*j+0, new QTableWidgetItem(QString::number(pointList.at(4*nFaceCount+j).x,'f',2)));
//            this->setItem(i,3*j+1, new QTableWidgetItem(QString::number(pointList.at(4*nFaceCount+j).y,'f',2)));
//            this->setItem(i,3*j+2, new QTableWidgetItem(QString::number(pointList.at(4*nFaceCount+j).z,'f',2)));
        }

    }
}
