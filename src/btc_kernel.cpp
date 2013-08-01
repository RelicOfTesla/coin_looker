#pragma once
#include "stdafx.h"
#include <set>
#include <sdk/win32_fstream.hpp>
#include "btc_peer.h"
#include "btc_stream.hpp"
#include "btc_helper.h"
#include "ICoinOption.h"
#include <boost/thread/mutex.hpp>
#include <boost/algorithm/string.hpp>
#include "IUserContext.h"

uint256 calc_transaction_hash(const CTransaction& tx)
{
	btcnet_ostream stm;
	stm << tx;
	return double_sha256(stm.ostm.m_data.data(), stm.ostm.m_data.size());
}


struct CLocalBlockDB
{
	uint256 last_block_hash;
	UINT32 last_block_time;
	UINT32 nBlockHeight;
	boost::mutex mutex_blocks;
	std::map< uint256, shared_ptr<CBlock> > blocks;

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
		else
		{
			stm << uint256_null
				<< UINT32(0)
				<< UINT32(0)
				<< UINT32(0);
		}
		stm.SetFilePointer(FILE_END, 0);
		std::map<uint256,shared_ptr<CBlock> >::const_iterator it;
		size_t append_count = 0;
		for (it = block_db.blocks.begin(); it != block_db.blocks.end(); ++it)
		{
			if (it->second->nTime > old_last_time)
			{
				++append_count;
				stm << it->first
					<< *it->second;
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
			uint256 hash;
			shared_ptr<CBlock> pBlock(new CBlock);
			stm >> hash
				>> *pBlock;
			block_db.blocks.insert(std::make_pair(hash, pBlock));
		}
	}
};
bool CLocalBlockDB::read_from_file(const std::string& name, CLocalBlockDB& block_db)
{
	boost::mutex::scoped_lock lock(block_db.mutex_blocks);
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
	boost::mutex::scoped_lock lock(*(boost::mutex*)&block_db.mutex_blocks);
	win32_fstream fstm(GetAppFile(name), OPEN_ALWAYS);
	fstm << block_db;
}

struct CUserBook
{
	std::set< CoinKey > RecvCoinList;
	
};



shared_ptr<IPeerGroup> CreatePeerGroup();

class CWorkContext : public enable_shared_from_this<CWorkContext>
{
public:
	CWorkContext(shared_ptr<ICoinOption> pCoinOption) : 
	  m_pCoinOption(pCoinOption), m_pBlockDB(new CLocalBlockDB), m_pUserData(new CUserBook), m_bChange(false), m_last_save_time(0)
	{
		m_pPeerGroup = CreatePeerGroup();
	}
public:
	void load_db_block()
	{
		if( !CLocalBlockDB::read_from_file(m_pCoinOption->prev_name+".blocks", *m_pBlockDB) )
		{
			m_pBlockDB->last_block_hash = m_pCoinOption->first_blockhash;
			m_pBlockDB->last_block_time = m_pCoinOption->first_blocktime;
		}
		load_tx_map();
	}
	void load_tx_map()
	{
		for (auto bit = m_pBlockDB->blocks.begin(); bit != m_pBlockDB->blocks.end(); ++bit)
		{
			int tx_index = 0;
			for (auto tit = bit->second->vtx.begin(); tit != bit->second->vtx.end(); ++tit)
			{
				CTransaction& tx = *tit;
				uint256 txid = calc_transaction_hash(tx);
				transaction_map_index tmi;
				tmi.block_hash = bit->first;
				tmi.tx_index = tx_index++;
				tmi.out_index = -1;
				bool isfind = false;
				for(auto iit = tx.vin.begin(); iit != tx.vin.end(); ++iit)
				{
					if( m_TxMap.find( iit->prevout.hash ) != m_TxMap.end() )
					{
						isfind = true;
						break;
					}
				}
				if (!isfind)
				{
					int out_index = 0;
					for (auto oit = tx.vout.begin(); oit != tx.vout.end(); ++oit)
					{
						CoinKey addr = script_get_coin_key(m_pCoinOption.get(), oit->scriptPubKey);
						if (!addr.empty())
						{
							if( m_pUserData->RecvCoinList.find(addr) != m_pUserData->RecvCoinList.end() )
							{
								tmi.out_index = out_index;
								break;
							}
						}
						++out_index;
					}
				}
				m_TxMap.insert(std::make_pair(txid, tmi));
			}
		}

	}
	void load_user_book()
	{
		win32_fstream fp;
		fp.Open(GetAppFile(m_pCoinOption->prev_name+".txt"), OPEN_EXISTING);
		if (!fp.IsOpen())
		{
			return;
		}
		binary bin;
		bin.resize( fp.GetFileSize() );
		fp.Read(bin.data(), bin.size());
		bin.push_back(0);
		const char* pFile = (char*)&bin[0];
		std::vector<std::string> pList;
		boost::split(pList, pFile, boost::is_space());
		for (auto it = pList.begin(); it != pList.end(); ++it)
		{
			CoinKey key(*it);
			if (!key.empty())
			{
				m_pUserData->RecvCoinList.insert(key);
			}
		}
	}

