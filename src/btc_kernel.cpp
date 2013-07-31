#pragma once
#include "stdafx.h"
#include <sdk/win32_fstream.hpp>
#include <sdk/binary_stream.hpp>
#include "btc_peer.h"
#include "ICoinProfile.h"
#include "util.h"
#include "btc_stream.hpp"

struct CBtcProfile : ICoinProfile 
{
	CBtcProfile()
	{
		this->prev_name = "BTC";

		static const char* g_BTC_DNSSeed[][2] = {
			{"bitcoin.sipa.be", "seed.bitcoin.sipa.be"},
			{"bluematt.me", "dnsseed.bluematt.me"},
			{"dashjr.org", "dnsseed.bitcoin.dashjr.org"},
			{"xf2.org", "bitseed.xf2.org"},
			{NULL, NULL}
		};

		for (int i = 0; g_BTC_DNSSeed[i][0]; ++i)
		{
			this->DNSSeed.push_back( g_BTC_DNSSeed[i][1] );
		}
		static std::array<BYTE,4> g_BTC_pchMessageStart = { 0xf9, 0xbe, 0xb4, 0xd9 };

		this->pchMessageStart = g_BTC_pchMessageStart;

		this->port = 8333;

		this->protocol_version = 70001;

		//this->first_blockhash = rstr_to_uint256("000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f");
		//this->first_blocktime = _time32(0) - 60*60*24*100;
		this->first_blockhash = rstr_to_uint256("000000000000008f78449a829626f875e60fdf0d9c65708d3a83f427161920d2");
		this->first_blocktime = 0;

		this->PubkeyStart = 0;
	}
};


struct CLtcProfile : ICoinProfile 
{
	CLtcProfile()
	{
		this->prev_name = "LTC";

		static const char* g_LTC_DNSSeed[][2] = {
			{"litecointools.com", "dnsseed.litecointools.com"},
			{"litecoinpool.org", "dnsseed.litecoinpool.org"},
			{"xurious.com", "dnsseed.ltc.xurious.com"},
			{"koin-project.com", "dnsseed.koin-project.com"},
			{"weminemnc.com", "dnsseed.weminemnc.com"},
			{NULL, NULL}
		};

		for (int i = 0; g_LTC_DNSSeed[i][0]; ++i)
		{
			this->DNSSeed.push_back( g_LTC_DNSSeed[i][1] );
		}
		static std::array<BYTE,4> g_LTC_pchMessageStart = { 0xfb, 0xc0, 0xb6, 0xdb }; 

		this->pchMessageStart = g_LTC_pchMessageStart;

		this->port = 9333;

		this->protocol_version = 60002;

		this->first_blockhash = rstr_to_uint256("12a765e31ffd4059bada1e25190f6e98c99d9714d334efa41a195a7e7e04bfe2");

		this->first_blocktime = _time32(0) - 60*60*24*100;

		this->PubkeyStart = 48;
	}
};

struct CTestProfile : CLtcProfile 
{
	CTestProfile()
	{
		this->DNSSeed.clear();
		this->DNSSeed.push_back("127.0.0.1");
		this->port = 9433;
		this->first_blockhash = rstr_to_uint256("6007529468cb16ffb239276a64fc91debf7dcec04bde6ddf107c2d5b9019c9bd");
	}
};
//////////////////////////////////////////////////////////////////////////





std::string GetAppFile(const std::string& name)
{
	std::string s;
	s.resize(MAX_PATH);
	s.resize( GetModuleFileNameA(0, (char*)s.c_str(), MAX_PATH) );
	size_t pos = s.rfind('\\');
	if (pos != std::string::npos)
	{
		s.erase( pos+1, std::string::npos );
	}
	s += name;
	return s;
}


struct CBlocksContext
{
	uint256 last_block_hash;
	UINT32 last_block_time;
	UINT32 nBlockHeight;
	std::map<uint256, CBlock> blocks;
};

