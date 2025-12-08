#ifndef TIMECOUNT_H
#define TIMECOUNT_H

#include "qtimer.h"
#include <QWidget>

class Timecount : public QWidget
{
    Q_OBJECT
public:
    explicit Timecount(QWidget *parent = nullptr);
    void Timestart();
    void Timeout();

signals:
    void S_Timemusic();
    void S_TimeOUt();
protected:
    virtual void paintEvent(QPaintEvent *event) override;
private:
    QPixmap _BgPix;
    QPixmap _Number;
    int count=0;
    QTimer *_Timer;
};

#endif // TIMECOUNT_H
