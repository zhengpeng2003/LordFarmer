#include "gamecontrol.h"
#include "maingame.h"
#include "qdebug.h"

gamecontrol::gamecontrol():_AllCards(new Cards()),_Countcard(0),_PlayHandCards(nullptr),_PlayHandplayer(nullptr)
{}

void gamecontrol::InitPlayer()
{
    //机器人初始化
    _Rightrobot=new robot("机器人A",player::ROOT,player::LEFT,this);
    _Leftrobot=new robot("机器人B",player::ROOT,player::RIGHT,this);
    _UserPlayer=new user("玩家",player::USER,player::RIGHT,this);

    //前后节点初始化
    _Rightrobot->SetNextPlayer(_Leftrobot);
    _Rightrobot->SetPrePlayer(_UserPlayer);

    _Leftrobot->SetNextPlayer(_UserPlayer);
    _Leftrobot->SetPrePlayer(_Rightrobot);

    _UserPlayer->SetNextPlayer(_Rightrobot);
    _UserPlayer->SetPrePlayer(_Leftrobot);

    //性别初始化我们用随机数
    player::Sex R1=(player::Sex)QRandomGenerator::global()->bounded(0,2);
    player::Sex R2=(player::Sex)QRandomGenerator::global()->bounded(0,2);
    player::Sex R3=(player::Sex)QRandomGenerator::global()->bounded(0,2);

    _Rightrobot->SetSex(R1);
    _Leftrobot->SetSex(R2);
    _UserPlayer->SetSex(R3);
    //下注信号
    connect(_UserPlayer,&player::notifygrablordbet,this,&gamecontrol::Onbet);
    connect(_Leftrobot,&player::notifygrablordbet,this,&gamecontrol::Onbet);
    connect(_Rightrobot,&player::notifygrablordbet,this,&gamecontrol::Onbet);

    //存储出牌信息的信号
    connect(this,&gamecontrol::S_PlayerPlayHand,_Rightrobot,&player::SetPendinfo);
    connect(this,&gamecontrol::S_PlayerPlayHand,_Leftrobot,&player::SetPendinfo);
    connect(this,&gamecontrol::S_PlayerPlayHand,_UserPlayer,&player::SetPendinfo);

    //用户出的牌
    connect(_UserPlayer,&player::notifyplayhand,this,&gamecontrol::GamePlayhand);
    connect(_Leftrobot,&player::notifyplayhand,this,&gamecontrol::GamePlayhand);
    connect(_Rightrobot,&player::notifyplayhand,this,&gamecontrol::GamePlayhand);


}

void gamecontrol::SetCurrentPlayer(player *a)
{
    this->_CurrentPlayer=a;
}

player *gamecontrol::GetCurrentPlayer()
{
    return this->_CurrentPlayer;
}

void gamecontrol::SetCurrentCards(Cards *a)
{
    this->_CurrentCards=a;
}

Cards * gamecontrol::GetCurrentCards()
{
    return this->_CurrentCards;
}

void gamecontrol::SetAllCards()
{

    for(int p=Card::Card_begin+1;p<Card::Card_SJ;p++)
    {

        for(int q=Card::Suit_Begin+1;q<Card::Suit_End;q++)
        {
            Card::cardpoint point =(Card::cardpoint)p;
            Card::cardsuit suit =(Card::cardsuit)q;
            Card *temp=new Card(suit,point);

            _AllCards->add(temp);
            _Countcard++;
        }
    }
    _AllCards->add(Card(Card::Suit_Begin,Card::Card_SJ));_Countcard++;
    _AllCards->add(Card(Card::Suit_Begin,Card::Card_BJ));_Countcard++;

}

Card *gamecontrol::TakeOneCard()
{
    _Countcard--;

    return _AllCards->takerandcard();
}

Cards* gamecontrol::TakeCards()
{

    return this->_AllCards;

}

void gamecontrol::RetCardDate()
{
    SetAllCards();

    _Rightrobot->ClearCards();
    _Leftrobot->ClearCards();
    _UserPlayer->ClearCards();
    //当前出牌者还有卡牌清空
    _CurrentPlayer=nullptr;
    _CurrentCards->clearcards();
}

player *gamecontrol::GetPendplayer()
{
    return _PlayHandplayer;
}

void gamecontrol::StartPrepareLord()
{

    _CurrentPlayer->PrepareGetLord();
    emit S_PlayerStateChange(_CurrentPlayer,USERSTATE::USERGETLORD);
}

void gamecontrol::BecomeLord(player *player, int Bet)
{
    _CurrentPlayer = player;
    player->setRole(player::LORD);
    player->GetNextPlayer()->setRole(player::FORMAR);
    player->GetPrePlayer()->setRole(player::FORMAR);

    player->StoreGetCards(TakeCards());

    // 发射地主确定信号，包含地主玩家信息
    emit S_LordDetermined(player);

    QTimer::singleShot(1000, this, [this](){
        emit S_gameStateChange(GIVECARD);
        emit S_PlayerStateChange(_CurrentPlayer, USERPEND);
    });

    _Bet = Bet;
    _CurrentPlayer->PreparePlayCard();
}

