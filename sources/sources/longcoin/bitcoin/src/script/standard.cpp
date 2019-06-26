// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "script/standard.h"

#include "pubkey.h"
#include "script/script.h"
#include "util.h"
#include "utilstrencodings.h"

#include <boost/foreach.hpp>

using namespace std;

typedef vector<unsigned char> valtype;

bool fAcceptDatacarrier = DEFAULT_ACCEPT_DATACARRIER;
unsigned nMaxDatacarrierBytes = MAX_OP_RETURN_RELAY;

CScriptID::CScriptID(const CScript& in) : uint160(Hash160(in.begin(), in.end())) {}

const char* GetTxnOutputType(txnouttype t)
{
    switch (t)
    {
    case TX_NONSTANDARD: return "nonstandard";
    case TX_PUBKEY: return "pubkey";
    case TX_PUBKEYHASH: return "pubkeyhash";
    case TX_SCRIPTHASH: return "scripthash";
    case TX_MULTISIG: return "multisig";
    case TX_NULL_DATA: return "nulldata";
    }
    return NULL;
}







/**
 * LONG
 */

bool getOffsetPushedReturnData(const CScript& scriptPubKey, unsigned int& offset)
{
	offset = 0;
    if (scriptPubKey.size() >= 1 && scriptPubKey[0] == OP_RETURN && scriptPubKey.IsPushOnly(scriptPubKey.begin()+1)) {
		// Immediate operand
        if (scriptPubKey[1] <= OP_PUSHDATA4)
        {
			if (scriptPubKey[1] < OP_PUSHDATA1) {
				offset = 1; // 1+0
			} else if (scriptPubKey[1] == OP_PUSHDATA1) {
				offset = 2; // 1+1
			} else if (scriptPubKey[1] == OP_PUSHDATA2) {
				offset = 3;  // 1+2
			} else if (scriptPubKey[1] == OP_PUSHDATA4) {
				offset = 5;  // 1+4
			}

			return true;
		}
		return false;
    } 
    return false;
}

bool getSizePushedReturnData(const CScript& scriptPubKey, unsigned int& size)
{
	size = 0;
    if (scriptPubKey.size() >= 1 && scriptPubKey[0] == OP_RETURN && scriptPubKey.IsPushOnly(scriptPubKey.begin()+1)) {
		// Immediate operand
        if (scriptPubKey[1] <= OP_PUSHDATA4)
        {
			unsigned int offset = 0;
			if (scriptPubKey[1] < OP_PUSHDATA1) {
				offset = 1;  // 1+0
				size = scriptPubKey[1];
			} else if (scriptPubKey[1] == OP_PUSHDATA1) {
				offset = 2;  // 1+1
				size = scriptPubKey[2];
			} else if (scriptPubKey[1] == OP_PUSHDATA2) {
				offset = 3;  // 1+2
				size = ReadLE16(&scriptPubKey[2]);
			} else if (scriptPubKey[1] == OP_PUSHDATA4) {
				offset = 5;  // 1+4
				size = ReadLE32(&scriptPubKey[2]);
			}

			return true;
		}
		return false;
    } 
    return false;
}

bool getPushedReturnData(const CScript& scriptPubKey, std::vector<unsigned char>& pushedReturnData)
{
	unsigned int offset = 0;
	unsigned int size = 0;
    if (getOffsetPushedReturnData(scriptPubKey, offset) && getSizePushedReturnData(scriptPubKey, size)) {
		std::vector<unsigned char> hashBytes(scriptPubKey.begin() +1 +offset, scriptPubKey.begin() +1 +offset +size);
		pushedReturnData = hashBytes;
		return true;
    } else {
		return false;
	}
}

bool isLong(const CScript& scriptPubKey)
{
	std::vector<unsigned char> pushedReturnData;
	if (getPushedReturnData(scriptPubKey, pushedReturnData)) {
		if (pushedReturnData[0] == 0x0c && pushedReturnData[1] == 0x0f && pushedReturnData[2] == 0x0e && pushedReturnData[3] == 0x07) {
			return true;
		} else {
			return false;
		}
	} else {
		return false;
	}
}

