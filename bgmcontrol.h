#ifndef BGMCONTROL_H
#define BGMCONTROL_H

#include <QWidget>
#include <QMediaPlayer>
#include <qfile.h>
#include <QJsonObject>
#include <qjsondocument.h>
#include <QJsonArray>
#include <qaudiooutput.h>
#include <player.h>
class Bgmcontrol : public QWidget
{
    Q_OBJECT
public:
    // 男女出牌音效
    enum class Sound {
        // 单牌音效 (0-14)
        _3 = 0, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _1, _2, _14, _15,

        // 对子音效 (15-27)
        DUI3, DUI4, DUI5, DUI6, DUI7, DUI8, DUI9, DUI10, DUI11, DUI12, DUI13, DUI1, DUI2,

        // 三张音效 (28-40)
        TUPLE3, TUPLE4, TUPLE5, TUPLE6, TUPLE7, TUPLE8, TUPLE9, TUPLE10, TUPLE11, TUPLE12, TUPLE13, TUPLE1, TUPLE2,

        // 特殊牌型音效 (41-49)
        FEIJI, LIANDUI, SANDAIYI, SANDAIYIDUI, SHUNZI, SIDAIER, SIDAILIANGDUI, ZHADAN, WANGZHA,

        // 游戏操作音效 (50-63)
        BUYAO1, BUYAO2, BUYAO3, BUYAO4,
        DANI1, DANI2, DANI3,
        NOORDER, NOROB, ORDER, ROB1, ROB3,
        BAOJING1, BAOJING2,

        // 其他音效 (64-66)
        SHARE, BUJIABEI, JIABEI,

        // 总数
        COUNT
    };
    // 背景音乐枚举
    enum class BGM {
        WELCOME = 0,    // MusicEx_Welcome.mp3
        NORMAL,         // MusicEx_Normal.mp3
        NORMAL2,        // MusicEx_Normal2.mp3
        EXCITING,       // MusicEx_Exciting.mp3

        COUNT
    };
    // 结束音效枚举
    enum class EndingSound {
        WIN = 0,        // MusicEx_Win.mp3
        LOSE,           // MusicEx_Lose.mp3

        COUNT
    };
    // 其他音效枚举
    enum class OtherSound {
        // 游戏操作音效 (0-4)
        DISPATCH = 0,       // 发牌
        SELECT_CARD,        // 选牌
        PLANE,              // 飞机
        BOMB,               // 炸弹
        ALERT,              // 时间倒计时提醒
    };
    explicit Bgmcontrol(QWidget *parent = nullptr);
    //音乐列表初始化json
    void InitMusicPlayer();
    //背景音乐播放
    void StartBgm();
    void StopBgm();
    //结束音乐播放
    void StartEndBgm(bool isWin);
    //出牌的音乐播放
    void PlayeHandBgm(player::Sex sex,bool isfirst,Cards *cards);
    //不出牌音乐
    void NoPlayerHandBgm(player::Sex sex);
    //辅助音乐的播放
    void OtherBgm(OtherSound type);
    void StopOtherBgm();
    //抢地主音乐播放
    void GetlordBgm(int point,player::Sex sex,bool isfirst);
    void playSoundBySex(player::Sex sex, Sound soundIndex, const QString &logType);
signals:
private:
    //播放器
    //0、男 1、女 2、背景音乐 3、辅助 4、结束
    QList<QMediaPlayer*> _MPlayer;
    //音频输出
    QList<QAudioOutput*>  _MOutput;
    //男
    QList<QString> _Manbgm;
    //女
    QList<QString> _Womanbgm;
    //背景音乐
    QList<QString> _Bgm;
    //辅助音乐
    QList<QString> _Otherbgm;
    //结束音乐
    QList<QString> _Endbgm;


};

#endif // BGMCONTROL_H