	void save_check(bool must = false)
	{
		if (m_bChange || must)
		{
			if (time(0) - m_last_save_time > 10*1000)
			{
				CLocalBlockDB::save_to_file(m_pCoinOption->prev_name+".blocks", *m_pBlockDB);
				m_bChange = false;
				m_last_save_time = time(0);
			}
		}
	}

	void start()
	{
		m_pPeerGroup->async_connect_from_profile(m_pCoinOption);
	}

	void peer_start(shared_ptr<IPeerNode> pPeer)
	{
		pPeer->pfn_OnNewBlock.connect( boost::bind(&CWorkContext::filter_block, shared_from_this(), _1, _2) );
		pPeer->pfn_OnNewHeaderList.connect( boost::bind(&CWorkContext::filter_heads, shared_from_this(), _1) );
		pPeer->async_download_headers(m_pBlockDB->last_block_hash);
	}
	shared_ptr<IPeerNode> pop_idle()
	{
		return m_pPeerGroup->pop_idle();
	}
	
protected:
	void filter_block(const uint256& block_hash, const CBlock& blk);
	void filter_heads(const std::vector<CBlock>& heads);
public:
	struct transaction_map_index 
	{
		uint256 block_hash;
		int tx_index;
		int out_index;
	};
protected:

	shared_ptr<CLocalBlockDB> m_pBlockDB;
	shared_ptr<CUserBook> m_pUserData;
	shared_ptr<ICoinOption> m_pCoinOption;
	shared_ptr<IPeerGroup> m_pPeerGroup;

	boost::mutex mutex_txmap;
	std::map< uint256, transaction_map_index > m_TxMap;
public:
	bool m_bChange;
	time_t m_last_save_time;
	shared_ptr<IPeerNode> m_pCurrentPeer;
};

void CWorkContext::filter_heads(const std::vector<CBlock>& newheaders)
{
	UINT32 last_block_time = m_pBlockDB->last_block_time;
	uint256 last_block_hash = uint256_null;
	std::vector<uint256> request_list;
	request_list.reserve(newheaders.size());
	for (auto it = newheaders.begin(); it != newheaders.end(); ++it)
	{
		const CBlockHeader& hdr = *it;
		if (hdr.nTime >= last_block_time )
		{
			uint256 hash = calc_block_hash(hdr);
			request_list.push_back(hash);
			last_block_time = hdr.nTime;
			last_block_hash = hash;
		}
	}
	if (request_list.size())
	{
		m_pCurrentPeer->async_download_blocks(request_list);
	}
	if (last_block_hash == uint256_null)
	{
		last_block_hash = calc_block_hash(*newheaders.rbegin());
	}
	m_pCurrentPeer->async_download_headers(last_block_hash);
}

