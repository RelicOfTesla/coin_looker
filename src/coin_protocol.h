#pragma once

#include <sdk/binary.h>
#include <array>

#include "coin_def.h"

/** Message header.
 * (4) message start.
 * (12) command.
 * (4) size.
 * (4) checksum.
 */
struct CMessageHeader 
{
	char pchMessageStart[4];
	char pchCommand[12];
	UINT nMessageSize;
	UINT nChecksum;

	CMessageHeader(bool tmp)
	{}
	CMessageHeader(const std::array<BYTE,4>& headstart)
	{
		memcpy(this->pchMessageStart, &headstart, sizeof(this->pchMessageStart));
		memset(pchCommand, 0, sizeof(pchCommand));
		pchCommand[1] = 1;
		nMessageSize = -1;
		nChecksum = 0;
	}
	CMessageHeader(const std::array<BYTE,4>& headstart, const char* pszCommand, unsigned int nMessageSizeIn)
	{
		memcpy(this->pchMessageStart, &headstart, sizeof(this->pchMessageStart));
		strncpy(pchCommand, pszCommand, sizeof(pchCommand));
		nMessageSize = nMessageSizeIn;
		nChecksum = 0;
	}

};

struct CAddress
{
	UINT64 nServices;
	sockaddr Addr;
	USHORT Port;
};

struct CVersionCmdHead
{
	UINT32 ProtocolVersion;
	UINT64 nService;
	UINT64 AdjustTime;
	CAddress remote;
	CAddress local;
	UINT64 nNonce;
	std::string ClientVersion;
	INT32 nBestHeight;
};

struct CBlockHeader 
{
	int nVersion;
	uint256 hashPrevBlock;
	uint256 hashMerkleRoot;
	unsigned int nTime;
	unsigned int nBits;
	unsigned int nNonce;
};

struct COutPoint
{
	uint256 hash;
	unsigned int n;
};

struct CScript 
{
	binary data;
};

struct CTxIn 
{
	COutPoint prevout;
	CScript scriptSig;
	unsigned int nSequence;
};

struct CTxOut 
{
	__int64 nValue;
	CScript scriptPubKey;
};

struct CTransaction 
{
	int nVersion;
	std::vector<CTxIn> vin;
	std::vector<CTxOut> vout;
	unsigned int nLockTime;
};

struct CBlock : CBlockHeader
{
	std::vector<CTransaction> vtx;
};

struct CompactSize 
{
	UINT64 n;

	CompactSize(size_t r = 0) : n(r)
	{}

	size_t size()const
	{
		return static_cast<size_t>( n );
	}
};

enum CInv_type
{
	MSG_TX = 1,
	MSG_BLOCK,
	MSG_FILTERED_BLOCK,
};
struct CInv 
{
	int type;
	uint256 hash;
};

#pragma pack(pop)