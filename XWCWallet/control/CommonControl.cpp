#include "CommonControl.h"

CommonControl::EllipsisLabel::EllipsisLabel(QWidget *parent)
    : QLabel (parent)
{

}

QString CommonControl::EllipsisLabel::text()
{
    return realText;
}

void CommonControl::EllipsisLabel::setText(const QString &t)
{
    realText = t;

    QFontMetrics fontWidth(this->font());
    QString showText = fontWidth.elidedText(t, Qt::ElideRight, this->width());

    QLabel::setText(showText);
}