bool getLongVersion(const CScript& scriptPubKey, unsigned int& version)
{
	std::vector<unsigned char> pushedReturnData;
	if (getPushedReturnData(scriptPubKey, pushedReturnData)) {
		if (pushedReturnData[0] == 0x0c && pushedReturnData[1] == 0x0f && pushedReturnData[2] == 0x0e && pushedReturnData[3] == 0x07) {
            version = pushedReturnData[4]; 
			return true;
		} else {
			return false;
		}
	} else {
		return false;
	}
}


bool getLongToPubKey(const CScript& scriptPubKey, std::vector<unsigned char>& pubKey, txnouttype& type)
{
    unsigned int offset = 0;
	if (isLong(scriptPubKey)) {
		std::vector<unsigned char> pushedReturnData;
		if (getPushedReturnData(scriptPubKey, pushedReturnData)) {
            if (pushedReturnData[0] == 0x0c && pushedReturnData[1] == 0x0f && pushedReturnData[2] == 0x0e && pushedReturnData[3] == 0x07) {
                offset = offset +5;
            }
			if (pushedReturnData[+offset] == 0xf0) {            // OP_TO
                offset = offset +1;
				if (pushedReturnData[+offset] == 0xfe) {        // OP_PUBKEYCOMP
                    offset = offset +1;
					std::vector<unsigned char> hashBytes(pushedReturnData.begin() +offset, pushedReturnData.begin() +offset +33);
					pubKey = hashBytes;
                    offset = offset +33;
                    type = TX_PUBKEY;
                    return true;
				} else if (pushedReturnData[+offset] == 0xff) { // OP_PUBKEY
                    offset = offset +1;
					std::vector<unsigned char> hashBytes(pushedReturnData.begin() +offset, pushedReturnData.begin() +offset +65);
					pubKey = hashBytes;
                    offset = offset +65;
                    type = TX_PUBKEY;
                    return true;
				} else if (pushedReturnData[+offset] == 0xfd) { // OP_PUBKEYHASH
                    offset = offset +1;
					std::vector<unsigned char> hashBytes(pushedReturnData.begin() +offset, pushedReturnData.begin() +offset +20);
					pubKey = hashBytes;
                    offset = offset +20;
                    type = TX_PUBKEYHASH;
                    return true;
				}
			}
			
		}
	}
    type = TX_NULL_DATA;
    return false;
}
bool getLongToPubKey(const CScript& scriptPubKey, std::vector<unsigned char>& pubKey)
{
    txnouttype type;
    return getLongToPubKey(scriptPubKey, pubKey, type);
}