void gamecontrol::GamePlayhand(player *player, Cards *cards)
{
    qDebug() << "=== GamePlayhand ===";
    qDebug() << "出牌玩家:" << player;
    qDebug() << "上一个出牌者:" << _PlayHandplayer;
    qDebug() << "牌数:" << (cards ? cards->GetCardtotal() : 0);

    // 告诉主界面出的牌
    emit S_gamePlayHand(player, cards);

    // 关键修复：正确处理"要不起"的情况
    if (!cards || cards->isempty()) {
        qDebug() << "玩家要不起";

        // 重要修复：要不起时不更新出牌记录，保持上一个有效出牌者
        // 只有真正出牌时才更新出牌记录

        // 轮到下个人出牌
        _CurrentPlayer = player->GetNextPlayer();
        qDebug() << "轮到下一个玩家:" << _CurrentPlayer;

        QTimer::singleShot(1000, this, [this]() {
            if (_CurrentPlayer) {
                _CurrentPlayer->PreparePlayCard();
                emit S_PlayerStateChange(_CurrentPlayer, USERPEND);
            }
        });

        return;
    }

    qDebug() << "玩家正常出牌";

    // 重要修复：只有真正出牌（不是要不起）时才更新出牌记录
    _PlayHandplayer = player;
    _PlayHandCards = cards;
    _CurrentCards = cards;
    _CurrentPlayer = player;

    emit S_PlayerPlayHand(_PlayHandplayer, _PlayHandCards);

    // 从玩家手牌中移除这些牌
    if (cards && !cards->isempty()) {
        player->GetCards().remove(cards);
    }
    qDebug() << "玩家剩余牌数:" << player->GetCards().GetCardtotal();

    // 倍数翻倍逻辑
    PlayHand playHand(cards);
    PlayHand::HandType type = playHand.Getplayhandtype();
    qDebug() << "牌型:" << type;

    if(type == PlayHand::Hand_Bomb) {
        _Bet *= 2;
        qDebug() << "炸弹！倍数变为:" << _Bet;
    }

    // 检查游戏是否结束
    if(player->GetCards().isempty()) {
        qDebug() << "游戏结束！";
        if(player->GetRole() == player::LORD)
        {
            player->SetScore(player->GetScore() + 2 * _Bet);
            player->GetPrePlayer()->SetScore(player->GetPrePlayer()->GetScore() - _Bet);
            player->GetNextPlayer()->SetScore(player->GetNextPlayer()->GetScore() - _Bet);
            player->Setwin(true);
            player->GetPrePlayer()->Setwin(false);
            player->GetNextPlayer()->Setwin(false);
        }
        else
        {
            player->SetScore(player->GetScore() + _Bet);
            if(player->GetPrePlayer()->GetRole() == player::LORD)
            {
                player->GetPrePlayer()->SetScore(player->GetPrePlayer()->GetScore() - 2 * _Bet);
                player->GetNextPlayer()->SetScore(player->GetNextPlayer()->GetScore() + _Bet);
                player->Setwin(true);
                player->GetPrePlayer()->Setwin(false);
                player->GetNextPlayer()->Setwin(true);
            }
            else
            {
                player->GetNextPlayer()->SetScore(player->GetNextPlayer()->GetScore() - 2 * _Bet);
                player->GetPrePlayer()->SetScore(player->GetPrePlayer()->GetScore() + _Bet);
                player->Setwin(true);
                player->GetNextPlayer()->Setwin(false);
                player->GetPrePlayer()->Setwin(true);
            }
        }
        emit S_PlayerStateChange(player, USERWIN);
        return;
    }

    // 轮到下个人出牌
    _CurrentPlayer = player->GetNextPlayer();
    qDebug() << "轮到下一个玩家:" << _CurrentPlayer;

    QTimer::singleShot(1, this, [this]() {
        if (_CurrentPlayer) {
            _CurrentPlayer->PreparePlayCard();
            emit S_PlayerStateChange(_CurrentPlayer, USERPEND);
        }
    });
}

void gamecontrol::ClearScore()
{
    _Rightrobot->SetScore(0);
    _Leftrobot->SetScore(0);
    _UserPlayer->SetScore(0);

}

int gamecontrol::GetCardcount()
{
    return _Countcard;
}

Cards* gamecontrol::GetAllCards()
{
    return _AllCards;
}

void gamecontrol::Onbet(player *player, int Bet)
{


    //发送到哪个玩家的游戏状态
    if(Bet==0||_Betrect.Bet>=Bet)
    {
        emit S_gamenotifyGetLoard(player,0,false);
    }
    else if(_Betrect.times==0&&Bet>0)
    {
        emit S_gamenotifyGetLoard(player,Bet,true);
    }
    else
    {
        emit S_gamenotifyGetLoard(player,Bet,false);
    }
    //记录分数 发送游戏结束信号
    if(_Betrect.Bet<Bet)
    {
        _Betrect.Bet=Bet;
        _Betrect.player=player;

    }
    _Betrect.times++;

    if(_Betrect.times==3)//触发重新开始
    {

        if(_Betrect.Bet==0)
        {

            gamecontrol::S_gameStateChange(PENDCARD);//极端就是发牌
        }
        else
        {
            BecomeLord(_Betrect.player,_Betrect.Bet);

            gamecontrol::S_gameStateChange(GIVECARD);//开始出牌
        }
        _Betrect.rest();
        return;
    }
    _CurrentPlayer=player->GetNextPlayer();
    _CurrentPlayer->PrepareGetLord();//下个玩家开始抢地主
}

player *gamecontrol::GetLeftroot()
{
    return _Leftrobot;
}

player *gamecontrol::GetRightroot()
{
    return _Rightrobot;
}

player *gamecontrol::GetUSer()
{
    return _UserPlayer;
}
// 在 gamecontrol 类中添加


