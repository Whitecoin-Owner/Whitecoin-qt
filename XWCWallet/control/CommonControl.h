#ifndef COMMONCONTROL_H
#define COMMONCONTROL_H

#include <QLabel>

namespace CommonControl {

    class EllipsisLabel : public QLabel
    {
        Q_OBJECT
    public:
        explicit EllipsisLabel(QWidget* parent = nullptr);
        QString text();

    public slots:
        void setText(const QString &);
    private:
        QString realText;
    };

}


#endif // COMMONCONTROL_H
