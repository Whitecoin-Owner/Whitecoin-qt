#include "LightModeConfig.h"
#include "ui_LightModeConfig.h"

#include "wallet.h"
#include "commondialog.h"
#include "extra/RegularExpression.h"

LightModeConfig::LightModeConfig(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LightModeConfig)
{
    ui->setupUi(this);

#ifdef TEST_WALLET
//    default_ip_port_vector.append(IP_Port("106.13.107.239", "19035"));
    default_ip_port_vector.append(IP_Port("127.0.0.1", "50906"));
#else
    default_ip_port_vector.append(IP_Port("47.56.106.1", "50806"));
    default_ip_port_vector.append(IP_Port("47.245.53.117", "60015"));
    default_ip_port_vector.append(IP_Port("112.5.37.28", "23456"));
    default_ip_port_vector.append(IP_Port("123.129.217.68", "23456"));
    default_ip_port_vector.append(IP_Port("27.159.82.205", "23456"));
    default_ip_port_vector.append(IP_Port("117.24.6.145", "23456"));
    default_ip_port_vector.append(IP_Port("223.111.134.138", "23456"));
    default_ip_port_vector.append(IP_Port("37.58.57.33", "23456"));
    default_ip_port_vector.append(IP_Port("23.106.33.176", "23456"));
    default_ip_port_vector.append(IP_Port("85.25.134.29", "23456"));
    default_ip_port_vector.append(IP_Port("46.29.163.206", "23456"));
    default_ip_port_vector.append(IP_Port("209.126.119.147", "23456"));
    default_ip_port_vector.append(IP_Port("107.148.209.50", "23456"));
    default_ip_port_vector.append(IP_Port("107.148.255.65", "23456"));
    default_ip_port_vector.append(IP_Port("107.148.255.66", "23456"));
    default_ip_port_vector.append(IP_Port("107.148.255.67", "23456"));
    default_ip_port_vector.append(IP_Port("107.148.255.68", "23456"));
    default_ip_port_vector.append(IP_Port("107.148.255.69", "23456"));
    default_ip_port_vector.append(IP_Port("107.148.255.70", "23456"));
    default_ip_port_vector.append(IP_Port("107.148.255.71", "23456"));
    default_ip_port_vector.append(IP_Port("107.148.255.72", "23456"));
    default_ip_port_vector.append(IP_Port("107.148.255.73", "23456"));
    default_ip_port_vector.append(IP_Port("107.148.255.74", "23456"));
    default_ip_port_vector.append(IP_Port("107.148.255.75", "23456"));
    default_ip_port_vector.append(IP_Port("107.148.255.76", "23456"));
    default_ip_port_vector.append(IP_Port("107.148.255.77", "23456"));
    default_ip_port_vector.append(IP_Port("103.96.3.217", "23456"));
    default_ip_port_vector.append(IP_Port("103.43.10.218", "23456"));
    default_ip_port_vector.append(IP_Port("103.43.10.219", "23456"));
    default_ip_port_vector.append(IP_Port("103.43.10.220", "23456"));
    default_ip_port_vector.append(IP_Port("103.43.10.221", "23456"));
    default_ip_port_vector.append(IP_Port("103.43.10.222", "23456"));
    default_ip_port_vector.append(IP_Port("127.0.0.1", "50806"));
#endif

    InitWidget();
}

LightModeConfig::~LightModeConfig()
{
    delete ui;
}

QString LightModeConfig::getIP()
{
    return ui->ipComboBox->currentText();
}

QString LightModeConfig::getPort()
{
    return ui->portLineEdit->text();
}

void LightModeConfig::InitWidget()
{
    InitStyle();

    ui->label_version->setText(QString("v") + WALLET_VERSION);

    ui->ipComboBox->setValidator( new QRegExpValidator(RegularExpression::getRegExp_IPV4()));
    ui->portLineEdit->setValidator( new QRegExpValidator(RegularExpression::getRegExp_port()));

    foreach (const IP_Port& v, default_ip_port_vector)
    {
        ui->ipComboBox->addItem(QString("%1:%2").arg(v.ip).arg(v.port), QVariant::fromValue(v));
    }
}

void LightModeConfig::InitStyle()
{
    setAutoFillBackground(true);
    QPalette palette;
    palette.setBrush(QPalette::Window,  QBrush(QPixmap(":/ui/wallet_ui/cover.png").scaled(this->size())));
    setPalette(palette);

    QPalette pa;
    pa.setColor(QPalette::WindowText,QColor(243,241,250));
    ui->label_version->setPalette(pa);

    ui->closeBtn->setStyleSheet("QToolButton{background-image:url(:/ui/wallet_ui/cover_close.png);background-repeat: no-repeat;background-position: center;border: none;}");
    ui->miniBtn->setStyleSheet("QToolButton{background-image:url(:/ui/wallet_ui/cover_min.png);background-repeat: no-repeat;background-position: center;border: none;}");

    ui->enterBtn->setStyleSheet("QToolButton{font: 12px \"Microsoft YaHei UI Light\";background-color:rgb(255,255,255); border:none;border-radius:3px;color: rgb(59, 22, 136);}" \
                                );


}

void LightModeConfig::on_closeBtn_clicked()
{
    emit closeWallet();
}

void LightModeConfig::on_miniBtn_clicked()
{
    emit minimum();
}

void LightModeConfig::on_enterBtn_clicked()
{
    if(!RegularExpression::testRegExp_IPV4(ui->ipComboBox->currentText()))
    {
        CommonDialog dia(CommonDialog::OkOnly);
        dia.setText(tr("Wrong IP!"));
        dia.pop();

        return;
    }

    if(!RegularExpression::testRegExp_port(ui->portLineEdit->text()))
    {
        CommonDialog dia(CommonDialog::OkOnly);
        dia.setText(tr("Wrong port!"));
        dia.pop();

        return;
    }

    emit enter();
}

void LightModeConfig::on_ipComboBox_currentIndexChanged(const QString &arg1)
{
    IP_Port v = ui->ipComboBox->currentData().value<IP_Port>();
    ui->ipComboBox->setCurrentText(v.ip);
    ui->portLineEdit->setText(v.port);
}