void CWorkContext::filter_block(const uint256& block_hash, const CBlock& blk)
{
	if( blk.hashPrevBlock != m_pBlockDB->last_block_hash )
	{
		// blockchain check
#if _DEBUG
//		DebugBreak();
#endif
	}
	m_pBlockDB->last_block_time = blk.nTime;
	m_pBlockDB->last_block_hash = block_hash;
#if _DEBUG
	std::string szHash = uint256_to_rstr(block_hash);
#endif
	OutputDebugString("Begin Block 1 filter \n");
	shared_ptr<CBlock> save_blk(new CBlock);
	*static_cast<CBlockHeader*>(save_blk.get()) = blk;

	for (auto tit = blk.vtx.begin(); tit != blk.vtx.end(); ++tit)
	{
		const CTransaction& t = *tit;
		uint256 txid = calc_transaction_hash(t);
#if _DEBUG
		std::string szTxid = uint256_to_rstr(txid);
#endif
		transaction_map_index new_tmi;
		bool save_tx = false;
		{
			boost::mutex::scoped_lock lock(mutex_txmap);
			for (auto iit = t.vin.begin(); iit != t.vin.end(); ++iit)
			{
				const CTxIn& In = *iit;
				auto xit = m_TxMap.find( In.prevout.hash );
				if( xit != m_TxMap.end() )
				{
					const transaction_map_index& tmi = xit->second;
					if (tmi.out_index == In.prevout.n)
					{
#if _DEBUG
						std::string pretx = uint256_to_rstr(In.prevout.hash);
#endif
						save_tx = true;
						new_tmi.out_index = -1;
						break;
					}
				}
			}
		}
		if (!save_tx)
		{
			int out_idx = 0;
			for (auto oit = t.vout.begin(); oit != t.vout.end(); ++oit)
			{
				const CTxOut& out = *oit;
				double money = ToMoney(out.nValue);
				CoinKey key = script_get_coin_key(m_pCoinOption.get(), out.scriptPubKey);
				if (!key.empty())
				{
					if (m_pUserData->RecvCoinList.find(key) != m_pUserData->RecvCoinList.end())
					{
						new_tmi.out_index = out_idx;
						save_tx = true;
						break;
					}
				}
				++out_idx;
			}
		}
		if (save_tx)
		{
			save_blk->vtx.push_back(*tit);
			new_tmi.block_hash = block_hash;
			new_tmi.tx_index = save_blk->vtx.size() - 1;
			boost::mutex::scoped_lock lock(mutex_txmap);
			m_TxMap.insert( std::make_pair( txid, new_tmi ) );
		}
	}
	if (save_blk->vtx.size())
	{
		boost::mutex::scoped_lock lock(m_pBlockDB->mutex_blocks);
		m_pBlockDB->blocks.insert(std::make_pair(block_hash, save_blk));
		m_bChange = true;
	}
	OutputDebugString("End Block 1 filter \n");
	Sleep(1);
}

static const CTxOut g_TxOut_null;
static const CTxOut& GetTransactionOut(const std::map<uint256, shared_ptr<CBlock> >& Blocks, 
	const std::map<uint256,CWorkContext::transaction_map_index>& TxMaps,
	const COutPoint& outp)
{
#if _DEBUG
	std::string szTxid = uint256_to_rstr(outp.hash);
#endif

	auto xit = TxMaps.find(outp.hash);
	if (xit != TxMaps.end())
	{
		const CWorkContext::transaction_map_index& tmi = xit->second;
#if _DEBUG
		std::string szTxid = uint256_to_rstr(tmi.block_hash);
#endif
		auto bit = Blocks.find(tmi.block_hash);
		if (bit != Blocks.end())
		{
			const shared_ptr<CBlock> blk = bit->second;
			if (  tmi.tx_index>=0 && tmi.tx_index<blk->vtx.size())
			{
				CTransaction& tx = blk->vtx[tmi.tx_index];
				if (outp.n>=0 && outp.n<tx.vout.size())
				{
					return tx.vout[outp.n];
				}
			}
		}
	}
	return g_TxOut_null;
}

class CUserContext : public CWorkContext, public IUserContext
{
public:
	CUserContext(shared_ptr<ICoinOption> pCoinOption) : CWorkContext(pCoinOption)
	{
		this->load_user_book();
	}

