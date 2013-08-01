#pragma once

#include "btc_peer.h"
#include <boost/thread/mutex.hpp>
#include <sdk/boost_lib/asio.hpp>
#include <boost/any.hpp>


namespace asio = boost::asio;
typedef boost::asio::ip::tcp tcp;

class CPeerNode : public IPeerNode, public enable_shared_from_this<CPeerNode>
{
public:
	CPeerNode(boost::asio::io_service& service, shared_ptr<ICoinOption> pCoinOption);
	void start_check();
	size_t GetDelaySpeed();
	bool IsCheckTimeout();
	tcp::socket& GetObject();
	void modal();
public:
	virtual void async_download_headers(const uint256& last_block_hash);
	virtual void async_download_blocks(const std::vector<uint256>& blocks);
	virtual shared_ptr<struct CVersionCmdHead> GetDestVersion();
protected:
	void send_package(const char* command, binary& bin);
	void send_version(int nBestHeight, const std::string& SubVer);
	void send_getheaders(const uint256& start_block, const uint256& stop_block = uint256_null);
	void send_getblockdatas(const std::vector<uint256>& hashs);
	void send_getblock(const uint256& start_block, const uint256& stop_block = uint256_null);
	void _send_getblockdata_n(const uint256* phashs, size_t n);
	void send_getblockdata(const uint256& hash);
protected:
	void on_recv(const boost::system::error_code& ec, size_t len);
	void filter_modal();
	void async_recv();
protected:
	shared_ptr<ICoinOption> m_pCoinOption;
	shared_ptr<struct CVersionCmdHead> m_pDestVersion;
	tcp::socket m_sk;
	UINT m_connect_start_tick;
	UINT m_ver_finish_tick;
	std::array<BYTE,1024> m_buffer;
	binary m_data;

	boost::mutex m_mutex_blocks;
	std::list< shared_ptr<CBlock> > m_block_pool;
	boost::mutex m_mutex_headers;
	std::list< shared_ptr< std::vector<CBlock> > > m_headers_pool;
};

class CPeerGroup : public IPeerGroup, public enable_shared_from_this<CPeerGroup>
{
public:
	CPeerGroup() : m_is_start(false), m_busy_count(0)
	{}

	virtual void async_connect_from_profile(shared_ptr<ICoinOption> ctx);
	virtual void async_connect(shared_ptr<ICoinOption>, boost::asio::ip::tcp::endpoint ep);
	virtual shared_ptr<IPeerNode> pop_idle();
protected:
	std::list< shared_ptr<CPeerNode> > m_idle;
	std::list< shared_ptr<CPeerNode> > m_checking_list;
	std::list< shared_ptr<CPeerNode> > m_working;
	boost::mutex m_mutex;
	boost::asio::io_service m_service;
	bool m_is_start;
	long m_busy_count;
protected:
	void on_connect(shared_ptr<CPeerNode> peer, const boost::system::error_code& ec);
	void domodal();
	void check_state();
};