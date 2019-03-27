#ifndef LOADPROGRESSBAR_H
#define LOADPROGRESSBAR_H
#include<QProgressBar>
#include<QDialog>
#include<QProgressDialog>

class loadProgressBar : public QDialog
{
    Q_OBJECT
public:
    loadProgressBar(QString str);
    void setLabelText(QString str);
public slots:
    void setProgressBar(float fraction,float total);//设置进度条的进度
signals:
    void signalExit();//消息函数可以有形参
private:
    QProgressDialog * progressDialog;
};
#endif // LOADPROGRESSBAR_H
