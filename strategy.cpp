#include "strategy.h"
#include <QDebug>

Strategy::Strategy(player *player, Cards &cards)
    : m_player(player), m_cards(cards)
{
}

Cards* Strategy::GetrootPlayhand()
{
    player* preplayer = m_player ? m_player->GetPendPlayer() : nullptr;
    Cards* preCards = m_player ? m_player->GetPendCard() : nullptr;

    if (preplayer == nullptr || preplayer == m_player) {
        return FirstPlayhand();
    } else {
        PlayHand type(preCards);
        bool canPlay = IsPlayHand(type);
        if (canPlay) {
            return Getbigplayhand(type);
        } else {
            return new Cards();
        }
    }
}

Cards* Strategy::FirstPlayhand()
{
    // 1. 先尝试出最小的单牌
    for (int p = Card::Card_3; p < Card::Card_end; ++p) {
        Card::cardpoint point = Card::cardpoint(p);
        if (m_cards.GetpointCard(point) >= 1) {
            Cards* single = Getsamepointcard(point, 1);
            if (!single->isempty()) {
                return single;
            }
            delete single;
        }
    }

    // 2. 尝试出最小的对子
    for (int p = Card::Card_3; p < Card::Card_end; ++p) {
        Card::cardpoint point = Card::cardpoint(p);
        if (m_cards.GetpointCard(point) >= 2) {
            Cards* pair = Getsamepointcard(point, 2);
            if (!pair->isempty()) {
                return pair;
            }
            delete pair;
        }
    }

    // 3. 尝试顺子
    Info seqInfo;
    seqInfo.beat = false;
    seqInfo.begin = Card::Card_3;
    seqInfo.end = Card::Card_10;
    seqInfo.number = 1;
    seqInfo.base = 5;
    QVector<Cards*> seqResults = FindHand_parisim(seqInfo);

    if (!seqResults.isEmpty()) {
        int bestIdx = 0;
        int bestLen = 0;
        for (int i = 0; i < seqResults.size(); ++i) {
            int len = seqResults[i]->GetCardtotal();
            if (len > bestLen) {
                bestLen = len;
                bestIdx = i;
            }
        }
        Cards* result = seqResults[bestIdx];
        for (int i = 0; i < seqResults.size(); ++i) {
            if (i != bestIdx) {
                delete seqResults[i];
            }
        }
        return result;
    }

    // 4. 尝试飞机
    PlayHand planePH(PlayHand::Hand_Plane, Card::Card_begin, 0);
    QVector<Cards*> planeCandidates = GetCardstype(planePH, false);
    if (!planeCandidates.isEmpty()) {
        Cards* result = planeCandidates[0];
        for (int i = 1; i < planeCandidates.size(); ++i) {
            delete planeCandidates[i];
        }
        return result;
    }

    // 5. 尝试三张
    PlayHand triPH(PlayHand::Hand_Triple, Card::Card_begin, 0);
    QVector<Cards*> tripleCandidates = GetCardstype(triPH, false);
    if (!tripleCandidates.isEmpty()) {
        Cards* result = tripleCandidates[0];
        for (int i = 1; i < tripleCandidates.size(); ++i) {
            delete tripleCandidates[i];
        }
        return result;
    }

    // 6. 尝试连对
    PlayHand seqPairPH(PlayHand::Hand_Seq_Pair, Card::Card_begin, 0);
    QVector<Cards*> seqPairCandidates = GetCardstype(seqPairPH, false);
    if (!seqPairCandidates.isEmpty()) {
        Cards* result = seqPairCandidates[0];
        for (int i = 1; i < seqPairCandidates.size(); ++i) {
            delete seqPairCandidates[i];
        }
        return result;
    }

    // 7. 尝试对子（再次检查）
    QVector<Cards*> pairs = FindHand_Sing123(Card::Card_3, 2);
    if (!pairs.isEmpty()) {
        Cards* result = pairs[0];
        for (int i = 1; i < pairs.size(); ++i) {
            delete pairs[i];
        }
        return result;
    }

    // 8. 尝试单牌（再次检查）
    QVector<Cards*> singles = FindHand_Sing123(Card::Card_3, 1);
    if (!singles.isEmpty()) {
        Cards* result = singles[0];
        for (int i = 1; i < singles.size(); ++i) {
            delete singles[i];
        }
        return result;
    }

    // 9. 最后尝试炸弹
    QVector<Cards*> bombs = Getsamecount(4);
    if (!bombs.isEmpty()) {
        Cards* result = bombs[0];
        for (int i = 1; i < bombs.size(); ++i) {
            delete bombs[i];
        }
        return result;
    }

    return new Cards();
}