template<>
struct serialize_t<win32_fstream, CBlocksContext> : serialize_load_save<win32_fstream,CBlocksContext>
{
	template<typename Archive>
	static void save(Archive& ar, const CBlocksContext& fctx)
	{
		win32_fstream& stm = ar.get_stream();
		UINT32 old_last_time = 0;
		UINT32 old_size = 0;
		stm.SetFilePointer(FILE_BEGIN, 0);
		if (stm.GetFileSize() > sizeof(uint256)+sizeof(uint256)+sizeof(UINT32)+sizeof(UINT32))
		{
			uint256 old_last_block;
			UINT32 old_nBlockHeight;
			stm >> old_last_block
				>> old_last_time
				>> old_nBlockHeight
				>> old_size;
		}
		stm.SetFilePointer(FILE_END, 0);
		std::map<uint256,CBlock>::const_iterator it;
		size_t append_count = 0;
		for (it = fctx.blocks.begin(); it != fctx.blocks.end(); ++it)
		{
			if (it->second.nTime > old_last_time)
			{
				++append_count;
				stm << it->first
					<< it->second;
			}
		}
		stm.SetFilePointer(FILE_BEGIN, 0);
		UINT32 new_size = old_size+append_count;
		stm << fctx.last_block_hash
			<< fctx.last_block_time
			<< fctx.nBlockHeight
			<< new_size;
	}
	template<typename Archive>
	static void load(Archive& ar, CBlocksContext& fctx)
	{
		win32_fstream& stm = ar.get_stream();
		stm.SetFilePointer(FILE_BEGIN, 0);
		UINT32 size = 0;
		stm >> fctx.last_block_hash
			>> fctx.last_block_time
			>> fctx.nBlockHeight
			>> size;
		for (UINT32 i = 0; i < size; ++i)
		{
			std::pair<uint256, CBlock> pv;
			stm >> pv.first
				>> pv.second;
			fctx.blocks.insert(pv);
		}
	}
};

bool read_from_file(const std::string& name, CBlocksContext& fctx)
{
	win32_fstream fstm(GetAppFile(name), OPEN_EXISTING);
	if (fstm.IsOpen())
	{
		fstm >> fctx;
		return true;
	}
	return false;
}
void save_to_file(const std::string& name, const CBlocksContext& fctx)
{
	win32_fstream fstm(GetAppFile(name), OPEN_ALWAYS);
	fstm << fctx;
}


CBlocksContext fctx;

bool load_blocks_context(ICoinProfile* pctx)
{
	if( read_from_file(pctx->prev_name+".blocks", fctx) )
	{
		return true;
	}
	fctx.last_block_hash = uint256_null;
	fctx.last_block_time = 0;
	return false;
}

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

std::string hash160_to_CoinAddress(const uint160& hash_value, ICoinProfile* profile)
{
	return hash160_to_CoinAddress(hash_value, profile->PubkeyStart);
}

std::string script_get_coin_address(ICoinProfile* pProfile, const CScript& script)
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

BOOL filter_block(ICoinProfile* pProfile, CBlock& blk, const std::function<void(BOOL*, const std::string&)>& pfn)
{
	BOOL bSave = FALSE;
	for (auto tit = blk.vtx.begin(); tit != blk.vtx.end(); ++tit)
	{
		CTransaction& t = *tit;
		for (auto iit = t.vin.begin(); iit != t.vin.end(); ++iit)
		{
			CTxIn& In = *iit;
			// check prev block money
		}
		for (auto oit = t.vout.begin(); oit != t.vout.end(); ++oit)
		{
			CTxOut& out = *oit;
			double money = double(out.nValue) / COIN;
			std::string coin_address = script_get_coin_address(pProfile, out.scriptPubKey);
			if (coin_address.size())
			{
				if (pfn)
				{
					pfn(&bSave, coin_address);
				}
			}
		}
	}
	return bSave;
}



shared_ptr<IPeerGroup> CreatePeerGroup();

shared_ptr<IPeerGroup> m_group = CreatePeerGroup();


