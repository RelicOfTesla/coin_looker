#pragma once
#include "stdafx.h"
#include <sdk/win32_fstream.hpp>
#include "btc_peer.h"
#include "ICoinProfile.h"
#include "btc_stream.hpp"
#include "btc_helper.h"
#include <set>

struct CBtcOption : ICoinOption 
{
	CBtcOption()
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


struct CLtcOption : ICoinOption 
{
	CLtcOption()
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

struct CTestOption : CLtcOption
{
	CTestOption()
	{
		this->DNSSeed.clear();
		this->DNSSeed.push_back("127.0.0.1");
		this->port = 9433;
		this->first_blockhash = rstr_to_uint256("6007529468cb16ffb239276a64fc91debf7dcec04bde6ddf107c2d5b9019c9bd");
	}
};
//////////////////////////////////////////////////////////////////////////

struct CLocalBlockDB
{
	uint256 last_block_hash;
	UINT32 last_block_time;
	UINT32 nBlockHeight;
	std::map<uint256, CBlock> blocks;

	static bool read_from_file(const std::string& name, CLocalBlockDB& block_db);
	static void save_to_file(const std::string& name, const CLocalBlockDB& block_db);
};

template<>
struct serialize_t<win32_fstream, CLocalBlockDB> : serialize_load_save<win32_fstream,CLocalBlockDB>
{
	template<typename Archive>
	static void save(Archive& ar, const CLocalBlockDB& block_db)
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
		for (it = block_db.blocks.begin(); it != block_db.blocks.end(); ++it)
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
		stm << block_db.last_block_hash
			<< block_db.last_block_time
			<< block_db.nBlockHeight
			<< new_size;
	}
	template<typename Archive>
	static void load(Archive& ar, CLocalBlockDB& block_db)
	{
		win32_fstream& stm = ar.get_stream();
		stm.SetFilePointer(FILE_BEGIN, 0);
		UINT32 size = 0;
		stm >> block_db.last_block_hash
			>> block_db.last_block_time
			>> block_db.nBlockHeight
			>> size;
		for (UINT32 i = 0; i < size; ++i)
		{
			std::pair<uint256, CBlock> pv;
			stm >> pv.first
				>> pv.second;
			block_db.blocks.insert(pv);
		}
	}
};
bool CLocalBlockDB::read_from_file(const std::string& name, CLocalBlockDB& block_db)
{
	win32_fstream fstm(GetAppFile(name), OPEN_EXISTING);
	if (fstm.IsOpen())
	{
		fstm >> block_db;
		return true;
	}
	block_db.last_block_hash = uint256_null;
	block_db.last_block_time = 0;
	return false;
}
void CLocalBlockDB::save_to_file(const std::string& name, const CLocalBlockDB& block_db)
{
	win32_fstream fstm(GetAppFile(name), OPEN_ALWAYS);
	fstm << block_db;
}

struct CUserBook
{
	std::set< std::string > RecvCoinAddr;
	
};

struct FilterContext 
{
	shared_ptr<CLocalBlockDB> pBlockDB;
	shared_ptr<CUserBook> pUserData;
	shared_ptr<ICoinOption> pCoinOption;
};

void filter_block(shared_ptr<FilterContext> pFilterCTX, const uint256& block_hash, const CBlock& blk)
{
	if (blk.nTime < pFilterCTX->pBlockDB->last_block_time)
	{
		return;
	}
	if( blk.hashPrevBlock != pFilterCTX->pBlockDB->last_block_hash )
	{
		// blockchain check
#if _DEBUG
		DebugBreak();
#endif
	}
	pFilterCTX->pBlockDB->last_block_time = blk.nTime;
	pFilterCTX->pBlockDB->last_block_hash = block_hash;

	CBlock save_blk;
	*static_cast<CBlockHeader*>(&save_blk) = blk;

	for (auto tit = blk.vtx.begin(); tit != blk.vtx.end(); ++tit)
	{
		const CTransaction& t = *tit;
		for (auto iit = t.vin.begin(); iit != t.vin.end(); ++iit)
		{
			const CTxIn& In = *iit;
			// check prev block money
		}
		for (auto oit = t.vout.begin(); oit != t.vout.end(); ++oit)
		{
			const CTxOut& out = *oit;
			double money = ToMoney(out.nValue);
			std::string coin_address = script_get_coin_address(pFilterCTX->pCoinOption.get(), out.scriptPubKey);
			if (coin_address.size())
			{
			}
		}
	}
}


shared_ptr<IPeerGroup> CreatePeerGroup();

shared_ptr<IPeerGroup> m_group = CreatePeerGroup();


void bitcoin_test()
{
	shared_ptr<FilterContext> fctx(new FilterContext);
	fctx->pBlockDB.reset(new CLocalBlockDB);
	fctx->pCoinOption.reset(new CBtcOption);
	fctx->pUserData.reset(new CUserBook);
	if (!CLocalBlockDB::read_from_file("local_block", *fctx->pBlockDB))
	{
		fctx->pBlockDB->last_block_hash = fctx->pCoinOption->first_blockhash;
		fctx->pBlockDB->last_block_time = fctx->pCoinOption->first_blocktime;
	}

	m_group->async_connect_from_profile(fctx->pCoinOption);
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
	pPeer->pfn_OnNewBlock.connect( boost::bind(filter_block, fctx, _1, _2) );
	pPeer->async_download(fctx->pBlockDB->last_block_hash, fctx->pBlockDB->last_block_time);
}