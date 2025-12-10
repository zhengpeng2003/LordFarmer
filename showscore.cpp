#include "showscore.h"
#include "ui_showscore.h"

ShowScore::ShowScore(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ShowScore)
{
    ui->setupUi(this);
    myLable<<ui->label_A<<ui->label_B<<ui->label_Me
           <<ui->label_TotalA<<ui->label_TotalB<<ui->label_TotalMe;
    QString scoreStyle = R"(
        QLabel {
            color: red;
            font-size: 18px;
            font-weight: bold;
            background: transparent;
        }
    )";
    for(int i=0;i<myLable.size();i++)
    {
        myLable.at(i)->setStyleSheet(scoreStyle);
    }

    QString titleStyle = R"(
        QLabel {
            color: white;
            font-size: 14px;
            font-weight: bold;
            background: transparent;
        }
    )";
    ui->label_round_title->setStyleSheet(titleStyle);
    ui->label_total_title->setStyleSheet(titleStyle);
}

void ShowScore::InitScore(int roundA, int roundB, int roundUser,
                         int totalA, int totalB, int totalUser)
{
    ui->label_A->setText(QString::number(roundA));
    ui->label_B->setText(QString::number(roundB));
    ui->label_Me->setText(QString::number(roundUser));

    ui->label_TotalA->setText(QString::number(totalA));
    ui->label_TotalB->setText(QString::number(totalB));
    ui->label_TotalMe->setText(QString::number(totalUser));
}

ShowScore::~ShowScore()
{
    delete ui;
}
