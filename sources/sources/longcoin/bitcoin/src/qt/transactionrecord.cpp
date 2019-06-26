// Copyright (c) 2011-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "transactionrecord.h"

#include "base58.h"
#include "consensus/consensus.h"
#include "main.h"
#include "timedata.h"
#include "wallet/wallet.h"
#include "rpcrawtransaction.cpp"

#include "utilstrencodings.h"

#include <stdint.h>

#include <boost/foreach.hpp>

void convertVectorToByteArray(const std::vector<unsigned char> &vec, QByteArray &qba);
void convertVectorToByteArray(const std::vector<unsigned char> &vec, QByteArray &qba)
{
    unsigned int size=vec.size();

    qba.resize(size);

    for(unsigned int i=0; i<size; i++)
        qba[i]=(unsigned char)vec[i];
}

/* 
 * Return positive answer if transaction should be shown in list.
 * Верните положительный ответ, если транзакция должна отображаться в списке.
 */
bool TransactionRecord::showTransaction(const CWalletTx &wtx)
{
    if (wtx.IsCoinBase())
    {
        // Ensures we show generated coins / mined transactions at depth 1
		// Обеспечивает отображение сгенерированных монет / добытых транзакций на глубине 1
        if (!wtx.IsInMainChain())
        {
            return false;
        }
    }
    return true;
}

/*
 * Decompose CWallet transaction to model transaction records.
 * Разложите транзакцию CWallet для моделирования записей транзакций.
 */
