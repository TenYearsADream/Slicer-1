#include "loadprogressbar.h"
#include<QProgressDialog>
#include<QFont>

loadProgressBar::loadProgressBar(QString str):QDialog(NULL)
{
    this->setWindowFlags(Qt::CustomizeWindowHint);//启动窗口自定义
    this->setWindowFlags(Qt::WindowCloseButtonHint);//添加关闭
    this->setWindowFlags(Qt::WindowMinimizeButtonHint);//添加最小化
    this->setWindowFlag(Qt::WindowStaysOnTopHint);
    progressDialog=new QProgressDialog(this);
    progressDialog->setWindowModality(Qt::WindowModal);
    progressDialog->setMinimumDuration(0);
    progressDialog->setWindowTitle(str);
    progressDialog->setCancelButtonText(QStringLiteral("cancel"));
    progressDialog->setRange(0,100);
    progressDialog->setAutoFillBackground(true);
    QPalette p=progressDialog->palette();
    p.setColor(QPalette::WindowText,QColor(0,0,0));
    progressDialog->setPalette(p);
}
void loadProgressBar::setProgressBar(float fraction,float total)
{
  progressDialog->setMaximum(total);
  progressDialog->setValue(fraction);
  //qDebug()<<fraction<<endl;
  if(progressDialog->wasCanceled())
  {
     emit signalExit();//信号确定抛出
     //qDebug()<<"111"<<endl;
     return;
  }
}
void loadProgressBar::setLabelText(QString str)
{
    progressDialog->setLabelText(str);
}