bool Strategy::IsPlayHand(PlayHand type)
{
    player* preplayer = m_player ? m_player->GetPendPlayer() : nullptr;
    Cards* preCards = m_player ? m_player->GetPendCard() : nullptr;

    if (!preplayer || !preCards) return false;

    if (preplayer->GetRole() != m_player->GetRole()) {
        QVector<Cards*> list = GetCardstype(type, true);
        bool ret = !list.isEmpty();
        for (auto p : list) delete p;
        return ret;
    } else {
        return m_player->GetCards().GetCardtotal() > 10;
    }
}

Cards* Strategy::Getbigplayhand(PlayHand type)
{
    player* preplayer = m_player ? m_player->GetPendPlayer() : nullptr;
    if (!preplayer) return new Cards();

    if (preplayer->GetRole() != m_player->GetRole() && preplayer->GetCards().GetCardtotal() <= 3) {
        QVector<Cards*> bombs = Getsamecount(4);
        for (auto b : bombs) {
            PlayHand bph(b);
            if (bph.CanBeat(type)) {
                Cards* result = b;
                for (auto bomb : bombs) {
                    if (bomb != b) {
                        delete bomb;
                    }
                }
                return result;
            }
        }
        for (auto p : bombs) delete p;

        Cards* sj = Getsamepointcard(Card::Card_SJ, 1);
        Cards* bj = Getsamepointcard(Card::Card_BJ, 1);
        if (!sj->isempty() && !bj->isempty()) {
            Cards* jokers = new Cards();
            for (auto card : sj->Listcardssort()) jokers->add(card);
            for (auto card : bj->Listcardssort()) jokers->add(card);
            delete sj;
            delete bj;
            return jokers;
        }
        delete sj;
        delete bj;
        return new Cards();
    }

    QVector<Cards*> candidates = GetCardstype(type, true);
    if (!candidates.isEmpty()) {
        player* nextplayer = m_player->GetNextPlayer();
        Cards* result = nullptr;

        if (nextplayer && nextplayer->GetRole() != m_player->GetRole() && nextplayer->GetCards().GetCardtotal() <= 2) {
            result = candidates.back();
            for (int i = 0; i < candidates.size() - 1; ++i) {
                delete candidates[i];
            }
        } else {
            result = candidates[0];
            for (int i = 1; i < candidates.size(); ++i) {
                delete candidates[i];
            }
        }
        return result;
    }

    QVector<Cards*> bombs2 = Getsamecount(4);
    for (auto b : bombs2) {
        PlayHand bph(b);
        if (bph.CanBeat(type)) {
            Cards* result = b;
            for (auto bomb : bombs2) {
                if (bomb != b) {
                    delete bomb;
                }
            }
            return result;
        }
    }
    for (auto p : bombs2) delete p;

    return new Cards();
}

Cards* Strategy::Getsamepointcard(Card::cardpoint point, int count)
{
    Cards* tempcards = new Cards();
    if (count < 1 || count > 4) return tempcards;

    if (point == Card::Card_SJ || point == Card::Card_BJ) {
        for (auto c : m_cards.Listcardssort()) {
            if (c.getcardpoint() == point) {
                tempcards->add(c);
                return tempcards;
            }
        }
        return tempcards;
    }

    int found = 0;
    for (auto c : m_cards.Listcardssort()) {
        if (c.getcardpoint() == point) {
            tempcards->add(c);
            ++found;
            if (found == count) break;
        }
    }
    if (found != count) {
        tempcards->clearcards();
    }
    return tempcards;
}

QVector<Cards*> Strategy::Getsamecount(int count)
{
    QVector<Cards*> out;
    if (count < 1 || count > 4) return out;

    for (int i = Card::Card_begin + 1; i < Card::Card_end; ++i) {
        Card::cardpoint point = Card::cardpoint(i);
        if (m_cards.GetpointCard(point) >= count) {
            Cards* cs = Getsamepointcard(point, count);
            if (!cs->isempty()) {
                out.append(cs);
            } else {
                delete cs;
            }
        }
    }
    return out;
}

Cards* Strategy::GetrangCard(Card::cardpoint begin, Card::cardpoint end)
{
    Cards* res = new Cards();
    for (int p = begin; p < end; ++p) {
        Card::cardpoint point = Card::cardpoint(p);
        int cnt = m_cards.GetpointCard(point);
        if (cnt > 0) {
            Cards tmp = findSamePointCardsValue(point, cnt);
            for (auto card : tmp.Listcardssort()) {
                res->add(card);
            }
        }
    }
    return res;
}

