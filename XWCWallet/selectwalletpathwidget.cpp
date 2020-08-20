#include "selectwalletpathwidget.h"
#include "ui_selectwalletpathwidget.h"
#include "wallet.h"
#include "commondialog.h"

#include <QDesktopServices>

SelectWalletPathWidget::SelectWalletPathWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SelectWalletPathWidget)
{
    ui->setupUi(this);



    InitWidget();
}

SelectWalletPathWidget::~SelectWalletPathWidget()
{
    delete ui;
}

void SelectWalletPathWidget::on_okBtn_clicked()
{
    qDebug() << "wallet path " << ui->pathLineEdit->text();
    XWCWallet::getInstance()->configFile->setValue("/settings/chainPath", ui->pathLineEdit->text());
    XWCWallet::getInstance()->appDataPath = ui->pathLineEdit->text();

    QString language = (ui->languageComboBox->currentIndex() == 0) ? "Simplified Chinese" : "English";
    XWCWallet::getInstance()->configFile->setValue("/settings/language",language);
    XWCWallet::getInstance()->language = language;

    QString path = ui->pathLineEdit->text() + "/wallet.json";
    QFileInfo info(path);
    if(info.exists())
    {
        XWCWallet::getInstance()->startExe();
        emit enter();
    }
    else
    {
        emit newOrImportWallet();
    }

}

void SelectWalletPathWidget::paintEvent(QPaintEvent *e)
{
//    QPainter painter(this);
//    painter.setPen(Qt::NoPen);
//    painter.setBrush(QBrush(QColor(255,255,255)));
//    painter.drawRect(0,0,228,24);
//    painter.drawPixmap(7,5,32,12,QPixmap(":/ui/wallet_ui/xwc_label_logo.png").scaled(32,12,Qt::IgnoreAspectRatio,Qt::SmoothTransformation));
//    painter.drawPixmap(94,38,36,36,QPixmap(":/ui/wallet_ui/logo_center.png").scaled(36,36,Qt::IgnoreAspectRatio,Qt::SmoothTransformation));

    QWidget::paintEvent(e);
}

void SelectWalletPathWidget::InitWidget()
{
    InitStyle();

    ui->languageComboBox->setCurrentIndex(1);
    connect(ui->languageComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(onLanguageChanged(QString)));

    ui->pathLineEdit->setText( XWCWallet::getInstance()->appDataPath);

    ui->label_version->setText(QString("v") + WALLET_VERSION);
}

void SelectWalletPathWidget::InitStyle()
{
    setAutoFillBackground(true);
    QPalette palette;
    palette.setBrush(QPalette::Window,  QBrush(QPixmap(":/ui/wallet_ui/cover.png").scaled(this->size())));
    setPalette(palette);


    QPalette pa;
    pa.setColor(QPalette::WindowText,QColor(255,255,255));
    ui->label_version->setPalette(pa);

    ui->closeBtn->setStyleSheet("QToolButton{background-image:url(:/ui/wallet_ui/cover_close.png);background-repeat: no-repeat;background-position: center;border: none;}");
    ui->okBtn->setStyleSheet("QToolButton{font: 12px \"Microsoft YaHei UI Light\";background-color:rgb(255,255,255); border:none;border-radius:3px;color: rgb(59, 22, 136);}" \
                             );


    ui->pathLineEdit->setStyleSheet("QLineEdit{max-height:32px;background: transparent;color: rgb(255,255,255);font: 12px \"Microsoft YaHei UI Light\";border:none;border-radius:0px;border-bottom:1px solid rgb(255,255,255);padding: 0px 10px 0px 6px;}"
                                    "QLineEdit:disabled{color: rgb(151,151,151);}");
}


void SelectWalletPathWidget::on_closeBtn_clicked()
{
    emit closeWallet();
}


void SelectWalletPathWidget::on_pathBtn_clicked()
{
    QString file = QFileDialog::getExistingDirectory(this, "Select the path to store the blockchain");
    if( !file.isEmpty())
    {
#ifdef WIN32
        file.replace("/","\\");
#endif
        ui->pathLineEdit->setText( file);
    }
}

void SelectWalletPathWidget::onLanguageChanged(const QString &arg1)
{
    // disconnect & connect,  because lower version of qt(before 5.11 tested) will clear combobox when you call retranslateUi()
    disconnect(ui->languageComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(onLanguageChanged(QString)));

    if(ui->languageComboBox->currentIndex() == 0)
    {
        XWCWallet::getInstance()->mainFrame->setLanguage("Simplified Chinese");
    }
    else if(ui->languageComboBox->currentIndex() == 1)
    {
        XWCWallet::getInstance()->mainFrame->setLanguage("English");
    }

    ui->retranslateUi(this);
    ui->label_version->setText(QString("v") + WALLET_VERSION);

    ui->languageComboBox->setCurrentText(arg1);
    connect(ui->languageComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(onLanguageChanged(QString)));

}

