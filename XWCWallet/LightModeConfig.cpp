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
    default_ip_port_vector.append(IP_Port("127.0.0.1", "50906"));
#else
    //xwc_node IP list at here: https://list.xwc.com/list.txt
    //福建泉州
    default_ip_port_vector.append(IP_Port("112.5.37.28", "23454"));
    default_ip_port_vector.append(IP_Port("112.5.37.28", "23455"));    
    default_ip_port_vector.append(IP_Port("117.24.6.145", "23454"));
    default_ip_port_vector.append(IP_Port("117.24.6.145", "23455"));

    //福建厦门
    default_ip_port_vector.append(IP_Port("27.159.82.205", "23454"));
    default_ip_port_vector.append(IP_Port("27.159.82.205", "23455"));

    //新加坡
    default_ip_port_vector.append(IP_Port("103.254.155.140", "23454"));
    default_ip_port_vector.append(IP_Port("103.254.155.140", "23455"));

    //日本东京
    default_ip_port_vector.append(IP_Port("113.212.90.171", "23454"));
    default_ip_port_vector.append(IP_Port("113.212.90.171", "23455"));

    //山东济南
    default_ip_port_vector.append(IP_Port("123.129.217.68", "23454"));
    default_ip_port_vector.append(IP_Port("123.129.217.68", "23455"));

    //中国香港
    default_ip_port_vector.append(IP_Port("154.209.69.80", "23454"));
    default_ip_port_vector.append(IP_Port("154.209.69.80", "23455"));
    default_ip_port_vector.append(IP_Port("154.222.23.66", "23454"));
    default_ip_port_vector.append(IP_Port("154.222.23.66", "23455"));
    default_ip_port_vector.append(IP_Port("154.86.17.133", "23454"));
    default_ip_port_vector.append(IP_Port("154.86.17.133", "23455"));

    //江苏扬州
    default_ip_port_vector.append(IP_Port("223.111.134.138", "23454"));
    default_ip_port_vector.append(IP_Port("223.111.134.138", "23455"));

    //美国
    default_ip_port_vector.append(IP_Port("23.106.63.80", "23454"));
    default_ip_port_vector.append(IP_Port("23.106.63.80", "23455"));
    default_ip_port_vector.append(IP_Port("69.64.39.121", "23454"));
    default_ip_port_vector.append(IP_Port("69.64.39.121", "23455"));

    //俄罗斯
    default_ip_port_vector.append(IP_Port("46.29.163.206", "23454"));
    default_ip_port_vector.append(IP_Port("46.29.163.206", "23455"));

    //法国
    default_ip_port_vector.append(IP_Port("62.75.216.82", "23454"));
    default_ip_port_vector.append(IP_Port("62.75.216.82", "23455"));
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
