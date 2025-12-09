#include "timecount.h"
#include <QPainter>

Timecount::Timecount(QWidget *parent)
    : QWidget{parent}
{
    this->setFixedSize(70,70);

    _Timer = new QTimer(this);
    connect(_Timer, &QTimer::timeout, this, [=](){
        count--;
        if(count < 10 && count > 0)
        {
            _BgPix.load(":/images/clock.png");
            _Number = QPixmap(":/images/number.png").copy(count * 40, 0, 30, 42).scaled(20, 30);
        }
        if(count == 5)
        {
            //音乐效果
            emit S_Timemusic();
        }
        else if(count <= 0)
        {
            //跳过不出
            emit S_TimeOUt();
            Timeout();
        }
        update();
    });
}

void Timecount::Timestart()
{
    // 重新开始计时时，先清理上一次的显示状态，避免残留的钟表图标闪现
    _BgPix = QPixmap();
    _Number = QPixmap();

    count = 15;
    _Timer->start(1000);
}

void Timecount::Timeout()
{
    _Timer->stop();
    _BgPix = QPixmap();
    _Number = QPixmap();
    count = 15;
}

void Timecount::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    // 启用高质量渲染
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // 绘制背景
    if(!_BgPix.isNull())
    {
        painter.drawPixmap(rect(), _BgPix);
    }

    // 绘制数字
    if(!_Number.isNull())
    {
        painter.drawPixmap(24, 24, _Number);
    }
}
