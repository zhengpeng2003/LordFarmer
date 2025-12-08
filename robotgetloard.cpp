#include "robotgetloard.h"

Robotgetloard::Robotgetloard(player * player,QObject * parent)
    : QThread(parent)
{
    this->_Player=player;
}

void Robotgetloard::run()
{
    msleep(1000);
    _Player->RobotGetLard();
}