bool getLongFromPubKey(const CScript& scriptPubKey, std::vector<unsigned char>& pubKey, txnouttype& type)
{
    unsigned int offset = 0;
	if (isLong(scriptPubKey)) {
		std::vector<unsigned char> pushedReturnData;
		if (getPushedReturnData(scriptPubKey, pushedReturnData)) {
            if (pushedReturnData[0] == 0x0c && pushedReturnData[1] == 0x0f && pushedReturnData[2] == 0x0e && pushedReturnData[3] == 0x07) {
                offset = offset +5;
            }
            if (pushedReturnData[+offset] == 0xf0) {            // OP_TO
                offset = offset +1;
				if (pushedReturnData[+offset ] == 0xfe) {        // OP_PUBKEYCOMP
                    offset = offset +1 +33;
				} else if (pushedReturnData[+offset ] == 0xff) { // OP_PUBKEY
					offset = offset +1 +65;
				} else if (pushedReturnData[+offset ] == 0xfd) { // OP_PUBKEYHASH
					offset = offset +1 +20;
				}
			}
			if (pushedReturnData[+offset] == 0xf1) {            // OP_FROM
                offset = offset +1;
				if (pushedReturnData[+offset] == 0xfe) {        // OP_PUBKEYCOMP
                    offset = offset +1;
					std::vector<unsigned char> hashBytes(pushedReturnData.begin() +offset, pushedReturnData.begin() +offset +33);
					pubKey = hashBytes;
                    offset = offset +33;
                    type = TX_PUBKEY;
			        return true;
				} else if (pushedReturnData[+offset] == 0xff) { // OP_PUBKEY
                    offset = offset +1;
					std::vector<unsigned char> hashBytes(pushedReturnData.begin() +offset, pushedReturnData.begin() +offset +65);
					pubKey = hashBytes;
                    offset = offset +65;
                    type = TX_PUBKEY;
			        return true;
				} else if (pushedReturnData[+offset] == 0xfd) { // OP_PUBKEYHASH
                    offset = offset +1;
					std::vector<unsigned char> hashBytes(pushedReturnData.begin() +offset, pushedReturnData.begin() +offset +20);
					pubKey = hashBytes;
                    offset = offset +20;
                    type = TX_PUBKEYHASH;
                    return true;
				}
			}
		}
	}
    type = TX_NULL_DATA;
    return false;
}
bool getLongFromPubKey(const CScript& scriptPubKey, std::vector<unsigned char>& pubKey)
{
    txnouttype type;
    return getLongFromPubKey(scriptPubKey, pubKey, type);
}




bool getOffsetLongData(const CScript& scriptPubKey, unsigned int& offset)
{
	offset = 0;
	if (isLong(scriptPubKey)) {
		std::vector<unsigned char> pushedReturnData;
		if (getPushedReturnData(scriptPubKey, pushedReturnData)) {
            if (pushedReturnData[0] == 0x0c && pushedReturnData[1] == 0x0f && pushedReturnData[2] == 0x0e && pushedReturnData[3] == 0x07) {
                offset = offset +5;
            }
			
				if (pushedReturnData[offset] == 0xf0) { // OP_TO
					offset = offset +1;
					if (pushedReturnData[offset] == 0xfe) { // OP_PUBKEYCOMP
						offset = offset +1 +33;
					} else if (pushedReturnData[offset] == 0xff) { // OP_PUBKEY
						offset = offset +1 +65;
					} else if (pushedReturnData[+offset ] == 0xfd) { // OP_PUBKEYHASH
                        offset = offset +1 +20;
                    }
				}
				if (pushedReturnData[offset] == 0xf1) { // OP_FROM
					offset = offset +1;
					if (pushedReturnData[offset] == 0xfe) { // OP_PUBKEYCOMP
						offset = offset +1 +33;
					} else if (pushedReturnData[offset] == 0xff) { // OP_PUBKEY
						offset = offset +1 +65;
					} else if (pushedReturnData[+offset ] == 0xfd) { // OP_PUBKEYHASH
                        offset = offset +1 +20;
                    }
				}
				return true;
		}
    }
    return false;
}

bool getTypeLongData(const CScript& scriptPubKey, unsigned int& type)
{
	type = 0;
    unsigned int offset = 0;
	if (isLong(scriptPubKey)) {
		std::vector<unsigned char> pushedReturnData;
		if (getPushedReturnData(scriptPubKey, pushedReturnData)) {
			
			if (getOffsetLongData(scriptPubKey, offset)) {
				if (pushedReturnData[offset] == 0xf2) { // OP_DATA_TYPE
					type = pushedReturnData[offset+1];  // OP_DATA_TYPE_TEXT | 000   | 0x00 | Тип данных - текст
					return true;
				}
			}
		}
    }
    return false;
}

bool getEncryptionLongData(const CScript& scriptPubKey, unsigned int& type)
{
	type = 0;
    unsigned int offset = 0;
	if (isLong(scriptPubKey)) {
		std::vector<unsigned char> pushedReturnData;
		if (getPushedReturnData(scriptPubKey, pushedReturnData)) {
			if (getOffsetLongData(scriptPubKey, offset)) {
				if (pushedReturnData[offset +2] == 0xf3) { // OP_ENCRYPTION
					type = pushedReturnData[offset +3];    // OP_ENCRYPTION_NO |   000 | 0x00 | Не зашифровано
					return true;
				}
			}
		}
    }
    return false;
}


