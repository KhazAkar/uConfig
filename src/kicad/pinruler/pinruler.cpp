#include "pinruler.h"

#include <QDebug>
#include <qmath.h>

PinRuler::PinRuler(RulesSet *ruleSet)
{
    _ruleSet = ruleSet;
}

void PinRuler::organize(Component *component)
{
    int x, y;
    PinClass *defaultClass = pinClass("default");
    foreach (Pin *pin, component->pins())
    {
        const QList<PinRule*> &rules = _ruleSet->rulesForPin(pin->name());
        if (rules.isEmpty())
        {
            pin->setPinType(Pin::NotVisible);
            defaultClass->addPin(pin);
        }
        else
        {
            QString className = rules.first()->className(pin->name());
            PinClass *mpinClass = pinClass(className);
            pin->setPinType(Pin::Normal);
            pin->setLength(rules.first()->length());
            mpinClass->addPin(pin);
        }
    }

    // sideSize computation and side class list assignment
    QList<PinClass *> topSide, bottomSide;
    QList<PinClass *> leftSide, rightSide;
    QList<PinClass *> aSide; // last assign
    QSize topSize = QSize(0, 0), bottomSize = QSize(0, 0);
    QSize leftSize = QSize(0, 0), rightSize = QSize(0, 0);
    foreach (PinClass *mpinClass, _pinClasses)
    {
        QRect rect = mpinClass->rect();
        switch (mpinClass->positionValue())
        {
        case ClassRule::PositionTop:
            topSide.append(mpinClass);
            topSize.rwidth() += rect.width();
            if (rect.height() > topSize.height())
                topSize.setHeight(rect.height());
            break;
        case ClassRule::PositionBottom:
            bottomSide.append(mpinClass);
            bottomSize.rwidth() += rect.width();
            if (rect.height() > bottomSize.height())
                bottomSize.setHeight(rect.height());
            break;
        case ClassRule::PositionLeft:
            leftSide.append(mpinClass);
            leftSize.rheight() += rect.height();
            if (rect.width() > leftSize.width())
                leftSize.setWidth(rect.width());
            break;
        case ClassRule::PositionRight:
            rightSide.append(mpinClass);
            rightSize.rheight() += rect.height();
            if (rect.width() > rightSize.width())
                rightSize.setWidth(rect.width());
            break;
        case ClassRule::PositionASide:
            aSide.append(mpinClass);
            break;
        }
    }

    // aside list assignment and right and left sides updates
    foreach (PinClass *mpinClass, aSide)
    {
        QRect rect = mpinClass->rect();
        if (leftSize.height() < rightSize.height())
        {
            mpinClass->setPosition(ClassRule::PositionLeft);
            leftSide.append(mpinClass);
            leftSize.rheight() += rect.height();
            if (rect.width() > leftSize.width())
                leftSize.setWidth(rect.width());
        }
        else
        {
            mpinClass->setPosition(ClassRule::PositionRight);
            rightSide.append(mpinClass);
            rightSize.rheight() += rect.height();
            if (rect.width() > rightSize.width())
                rightSize.setWidth(rect.width());
        }
    }

    // margins
    leftSize.rheight() += (leftSide.size() - 1) * 100;
    rightSize.rheight() += (rightSide.size() - 1) * 100;
    leftSize.rwidth() += 100;
    rightSize.rwidth() += 100;

    // placement
    int sideX = qMax((leftSize.width() + rightSize.width()) / 2 + 300, qMax(topSize.width() / 2, bottomSize.width() / 2));
    sideX = (sideX / 100) * 100; // grid align KLC4.1
    int sideY = qMax(leftSize.height(), rightSize.height()) / 2;
    sideY = (qCeil(sideY / 100) + 1) * 100; // grid align KLC4.1

    x = -sideX;
    y = -sideY+100;
    if (leftSize.height() < rightSize.height())
        y += (rightSize.height() - leftSize.height()) / 2;
    foreach (PinClass *mpinClass, leftSide)
    {
        mpinClass->placePins(QPoint(x, y));
        y += mpinClass->rect().height() + 100;
    }

    x = sideX;
    y = -sideY+100;
    if (rightSize.height() < leftSize.height())
        y += (leftSize.height() - rightSize.height()) / 2;
    foreach (PinClass *mpinClass, rightSide)
    {
        mpinClass->placePins(QPoint(x, y));
        y += mpinClass->rect().height() + 100;
    }

    x = -topSize.width() / 2;
    x = qCeil(x / 100) * 100; // grid align KLC4.1
    y = -sideY - 300;
    foreach (PinClass *mpinClass, topSide)
    {
        mpinClass->placePins(QPoint(x, y));
        x += mpinClass->rect().width() + 100;
    }

    x = -bottomSize.width() / 2;
    x = qCeil(x / 100) * 100; // grid align KLC4.1
    y = sideY + 300;
    foreach (PinClass *mpinClass, bottomSide)
    {
        mpinClass->placePins(QPoint(x, y));
        x += mpinClass->rect().width() + 100;
    }

    // rect compute
    // TODO review this part
    QRect rect;
    int pinLenght = component->pins()[0]->length();
    rect.setLeft(-sideX + pinLenght);
    rect.setWidth(sideX * 2 - pinLenght * 2 + 1);
    rect.setTop(-sideY);
    rect.setHeight(sideY * 2 + 1);
    component->setRect(rect);

    // debug
    /*qDebug()<<"";
    qDebug()<<sideX<<sideY;
    qDebug()<<topSize<<bottomSize;
    qDebug()<<leftSize<<rightSize;
    foreach (PinClass *mpinClass, _pinClasses)
    {
        qDebug()<<">"<<mpinClass->className()<<mpinClass->positionStr()<<mpinClass->sortStr()<<mpinClass->sortPattern()<<mpinClass->rect();
        foreach (Pin *pin, mpinClass->pins())
        {
            //qDebug()<<" - "<<pin->name()<<pin->pos();
        }
    }*/

    foreach (PinClass *mpinClass, _pinClasses)
        delete mpinClass;
    _pinClasses.clear();
}

RulesSet *PinRuler::ruleSet() const
{
    return _ruleSet;
}

void PinRuler::setRuleSet(RulesSet *ruleSet)
{
    delete _ruleSet;
    _ruleSet = ruleSet;
}

PinClass *PinRuler::addClass(const QString &className)
{
    PinClass *pinClass = new PinClass(className);

    const QList<ClassRule*> &rules = _ruleSet->rulesForClass(className);
    pinClass->applyRules(rules);

    _pinClasses.insert(className, pinClass);
    return pinClass;
}

PinClass *PinRuler::pinClass(const QString &className)
{
    QMap<QString, PinClass *>::const_iterator classFind = _pinClasses.constFind(className);
    if (classFind == _pinClasses.cend())
        return addClass(className);
    else
        return *classFind;
}
