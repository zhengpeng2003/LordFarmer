#include "loadprocess.h"

#include <qpainter.h>
#include <qtimer.h>
#include <maingame.h>
Loadprocess::Loadprocess(QWidget *parent)
    : QWidget{parent}
{

    _MainPix=QPixmap();
    _MainPix.load(":/images/loading.png");
    this->setFixedSize(_MainPix.size());
    QPixmap temp=QPixmap();
    temp.load(":/images/progress.png");
    this->setWindowFlags(windowFlags()|Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);
    _Timer=new QTimer(this);
    connect(_Timer,&QTimer::timeout,this,[=](){

        _Rate+=5;
        _Process=temp.copy(0,0,_Rate,temp.height());
        update();
        if(_Rate>=temp.width())
        {
            _Timer->stop();
            _Timer->deleteLater();
            Maingame * maingame=new Maingame();
            maingame->show();
            this->close();
        }
    });
    _Timer->start(15);
}

void Loadprocess::paintEvent(QPaintEvent *event)
{
    QPainter *painter=new QPainter(this);
    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);//启用图片平滑变换当图片需要缩放时，使用高质量的插值算法（如双线性插值）
    painter->setRenderHint(QPainter::Antialiasing, true);//启用抗锯齿对图形边缘进行平滑处理，消除锯齿
    painter->drawPixmap(0,0,_MainPix);
    painter->drawPixmap(62,417,_Process);
}
