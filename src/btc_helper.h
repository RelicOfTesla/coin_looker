#pragma once

#include "util.h"

struct ICoinOption;
struct CScript;
struct CBlockHeader;

std::string hash160_to_CoinAddress(const uint160& hash_value, BYTE addrType);
std::string hash160_to_CoinAddress(const uint160& hash_value, ICoinOption* profile);
std::string script_get_coin_address(ICoinOption* pProfile, const CScript& script);

uint160 _hash_160(const BYTE* src, size_t len);
uint256 _hash_256(const BYTE* src, size_t len);

uint256 calc_block_hash(const CBlockHeader&);

double ToMoney(__int64 nValue);