// Copyright (c) 2011-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "sendcoinsentry.h"
#include "ui_sendcoinsentry.h"

#include "addressbookpage.h"
#include "addresstablemodel.h"
#include "guiutil.h"
#include "optionsmodel.h"
#include "platformstyle.h"
#include "walletmodel.h"

#include "utilstrencodings.h"

#include <QApplication>
#include <QClipboard>

SendCoinsEntry::SendCoinsEntry(const PlatformStyle *platformStyle, QWidget *parent) :
    QStackedWidget(parent),
    ui(new Ui::SendCoinsEntry),
    model(0),
    platformStyle(platformStyle)
{
    ui->setupUi(this);

    ui->addressBookButton->setIcon(platformStyle->SingleColorIcon(":/icons/address-book"));
    ui->addressBookFromButton->setIcon(platformStyle->SingleColorIcon(":/icons/address-book"));
    ui->pasteButton->setIcon(platformStyle->SingleColorIcon(":/icons/editpaste"));
    ui->deleteButton->setIcon(platformStyle->SingleColorIcon(":/icons/remove"));
    ui->deleteButton_is->setIcon(platformStyle->SingleColorIcon(":/icons/remove"));
    ui->deleteButton_s->setIcon(platformStyle->SingleColorIcon(":/icons/remove"));

    setCurrentWidget(ui->SendCoins);

    if (platformStyle->getUseExtraSpacing())
        ui->payToLayout->setSpacing(4);
#if QT_VERSION >= 0x040700
    ui->addAsLabel->setPlaceholderText(tr("Enter a label for this address to add it to your address book"));
#endif


// LONG
    // получатель
    ui->payToPubKeyLabel->setEnabled(false);
    ui->payToPubKeyLabel->setVisible(false);
    ui->payToPubKeyHexLineEdit->setEnabled(false);
    ui->payToPubKeyHexLineEdit->setVisible(false);
    ui->payToPubKeyHexLineEdit->clear();
    
    // от кого отправка, для отправки данных
    ui->addressBookFromButton->setEnabled(false); // кнопка адресной книги
    ui->addressBookFromButton->setVisible(false);
    ui->payFromLabel->setEnabled(false); // Подпись "отправитель"
    ui->payFromLabel->setVisible(false);
    ui->payFrom->setEnabled(false); // Строка с адресом отправителя
    ui->payFrom->setVisible(false);
    ui->payFrom->setReadOnly(true);
    ui->payFrom->clear();
    ui->payFromPubKeyHexLabel->setEnabled(false); // PubKeyHex надпись
    ui->payFromPubKeyHexLabel->setVisible(false);
    ui->payFromPubKeyHex->setEnabled(false); // PubKeyHex отправителя
    ui->payFromPubKeyHex->setVisible(false);
    ui->payFromPubKeyHex->setReadOnly(true);
    ui->payFromPubKeyHex->clear();

	ui->transactionTypeCoinsRadioButton->setChecked(true);
	ui->payAmount->setEnabled(true);
    ui->payAmount->setValue(0);
	ui->checkboxSubtractFeeFromAmount->setEnabled(true);
    ui->checkboxSubtractFeeFromAmount->setChecked(false);
	
	ui->transactionTypeDataRadioButton->setChecked(false);
	ui->dataTypeComboBox->setEnabled(false);
    ui->dataTypeComboBox->clear();

    ui->encryptionTypeComboBox->setEnabled(false);
    ui->encryptionTypeComboBox->clear();
    ui->encryptionParam1Label->setEnabled(false);
    ui->encryptionParam1Label->setVisible(false);
    ui->encryptionParam1LineEdit->setEnabled(false);
    ui->encryptionParam1LineEdit->setVisible(false);
    ui->encryptionParam1LineEdit->clear();

	ui->dataBodyTextLineEdit->setEnabled(false);
	ui->dataBodyTextLineEdit->clear();
    ui->dataBodyTextLineEdit->setMaxLength(255); // максимальная длинна символов , возможно доп проверка на кол-во байт
	
    // normal bitcoin address field
    GUIUtil::setupAddressWidget(ui->payTo, this);
    // just a label for displaying bitcoin address(es)
    ui->payTo_is->setFont(GUIUtil::fixedPitchFont());

    // Connect signals
    connect(ui->payAmount, SIGNAL(valueChanged()), this, SIGNAL(payAmountChanged()));
    connect(ui->checkboxSubtractFeeFromAmount, SIGNAL(toggled(bool)), this, SIGNAL(subtractFeeFromAmountChanged()));
    connect(ui->deleteButton, SIGNAL(clicked()), this, SLOT(deleteClicked()));
    connect(ui->deleteButton_is, SIGNAL(clicked()), this, SLOT(deleteClicked()));
    connect(ui->deleteButton_s, SIGNAL(clicked()), this, SLOT(deleteClicked()));

    //LONG
    connect(ui->transactionTypeCoinsRadioButton, SIGNAL(clicked(bool)), this, SLOT(changeTransactionType()));
    connect(ui->transactionTypeDataRadioButton, SIGNAL(clicked(bool)), this, SLOT(changeTransactionType()));
    connect(ui->encryptionTypeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeEncryptionType()));
}

