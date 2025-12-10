#include "bgmcontrol.h"
#include "player.h"
#include "PlayHand.h"
#include <QDebug>
#include <QRandomGenerator>
#include <QTimer>

Bgmcontrol::Bgmcontrol(QWidget *parent)
    : QWidget(parent)
{
    // 初始化播放器和音频输出
    for (int i = 0; i < 5; ++i) {
        QMediaPlayer *player = new QMediaPlayer(this);
        QAudioOutput *output = new QAudioOutput(this);
        player->setAudioOutput(output);

        // 设置不同播放器的音量
        if (i == 2) { // 背景音乐
            output->setVolume(0.4f); // 背景音乐音量较低
        } else {
            output->setVolume(0.8f); // 音效音量较高
        }

        _MPlayer.append(player);
        _MOutput.append(output);
    }

    // 初始化音乐列表

}

void Bgmcontrol::InitMusicPlayer()
{
    // 读取JSON配置文件
    QFile file(":/conf/playList.json");
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "无法打开音频配置文件";
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (doc.isNull()) {
        qWarning() << "JSON解析失败";
        return;
    }

    QJsonObject root = doc.object();

    // 读取男性音效
    if (root.contains("Man") && root["Man"].isArray()) {
        QJsonArray array = root["Man"].toArray();
        for (const QJsonValue &value : array) {
            if (value.isString()) {
                _Manbgm.append(value.toString());
            }
        }
    }

    // 读取女性音效
    if (root.contains("Woman") && root["Woman"].isArray()) {
        QJsonArray array = root["Woman"].toArray();
        for (const QJsonValue &value : array) {
            if (value.isString()) {
                _Womanbgm.append(value.toString());
            }
        }
    }

    // 读取背景音乐
    if (root.contains("BGM") && root["BGM"].isArray()) {
        QJsonArray array = root["BGM"].toArray();
        for (const QJsonValue &value : array) {
            if (value.isString()) {
                _Bgm.append(value.toString());
            }
        }
    }

    // 读取结果音效
    if (root.contains("Result") && root["Result"].isObject()) {
        QJsonObject resultObj = root["Result"].toObject();
        _WinResult = resultObj.value("Win").toString();
        _LoseResult = resultObj.value("Lose").toString();
    }

    // 读取其他音效
    if (root.contains("Other") && root["Other"].isArray()) {
        QJsonArray array = root["Other"].toArray();
        for (const QJsonValue &value : array) {
            if (value.isString()) {
                _Otherbgm.append(value.toString());
            }
        }
    }

    qDebug() << "音频初始化完成:";
    qDebug() << "男性音效:" << _Manbgm.size();
    qDebug() << "女性音效:" << _Womanbgm.size();
    qDebug() << "背景音乐:" << _Bgm.size();
    qDebug() << "胜利音效路径:" << _WinResult;
    qDebug() << "失败音效路径:" << _LoseResult;
    qDebug() << "其他音效:" << _Otherbgm.size();
}

void Bgmcontrol::StartBgm()
{
    if (_Bgm.isEmpty()) return;

    QMediaPlayer *bgmPlayer = _MPlayer[2]; // 背景音乐播放器

    // 使用成员变量或静态变量来保存索引，确保在lambda中持久化
    static int index = 0; // 或者改为成员变量 m_currentBgmIndex

    QString bgmPath = _Bgm[index];
    bgmPlayer->setSource(QUrl(bgmPath));

    connect(bgmPlayer, &QMediaPlayer::mediaStatusChanged, [=](QMediaPlayer::MediaStatus status) {
        if (status == QMediaPlayer::EndOfMedia) {
            // 更新索引，循环播放
            index = (index + 1) % _Bgm.size(); // 使用取模实现循环

            QString nextBgmPath = _Bgm[index];
            bgmPlayer->setSource(QUrl(nextBgmPath));
            bgmPlayer->play();

            qDebug() << "切换到下一首背景音乐:" << nextBgmPath;
        }
    });

    bgmPlayer->play();
    qDebug() << "开始播放背景音乐:" << bgmPath;
}
void Bgmcontrol::StopBgm()
{
    QMediaPlayer *bgmPlayer = _MPlayer[2];
    bgmPlayer->stop();

    // 断开循环连接
    bgmPlayer->disconnect(); // 断开所有连接
    qDebug() << "停止背景音乐";
}

void Bgmcontrol::StartEndBgm(bool isWin)
{
    playResultBgm(isWin);
}