QList<TransactionRecord> TransactionRecord::decomposeTransaction(const CWallet *wallet, const CWalletTx &wtx)
{
    QList<TransactionRecord> parts;
    int64_t nTime = wtx.GetTxTime();
    CAmount nCredit = wtx.GetCredit(ISMINE_ALL);
    CAmount nDebit = wtx.GetDebit(ISMINE_ALL);
    CAmount nNet = nCredit - nDebit;
    uint256 hash = wtx.GetHash();
    std::map<std::string, std::string> mapValue = wtx.mapValue;

    if (nNet > 0 || wtx.IsCoinBase()) {
        //
        // Credit
        //
        BOOST_FOREACH(const CTxOut& txout, wtx.vout) {
            isminetype mine = wallet->IsMine(txout);
			
            if (mine) {
				// полученные мной коинбейс транзакции
                TransactionRecord sub(hash, nTime);
                CTxDestination address;
                sub.idx = parts.size(); // sequence number
                sub.credit = txout.nValue;
                sub.involvesWatchAddress = mine & ISMINE_WATCH_ONLY;
                if (ExtractDestination(txout.scriptPubKey, address) && IsMine(*wallet, address)) {
                    // Received by Bitcoin Address
                    sub.type = TransactionRecord::RecvWithAddress;
                    sub.address = CBitcoinAddress(address).ToString();
                    sub.addressTo = sub.address; 

                    if (isLong(txout.scriptPubKey)) {
                        sub.isLongTx = true;
                        sub.isCoinTx = false;
                        sub.isDataTx = true;
                        getLongVersion(txout.scriptPubKey, sub.longVersion);

                        // TO
                        std::vector<unsigned char> vchToPubKey;
                        CPubKey toCPubKey;
                        CKeyID  toCPubKeyID;
                        CKey    toPrivKey;
                        getLongToPubKey(txout.scriptPubKey, vchToPubKey);
                        if (vchToPubKey.size() == 20) {
                            toCPubKeyID = uint160(vchToPubKey);
                            wallet->GetPubKey(toCPubKeyID, toCPubKey);
                        } else if (vchToPubKey.size() == 33 || vchToPubKey.size() == 65) {
                            toCPubKey.Set(vchToPubKey.begin(), vchToPubKey.end());
                            toCPubKeyID = toCPubKey.GetID();
                        }
                        if (wallet->HaveKey(toCPubKeyID)) {
                            pwalletMain->GetKey(toCPubKeyID, toPrivKey);
                        }                            
                        sub.address = CBitcoinAddress(toCPubKeyID).ToString();
                        sub.addressTo = sub.address;
                        sub.pubKeyHexTo = HexStr(toCPubKey.begin(), toCPubKey.end());

                        // FROM
                        std::vector<unsigned char> vchFromPubKey;
                        CPubKey fromCPubKey; // CPubKey fromCPubKey(fromPubKey);
                        CKeyID  fromCPubKeyID; //CKeyID  fromCPubKeyID = fromCPubKey.GetID();
                        CKey    fromPrivKey;
                        getLongFromPubKey(txout.scriptPubKey, vchFromPubKey);
                        if (vchFromPubKey.size() == 20) {
                            fromCPubKeyID = uint160(vchFromPubKey);
                            wallet->GetPubKey(fromCPubKeyID, fromCPubKey);
                        } else if (vchFromPubKey.size() == 33 || vchFromPubKey.size() == 65) {
                            fromCPubKey.Set(vchFromPubKey.begin(), vchFromPubKey.end());
                            fromCPubKeyID = fromCPubKey.GetID();
                        }
                        if (wallet->HaveKey(fromCPubKeyID)) {
                            pwalletMain->GetKey(fromCPubKeyID, fromPrivKey);
                        }                            
                        sub.addressFrom = CBitcoinAddress(fromCPubKeyID).ToString();
                        sub.pubKeyHexFrom = HexStr(fromCPubKey.begin(), fromCPubKey.end());
                        
                        // DATA Type
                        getTypeLongData(txout.scriptPubKey, sub.dataType);

                        // ENCRYPTION
                        getEncryptionLongData(txout.scriptPubKey, sub.encryptionType);
                        
                        // DATA
                        std::vector<unsigned char> vchDataBody;
                        std::vector<unsigned char> vchDecryptedDataBody;
                        getBodyLongData(txout.scriptPubKey, vchDataBody);
                        
                        if (sub.encryptionType == 1 && toPrivKey.IsValid() && fromCPubKey.IsFullyValid()) {
                            std::vector<unsigned char> vchSharedSecret;
                            toPrivKey.ComputSharedSecret(fromCPubKey, vchSharedSecret); // Shared Secret - общий секретный ключ ECDH 
                            
                            { // Шифрование aes_256_cbc на Shared Secret
                                CKeyingMaterial ckmSecret(vchSharedSecret.begin(), vchSharedSecret.end()); // std::vector<unsigned char> -> CKeyingMaterial
                                
                                // chIV - вектор инициализации
                                std::vector<unsigned char> chNuller;
                                chNuller.resize(32, 0);
                                const std::vector<unsigned char> chIV = chNuller;

                                // Шифратор
                                CCrypter crypter;
                                // Установка ключа шифрования
                                crypter.SetKey(ckmSecret, chIV);
                                // Дешифровка
                                CKeyingMaterial ckmPlaintext;
                                crypter.Decrypt(vchDataBody, *((CKeyingMaterial*)&ckmPlaintext));
                                vchDecryptedDataBody.insert(vchDecryptedDataBody.end(), ckmPlaintext.begin(), ckmPlaintext.end());
                            }

                            QByteArray qba;
                            convertVectorToByteArray(vchDecryptedDataBody, qba);
                            sub.dataBodyText = QString::fromUtf8(qba);
                        } else {                        
                            QByteArray qba;
                            convertVectorToByteArray(vchDataBody, qba);
                            sub.dataBodyText = QString::fromUtf8(qba);
                        }
                    }
                } else {
                    // Received by IP connection (deprecated features), or a multisignature or other non-simple transaction
                    sub.type = TransactionRecord::RecvFromOther;
                    sub.address = mapValue["from"];
                }
                if (wtx.IsCoinBase()) {
                    // Generated
                    sub.type = TransactionRecord::Generated;
                }

                parts.append(sub); // сгенерированные
            }
        }
    } else {
        bool involvesWatchAddress = false;
        isminetype fAllFromMe = ISMINE_SPENDABLE;
        BOOST_FOREACH(const CTxIn& txin, wtx.vin)
        {
            isminetype mine = wallet->IsMine(txin);
            if(mine & ISMINE_WATCH_ONLY) involvesWatchAddress = true;
            if(fAllFromMe > mine) fAllFromMe = mine;
        }

        isminetype fAllToMe = ISMINE_SPENDABLE;
        BOOST_FOREACH(const CTxOut& txout, wtx.vout)
        {
            isminetype mine = wallet->IsMine(txout);
            if(mine & ISMINE_WATCH_ONLY) involvesWatchAddress = true;
            if(fAllToMe > mine) fAllToMe = mine;
        }

        if (fAllFromMe && fAllToMe) {
            //LONG отображение, когда отправил себе
            CAmount nChange = wtx.GetChange();
            
            TransactionRecord sub(hash, nTime, TransactionRecord::SendToSelf, "",
                            -(nDebit - nChange), nCredit - nChange);
            for (unsigned int nOut = 0; nOut < wtx.vout.size(); nOut++) {
                const CTxOut& txout = wtx.vout[nOut];
                isminetype isMine = wallet->IsMine(txout);

                //TransactionRecord sub(hash, nTime, TransactionRecord::SendToSelf, "", 0, 0); // nNet
                
                if (isLong(txout.scriptPubKey) && isMine) {
                    //SendToSelf -> SendToAddress
                    sub.type = TransactionRecord::SendToAddress;
                    sub.idx = nOut;
                    sub.isLongTx = true;
                    sub.isCoinTx = false;
                    sub.isDataTx = true;
                    getLongVersion(txout.scriptPubKey, sub.longVersion);

                    // TO
                    std::vector<unsigned char> vchToPubKey;
                    CPubKey toCPubKey;
                    CKeyID  toCPubKeyID;
                    CKey    toPrivKey;
                    getLongToPubKey(txout.scriptPubKey, vchToPubKey);
                    if (vchToPubKey.size() == 20) {
                        toCPubKeyID = uint160(vchToPubKey);
                        wallet->GetPubKey(toCPubKeyID, toCPubKey);
                    } else if (vchToPubKey.size() == 33 || vchToPubKey.size() == 65) {
                        toCPubKey.Set(vchToPubKey.begin(), vchToPubKey.end());
                        toCPubKeyID = toCPubKey.GetID();
                    }
                    if (wallet->HaveKey(toCPubKeyID))
                        pwalletMain->GetKey(toCPubKeyID, toPrivKey);
                    sub.address = CBitcoinAddress(toCPubKeyID).ToString();
                    sub.addressTo = sub.address; 
                    sub.pubKeyHexTo = HexStr(toCPubKey.begin(), toCPubKey.end());

                    // FROM
                    std::vector<unsigned char> vchFromPubKey;
                    CPubKey fromCPubKey; // CPubKey fromCPubKey(fromPubKey);
                    CKeyID  fromCPubKeyID; //CKeyID  fromCPubKeyID = fromCPubKey.GetID();
                    CKey    fromPrivKey;
                    getLongFromPubKey(txout.scriptPubKey, vchFromPubKey);
                    if (vchFromPubKey.size() == 20) {
                        fromCPubKeyID = uint160(vchFromPubKey);
                        wallet->GetPubKey(fromCPubKeyID, fromCPubKey);
                    } else if (vchFromPubKey.size() == 33 || vchFromPubKey.size() == 65) {
                        fromCPubKey.Set(vchFromPubKey.begin(), vchFromPubKey.end());
                        fromCPubKeyID = fromCPubKey.GetID();
                    }
                    if (wallet->HaveKey(fromCPubKeyID))
                        pwalletMain->GetKey(fromCPubKeyID, fromPrivKey);
                    sub.addressFrom = CBitcoinAddress(fromCPubKeyID).ToString();
                    sub.pubKeyHexFrom = HexStr(fromCPubKey.begin(), fromCPubKey.end());
                    
                    // DATA Type
                    getTypeLongData(txout.scriptPubKey, sub.dataType);

                    // ENCRYPTION
                    getEncryptionLongData(txout.scriptPubKey, sub.encryptionType);
                    
                    // DATA
                    std::vector<unsigned char> vchDataBody;
                    std::vector<unsigned char> vchDecryptedDataBody;
                    getBodyLongData(txout.scriptPubKey, vchDataBody);
                    
                    if (sub.encryptionType == 1 && toPrivKey.IsValid() && fromCPubKey.IsFullyValid()) {
                        std::vector<unsigned char> vchSharedSecret;
                        toPrivKey.ComputSharedSecret(fromCPubKey, vchSharedSecret); // Shared Secret - общий секретный ключ ECDH 
                        
                        { // Шифрование aes_256_cbc на Shared Secret
                            CKeyingMaterial ckmSecret(vchSharedSecret.begin(), vchSharedSecret.end()); // std::vector<unsigned char> -> CKeyingMaterial
                            
                            // chIV - вектор инициализации
                            std::vector<unsigned char> chNuller;
                            chNuller.resize(32, 0);
                            const std::vector<unsigned char> chIV = chNuller;

                            // Шифратор
                            CCrypter crypter;
                            // Установка ключа шифрования
                            crypter.SetKey(ckmSecret, chIV);
                            // Дешифровка
                            CKeyingMaterial ckmPlaintext;
                            crypter.Decrypt(vchDataBody, *((CKeyingMaterial*)&ckmPlaintext));
                            vchDecryptedDataBody.insert(vchDecryptedDataBody.end(), ckmPlaintext.begin(), ckmPlaintext.end());
                        }

                        QByteArray qba;
                        convertVectorToByteArray(vchDecryptedDataBody, qba);
                        sub.dataBodyText = QString::fromUtf8(qba);
                        //sub.dataBodyText = QString("encrypted");
                        //parts.append(sub);
                    } else {                        
                        QByteArray qba;
                        convertVectorToByteArray(vchDataBody, qba);
                        sub.dataBodyText = QString::fromUtf8(qba);
                        //parts.append(sub);
                    }
                } else {
                    // Payment to self
                    //CAmount nChange = wtx.GetChange();
                    //parts.append(TransactionRecord(hash, nTime, TransactionRecord::SendToSelf, "",
                    //                -(nDebit - nChange), nCredit - nChange));
                }
            } 
            
            parts.append(sub);
            parts.last().involvesWatchAddress = involvesWatchAddress;   // maybe pass to TransactionRecord as constructor argument
        } else if (fAllFromMe) {
            //
            // Debit
            //
            CAmount nTxFee = nDebit - wtx.GetValueOut();

            for (unsigned int nOut = 0; nOut < wtx.vout.size(); nOut++) {
                const CTxOut& txout = wtx.vout[nOut];

                TransactionRecord sub(hash, nTime);
                sub.idx = nOut;
                sub.involvesWatchAddress = involvesWatchAddress;

                if(wallet->IsMine(txout)) { //LONG отображение, когда отправил себе
                    sub.type = TransactionRecord::SendToSelf;
                    if (isLong(txout.scriptPubKey)) {
                        sub.isLongTx = true;
                        sub.isCoinTx = false;
                        sub.isDataTx = true;
                        getLongVersion(txout.scriptPubKey, sub.longVersion);

                        // TO
                        std::vector<unsigned char> vchToPubKey;
                        CPubKey toCPubKey;
                        CKeyID  toCPubKeyID;
                        CKey    toPrivKey;
                        getLongToPubKey(txout.scriptPubKey, vchToPubKey);
                        if (vchToPubKey.size() == 20) {
                            toCPubKeyID = uint160(vchToPubKey);
                            wallet->GetPubKey(toCPubKeyID, toCPubKey);
                        } else if (vchToPubKey.size() == 33 || vchToPubKey.size() == 65) {
                            toCPubKey.Set(vchToPubKey.begin(), vchToPubKey.end());
                            toCPubKeyID = toCPubKey.GetID();
                        }
                        if (wallet->HaveKey(toCPubKeyID))
                            pwalletMain->GetKey(toCPubKeyID, toPrivKey);
                        sub.address = CBitcoinAddress(toCPubKeyID).ToString();
                        sub.addressTo = sub.address; 
                        sub.pubKeyHexTo = HexStr(toCPubKey.begin(), toCPubKey.end());

                        // FROM
                        std::vector<unsigned char> vchFromPubKey;
                        CPubKey fromCPubKey; // CPubKey fromCPubKey(fromPubKey);
                        CKeyID  fromCPubKeyID; //CKeyID  fromCPubKeyID = fromCPubKey.GetID();
                        CKey    fromPrivKey;
                        getLongFromPubKey(txout.scriptPubKey, vchFromPubKey);
                        if (vchFromPubKey.size() == 20) {
                            fromCPubKeyID = uint160(vchFromPubKey);
                            wallet->GetPubKey(fromCPubKeyID, fromCPubKey);
                        } else if (vchFromPubKey.size() == 33 || vchFromPubKey.size() == 65) {
                            fromCPubKey.Set(vchFromPubKey.begin(), vchFromPubKey.end());
                            fromCPubKeyID = fromCPubKey.GetID();
                        }
                        if (wallet->HaveKey(fromCPubKeyID))
                            pwalletMain->GetKey(fromCPubKeyID, fromPrivKey);
                        sub.addressFrom = CBitcoinAddress(fromCPubKeyID).ToString();
                        sub.pubKeyHexFrom = HexStr(fromCPubKey.begin(), fromCPubKey.end());
                        
                        // DATA Type
                        getTypeLongData(txout.scriptPubKey, sub.dataType);

                        // ENCRYPTION
                        getEncryptionLongData(txout.scriptPubKey, sub.encryptionType);
                        
                        // DATA
                        std::vector<unsigned char> vchDataBody;
                        std::vector<unsigned char> vchDecryptedDataBody;
                        getBodyLongData(txout.scriptPubKey, vchDataBody);
                        
                        if (sub.encryptionType == 1 && toPrivKey.IsValid() && fromCPubKey.IsFullyValid()) {
                            std::vector<unsigned char> vchSharedSecret;
                            toPrivKey.ComputSharedSecret(fromCPubKey, vchSharedSecret); // Shared Secret - общий секретный ключ ECDH 
                            
                            { // Шифрование aes_256_cbc на Shared Secret
                                CKeyingMaterial ckmSecret(vchSharedSecret.begin(), vchSharedSecret.end()); // std::vector<unsigned char> -> CKeyingMaterial
                                
                                // chIV - вектор инициализации
                                std::vector<unsigned char> chNuller;
                                chNuller.resize(32, 0);
                                const std::vector<unsigned char> chIV = chNuller;

                                // Шифратор
                                CCrypter crypter;
                                // Установка ключа шифрования
                                crypter.SetKey(ckmSecret, chIV);
                                // Дешифровка
                                CKeyingMaterial ckmPlaintext;
                                crypter.Decrypt(vchDataBody, *((CKeyingMaterial*)&ckmPlaintext));
                                vchDecryptedDataBody.insert(vchDecryptedDataBody.end(), ckmPlaintext.begin(), ckmPlaintext.end());
                            }

                            QByteArray qba;
                            convertVectorToByteArray(vchDecryptedDataBody, qba);
                            sub.dataBodyText = QString::fromUtf8(qba);
                            //sub.dataBodyText = QString("encrypted");
                            parts.append(sub);
                        } else {                        
                            QByteArray qba;
                            convertVectorToByteArray(vchDataBody, qba);
                            sub.dataBodyText = QString::fromUtf8(qba);
                            parts.append(sub);
                        }
                    } else {
                        // Ignore parts sent to self, as this is usually the change from a transaction sent back to our own address.
                        // Игнорировать части, отправленные на себя, так как обычно это изменение от транзакции, отправленной обратно на наш собственный адрес.
                        continue;
                    }
                }

                CTxDestination address;
                if (ExtractDestination(txout.scriptPubKey, address)) {
                    // Sent to Bitcoin Address
					// Отправлено в биткойн-адрес
                    sub.type = TransactionRecord::SendToAddress;
                    sub.address = CBitcoinAddress(address).ToString();
                    sub.addressTo = sub.address; 
                    //LONG отображение, когда отправил неизвестному
                    if (isLong(txout.scriptPubKey)) {
                        sub.isLongTx = true;
                        sub.isCoinTx = false;
                        sub.isDataTx = true;
                        getLongVersion(txout.scriptPubKey, sub.longVersion);

                        // TO
                        std::vector<unsigned char> vchToPubKey;
                        CPubKey toCPubKey;
                        CKeyID  toCPubKeyID;
                        CKey    toPrivKey;
                        getLongToPubKey(txout.scriptPubKey, vchToPubKey);
                        if (vchToPubKey.size() == 20) {
                            toCPubKeyID = uint160(vchToPubKey);
                            wallet->GetPubKey(toCPubKeyID, toCPubKey);
                        } else if (vchToPubKey.size() == 33 || vchToPubKey.size() == 65) {
                            toCPubKey.Set(vchToPubKey.begin(), vchToPubKey.end());
                            toCPubKeyID = toCPubKey.GetID();
                        }
                        if (wallet->HaveKey(toCPubKeyID))
                            pwalletMain->GetKey(toCPubKeyID, toPrivKey);
                        sub.address = CBitcoinAddress(toCPubKeyID).ToString();
                        sub.addressTo = sub.address; 
                        sub.pubKeyHexTo = HexStr(toCPubKey.begin(), toCPubKey.end());

                        // FROM
                        std::vector<unsigned char> vchFromPubKey;
                        CPubKey fromCPubKey; // CPubKey fromCPubKey(fromPubKey);
                        CKeyID  fromCPubKeyID; //CKeyID  fromCPubKeyID = fromCPubKey.GetID();
                        CKey    fromPrivKey;
                        getLongFromPubKey(txout.scriptPubKey, vchFromPubKey);
                        if (vchFromPubKey.size() == 20) {
                            fromCPubKeyID = uint160(vchFromPubKey);
                            wallet->GetPubKey(fromCPubKeyID, fromCPubKey);
                        } else if (vchFromPubKey.size() == 33 || vchFromPubKey.size() == 65) {
                            fromCPubKey.Set(vchFromPubKey.begin(), vchFromPubKey.end());
                            fromCPubKeyID = fromCPubKey.GetID();
                        }
                        if (wallet->HaveKey(fromCPubKeyID))
                            pwalletMain->GetKey(fromCPubKeyID, fromPrivKey);
                        sub.addressFrom = CBitcoinAddress(fromCPubKeyID).ToString();
                        sub.pubKeyHexFrom = HexStr(fromCPubKey.begin(), fromCPubKey.end());
                        
                        // DATA Type
                        getTypeLongData(txout.scriptPubKey, sub.dataType);

                        // ENCRYPTION
                        getEncryptionLongData(txout.scriptPubKey, sub.encryptionType);
                        
                        // DATA
                        std::vector<unsigned char> vchDataBody;
                        std::vector<unsigned char> vchDecryptedDataBody;
                        getBodyLongData(txout.scriptPubKey, vchDataBody);
                        
                        if (sub.encryptionType == 1 && fromPrivKey.IsValid() && toCPubKey.IsFullyValid()) {
                            std::vector<unsigned char> vchSharedSecret;
                            fromPrivKey.ComputSharedSecret(toCPubKey, vchSharedSecret); // Shared Secret - общий секретный ключ ECDH 
                            
                            { // Шифрование aes_256_cbc на Shared Secret
                                CKeyingMaterial ckmSecret(vchSharedSecret.begin(), vchSharedSecret.end()); // std::vector<unsigned char> -> CKeyingMaterial
                                
                                // chIV - вектор инициализации
                                std::vector<unsigned char> chNuller;
                                chNuller.resize(32, 0);
                                const std::vector<unsigned char> chIV = chNuller;

                                // Шифратор
                                CCrypter crypter;
                                // Установка ключа шифрования
                                crypter.SetKey(ckmSecret, chIV);
                                // Дешифровка
                                CKeyingMaterial ckmPlaintext;
                                crypter.Decrypt(vchDataBody, *((CKeyingMaterial*)&ckmPlaintext));
                                vchDecryptedDataBody.insert(vchDecryptedDataBody.end(), ckmPlaintext.begin(), ckmPlaintext.end());
                            }

                            QByteArray qba;
                            convertVectorToByteArray(vchDecryptedDataBody, qba);
                            sub.dataBodyText = QString::fromUtf8(qba);

                            CAmount nValue = txout.nValue;
                            /* Add fee to first output */
                            /* Добавить плату за первый выпуск */
                            if (nTxFee > 0) {
                                nValue += nTxFee;
                                nTxFee = 0;
                            }
                            sub.debit = -nValue;

                            parts.append(sub);
                            continue;
                        } else {                        
                            QByteArray qba;
                            convertVectorToByteArray(vchDataBody, qba);
                            sub.dataBodyText = QString::fromUtf8(qba);

                            CAmount nValue = txout.nValue;
                            /* Add fee to first output */
                            /* Добавить плату за первый выпуск */
                            if (nTxFee > 0) {
                                nValue += nTxFee;
                                nTxFee = 0;
                            }
                            sub.debit = -nValue;
                            
                            parts.append(sub);
                            continue;
                        }
                    }
                } else {
                    // Sent to IP, or other non-address transaction like OP_EVAL
					// Отправлено на IP или другие неадресные транзакции, такие как OP_EVAL
                    sub.type = TransactionRecord::SendToOther;
                    sub.address = mapValue["to"];
                }

                CAmount nValue = txout.nValue;
                /* Add fee to first output */
				/* Добавить плату за первый выпуск */
                if (nTxFee > 0) {
                    nValue += nTxFee;
                    nTxFee = 0;
                }
                sub.debit = -nValue;

                parts.append(sub);
            }
        } else {
            // Mixed debit transaction, can't break down payees
            // Смешанная дебетовая транзакция, не может разбить получателей
            // LONG отображение когда получил от неизвестного адреса в смешанной транзакции
            for (unsigned int nOut = 0; nOut < wtx.vout.size(); nOut++) {
                const CTxOut& txout = wtx.vout[nOut];
                isminetype isMine = wallet->IsMine(txout);

                TransactionRecord sub(hash, nTime, TransactionRecord::RecvWithAddress, "", 0, 0); // nNet
                sub.idx = nOut;
                sub.type = TransactionRecord::RecvWithAddress;
                if (isLong(txout.scriptPubKey) && isMine) {
                    sub.isLongTx = true;
                    sub.isCoinTx = false;
                    sub.isDataTx = true;
                    getLongVersion(txout.scriptPubKey, sub.longVersion);

                    // TO
                    std::vector<unsigned char> vchToPubKey;
                    CPubKey toCPubKey;
                    CKeyID  toCPubKeyID;
                    CKey    toPrivKey;
                    getLongToPubKey(txout.scriptPubKey, vchToPubKey);
                    if (vchToPubKey.size() == 20) {
                        toCPubKeyID = uint160(vchToPubKey);
                        wallet->GetPubKey(toCPubKeyID, toCPubKey);
                    } else if (vchToPubKey.size() == 33 || vchToPubKey.size() == 65) {
                        toCPubKey.Set(vchToPubKey.begin(), vchToPubKey.end());
                        toCPubKeyID = toCPubKey.GetID();
                    }
                    if (wallet->HaveKey(toCPubKeyID))
                        pwalletMain->GetKey(toCPubKeyID, toPrivKey);
                    sub.address = CBitcoinAddress(toCPubKeyID).ToString();
                    sub.addressTo = sub.address; 
                    sub.pubKeyHexTo = HexStr(toCPubKey.begin(), toCPubKey.end());

                    // FROM
                    std::vector<unsigned char> vchFromPubKey;
                    CPubKey fromCPubKey; // CPubKey fromCPubKey(fromPubKey);
                    CKeyID  fromCPubKeyID; //CKeyID  fromCPubKeyID = fromCPubKey.GetID();
                    CKey    fromPrivKey;
                    getLongFromPubKey(txout.scriptPubKey, vchFromPubKey);
                    if (vchFromPubKey.size() == 20) {
                        fromCPubKeyID = uint160(vchFromPubKey);
                        wallet->GetPubKey(fromCPubKeyID, fromCPubKey);
                    } else if (vchFromPubKey.size() == 33 || vchFromPubKey.size() == 65) {
                        fromCPubKey.Set(vchFromPubKey.begin(), vchFromPubKey.end());
                        fromCPubKeyID = fromCPubKey.GetID();
                    }
                    if (wallet->HaveKey(fromCPubKeyID))
                        pwalletMain->GetKey(fromCPubKeyID, fromPrivKey);
                    sub.addressFrom = CBitcoinAddress(fromCPubKeyID).ToString();
                    sub.pubKeyHexFrom = HexStr(fromCPubKey.begin(), fromCPubKey.end());
                    
                    // DATA Type
                    getTypeLongData(txout.scriptPubKey, sub.dataType);

                    // ENCRYPTION
                    getEncryptionLongData(txout.scriptPubKey, sub.encryptionType);
                    
                    // DATA
                    std::vector<unsigned char> vchDataBody;
                    std::vector<unsigned char> vchDecryptedDataBody;
                    getBodyLongData(txout.scriptPubKey, vchDataBody);
                    
                    if (sub.encryptionType == 1 && toPrivKey.IsValid() && fromCPubKey.IsFullyValid()) {
                        std::vector<unsigned char> vchSharedSecret;
                        toPrivKey.ComputSharedSecret(fromCPubKey, vchSharedSecret); // Shared Secret - общий секретный ключ ECDH 
                        
                        { // Шифрование aes_256_cbc на Shared Secret
                            CKeyingMaterial ckmSecret(vchSharedSecret.begin(), vchSharedSecret.end()); // std::vector<unsigned char> -> CKeyingMaterial
                            
                            // chIV - вектор инициализации
                            std::vector<unsigned char> chNuller;
                            chNuller.resize(32, 0);
                            const std::vector<unsigned char> chIV = chNuller;

                            // Шифратор
                            CCrypter crypter;
                            // Установка ключа шифрования
                            crypter.SetKey(ckmSecret, chIV);
                            // Дешифровка
                            CKeyingMaterial ckmPlaintext;
                            crypter.Decrypt(vchDataBody, *((CKeyingMaterial*)&ckmPlaintext));
                            vchDecryptedDataBody.insert(vchDecryptedDataBody.end(), ckmPlaintext.begin(), ckmPlaintext.end());
                        }

                        QByteArray qba;
                        convertVectorToByteArray(vchDecryptedDataBody, qba);
                        sub.dataBodyText = QString::fromUtf8(qba);
                        //sub.dataBodyText = QString("encrypted");
                        parts.append(sub);
                    } else {                        
                        QByteArray qba;
                        convertVectorToByteArray(vchDataBody, qba);
                        sub.dataBodyText = QString::fromUtf8(qba);
                        parts.append(sub);
                    }
                } else if (isMine) {
                    sub.type = TransactionRecord::RecvWithAddress;
                    CTxDestination address;
                    const CTxOut& txout = wtx.vout[0];
                    if (ExtractDestination(txout.scriptPubKey, address)) {
                        sub.address = CBitcoinAddress(address).ToString();
                    }
                    sub.involvesWatchAddress = involvesWatchAddress;
                    parts.append(sub);
                    //
                    // Mixed debit transaction, can't break down payees
                    //
                    //parts.append(TransactionRecord(hash, nTime, TransactionRecord::Other, "", nNet, 0));
                    
                } 
            }

            if (parts.size() == 0) {
                // Все исходящие не на адреса из wallet.dat 
                TransactionRecord sub(hash, nTime, TransactionRecord::SendToOther, "", nNet, 0); // nNet
                
                if (wtx.vin.size() > 1) {
                    sub.type = TransactionRecord::SendToOther;
                } else if (wtx.vin.size() == 1) {
                    sub.type = TransactionRecord::SendToAddress;
                    CTxDestination address;
                    const CTxOut& txout = wtx.vout[0];
                    if (ExtractDestination(txout.scriptPubKey, address)) {
                        sub.address = CBitcoinAddress(address).ToString();
                    }
                }
                parts.append(sub);
                parts.last().involvesWatchAddress = involvesWatchAddress;
            }
            
            //parts.last().involvesWatchAddress = involvesWatchAddress;
            
        }
    }

    return parts;
}