bool getSizeLongData(const CScript& scriptPubKey, unsigned int& size)
{
	size = 0;
    unsigned int offset = 0;
	if (isLong(scriptPubKey)) {
		std::vector<unsigned char> pushedReturnData;
		if (getPushedReturnData(scriptPubKey, pushedReturnData)) {
			if (getOffsetLongData(scriptPubKey, offset)) {
                if (pushedReturnData[offset] == 0xf2) { // OP_DATA_TYPE
                    offset = offset+2; 
                }
                if (pushedReturnData[offset ] == 0xf3) { // OP_ENCRYPTION
                    offset = offset+2; 
                }
				if (pushedReturnData[offset] <= OP_PUSHDATA4) { //  +2 = 1=type, 2=encrypt
					if (pushedReturnData[offset ] < OP_PUSHDATA1) {
						size = pushedReturnData[offset]; // < 75
                        offset = offset +1; 
					} else if (pushedReturnData[offset ] == OP_PUSHDATA1) {
						size = pushedReturnData[offset +1]; // >= 76 , <= 255
                        offset = offset +2; 
					} else if (pushedReturnData[offset ] == OP_PUSHDATA2) {
						size = ReadLE16(&pushedReturnData[offset +1]); // >= 256, < 65535
                        offset = offset +3; 
					} else if (pushedReturnData[offset ] == OP_PUSHDATA4) {
						size = ReadLE32(&pushedReturnData[offset +1]); // >= 65535, < 4294967295
                        offset = offset +5; 
					}
					return true;
				}
			}
		}
    }
    return false;
}

bool getBodyLongData(const CScript& scriptPubKey, std::vector<unsigned char>& dataBody)
{
	unsigned int size = 0;
	unsigned int offset = 0;
	unsigned int shift = 0;
	if (isLong(scriptPubKey)) {
		std::vector<unsigned char> pushedReturnData;
		if (getPushedReturnData(scriptPubKey, pushedReturnData)) {
			if (getOffsetLongData(scriptPubKey, offset)) {
				if (getSizeLongData(scriptPubKey, size)) {
                    if (pushedReturnData[offset] == 0xf2) { // OP_DATA_TYPE
                        offset = offset+2; 
                    }
                    if (pushedReturnData[offset ] == 0xf3) { // OP_ENCRYPTION
                        offset = offset+2; 
                    }

					if (size <= 75) {
						shift = 1;
					} else if (size <= 255) {
						shift = 2;
					} else if (size <= 65535) {
						shift = 3;
					} else if (size <= 4294967295) {
						shift = 5;
					}
					std::vector<unsigned char> hashBytes(pushedReturnData.begin() +offset +shift, pushedReturnData.begin() +offset +shift +size);
					dataBody = hashBytes;
					return true;
				}
			}
		}
    }
    return false;
}





/**
 * Return public keys or hashes from scriptPubKey, for 'standard' transaction types.
 * Возвращать открытые ключи или хэши из scriptPubKey, для стандартных типов транзакций.
 */