void Bgmcontrol::playResultBgm(bool isWin)
{
    StopBgm();

    const QString &endPath = isWin ? _WinResult : _LoseResult;
    if (endPath.isEmpty()) return;

    QMediaPlayer *endPlayer = _MPlayer[4];
    endPlayer->stop();
    endPlayer->setPosition(0);
    endPlayer->setSource(QUrl(endPath));
    endPlayer->play();

    qDebug() << "播放结果音效:" << endPath;
}

void Bgmcontrol::PlayeHandBgm(player::Sex sex, bool isfirst, Cards *cards)
{
    if (!cards || cards->isempty()) {
        NoPlayerHandBgm(sex);
        return;
    }

    PlayHand playHand(cards);
    PlayHand::HandType handType = playHand.Getplayhandtype();

    if (handType == PlayHand::Hand_Unknown) {
        NoPlayerHandBgm(sex);
        return;
    }

    Sound soundIndex = Sound::_3;
    Card::cardpoint point = playHand.Getplayhandpoint();
    bool shouldPlayDani = false;

    // 修复牌点到音效索引的映射
    switch (handType) {
    // 单牌系列
    case PlayHand::Hand_Single:
        if (point >= Card::Card_3 && point <= Card::Card_BJ) {
            if (point >= Card::Card_3 && point <= Card::Card_2) {
                soundIndex = static_cast<Sound>(point - Card::Card_3);
            }
            else if (point == Card::Card_SJ) {
                soundIndex = Sound::_14;
            }
            else if (point == Card::Card_BJ) {
                soundIndex = Sound::_15;
            }
        }
        break;

    // 对子系列
    case PlayHand::Hand_Pair:
        if (point >= Card::Card_3 && point <= Card::Card_2) {
            soundIndex = static_cast<Sound>(15 + (point - Card::Card_3));
        }
        break;

    // 三张系列 - 这里修复三带一、三带二的音效
    case PlayHand::Hand_Triple:
        if (point >= Card::Card_3 && point <= Card::Card_2) {
            soundIndex = static_cast<Sound>(28 + (point - Card::Card_3));
        }
        break;

    case PlayHand::Hand_Triple_Single:
        soundIndex = Sound::SANDAIYI;  // 使用"三带一"特殊音效
        shouldPlayDani = true;
        break;

    case PlayHand::Hand_Triple_Pair:
        soundIndex = Sound::SANDAIYIDUI;  // 使用"三带一对"特殊音效
        shouldPlayDani = true;
        break;

    // 连牌系列
    case PlayHand::Hand_Seq_Sim:
        soundIndex = Sound::SHUNZI;
        shouldPlayDani = true;
        break;

    case PlayHand::Hand_Seq_Pair:
        soundIndex = Sound::LIANDUI;
        shouldPlayDani = true;
        break;

    case PlayHand::Hand_Plane:
        soundIndex = Sound::FEIJI;
        shouldPlayDani = true;
        break;

    case PlayHand::Hand_Plane_Two_Single:
    case PlayHand::Hand_Plane_Two_Pair:
        soundIndex = Sound::FEIJI;
        shouldPlayDani = true;
        break;

    // 炸弹系列
    case PlayHand::Hand_Bomb:
        soundIndex = Sound::ZHADAN;
        shouldPlayDani = true;
        break;

    case PlayHand::Hand_Bomb_Jokers:
        soundIndex = Sound::WANGZHA;
        shouldPlayDani = true;
        break;

    default:
        // 默认使用主牌的点数
        if (point >= Card::Card_3 && point <= Card::Card_BJ) {
            if (point >= Card::Card_3 && point <= Card::Card_2) {
                soundIndex = static_cast<Sound>(point - Card::Card_3);
            } else if (point == Card::Card_SJ) {
                soundIndex = Sound::_14;
            } else if (point == Card::Card_BJ) {
                soundIndex = Sound::_15;
            }
        }
        break;
    }

    // 自由出牌：仅播放牌型音效
    playSoundBySex(sex, soundIndex, "出牌音效");

    // 压牌阶段：在播放完牌型音效后追加"大你"特效
    if(!isfirst && shouldPlayDani)
    {
        QTimer::singleShot(150, this, [=](){
            playSoundBySex(sex, Sound::DANI3, "压牌特效音效");
        });
    }
}

void Bgmcontrol::NoPlayerHandBgm(player::Sex sex)
{
    // 使用固定的"不要"音效，避免随机索引导致的遗漏
    playSoundBySex(sex, Sound::BUYAO1, "不要音效");
}

