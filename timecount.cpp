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
        if(count <= 0)
        {
            //跳过不出
            emit S_TimeOUt();
            Reset();
        }
        update();
    });
}

void Timecount::Start(int seconds)
{
    // 启动倒计时并记录剩余秒数
    _Timer->start(1000);
    count = seconds;
}

void Timecount::Stop()
{
    // 主动停止计时，通常在玩家已出牌时调用
    _Timer->stop();
}

void Timecount::Reset()
{
    // 复位计时器，清空显示
    Stop();
    _BgPix = QPixmap();
    _Number = QPixmap();
    count = 0;
    update();
}

void Timecount::Timestart()
{
    Start();
}

void Timecount::Timeout()
{
    // 兼容旧接口的超时处理
    Reset();
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