bool Solver(const CScript& scriptPubKey, txnouttype& typeRet, vector<vector<unsigned char> >& vSolutionsRet)
{
    // Templates
    static multimap<txnouttype, CScript> mTemplates;
    if (mTemplates.empty())
    {
        // Standard tx, sender provides pubkey, receiver adds signature
        mTemplates.insert(make_pair(TX_PUBKEY, CScript() << OP_PUBKEY << OP_CHECKSIG));

        // Bitcoin address tx, sender provides hash of pubkey, receiver provides signature and pubkey
        mTemplates.insert(make_pair(TX_PUBKEYHASH, CScript() << OP_DUP << OP_HASH160 << OP_PUBKEYHASH << OP_EQUALVERIFY << OP_CHECKSIG));

        // Sender provides N pubkeys, receivers provides M signatures
        mTemplates.insert(make_pair(TX_MULTISIG, CScript() << OP_SMALLINTEGER << OP_PUBKEYS << OP_SMALLINTEGER << OP_CHECKMULTISIG));
    }

    vSolutionsRet.clear();

    // Shortcut for pay-to-script-hash, which are more constrained than the other types:
    // it is always OP_HASH160 20 [20 byte hash] OP_EQUAL
	// Ярлык для хэша «pay-to-script-hash», который более ограничен, чем другие типы:
	// он всегда OP_HASH160 20 [20-байтовый хеш] OP_EQUAL
    if (scriptPubKey.IsPayToScriptHash())
    {
        typeRet = TX_SCRIPTHASH;
        vector<unsigned char> hashBytes(scriptPubKey.begin()+2, scriptPubKey.begin()+22);
        vSolutionsRet.push_back(hashBytes);
        return true;
    }

    // Provably prunable, data-carrying output
	// Продуманный, вывод данных
    //
    // So long as script passes the IsUnspendable() test and all but the first
    // byte passes the IsPushOnly() test we don't care what exactly is in the
    // script.
	// До тех пор, пока скрипт проходит тест IsUnspendable (), и все, кроме первого байта, проходят тест IsPushOnly (), нам все равно, что именно находится в скрипте.
    if (scriptPubKey.size() >= 1 && scriptPubKey[0] == OP_RETURN && scriptPubKey.IsPushOnly(scriptPubKey.begin()+1)) {
		
		if (isLong(scriptPubKey)) {
			vector<unsigned char> toPubKey;
			if (getLongToPubKey(scriptPubKey, toPubKey, typeRet))  {
				vSolutionsRet.push_back(toPubKey);
		        return true;
			}

		}
        typeRet = TX_NULL_DATA;
		return true;
    }

    // Scan templates
    const CScript& script1 = scriptPubKey;
    BOOST_FOREACH(const PAIRTYPE(txnouttype, CScript)& tplate, mTemplates)
    {
        const CScript& script2 = tplate.second;
        vSolutionsRet.clear();

        opcodetype opcode1, opcode2;
        vector<unsigned char> vch1, vch2;

        // Compare
        CScript::const_iterator pc1 = script1.begin();
        CScript::const_iterator pc2 = script2.begin();
        while (true)
        {
            if (pc1 == script1.end() && pc2 == script2.end())
            {
                // Found a match
                typeRet = tplate.first;
                if (typeRet == TX_MULTISIG)
                {
                    // Additional checks for TX_MULTISIG:
                    unsigned char m = vSolutionsRet.front()[0];
                    unsigned char n = vSolutionsRet.back()[0];
                    if (m < 1 || n < 1 || m > n || vSolutionsRet.size()-2 != n)
                        return false;
                }
                return true;
            }
            if (!script1.GetOp(pc1, opcode1, vch1))
                break;
            if (!script2.GetOp(pc2, opcode2, vch2))
                break;

            // Template matching opcodes:
            if (opcode2 == OP_PUBKEYS)
            {
                while (vch1.size() >= 33 && vch1.size() <= 65)
                {
                    vSolutionsRet.push_back(vch1);
                    if (!script1.GetOp(pc1, opcode1, vch1))
                        break;
                }
                if (!script2.GetOp(pc2, opcode2, vch2))
                    break;
                // Normal situation is to fall through
                // to other if/else statements
            }

            if (opcode2 == OP_PUBKEY)
            {
                if (vch1.size() < 33 || vch1.size() > 65)
                    break;
                vSolutionsRet.push_back(vch1);
            }
            else if (opcode2 == OP_PUBKEYHASH)
            {
                if (vch1.size() != sizeof(uint160))
                    break;
                vSolutionsRet.push_back(vch1);
            }
            else if (opcode2 == OP_SMALLINTEGER)
            {   // Single-byte small integer pushed onto vSolutions
                if (opcode1 == OP_0 ||
                    (opcode1 >= OP_1 && opcode1 <= OP_16))
                {
                    char n = (char)CScript::DecodeOP_N(opcode1);
                    vSolutionsRet.push_back(valtype(1, n));
                }
                else
                    break;
            }
            else if (opcode1 != opcode2 || vch1 != vch2)
            {
                // Others must match exactly
                break;
            }
        }
    }

    vSolutionsRet.clear();
    typeRet = TX_NONSTANDARD;
    return false;
}




