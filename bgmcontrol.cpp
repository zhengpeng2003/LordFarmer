#include "bgmcontrol.h"
#include "player.h"
#include "PlayHand.h"
#include <QDebug>
#include <QRandomGenerator>

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

    // 读取结束音效
    if (root.contains("Ending") && root["Ending"].isArray()) {
        QJsonArray array = root["Ending"].toArray();
        for (const QJsonValue &value : array) {
            if (value.isString()) {
                _Endbgm.append(value.toString());
            }
        }
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
    qDebug() << "结束音效:" << _Endbgm.size();
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
    // 先停止背景音乐
    StopBgm();

    if (_Endbgm.isEmpty()) return;

    QMediaPlayer *endPlayer = _MPlayer[4]; // 结束音效播放器

    int index = isWin ? 0 : 1;
    if (index < 0 || index >= _Endbgm.size()) return;
    QString endPath = _Endbgm[index];

    endPlayer->setSource(QUrl(endPath));
    endPlayer->play();

    qDebug() << "播放结束音效:" << endPath;
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
        break;

    case PlayHand::Hand_Triple_Pair:
        soundIndex = Sound::SANDAIYIDUI;  // 使用"三带一对"特殊音效
        break;

    // 连牌系列
    case PlayHand::Hand_Seq_Sim:
        soundIndex = Sound::SHUNZI;
        break;

    case PlayHand::Hand_Seq_Pair:
        soundIndex = Sound::LIANDUI;
        break;

    case PlayHand::Hand_Plane:
        soundIndex = Sound::FEIJI;
        break;

    case PlayHand::Hand_Plane_Two_Single:
    case PlayHand::Hand_Plane_Two_Pair:
        soundIndex = Sound::FEIJI;
        break;

    // 炸弹系列
    case PlayHand::Hand_Bomb:
        soundIndex = Sound::ZHADAN;
        break;

    case PlayHand::Hand_Bomb_Jokers:
        soundIndex = Sound::WANGZHA;
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

    // 播放音效
    playSoundBySex(sex, soundIndex, "出牌音效");
}

void Bgmcontrol::NoPlayerHandBgm(player::Sex sex)
{
    // 播放"不要"音效，随机选择一个
    int buyaoStart = static_cast<int>(Sound::BUYAO1);
    int randomBuyao = buyaoStart + QRandomGenerator::global()->bounded(4); // BUYAO1-BUYAO4

    playSoundBySex(sex, static_cast<Sound>(randomBuyao), "不要音效");
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
    Sound soundIndex;

    if (point == 0) {
        // 不叫地主
        soundIndex = isfirst ? Sound::NOORDER : Sound::NOROB;
    } else if (point == 1) {
        // 叫地主
        soundIndex = isfirst ? Sound::ORDER : Sound::ROB1;
    } else if (point == 2) {
        // 抢地主
        soundIndex = Sound::ROB3;
    } else {
        return;
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
        player->setSource(QUrl(soundPath));
        player->play();


    } else {

    }
}