QVector<Cards*> Strategy::GetCardstype(PlayHand playhand, bool beat)
{
    QVector<Cards*> result;
    PlayHand::HandType t = playhand.Getplayhandtype();
    Card::cardpoint pt = playhand.Getplayhandpoint();
    Card::cardpoint start = beat ? Card::cardpoint(pt + 1) : Card::Card_3;

    switch (t) {
    case PlayHand::Hand_Single:
        return FindHand_Sing123(start, 1);
    case PlayHand::Hand_Pair:
        return FindHand_Sing123(start, 2);
    case PlayHand::Hand_Triple:
        return FindHand_Sing123(start, 3);
    case PlayHand::Hand_Triple_Single:
        return FindHand_Triple12(start, 1);
    case PlayHand::Hand_Triple_Pair:
        return FindHand_Triple12(start, 2);
    case PlayHand::Hand_Plane:
        return FindHand_Plane12(start, 0);
    case PlayHand::Hand_Plane_Two_Single:
        return FindHand_Plane12(start, 1);
    case PlayHand::Hand_Plane_Two_Pair:
        return FindHand_Plane12(start, 2);
    case PlayHand::Hand_Seq_Pair:
    {
        Info info;
        info.base = 3;
        info.beat = beat;
        info.begin = Card::Card_3;
        info.end = Card::Card_Q;
        info.number = 2;
        return FindHand_parisim(info);
    }
    case PlayHand::Hand_Seq_Sim:
    {
        Info info;
        info.base = 5;
        info.beat = beat;
        info.begin = Card::Card_3;
        info.end = Card::Card_10;
        info.number = 1;
        return FindHand_parisim(info);
    }
    case PlayHand::Hand_Bomb:
        return Getsamecount(4);
    default:
        return result;
    }
}

QVector<Cards*> Strategy::FindHand_Sing123(Card::cardpoint point, int count)
{
    QVector<Cards*> result;
    if (m_cards.isempty()) return result;

    for (int p = point; p < Card::Card_end; ++p) {
        Card::cardpoint currentPoint = Card::cardpoint(p);
        if (m_cards.GetpointCard(currentPoint) >= count) {
            Cards* cs = Getsamepointcard(currentPoint, count);
            if (!cs->isempty()) {
                result.append(cs);
            } else {
                delete cs;
            }
        }
    }
    return result;
}

QVector<Cards*> Strategy::FindHand_Triple12(Card::cardpoint beginPoint, int count)
{
    QVector<Cards*> result;
    if (m_cards.isempty()) return result;

    QVector<Card::cardpoint> triplePoints;
    for (int p = Card::Card_3; p < Card::Card_end; ++p) {
        Card::cardpoint point = Card::cardpoint(p);
        if (m_cards.GetpointCard(point) >= 3) {
            triplePoints.append(point);
        }
    }

    for (Card::cardpoint tp : triplePoints) {
        if (tp < beginPoint) continue;

        Cards* three = Getsamepointcard(tp, 3);
        if (three->isempty()) {
            delete three;
            continue;
        }

        if (count == 0) {
            result.append(three);
            continue;
        }

        Cards remain = m_cards;
        remain.remove(three);

        QVector<Cards*> attaches;
        for (int q = Card::Card_3; q < Card::Card_end && attaches.size() < count; ++q) {
            Card::cardpoint attachPoint = Card::cardpoint(q);
            if (remain.GetpointCard(attachPoint) >= count) {
                Cards* att = Getsamepointcard(attachPoint, count);
                if (!att->isempty()) {
                    attaches.append(att);
                } else {
                    delete att;
                }
            }
        }

        if (attaches.size() >= count) {
            for (int i = 0; i < count; ++i) {
                for (auto card : attaches[i]->Listcardssort()) {
                    three->add(card);
                }
            }
            result.append(three);
            for (auto a : attaches) delete a;
        } else {
            delete three;
            for (auto a : attaches) delete a;
        }
    }
    return result;
}

QVector<Cards*> Strategy::FindHand_Plane12(Card::cardpoint beginPoint, int attachCount)
{
    QVector<Cards*> result;
    if (m_cards.isempty()) return result;

    for (int p = beginPoint; p <= Card::Card_A; ++p) {
        Card::cardpoint point1 = Card::cardpoint(p);
        Card::cardpoint point2 = Card::cardpoint(p + 1);

        Cards* t1 = Getsamepointcard(point1, 3);
        Cards* t2 = Getsamepointcard(point2, 3);

        if (t1->isempty() || t2->isempty()) {
            delete t1;
            delete t2;
            continue;
        }

        Cards* plane = new Cards();
        for (auto card : t1->Listcardssort()) plane->add(card);
        for (auto card : t2->Listcardssort()) plane->add(card);

        if (attachCount > 0) {
            Cards remain = m_cards;
            remain.remove(plane);

            QVector<Cards*> attaches = FindHand_Sing123(Card::Card_3, attachCount);
            if (attaches.size() >= attachCount) {
                for (int i = 0; i < attachCount; ++i) {
                    for (auto card : attaches[i]->Listcardssort()) {
                        plane->add(card);
                    }
                }
            }
            for (auto a : attaches) delete a;
        }

        result.append(plane);
        delete t1;
        delete t2;
    }
    return result;
}