SendCoinsEntry::~SendCoinsEntry()
{
    delete ui;
}


void SendCoinsEntry::changeTransactionType()
{
    if (ui->transactionTypeCoinsRadioButton->isChecked()) {
        ui->payAmount->setEnabled(true);
        ui->checkboxSubtractFeeFromAmount->setEnabled(true);
	
        ui->dataTypeComboBox->setEnabled(false);
        ui->dataTypeComboBox->setCurrentIndex(0); //default
        ui->encryptionTypeComboBox->setEnabled(false);
        ui->encryptionTypeComboBox->setCurrentIndex(0); //default
        ui->encryptionParam1Label->setEnabled(false);
        ui->encryptionParam1Label->setVisible(false);
        ui->encryptionParam1LineEdit->setEnabled(false);
        ui->encryptionParam1LineEdit->setVisible(false);
        ui->encryptionParam1LineEdit->clear();
        ui->dataBodyTextLineEdit->setEnabled(false);
        ui->dataBodyTextLineEdit->clear();

        ui->payToPubKeyLabel->setEnabled(false);
        ui->payToPubKeyLabel->setVisible(false);
        ui->payToPubKeyHexLineEdit->setEnabled(false);
        ui->payToPubKeyHexLineEdit->setVisible(false);

        // от кого отправка, для отправки данных
        ui->addressBookFromButton->setEnabled(false); // кнопка адресной книги
        ui->addressBookFromButton->setVisible(false);
        ui->payFromLabel->setEnabled(false); // Подпись "отправитель"
        ui->payFromLabel->setVisible(false);
        ui->payFrom->setEnabled(false); // Строка с адресом отправителя
        ui->payFrom->setVisible(false);
        ui->payFrom->setReadOnly(true);
        ui->payFrom->clear();
        ui->payFromPubKeyHexLabel->setEnabled(false); // PubKeyHex надпись
        ui->payFromPubKeyHexLabel->setVisible(false);
        ui->payFromPubKeyHex->setEnabled(false); // PubKeyHex отправителя
        ui->payFromPubKeyHex->setVisible(false);
        ui->payFromPubKeyHex->setReadOnly(true);
        ui->payFromPubKeyHex->clear();

    } else if (ui->transactionTypeDataRadioButton->isChecked()) {
        ui->payToPubKeyLabel->setEnabled(true);
        ui->payToPubKeyLabel->setVisible(true);
        ui->payToPubKeyHexLineEdit->setEnabled(true);
        ui->payToPubKeyHexLineEdit->setVisible(true);

        ui->payAmount->setEnabled(false);
        ui->payAmount->setValue(0);
        ui->checkboxSubtractFeeFromAmount->setEnabled(false);
        ui->checkboxSubtractFeeFromAmount->setChecked(false);

        ui->dataTypeComboBox->setEnabled(true);
        ui->encryptionTypeComboBox->setEnabled(true);
        ui->dataBodyTextLineEdit->setEnabled(true);

        // от кого отправка, для отправки данных
        ui->addressBookFromButton->setEnabled(true); // кнопка адресной книги
        ui->addressBookFromButton->setVisible(true);
        ui->payFromLabel->setEnabled(true); // Подпись "отправитель"
        ui->payFromLabel->setVisible(true);
        ui->payFrom->setEnabled(true); // Строка с адресом отправителя
        ui->payFrom->setVisible(true);
        ui->payFrom->setReadOnly(true);
        ui->payFrom->clear();
        ui->payFromPubKeyHexLabel->setEnabled(true); // PubKeyHex надпись
        ui->payFromPubKeyHexLabel->setVisible(true);
        ui->payFromPubKeyHex->setEnabled(true); // PubKeyHex отправителя
        ui->payFromPubKeyHex->setVisible(true);
        ui->payFromPubKeyHex->setReadOnly(true);
        ui->payFromPubKeyHex->clear();
    }
}


