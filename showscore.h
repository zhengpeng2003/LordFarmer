#ifndef SHOWSCORE_H
#define SHOWSCORE_H

#include <QWidget>
#include <QLabel>
namespace Ui {
class ShowScore;
}

class ShowScore : public QWidget
{
    Q_OBJECT

public:
    explicit ShowScore(QWidget *parent = nullptr);
    void InitScore(int roundA,int roundB,int roundUser,
                  int totalA,int totalB,int totalUser);
    ~ShowScore();

private:
    QList<QLabel*> myLable;
    Ui::ShowScore *ui;
};

#endif // SHOWSCORE_H