void TransactionRecord::updateStatus(const CWalletTx &wtx)
{
    AssertLockHeld(cs_main);
    // Determine transaction status
    // Определение статуса транзакции
    // Find the block the tx is in
    // Найдите блок, в котором находится tx.
    CBlockIndex* pindex = NULL;
    BlockMap::iterator mi = mapBlockIndex.find(wtx.hashBlock);
    if (mi != mapBlockIndex.end())
        pindex = (*mi).second;

    // Sort order, unrecorded transactions sort to the top
    // Порядок сортировки, незаписанные транзакции сортируются вверху
    status.sortKey = strprintf("%010d-%01d-%010u-%03d",
        (pindex ? pindex->nHeight : std::numeric_limits<int>::max()),
        (wtx.IsCoinBase() ? 1 : 0),
        wtx.nTimeReceived,
        idx);
    status.countsForBalance = wtx.IsTrusted() && !(wtx.GetBlocksToMaturity() > 0);
    status.depth = wtx.GetDepthInMainChain();
    status.cur_num_blocks = chainActive.Height();

    if (!CheckFinalTx(wtx))
    {
        if (wtx.nLockTime < LOCKTIME_THRESHOLD)
        {
            status.status = TransactionStatus::OpenUntilBlock;
            status.open_for = wtx.nLockTime - chainActive.Height();
        }
        else
        {
            status.status = TransactionStatus::OpenUntilDate;
            status.open_for = wtx.nLockTime;
        }
    }
    // For generated transactions, determine maturity
    else if(type == TransactionRecord::Generated)
    {
        if (wtx.GetBlocksToMaturity() > 0)
        {
            status.status = TransactionStatus::Immature;

            if (wtx.IsInMainChain())
            {
                status.matures_in = wtx.GetBlocksToMaturity();

                // Check if the block was requested by anyone
                if (GetAdjustedTime() - wtx.nTimeReceived > 2 * 60 && wtx.GetRequestCount() == 0)
                    status.status = TransactionStatus::MaturesWarning;
            }
            else
            {
                status.status = TransactionStatus::NotAccepted;
            }
        }
        else
        {
            status.status = TransactionStatus::Confirmed;
        }
    }
    else
    {
        if (status.depth < 0)
        {
            status.status = TransactionStatus::Conflicted;
        }
        else if (GetAdjustedTime() - wtx.nTimeReceived > 2 * 60 && wtx.GetRequestCount() == 0)
        {
            status.status = TransactionStatus::Offline;
        }
        else if (status.depth == 0)
        {
            status.status = TransactionStatus::Unconfirmed;
        }
        else if (status.depth < RecommendedNumConfirmations)
        {
            status.status = TransactionStatus::Confirming;
        }
        else
        {
            status.status = TransactionStatus::Confirmed;
        }
    }

}

bool TransactionRecord::statusUpdateNeeded()
{
    AssertLockHeld(cs_main);
    return status.cur_num_blocks != chainActive.Height();
}

QString TransactionRecord::getTxID() const
{
    return formatSubTxId(hash, idx);
}

QString TransactionRecord::formatSubTxId(const uint256 &hash, int vout)
{
    return QString::fromStdString(hash.ToString() + strprintf("-%03d", vout));
}



QString TransactionRecord::getOP_RETURN_DATA() const
{ ////return "(n/a)"; //"Message"
    
    //return QString::fromStdString(scriptPubKey); // hex
	//return QString::fromStdString(strprintf("%d", debit));
	//return QString::fromStdString(scriptPubKey);
    //QByteArray qba;
    //convertVectorToByteArray(scriptPubKey, qba);
    //return QString::fromUtf8(qba);
    
	//return "(n/a)";
    return dataBodyText;
}
