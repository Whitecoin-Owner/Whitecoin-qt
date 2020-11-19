#ifndef ASSETICONITEM_H
#define ASSETICONITEM_H

#include <QWidget>

namespace Ui {
class AssetIconItem;
}

class AssetIconItem : public QWidget
{
    Q_OBJECT

public:
    explicit AssetIconItem(QWidget *parent = 0);
    ~AssetIconItem();

    void setAsset(QString assetSymbol);

    void setBackgroundColor(QString color);

private slots:
    void on_assetLabel_destroyed();

private:
    Ui::AssetIconItem *ui;
};

#endif // ASSETICONITEM_H
