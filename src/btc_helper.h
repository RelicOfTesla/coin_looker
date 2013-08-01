#pragma once

#include "util.h"

struct ICoinOption;
struct CScript;
struct CBlockHeader;


#pragma pack(push, 1)
struct CoinRawAddress
{
	BYTE AddrType;
	ripemd160 hash160;
	UINT32 nChecksum;
};
#pragma pack(pop)

struct CoinKey
{
	CoinRawAddress m_data;

	CoinKey(const std::string& src);
	CoinKey(const CoinRawAddress& src);

	bool operator == (const CoinKey& r)const;
	bool operator < (const CoinKey& r)const;

	std::string to_str()const;

	bool empty()const;
};

CoinKey script_get_coin_key(ICoinOption* pCoinOption, const CScript& script);

uint160 _hash_160(const BYTE* src, size_t len);
uint256 _hash_256(const BYTE* src, size_t len);

uint256 calc_block_hash(const CBlockHeader&);

double ToMoney(__int64 nValue);