Cards* Strategy::Getpair(Card::cardpoint point)
{
    Cards* seq = new Cards();
    bool found = false;

    for (int p = Card::Card_3; p <= Card::Card_Q; ++p) {
        Card::cardpoint point1 = Card::cardpoint(p);
        Card::cardpoint point2 = Card::cardpoint(p + 1);
        Card::cardpoint point3 = Card::cardpoint(p + 2);

        Cards* t1 = Getsamepointcard(point1, 2);
        Cards* t2 = Getsamepointcard(point2, 2);
        Cards* t3 = Getsamepointcard(point3, 2);

        if (!t1->isempty() && !t2->isempty() && !t3->isempty()) {
            for (auto card : t1->Listcardssort()) seq->add(card);
            for (auto card : t2->Listcardssort()) seq->add(card);
            for (auto card : t3->Listcardssort()) seq->add(card);
            found = true;
        }

        delete t1;
        delete t2;
        delete t3;

        if (found) break;
    }

    if (!found) {
        delete seq;
        return new Cards();
    }
    return seq;
}

Cards* Strategy::Getsim(Card::cardpoint point)
{
    Q_UNUSED(point)
    Cards* seq = new Cards();
    bool found = false;

    for (int p = Card::Card_3; p <= Card::Card_10; ++p) {
        Card::cardpoint point1 = Card::cardpoint(p);
        Card::cardpoint point2 = Card::cardpoint(p + 1);
        Card::cardpoint point3 = Card::cardpoint(p + 2);

        Cards* t1 = Getsamepointcard(point1, 1);
        Cards* t2 = Getsamepointcard(point2, 1);
        Cards* t3 = Getsamepointcard(point3, 1);

        if (!t1->isempty() && !t2->isempty() && !t3->isempty()) {
            for (auto card : t1->Listcardssort()) seq->add(card);
            for (auto card : t2->Listcardssort()) seq->add(card);
            for (auto card : t3->Listcardssort()) seq->add(card);
            found = true;
        }

        delete t1;
        delete t2;
        delete t3;

        if (found) break;
    }

    if (!found) {
        delete seq;
        return new Cards();
    }
    return seq;
}

QVector<Cards*> Strategy::FindHand_parisim(Info info)
{
    QVector<Cards*> result;
    if (m_cards.isempty()) return result;

    for (int p = info.begin; p <= info.end; ++p) {
        bool ok = true;
        Cards* seq = new Cards();

        for (int k = 0; k < info.base; ++k) {
            Card::cardpoint currentPoint = Card::cardpoint(p + k);
            if (currentPoint >= Card::Card_2) {
                ok = false;
                break;
            }

            Cards* cs = Getsamepointcard(currentPoint, info.number);
            if (cs->isempty()) {
                ok = false;
                delete cs;
                break;
            } else {
                for (auto card : cs->Listcardssort()) {
                    seq->add(card);
                }
                delete cs;
            }
        }

        if (ok) {
            result.append(seq);
        } else {
            delete seq;
        }
    }
    return result;
}

QVector<Cards> Strategy::findCardsByCount(int count)
{
    QVector<Cards> arr;
    if (count < 1 || count > 4) return arr;

    for (int p = Card::Card_3; p < Card::Card_end; ++p) {
        Card::cardpoint point = Card::cardpoint(p);
        if (m_cards.GetpointCard(point) == count) {
            Cards cs = findSamePointCardsValue(point, count);
            if (!cs.isempty()) {
                arr.append(cs);
            }
        }
    }
    return arr;
}

QVector<Cards*> Strategy::findCardsByCountVec(int count)
{
    QVector<Cards*> out;
    if (count < 1 || count > 4) return out;

    for (int p = Card::Card_3; p < Card::Card_end; ++p) {
        Card::cardpoint point = Card::cardpoint(p);
        if (m_cards.GetpointCard(point) == count) {
            Cards* cs = Getsamepointcard(point, count);
            if (!cs->isempty()) {
                out.append(cs);
            } else {
                delete cs;
            }
        }
    }
    return out;
}

Cards Strategy::findSamePointCardsValue(Card::cardpoint point, int count)
{
    Cards cs;
    if (count < 1 || count > 4) return cs;

    if (point == Card::Card_SJ || point == Card::Card_BJ) {
        if (count != 1) return cs;
        for (auto card : m_cards.Listcardssort()) {
            if (card.getcardpoint() == point) {
                cs.add(card);
                return cs;
            }
        }
        return cs;
    }

    int found = 0;
    for (auto card : m_cards.Listcardssort()) {
        if (card.getcardpoint() == point) {
            cs.add(card);
            ++found;
            if (found == count) break;
        }
    }
    if (found != count) {
        cs.clearcards();
    }
    return cs;
}