void bitcoin_test()
{
	shared_ptr<ICoinProfile> pProfile ( new CTestProfile );
	ICoinProfile* ctx = pProfile.get();
	if (!load_blocks_context(ctx))
	{
		fctx.last_block_hash = ctx->first_blockhash;
		fctx.last_block_time = ctx->first_blocktime;
	}
	UINT32 old_last_time = fctx.last_block_time;

	m_group->async_connect_from_profile(pProfile);
	shared_ptr<IPeerNode> pPeer;
	for (;;)
	{
		pPeer = m_group->pop_idle();
		if (pPeer)
		{
			break;
		}
		Sleep(100);
	}	
	pPeer->async_download(fctx.last_block_hash, fctx.last_block_time);
// 	tcp::socket& sk = pPeer->sk;
// 	CVersionCmdHead destver = boost::any_cast<CVersionCmdHead>(pPeer->param);
// 
// // 	cmd = read_package(ctx, sk, package);
// // 	assert(cmd == "verack");
// 	UINT tick = GetTickCount();
// 	std::vector<uint256> download_pool;
// 	while ((int)fctx.nBlockHeight < destver.nBestHeight)
// 	{
// 		send_getheaders(ctx, sk, fctx.last_block_hash);
// 		if( !wait_package(ctx, sk, net_stream, "headers") )
// 		{
// 			break;
// 		}
// 		int offset = 0;
// 		std::vector<CBlock> newheaders;
// 		net_stream >> newheaders;
// 		if (newheaders.empty())
// 		{
// 			break;
// 		}
// 		else if (newheaders.size() == 1)
// 		{
// 			if( memcmp(&newheaders[0], &uint256_null, sizeof(uint256)) == 0 )
// 			{
// 				break;
// 			}
// 		}
// 		if (newheaders.empty())
// 		{
// 			break;
// 		}
// 		uint256 new_last_block_hash = uint256_null;
// 		UINT32 MaxTime = 0;
// 		CBlockHeader* pMaxRecord = nullptr;
// 		for (auto it = newheaders.begin(); it != newheaders.end(); ++it)
// 		{
// 			CBlockHeader& hdr = *it;
// 			if (hdr.nTime > MaxTime)
// 			{
// 				MaxTime = hdr.nTime;
// 				pMaxRecord = &hdr;
// 			}
// 			if (hdr.nTime > fctx.last_block_time)
// 			{
// 				uint256 hash = calc_block_hash(hdr);
// 				download_pool.push_back(hash);
// 				new_last_block_hash = hash;
// 			}
// 			++fctx.nBlockHeight;
// 		}
// 		if (new_last_block_hash != uint256_null)
// 		{
// 			fctx.last_block_hash = new_last_block_hash;
// 		}
// 		else
// 		{
// 			assert(pMaxRecord);
// 			fctx.last_block_hash = calc_block_hash(*pMaxRecord);
// 		}
// #if _DEBUG
// 		std::string strHashLast = uint256_to_rstr(fctx.last_block_hash);
// #endif
// 		if (download_pool.size())
// 		{
// 			send_getblockdatas(ctx, sk, download_pool);
// 			size_t nmax = download_pool.size();
// 			for( size_t i = 0; i < nmax; ++i)
// 			{
// 				if( !wait_package(ctx, sk, net_stream, "block", 30*1000) )
// 				{
// 					throw std::logic_error("recv block timeout");
// 				}
// 				CBlock blk;
// 				net_stream >> blk;
// 				uint256 block_hash = calc_block_hash(blk);
// #if _DEBUG
// 				std::string strHashLast = uint256_to_rstr(block_hash);
// #endif
// 				if( filter_block(ctx, blk, 0) )
// 				{
// 					fctx.blocks.insert(std::make_pair(block_hash, blk));
// 				}
// 				auto it = std::find(download_pool.begin(), download_pool.end(), block_hash);
// 				if (it != download_pool.end())
// 				{
// 					download_pool.erase(it);
// 				}
// 				Sleep(1);
// 			}
// 		}
// 		tick = GetTickCount() - tick;
// 		if (tick > 1000)
// 		{
// 			tick = GetTickCount();
// 			printf("download header %f [%d/%d]\n", double(fctx.blocks.size())/destver.nBestHeight*100, fctx.blocks.size(), destver.nBestHeight);
// 		}
// 	}
// 
// 	if (old_last_time == fctx.last_block_time)
// 	{
// 		save_to_file(ctx->prev_name+".blocks", fctx);
// 	}
}