void SendCoinsEntry::changeEncryptionType() 
{
    if (ui->encryptionTypeComboBox->currentIndex() == 0) { // без шифрования
    /*
        ui->encryptionParam1Label->setEnabled(false);
        ui->encryptionParam1Label->setVisible(false);
        ui->encryptionParam1Label->setText("Param 1:"); //Имя параметра
        ui->encryptionParam1LineEdit->setEnabled(false);
        ui->encryptionParam1LineEdit->setVisible(false);
        ui->encryptionParam1LineEdit->clear();
    */


    } else if (ui->encryptionTypeComboBox->currentIndex() == 1) { // шифрование на публчный ключ
        
    /*
        ui->encryptionParam1Label->setEnabled(true);
        ui->encryptionParam1Label->setVisible(true);
        ui->encryptionParam1Label->setText("PubKeyHex:"); //Имя параметра
        ui->encryptionParam1LineEdit->setEnabled(true);
        ui->encryptionParam1LineEdit->setVisible(true);
        ui->encryptionParam1LineEdit->clear();
        ui->encryptionParam1LineEdit->setMaxLength(2*33); // 33 - сжатый, 65 - не сжатый пуб кей
    */

        // Поиск публичного ключа по адресу биткоина
        QString payToAddress = ui->payTo->text();
        CKeyID payToPubKeyID;
        CBitcoinAddress(payToAddress.toStdString()).GetKeyID(payToPubKeyID);
        CPubKey payToPubKey;
        if (model->getPubKey(payToPubKeyID, payToPubKey)) { // публичный ключ получателя в кошельке есть
            QString qstrPayToPubKeyHex = QString::fromStdString(HexStr(payToPubKey.begin(), payToPubKey.end()));
            ui->payToPubKeyHexLineEdit->setText(qstrPayToPubKeyHex);
        } else { // публичного ключа получателя в кошельке нет
            //ui->payToPubKeyHexLineEdit->setText(tr("Need the public key of the recipient in HEX format!"));
        }

    }
}

void SendCoinsEntry::on_pasteButton_clicked()
{
    // Paste text from clipboard into recipient field
    ui->payTo->setText(QApplication::clipboard()->text());
}

void SendCoinsEntry::on_addressBookButton_clicked()
{
    if(!model)
        return;
    AddressBookPage dlg(platformStyle, AddressBookPage::ForSelection, AddressBookPage::SendingTab, this);
    dlg.setModel(model->getAddressTableModel());
    if(dlg.exec())
    {
        ui->payTo->setText(dlg.getReturnValue());
        ui->payAmount->setFocus();
    }
}
void SendCoinsEntry::on_addressBookFromButton_clicked()
{
    if(!model)
        return;
    AddressBookPage dlg(platformStyle, AddressBookPage::ForSelection, AddressBookPage::ReceivingTab, this);
    dlg.setModel(model->getAddressTableModel());
    if(dlg.exec())
    {
        ui->payFrom->setText(dlg.getReturnValue());
        ui->dataBodyTextLineEdit->setFocus();
    }
}
void SendCoinsEntry::on_transactionTypeDataRadioButton_toggled()
{
    changeTransactionType();
}
void SendCoinsEntry::on_payTo_textChanged(const QString &address)
{
    updateLabel(address);
    updatePubKeyHexTo(address);
}

