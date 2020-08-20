#include "GuardAccountPage.h"
#include "ui_GuardAccountPage.h"

#include "wallet.h"
#include "CreateSenatorDialog.h"
#include "SenatorChangeSenatorDialog.h"

GuardAccountPage::GuardAccountPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GuardAccountPage)
{
    ui->setupUi(this);
    ui->newSenatorBtn->setStyleSheet(TOOLBUTTON_STYLE_1);
    ui->changeSenator->setStyleSheet(TOOLBUTTON_STYLE_1);

    init();
}

GuardAccountPage::~GuardAccountPage()
{
    delete ui;
}

void GuardAccountPage::init()
{
    ui->changeSenator->setVisible(false);
    ui->accountComboBox->clear();
    QStringList accounts = XWCWallet::getInstance()->getMyGuards();
    if(accounts.size() > 0)
    {
        ui->accountComboBox->addItems(accounts);

        if(accounts.contains(XWCWallet::getInstance()->currentAccount))
        {
            ui->accountComboBox->setCurrentText(XWCWallet::getInstance()->currentAccount);
        }
    }
    else
    {
        ui->guardLabel->hide();
        ui->label->hide();
        ui->accountComboBox->hide();

        QLabel* label = new QLabel(this);
        label->setGeometry(QRect(ui->label->pos(), QSize(300,30)));
        label->setText(tr("There are no wallfacer accounts in the wallet."));
        label->setStyleSheet(NOACCOUNT_TIP_LABEL);
    }

    XWCWallet::getInstance()->mainFrame->installBlurEffect(ui->label_back);

    ui->reloadBtn->hide();
}

void GuardAccountPage::on_accountComboBox_currentIndexChanged(const QString &arg1)
{
    GuardInfo info = XWCWallet::getInstance()->allGuardMap.value(ui->accountComboBox->currentText());

    ui->guardIdLabel->setText(info.guardId);
    ui->addressLabel->setText(info.address);

    if(info.isFormal)
    {
        ui->guardTypeLabel->setText(tr("formal wallfacer"));
        ui->guardLabel->setPixmap(QPixmap(":/ui/wallet_ui/guard_formal.png"));
    }
    else
    {
        ui->guardTypeLabel->setText(tr("informal wallfacer"));
        ui->guardLabel->setPixmap(QPixmap(":/ui/wallet_ui/guard_normal.png"));
    }

    if("PERMANENT" == info.senatorType && info.isFormal)
    {
        ui->changeSenator->setVisible(true);
    }
    else
    {
        ui->changeSenator->setVisible(false);
    }
}

void GuardAccountPage::on_newSenatorBtn_clicked()
{
    CreateSenatorDialog dia;
    connect(&dia,&CreateSenatorDialog::CreateSenatorSuccess,this,&GuardAccountPage::init);
    dia.exec();
}

void GuardAccountPage::on_changeSenator_clicked()
{
    SenatorChangeSenatorDialog dia;
    dia.exec();
}

void GuardAccountPage::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setPen(QPen(QColor(239,242,245),Qt::SolidLine));
    painter.setBrush(QBrush(QColor(239,242,245),Qt::SolidPattern));

    painter.drawRect(rect());
}

void GuardAccountPage::on_reloadBtn_clicked()
{
    XWCWallet::getInstance()->loadWalletFile();
}