void Bgmcontrol::OtherBgm(OtherSound type)
{
    if (_Otherbgm.isEmpty()) return;

    int index = static_cast<int>(type);
    if (index < 0 || index >= _Otherbgm.size()) return;

    // 总是重新创建播放器来避免解码问题
    _MOutput[3]->deleteLater();
    _MPlayer[3]->deleteLater();

    QMediaPlayer *otherPlayer = new QMediaPlayer(this);
    QAudioOutput *newOutput = new QAudioOutput(this);
    otherPlayer->setAudioOutput(newOutput);
    newOutput->setVolume(0.8f);
    _MPlayer[3] = otherPlayer;
    _MOutput[3] = newOutput;

    QString soundPath = _Otherbgm[index];
    otherPlayer->setSource(QUrl(soundPath));

    // 错误处理
    connect(otherPlayer, &QMediaPlayer::errorOccurred, this,
            [soundPath, type](QMediaPlayer::Error error, const QString &errorString) {

            });

    // 播放逻辑
    if (type == OtherSound::DISPATCH) {
        connect(otherPlayer, &QMediaPlayer::mediaStatusChanged, this,
                [otherPlayer, soundPath](QMediaPlayer::MediaStatus status) {
                    if (status == QMediaPlayer::EndOfMedia) {
                        otherPlayer->setPosition(0);
                        otherPlayer->play();
                    }
                });
    } else {
        connect(otherPlayer, &QMediaPlayer::mediaStatusChanged, this,
                [otherPlayer](QMediaPlayer::MediaStatus status) {
                    if (status == QMediaPlayer::EndOfMedia) {
                        otherPlayer->stop();
                    }
                });
    }

    otherPlayer->play();

}
void Bgmcontrol::StopOtherBgm()
{
    QMediaPlayer *otherPlayer = _MPlayer[3];
    otherPlayer->stop();
    otherPlayer->disconnect(); // 断开循环连接

}

void Bgmcontrol::GetlordBgm(int point, player::Sex sex, bool isfirst)
{
    // 兼容 1/2/3 分和叫、抢两种语境，确保不会因为未覆盖的下注值而静音
    Sound soundIndex = Sound::NOORDER;

    if (point <= 0) {
        // 不叫/不抢
        soundIndex = isfirst ? Sound::NOORDER : Sound::NOROB;
    } else if (isfirst) {
        // 首家叫分（1/2/3 分统一用叫地主语音）
        soundIndex = Sound::ORDER;
    } else {
        // 后手抢分：1 分用 ROB1，2/3 分都使用力度更大的 ROB3 语音
        switch (point) {
        case 1:
            soundIndex = Sound::ROB1;
            break;
        case 2:
        case 3:
            soundIndex = Sound::ROB3;
            break;
        default:
            // 非法下注值也兜底到 ROB3，避免静音
            soundIndex = Sound::ROB3;
            break;
        }
    }

    playSoundBySex(sex, soundIndex, "抢地主音效");
}

// 辅助函数：根据性别播放音效
void Bgmcontrol::playSoundBySex(player::Sex sex, Sound soundIndex, const QString &logType)
{
    int playerIndex = (sex == player::Sex::MAN) ? 0 : 1;
    QMediaPlayer *player = _MPlayer[playerIndex];
    QList<QString> &soundList = (sex == player::Sex::MAN) ? _Manbgm : _Womanbgm;

    int index = static_cast<int>(soundIndex);
    qDebug() << "尝试播放音效 - 类型:" << logType
             << "性别:" << (sex == player::Sex::MAN ? "男" : "女")
             << "索引:" << index
             << "音效列表大小:" << soundList.size();

    if (index >= 0 && index < soundList.size()) {
        QString soundPath = soundList[index];
        qDebug() << "音效文件路径:" << soundPath;

        // 检查文件是否存在
        QFile file(soundPath);
        if (file.exists()) {
            qDebug() << "音效文件存在";
        } else {
            qWarning() << "音效文件不存在:" << soundPath;
        }

        player->stop();
        player->setPosition(0);
        player->setSource(QUrl(soundPath));
        player->play();


    } else {
        qWarning() << "音效索引无效，尝试使用列表首个音效作为兜底" << index;
        if(!soundList.isEmpty())
        {
            player->stop();
            player->setPosition(0);
            player->setSource(QUrl(soundList.first()));
            player->play();
        }
    }
}