void SendCoinsEntry::on_payToPubKeyHexLineEdit_textChanged(const QString &pubKeyHex)
{
    CPubKey payToPubKey(ParseHex(pubKeyHex.toStdString()));
    //if (model && model->validatePubKeyHex(pubKeyHex)) {
    if (payToPubKey.IsFullyValid()) {
        CBitcoinAddress addressParsed(payToPubKey);
        QString address;
        address = QString::fromStdString(addressParsed.ToString());
        ui->payTo->setText(address);
    } else {
        ui->payTo->clear();
    }
}

void SendCoinsEntry::on_payFrom_textChanged(const QString &address)
{
    updatePubKeyHexFrom(address);
}

void SendCoinsEntry::setModel(WalletModel *model)
{
    this->model = model;

    if (model && model->getOptionsModel())
        connect(model->getOptionsModel(), SIGNAL(displayUnitChanged(int)), this, SLOT(updateDisplayUnit()));

    clear();
}

void SendCoinsEntry::clear()
{
    // clear UI elements for normal payment
    ui->payTo->clear();
    ui->addAsLabel->clear();
    ui->payAmount->clear();
    ui->checkboxSubtractFeeFromAmount->setCheckState(Qt::Unchecked);
    ui->messageTextLabel->clear();
    ui->messageTextLabel->hide();
    ui->messageLabel->hide();
	
    // LONG
    QList<QString> dataTypesList; // Список отправляемых типов
    dataTypesList.append(tr("Text Data")); // 0x00

    QList<QString> encryptionTypesList; // Список вариантов шифрования
    //encryptionTypesList.append(tr("Unencrypted Data")); // 0x00 без шифрования 
    encryptionTypesList.append(tr("Encrypt on Shared Secret")); // 0x01 // aes_256_cbc шифрование на пубкей

    ui->payToPubKeyLabel->setEnabled(false);
    ui->payToPubKeyLabel->setVisible(false);
    ui->payToPubKeyHexLineEdit->setEnabled(false);
    ui->payToPubKeyHexLineEdit->setVisible(false);
    ui->payToPubKeyHexLineEdit->clear();
    
    // от кого отправка, для отправки данных
    ui->addressBookFromButton->setEnabled(false); // кнопка адресной книги
    ui->addressBookFromButton->setVisible(false);
    ui->payFromLabel->setEnabled(false); // Подпись "отправитель"
    ui->payFromLabel->setVisible(false);
    ui->payFrom->setEnabled(false); // Строка с адресом отправителя
    ui->payFrom->setVisible(false);
    ui->payFrom->setReadOnly(true);
    ui->payFrom->clear();
    ui->payFromPubKeyHexLabel->setEnabled(false); // PubKeyHex надпись
    ui->payFromPubKeyHexLabel->setVisible(false);
    ui->payFromPubKeyHex->setEnabled(false); // PubKeyHex отправителя
    ui->payFromPubKeyHex->setVisible(false);
    ui->payFromPubKeyHex->setReadOnly(true);
    ui->payFromPubKeyHex->clear();

	ui->transactionTypeCoinsRadioButton->setChecked(true);
	ui->payAmount->setEnabled(true);
    ui->payAmount->clear();
	ui->checkboxSubtractFeeFromAmount->setEnabled(true);
    ui->checkboxSubtractFeeFromAmount->setChecked(false);
	
	ui->transactionTypeDataRadioButton->setChecked(false);
	ui->dataTypeComboBox->setEnabled(false);
    ui->dataTypeComboBox->clear();
    ui->dataTypeComboBox->insertItems(0, dataTypesList);
    ui->dataTypeComboBox->setCurrentIndex(0);

	ui->encryptionTypeComboBox->setEnabled(false);
    ui->encryptionTypeComboBox->clear();
    ui->encryptionTypeComboBox->insertItems(0, encryptionTypesList);
    ui->encryptionTypeComboBox->setCurrentIndex(0);

    ui->encryptionParam1Label->setEnabled(false);
    ui->encryptionParam1Label->setVisible(false);
    ui->encryptionParam1Label->setText("Param 1:"); //Имя параметра
    ui->encryptionParam1LineEdit->setEnabled(false);
    ui->encryptionParam1LineEdit->setVisible(false);
    ui->encryptionParam1LineEdit->clear();

	ui->dataBodyTextLineEdit->setEnabled(false);
	ui->dataBodyTextLineEdit->clear();

    // clear UI elements for unauthenticated payment request
    ui->payTo_is->clear();
    ui->memoTextLabel_is->clear();
    ui->payAmount_is->clear();
    // clear UI elements for authenticated payment request
    ui->payTo_s->clear();
    ui->memoTextLabel_s->clear();
    ui->payAmount_s->clear();

    // update the display unit, to not use the default ("BTC")
    updateDisplayUnit();
}

