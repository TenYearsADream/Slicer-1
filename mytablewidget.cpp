#include <QHeaderView>
#include <QDebug>
#include "mytablewidget.h"
using namespace std;
MyTableWidget::MyTableWidget(QWidget *parent) :
    QTableWidget(parent)
{
    this->setColumnCount(9);
    QStringList headers;
    this->setEditTriggers(QAbstractItemView::NoEditTriggers);
    headers << "X1" << "Y1" << "Z1"<< "X2" << "Y2" << "Z2"<< "X3" << "Y3" << "Z3";
    this->setHorizontalHeaderLabels(headers);
    this->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    this->setSelectionBehavior(QAbstractItemView::SelectRows); //整行选中的方式
    this->setStyleSheet("background-color:rgba(144,238,144,1)");
    connect(this,SIGNAL(itemPressed(QTableWidgetItem*)),this,SLOT(slotItemClicked(QTableWidgetItem *)));
}

void MyTableWidget::slotItemClicked(QTableWidgetItem *item)
{
    int row=item->row();
    //qDebug()<< item->backgroundColor()<<endl;
    if(item->backgroundColor()=="QColor(Invalid)" || item->backgroundColor()==QColor(144,238,144))
    {
        for(int i=0;i<9;i++)
        {
            QTableWidgetItem *selectedItem =  this->item(row,i);
            selectedItem->setBackgroundColor(Qt::red);
        }
    }
    else
    {
        for(int i=0;i<9;i++)
        {
            QTableWidgetItem *selectedItem =  this->item(row,i);
            selectedItem->setBackgroundColor(QColor(144,238,144));
        }
        //qDebug()<< item->backgroundColor()<<endl;

    }
    emit rowClicked(row);
    //qDebug()<<"You clicked!"<<row<<endl;
}

void MyTableWidget::setData(vector <tableNode *> vertices, vector<vector<size_t>> faceList)
{
    for(int i=0; i<faceList.size(); i++)
    {
        for (int j=0;j<faceList[i].size();j++)
        {
            tableNode *v = vertices[faceList[i][j]];
            this->setItem(i,3*j+0, new QTableWidgetItem(QString::number(v->point.x,'f',2)));
            this->setItem(i,3*j+1, new QTableWidgetItem(QString::number(v->point.y,'f',2)));
            this->setItem(i,3*j+2, new QTableWidgetItem(QString::number(v->point.z,'f',2)));
        }

    }
}
