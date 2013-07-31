#include "stdafx.h"
#include "btc_helper.h"
#include "ICoinProfile.h"
#include <sdk/binary_stream.hpp>
#include "btc_protocol.h"

enum {
	OP_DUP = 0x76,

	OP_INVERT = 0x83,
	OP_AND = 0x84,
	OP_OR = 0x85,
	OP_XOR = 0x86,
	OP_EQUAL = 0x87,
	OP_EQUALVERIFY = 0x88,

	OP_RIPEMD160 = 0xa6,
	OP_SHA1 = 0xa7,
	OP_SHA256 = 0xa8,
	OP_HASH160 = 0xa9,
	OP_HASH256 = 0xaa,
};



#pragma pack(push, 1)
struct CoinRawAddress
{
	BYTE AddrType;
	ripemd160 hash160;
	UINT32 nChecksum;
};
#pragma pack(pop)
std::string hash160_to_CoinAddress(const uint160& hash_value, BYTE addrType)
{
	CoinRawAddress raddr;
	raddr.AddrType = addrType;
	raddr.hash160 = hash_value;
	raddr.nChecksum = 0;
	uint256 h256 = double_sha256(&raddr, sizeof(BYTE)+sizeof(ripemd160));
	raddr.nChecksum = *(UINT32*)&h256;
#if _DEBUG
	std::string szHash160 = to_hex(&hash_value, sizeof(hash_value));
#endif
	std::string str = base58_encode(&raddr, sizeof(raddr));
	return str;
};

std::string hash160_to_CoinAddress(const uint160& hash_value, ICoinOption* profile)
{
	return hash160_to_CoinAddress(hash_value, profile->PubkeyStart);
}

std::string script_get_coin_address(ICoinOption* pProfile, const CScript& script)
{
	binary_istream bi(script.data);
	if (bi.has_buffer(3))
	{
		BYTE op_dup;
		bi >> op_dup; // 1
		if (op_dup == OP_DUP)
		{
			BYTE op_hashtype;
			bi >> op_hashtype; // 2
			BYTE buflen = 0;

			switch (op_hashtype)
			{
			case OP_RIPEMD160:
			case OP_SHA1:
			case OP_SHA256:
			case OP_HASH160:
			case OP_HASH256:
				bi >> buflen; // 3
				break;
			default:
				//bi >> ??;
				break;
			}
			if (buflen > 0)
			{
				std::vector<BYTE> buf;
				if( bi.has_buffer(buflen) )
				{
					buf.resize(buflen);
					bi >> pod_buffer(buf.data(), buflen);
					switch (op_hashtype)
					{
					case OP_RIPEMD160:
						if(buf.size() == sizeof(ripemd160))
						{
							ripemd160 v = *(ripemd160*)&buf[0];
							//							return ripemd160_to_CoinAddress(v, pProfile);
						}
						break;
					case OP_SHA1:
						typedef uint160 sha1;
						if(buf.size() == sizeof(sha1))
						{
							sha1 v = *(sha1*)&buf[0];
							//							return sha1_to_CoinAddress(v, pProfile);							
						}
						break;
					case OP_SHA256:
						typedef uint256 sha256;
						if(buf.size() == sizeof(sha256))
						{
							sha256 v = *(sha256*)&buf[0];
							//							return sha256_to_CoinAddress(v, pProfile);							
						}
						break;
					case OP_HASH160:
						typedef uint160 hash160;
						if (buf.size() == sizeof(hash160))
						{
							hash160 v = *(hash160*)&buf[0];
							return hash160_to_CoinAddress(v, pProfile);							
						}
						break;
					case OP_HASH256:
						typedef uint256 hash256;
						if (buf.size() == sizeof(hash256))
						{
							hash256 v = *(hash256*)&buf[0];
							//							return hash256_to_CoinAddress(v, pProfile);							
						}
						break;
					}
				}
			}

		}
	}
	return "";
};




uint160 _hash_160(const BYTE* src, size_t len)
{
	uint256 h256;
	SHA256(src, len, h256.data);
	uint160 r;
	RIPEMD160(h256.data, sizeof(h256), r.data);
	return r;
}

uint256 _hash_256(const BYTE* src, size_t len)
{
	return double_sha256(src, len);
}


uint256 calc_block_hash(const struct CBlockHeader& hdr)
{
	return double_sha256(&hdr, sizeof(CBlockHeader));
}
double ToMoney(__int64 nValue)
{
	return double(nValue) / double(COIN);
}