#ifndef PINRULE_H
#define PINRULE_H

#include "kicad_global.h"

#include <QString>
#include <QStringList>

#include "rule.h"

class KICAD_EXPORT PinRule : public Rule
{
public:
    PinRule(const QString &selector = QString(".*"));

    virtual Type type() const;

    virtual bool setProperty(const QString &name, const QString &value);

    QString className(const QString &pinName = QString()) const;
    void setClassName(const QString &className);

    int length() const;
    void setLength(int length);

    bool lengthSet() const;

    static QStringList propsName;

protected:
    QString _className;

    int _length;
    bool _lengthSet;
};

#endif // PINRULE_H
