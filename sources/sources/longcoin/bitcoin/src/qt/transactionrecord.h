// Copyright (c) 2011-2013 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QT_TRANSACTIONRECORD_H
#define BITCOIN_QT_TRANSACTIONRECORD_H

#include "amount.h"
#include "uint256.h"

#include <QList>
#include <QString>

class CWallet;
class CWalletTx;

/** 
 * UI model for transaction status. The transaction status is the part of a transaction that will change over time.
 * Модель пользовательского интерфейса для состояния транзакции. Статус транзакции является частью транзакции, которая со временем изменится.
 */
class TransactionStatus
{
public:
    TransactionStatus():
        countsForBalance(false), sortKey(""),
        matures_in(0), status(Offline), depth(0), open_for(0), cur_num_blocks(-1)
    { }

    enum Status {
        Confirmed,          /**< Have 6 or more confirmations (normal tx) or fully mature (mined tx) **/
        /// Normal (sent/received) transactions
        OpenUntilDate,      /**< Transaction not yet final, waiting for date */
        OpenUntilBlock,     /**< Transaction not yet final, waiting for block */
        Offline,            /**< Not sent to any other nodes **/
        Unconfirmed,        /**< Not yet mined into a block **/
        Confirming,         /**< Confirmed, but waiting for the recommended number of confirmations **/
        Conflicted,         /**< Conflicts with other transaction or mempool **/
        /// Generated (mined) transactions
        Immature,           /**< Mined but waiting for maturity */
        MaturesWarning,     /**< Transaction will likely not mature because no nodes have confirmed */
        NotAccepted         /**< Mined but not accepted */
    };

    /// Transaction counts towards available balance
    bool countsForBalance;
    /// Sorting key based on status
    std::string sortKey;

    /** @name Generated (mined) transactions
       @{*/
    int matures_in;
    /**@}*/

    /** @name Reported status
       @{*/
    Status status;
    qint64 depth;
    qint64 open_for; /**< Timestamp if status==OpenUntilDate, otherwise number
                      of additional blocks that need to be mined before
                      finalization */
    /**@}*/

    /** Current number of blocks (to know whether cached status is still valid) */
    int cur_num_blocks;
};

/** UI model for a transaction. A core transaction can be represented by multiple UI transactions if it has
    multiple outputs.
 */
class TransactionRecord
{
public:
    enum Type
    {
        Other,
        Generated,
        SendToAddress,
        SendToOther,
        RecvWithAddress,
        RecvFromOther,
        SendToSelf
    };

    /** Number of confirmation recommended for accepting a transaction */
    static const int RecommendedNumConfirmations = 6;

    TransactionRecord():
        hash(), time(0), type(Other), address(""), debit(0), credit(0), idx(0), 
        isLongTx(false), isCoinTx(true), isDataTx(false), addressTo(""), addressFrom(""), pubKeyHexTo(""), pubKeyHexFrom(""), longVersion(0), dataType(0), encryptionType(0), dataBodyText("")
    {
    }

    TransactionRecord(uint256 hash, qint64 time):
        hash(hash), time(time), type(Other), address(""), debit(0), credit(0), idx(0), 
        isLongTx(false), isCoinTx(true), isDataTx(false), addressTo(""), addressFrom(""), pubKeyHexTo(""), pubKeyHexFrom(""), longVersion(0), dataType(0), encryptionType(0), dataBodyText("")
    {
    }

    TransactionRecord(
        uint256 hash, qint64 time, Type type, const std::string &address, const CAmount& debit, const CAmount& credit
    ):
        hash(hash), time(time), type(type), address(address), debit(debit), credit(credit), idx(0), 
        isLongTx(false), isCoinTx(true), isDataTx(false), addressTo(""), addressFrom(""), pubKeyHexTo(""), pubKeyHexFrom(""), longVersion(0), dataType(0), encryptionType(0), dataBodyText("")
    {
    }

    /** 
	 * Decompose CWallet transaction to model transaction records.
	 * Разложите транзакцию CWallet для моделирования записей транзакций.
     */
    static bool showTransaction(const CWalletTx &wtx);
    static QList<TransactionRecord> decomposeTransaction(const CWallet *wallet, const CWalletTx &wtx);

    /** @name Immutable transaction attributes
      @{*/
    uint256 hash;
    qint64 time;
    Type type;
    std::string address;
    CAmount debit;
    CAmount credit;
    /**@}*/

    /** Subtransaction index, for sort key */
    int idx;

    // LONG 
    bool isLongTx;
    bool isCoinTx;
    bool isDataTx;
    
    std::string addressTo;
    std::string addressFrom;
    std::string pubKeyHexTo; 
    std::string pubKeyHexFrom;
    unsigned int longVersion;
    unsigned int dataType;
    unsigned int encryptionType;
    QString dataBodyText;

    /** Status: can change with block chain update */
    TransactionStatus status;

    /** Whether the transaction was sent/received with a watch-only address */
    bool involvesWatchAddress;

    /** Return the unique identifier for this transaction (part) */
	/** Возврат уникального идентификатора для этой транзакции (части) */
    QString getTxID() const;

    /** Format subtransaction id */
    static QString formatSubTxId(const uint256 &hash, int vout);



    /** Update status from core wallet tx.
     */
    void updateStatus(const CWalletTx &wtx);

    /** Return whether a status update is needed.
     */
    bool statusUpdateNeeded();
	
	/* */
	QString getOP_RETURN_DATA() const;
};

#endif // BITCOIN_QT_TRANSACTIONRECORD_H