	void load_db()
	{
		this->load_db_block();
		this->start();
	}
	void modal()
	{
		if (!this->m_pCurrentPeer)
		{
			shared_ptr<IPeerNode> pPeer = this->pop_idle();
			if (pPeer)
			{
				this->m_pCurrentPeer = pPeer;
				this->peer_start(pPeer);
			}
		}
		else
		{
			//worker->save_check();
		}
	}

	void uninit()
	{
		this->save_check(true);
	}

	std::vector<std::string> work_get_books()
	{
		std::vector<std::string> result;
		for (auto it = this->m_pUserData->RecvCoinList.begin(); it!=this->m_pUserData->RecvCoinList.end(); ++it)
		{
			if (!it->empty())
			{
				result.push_back(it->to_str());
			}
		}
		return result;
	}

	workdata work_get_data(const char* pCoinAddr)
	{
		workdata wd;
		wd.RecvCoinAddr = pCoinAddr;
		wd.LastMoney = 0;
		wd.RecvMoney = 0;
		wd.LastTime = 0;
		CoinKey CoinAddr(pCoinAddr);
		if (CoinAddr.empty())
		{
			return wd;
		}

		__int64 LastMoney = 0;
		__int64 RecvMoney = 0;
		std::map< uint256,shared_ptr<CBlock> > blocks;
		{
			boost::mutex::scoped_lock lock(this->m_pBlockDB->mutex_blocks);
			blocks = this->m_pBlockDB->blocks;
		}
		std::map< uint256, CWorkContext::transaction_map_index > TxMaps;
		{
			boost::mutex::scoped_lock lock(this->mutex_txmap);
			TxMaps = this->m_TxMap;
		}
		for (auto bit = blocks.begin(); bit != blocks.end(); ++bit)
		{
			shared_ptr<CBlock>& blk = bit->second;
			for(auto tit = blk->vtx.begin(); tit != blk->vtx.end(); ++tit)
			{
				const CTransaction& t = *tit;
				{
					for (auto iit = t.vin.begin(); iit != t.vin.end(); ++iit)
					{
						const CTxOut& vout = GetTransactionOut(blocks, TxMaps, iit->prevout );
						if (&vout != &g_TxOut_null)
						{
							CoinKey key = script_get_coin_key(this->m_pCoinOption.get(), vout.scriptPubKey);
							if (!key.empty())
							{
								if ( key == CoinAddr)
								{
									RecvMoney -= vout.nValue;
									if (wd.LastTime < blk->nTime)
									{
										wd.LastTime = blk->nTime;
										LastMoney = -vout.nValue;
									}
								}
							}
						}
					}
				}
				for (auto oit = t.vout.begin(); oit != t.vout.end(); ++oit)
				{
					CoinKey key = script_get_coin_key(this->m_pCoinOption.get(), oit->scriptPubKey);
					if (!key.empty())
					{
						if (key == CoinAddr)
						{
							RecvMoney += oit->nValue;
							if (wd.LastTime < blk->nTime)
							{
								wd.LastTime = blk->nTime;
								LastMoney = oit->nValue;
							}
						}
					}
				}
			}
		}
		wd.LastMoney = ToMoney(LastMoney);
		wd.RecvMoney = ToMoney(RecvMoney);
		return wd;
	}

	time_t work_get_current_time()
	{
		return this->m_pBlockDB->last_block_time;
	}

};

shared_ptr<IUserContext> create_coin_work(shared_ptr<ICoinOption> pCoinOption)
{
	boost::shared_ptr<CUserContext> worker(new CUserContext(pCoinOption));
	return worker;
}



void bitcoin_test()
{
// 	shared_ptr<ICoinOption> pCoinOption(new CBtcOption);
// 	boost::shared_ptr<CWorkContext> pWork = create_coin_work(pCoinOption);
// 	for (;;)
// 	{
// 		work_modal(pWork);
// 		Sleep(1);
// 	}
}