void SendCoinsEntry::deleteClicked()
{
    Q_EMIT removeEntry(this);
}

bool SendCoinsEntry::validate()
{
    if (!model)
        return false;

    // Check input validity
    bool retval = true;

    // Skip checks for payment request
    if (recipient.paymentRequest.IsInitialized())
        return retval;

    if (!model->validateAddress(ui->payTo->text()))
    {
        ui->payTo->setValid(false);
        retval = false;
    }

    if (!ui->payAmount->validate())
    {
        retval = false;
    }

    // Sending a zero amount is invalid
    // Отправка отрицательной суммы недействительна.
    //if (ui->payAmount->value(0) <= 0)
    if (ui->payAmount->value(0) < 0)
    {
        ui->payAmount->setValid(false);
        retval = false;
    }

    // Reject dust outputs:
    // Отклонить выходы пыли:
    //if (retval && GUIUtil::isDust(ui->payTo->text(), ui->payAmount->value())) {
    //    ui->payAmount->setValid(false);
    //    retval = false;
    //}

    if (ui->encryptionTypeComboBox->currentIndex() == 1) { // шифрование на публчный ключ
        if (!model->validatePubKeyHex(ui->payFromPubKeyHex->text()))
        {
            retval = false;
        }
    }

    return retval;
}

SendCoinsRecipient SendCoinsEntry::getValue()
{
    // Payment request
    if (recipient.paymentRequest.IsInitialized())
        return recipient;

    // Normal payment
    recipient.address = ui->payTo->text();
    recipient.pubkeyhex = ui->payToPubKeyHexLineEdit->text();
    recipient.label = ui->addAsLabel->text();

    //LONG
    recipient.isCoinTx = ui->transactionTypeCoinsRadioButton->isChecked();
    recipient.isDataTx = ui->transactionTypeDataRadioButton->isChecked();

    recipient.dataType = (unsigned int)ui->dataTypeComboBox->currentIndex();
    recipient.encryptionType = (unsigned int)ui->encryptionTypeComboBox->currentIndex();
    recipient.encryptionParam1 = ui->payFromPubKeyHex->text();
    recipient.addressFrom = ui->payFrom->text();
    recipient.pubkeyhexFrom = ui->payFromPubKeyHex->text();
    recipient.dataBodyText = ui->dataBodyTextLineEdit->text();


    recipient.amount = ui->payAmount->value();
    recipient.message = ui->messageTextLabel->text();
    recipient.fSubtractFeeFromAmount = (ui->checkboxSubtractFeeFromAmount->checkState() == Qt::Checked);

    return recipient;
}

QWidget *SendCoinsEntry::setupTabChain(QWidget *prev)
{
    QWidget::setTabOrder(prev, ui->payTo);
    QWidget::setTabOrder(ui->payTo, ui->addAsLabel);
    QWidget *w = ui->payAmount->setupTabChain(ui->addAsLabel);
    QWidget::setTabOrder(w, ui->checkboxSubtractFeeFromAmount);
    QWidget::setTabOrder(ui->checkboxSubtractFeeFromAmount, ui->addressBookButton);
    QWidget::setTabOrder(ui->addressBookButton, ui->pasteButton);
    QWidget::setTabOrder(ui->pasteButton, ui->deleteButton);
    return ui->deleteButton;
}

