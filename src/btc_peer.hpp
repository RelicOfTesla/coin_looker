#pragma once

#include "btc_peer.h"
#include <boost/thread/mutex.hpp>
#include <sdk/boost_lib/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/any.hpp>

using std::enable_shared_from_this;

namespace asio = boost::asio;
typedef boost::asio::ip::tcp tcp;

class CPeerNode : public IPeerNode, public enable_shared_from_this<CPeerNode>
{
public:
	CPeerNode(boost::asio::io_service& service, shared_ptr<ICoinProfile> profile);
	void start_check();
	void async_download(const uint256& last_block_hash, UINT32 last_block_time);
public:
	virtual void send_package(const char* command, binary& bin);

	virtual void send_version(int nBestHeight, const std::string& SubVer);
	virtual void send_getheaders(const uint256& start_block, const uint256& stop_block = uint256_null);
	virtual void send_getblockdatas(const std::vector<uint256>& hashs);
	virtual void send_getblock(const uint256& start_block, const uint256& stop_block = uint256_null);
	virtual shared_ptr<struct CVersionCmdHead> GetDestVersion();
protected:
	void _send_getblockdata_n(const uint256* phashs, size_t n);
	void send_getblockdata(const uint256& hash);
protected:
	void on_recv(const boost::system::error_code& ec, size_t len);
	void async_work_domodal();
	void on_new_head(struct CBlockHeader&);
	void on_new_block(shared_ptr<struct CBlock>);
	void async_recv();
protected:
	friend class CPeerGroup;
	shared_ptr<ICoinProfile> m_pProfile;
	shared_ptr<struct CVersionCmdHead> m_pDestVersion;
	tcp::socket m_sk;
	time_t m_connect_start_time;
	time_t m_ver_finish_time;
	std::array<BYTE,1024> m_buffer;
	binary m_data;
	uint256 m_last_block_hash;
	UINT32 m_last_block_time;
};

class CPeerGroup : public IPeerGroup, public enable_shared_from_this<CPeerGroup>
{
public:
	CPeerGroup() : m_is_start(false), m_busy_count(0)
	{}

	virtual void async_connect_from_profile(shared_ptr<struct ICoinProfile> ctx);
	virtual void async_connect(shared_ptr<struct ICoinProfile>, boost::asio::ip::tcp::endpoint ep);
	virtual shared_ptr<IPeerNode> pop_idle();
protected:
	std::list< shared_ptr<CPeerNode> > m_idle;
	std::list< shared_ptr<CPeerNode> > m_checking_list;
	boost::mutex m_mutex;
	boost::asio::io_service m_service;
	bool m_is_start;
	long m_busy_count;
protected:
	void on_connect(shared_ptr<struct ICoinProfile> profile, shared_ptr<CPeerNode> peer, const boost::system::error_code& ec);
	void domodal();
	void check_state();
};