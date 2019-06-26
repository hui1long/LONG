// Copyright (c) 2011-2013 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "transactiondescdialog.h"
#include "ui_transactiondescdialog.h"

#include "transactiontablemodel.h"

#include <QModelIndex>


#include "walletframe.h"
#include "walletmodel.h"

TransactionDescDialog::TransactionDescDialog(const QModelIndex &idx, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TransactionDescDialog)
{
    

    ui->setupUi(this);
    connect(ui->replyToSenderPushButton, SIGNAL(clicked()), this, SLOT(replyToSenderPushButtonClicked()));
    connect(ui->replyToChatPushButton, SIGNAL(clicked()), this, SLOT(replyToChatPushButtonClicked()));

    QString desc = idx.data(TransactionTableModel::LongDescriptionRole).toString();
    ui->detailText->setHtml(desc);
    
    isLongTx = idx.data(TransactionTableModel::isLongTxRole).toBool();
    
    ui->replyToSenderPushButton->setText(tr("Reply to sender"));
    ui->replyToChatPushButton->setText(tr("Reply to chat"));

    if (isLongTx) {
        toHex = idx.data(TransactionTableModel::toHexRole).toString();
        fromHex = idx.data(TransactionTableModel::fromHexRole).toString();

        ui->replyToSenderPushButton->setEnabled(true);
        ui->replyToSenderPushButton->setVisible(true); 

        ui->replyToChatPushButton->setEnabled(true);
        ui->replyToChatPushButton->setVisible(true);
    } else {
        ui->replyToSenderPushButton->setEnabled(false);
        ui->replyToSenderPushButton->setVisible(false);

        ui->replyToChatPushButton->setEnabled(false);
        ui->replyToChatPushButton->setVisible(false);
    }
}

TransactionDescDialog::~TransactionDescDialog()
{
    delete ui;
}


void TransactionDescDialog::replyToSenderPushButtonClicked()
{
    Q_EMIT TransactionDescDialog::replyToPushButtonClicked(fromHex);
    reject();
}

void TransactionDescDialog::replyToChatPushButtonClicked()
{
    Q_EMIT TransactionDescDialog::replyToPushButtonClicked(toHex);
    reject();
}