void SendCoinsEntry::setValue(const SendCoinsRecipient &value)
{
    recipient = value;

    if (recipient.paymentRequest.IsInitialized()) // payment request
    {
        if (recipient.authenticatedMerchant.isEmpty()) // unauthenticated
        {
            ui->payTo_is->setText(recipient.address);
            ui->memoTextLabel_is->setText(recipient.message);
            ui->payAmount_is->setValue(recipient.amount);
            ui->payAmount_is->setReadOnly(true);
            setCurrentWidget(ui->SendCoins_UnauthenticatedPaymentRequest);
        }
        else // authenticated
        {
            ui->payTo_s->setText(recipient.authenticatedMerchant);
            ui->memoTextLabel_s->setText(recipient.message);
            ui->payAmount_s->setValue(recipient.amount);
            ui->payAmount_s->setReadOnly(true);
            setCurrentWidget(ui->SendCoins_AuthenticatedPaymentRequest);
        }
    }
    else // normal payment
    {
        // message
        ui->messageTextLabel->setText(recipient.message);
        ui->messageTextLabel->setVisible(!recipient.message.isEmpty());
        ui->messageLabel->setVisible(!recipient.message.isEmpty());

        ui->addAsLabel->clear();
        ui->payTo->setText(recipient.address); // this may set a label from addressbook
        if (!recipient.label.isEmpty()) // if a label had been set from the addressbook, don't overwrite with an empty label
            ui->addAsLabel->setText(recipient.label);
        ui->payAmount->setValue(recipient.amount);
    }
}

void SendCoinsEntry::setAddress(const QString &address)
{
    ui->payTo->setText(address);
    ui->payAmount->setFocus();
}

void SendCoinsEntry::setPubKeyHex(const QString &pubKeyHex)
{
    ui->transactionTypeDataRadioButton->setChecked(true);
    ui->payToPubKeyHexLineEdit->setText(pubKeyHex);
    //ui->payAmount->setFocus();
}

bool SendCoinsEntry::isClear()
{
    return ui->payTo->text().isEmpty() && ui->payTo_is->text().isEmpty() && ui->payTo_s->text().isEmpty();
}

void SendCoinsEntry::setFocus()
{
    ui->payTo->setFocus();
}

void SendCoinsEntry::updateDisplayUnit()
{
    if(model && model->getOptionsModel())
    {
        // Update payAmount with the current unit
        ui->payAmount->setDisplayUnit(model->getOptionsModel()->getDisplayUnit());
        ui->payAmount_is->setDisplayUnit(model->getOptionsModel()->getDisplayUnit());
        ui->payAmount_s->setDisplayUnit(model->getOptionsModel()->getDisplayUnit());
    }
}

bool SendCoinsEntry::updateLabel(const QString &address)
{
    if(!model)
        return false;

    // Fill in label from address book, if address has an associated label
    QString associatedLabel = model->getAddressTableModel()->labelForAddress(address);
    if(!associatedLabel.isEmpty())
    {
        ui->addAsLabel->setText(associatedLabel);
        return true;
    }

    return false;
}


bool SendCoinsEntry::updatePubKeyHexTo(const QString &address)
{
    if(!model)
        return false;

    // Fill in label from address book, if address has an associated label
    QString associatedLabel = model->getAddressTableModel()->pubKeyHexForAddress(address);
    if(!associatedLabel.isEmpty())
    {
        ui->payToPubKeyHexLineEdit->setText(associatedLabel);
        return true;
    }

    return false;
}

bool SendCoinsEntry::updatePubKeyHexFrom(const QString &address)
{
    if(!model)
        return false;

    // Fill in label from address book, if address has an associated label
    QString associatedLabel = model->getAddressTableModel()->pubKeyHexForAddress(address);
    if(!associatedLabel.isEmpty())
    {
        ui->payFromPubKeyHex->setText(associatedLabel);
        return true;
    }

    return false;
}