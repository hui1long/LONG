// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Copyright (c) 2018-2018 The LONG NETWORK Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_CONSENSUS_CONSENSUS_H
#define BITCOIN_CONSENSUS_CONSENSUS_H

/** The maximum allowed size for a serialized block, in bytes (network rule) */
/** Максимально допустимый размер для сериализованного блока, в байтах (сетевое правило) */
static const unsigned int MAX_BLOCK_SIZE = 1000000;

/** The maximum allowed number of signature check operations in a block (network rule) */
/** Максимально допустимое количество операций проверки подписи в блоке (сетевое правило) */
static const unsigned int MAX_BLOCK_SIGOPS = MAX_BLOCK_SIZE/50;

/** Coinbase transaction outputs can only be spent after this number of new blocks (network rule) */
/** Выходы транзакции Coinbase могут быть потрачены только после этого количества новых блоков (правила сети) */
//static const int COINBASE_MATURITY = 100; Bitcoin Default
static const int COINBASE_MATURITY = 60; //60

/** Flags for nSequence and nLockTime locks */
/** Флаги для nSequence и блокировки nLockTime */
enum {
    /* Interpret sequence numbers as relative lock-time constraints. */
    /* Интерпретировать порядковые номера как относительные ограничения времени блокировки. */
    LOCKTIME_VERIFY_SEQUENCE = (1 << 0),

    /* Use GetMedianTimePast() instead of nTime for end point timestamp. */
    /* Используйте GetMedianTimePast() вместо nTime для временной метки конечной точки. */
    LOCKTIME_MEDIAN_TIME_PAST = (1 << 0),
};

#endif // BITCOIN_CONSENSUS_CONSENSUS_H
