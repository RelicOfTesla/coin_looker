#pragma once

#include <openssl/sha.h>
#include <openssl/ripemd.h>

#pragma pack(push, 1)
struct ripemd160
{
	BYTE data[RIPEMD160_DIGEST_LENGTH];
};

struct hash256 
{
	BYTE data[SHA256_DIGEST_LENGTH];
};
typedef ripemd160	uint160;
typedef hash256		uint256 ;


static const __int64 COIN = 100000000;


static const uint160 uint160_null = {0};
static const uint256 uint256_null = {0};


inline bool operator<(const uint256& l, const uint256& r)
{
	return memcmp(&l, &r, sizeof(l))<0;
}
inline bool operator==(const uint256& l, const uint256& r)
{
	return memcmp(&l, &r, sizeof(l))==0;
}
inline bool operator!=(const uint256& l, const uint256& r)
{
	return memcmp(&l, &r, sizeof(l))!=0;
}