bool ExtractDestination(const CScript& scriptPubKey, CTxDestination& addressRet)
{
    vector<valtype> vSolutions;
    txnouttype whichType;
    if (!Solver(scriptPubKey, whichType, vSolutions))
        return false;

    if (whichType == TX_PUBKEY) {
        CPubKey pubKey(vSolutions[0]);
        if (!pubKey.IsValid())
            return false;

        addressRet = pubKey.GetID();
        return true;
    } else if (whichType == TX_PUBKEYHASH) {
        addressRet = CKeyID(uint160(vSolutions[0]));
        return true;
    } else if (whichType == TX_SCRIPTHASH) {
        addressRet = CScriptID(uint160(vSolutions[0]));
        return true;
    }
    // Multisig txns have more than one address...
    return false;
}

bool ExtractDestinations(const CScript& scriptPubKey, txnouttype& typeRet, vector<CTxDestination>& addressRet, int& nRequiredRet)
{
    addressRet.clear();
    typeRet = TX_NONSTANDARD;
    vector<valtype> vSolutions;
    if (!Solver(scriptPubKey, typeRet, vSolutions))
        return false;
    if (typeRet == TX_NULL_DATA){
        // This is data, not addresses
        return false;
    }

    if (typeRet == TX_MULTISIG)
    {
        nRequiredRet = vSolutions.front()[0];
        for (unsigned int i = 1; i < vSolutions.size()-1; i++)
        {
            CPubKey pubKey(vSolutions[i]);
            if (!pubKey.IsValid())
                continue;

            CTxDestination address = pubKey.GetID();
            addressRet.push_back(address);
        }

        if (addressRet.empty())
            return false;
    }
    else
    {
        nRequiredRet = 1;
        CTxDestination address;
        if (!ExtractDestination(scriptPubKey, address))
           return false;
        addressRet.push_back(address);
    }

    return true;
}

namespace
{
class CScriptVisitor : public boost::static_visitor<bool>
{
private:
    CScript *script;
public:
    CScriptVisitor(CScript *scriptin) { script = scriptin; }

    bool operator()(const CNoDestination &dest) const {
        script->clear();
        return false;
    }

    bool operator()(const CKeyID &keyID) const {
        script->clear();
        *script << OP_DUP << OP_HASH160 << ToByteVector(keyID) << OP_EQUALVERIFY << OP_CHECKSIG;
        return true;
    }

    bool operator()(const CScriptID &scriptID) const {
        script->clear();
        *script << OP_HASH160 << ToByteVector(scriptID) << OP_EQUAL;
        return true;
    }
};
}

CScript GetScriptForDestination(const CTxDestination& dest)
{
    CScript script;

    boost::apply_visitor(CScriptVisitor(&script), dest);
    return script;
}

CScript GetScriptForRawPubKey(const CPubKey& pubKey)
{
    return CScript() << std::vector<unsigned char>(pubKey.begin(), pubKey.end()) << OP_CHECKSIG;
}

CScript GetScriptForMultisig(int nRequired, const std::vector<CPubKey>& keys)
{
    CScript script;

    script << CScript::EncodeOP_N(nRequired);
    BOOST_FOREACH(const CPubKey& key, keys)
        script << ToByteVector(key);
    script << CScript::EncodeOP_N(keys.size()) << OP_CHECKMULTISIG;
    return script;
}
