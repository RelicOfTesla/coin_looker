#pragma once

//#include <boost/asio/ip/tcp.hpp>
#include <sdk/binary.h>
#include <sdk/boost_lib/signals.hpp>
#include "coin_def.h"

struct ICoinOption;
struct CBlock;
struct CBlockHeader;

struct IPeerNode 
{
	virtual void async_download_headers(const uint256& last_block_hash) = 0;
	virtual void async_download_blocks(const std::vector<uint256>& blocks) = 0;

	signals::signal< void(const std::vector<CBlock>&) > pfn_OnNewHeaderList;
	signals::signal< void(const uint256& hash, const CBlock&) > pfn_OnNewBlock;
};

struct IPeerGroup 
{
	virtual void async_connect_from_profile(shared_ptr<ICoinOption> ctx) = 0;
//	virtual void async_connect(shared_ptr<ICoinOption>, boost::asio::ip::tcp::endpoint ep) = 0;
	virtual shared_ptr<IPeerNode> pop_idle() = 0;
};

