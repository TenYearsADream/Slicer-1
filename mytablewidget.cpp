#include <QHeaderView>
#include <QDebug>
#include "mytablewidget.h"

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

    connect(this,SIGNAL(itemPressed(QTableWidgetItem*)),this,SLOT(slotItemClicked(QTableWidgetItem *)));
}

void MyTableWidget::slotItemClicked(QTableWidgetItem *item)
{
    int row=item->row();
    emit rowClicked(row);
    //qDebug()<<"You clicked!"<<row<<endl;
}
