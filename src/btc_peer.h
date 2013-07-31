#pragma once

#include <boost/asio/ip/tcp.hpp>
#include <sdk/binary.h>
#include "btc_def.h"

struct ICoinProfile;

struct IPeerNode 
{
	virtual void async_download(const uint256& last_block_hash, UINT32 last_block_time) = 0;

	virtual void send_package(const char* command, binary& bin) = 0;

	virtual void send_version(int nBestHeight, const std::string& SubVer) = 0;
	virtual void send_getheaders(const uint256& start_block, const uint256& stop_block = uint256_null) = 0;
	virtual void send_getblockdatas(const std::vector<uint256>& hashs) = 0;
	virtual void send_getblock(const uint256& start_block, const uint256& stop_block = uint256_null) = 0;
	virtual shared_ptr<struct CVersionCmdHead> GetDestVersion() = 0;

	inline void send_getblockdata(const uint256& hash) {
		std::vector<uint256> hashs;
		hashs.push_back(hash);
		send_getblockdatas(hashs);
	}

};

struct IPeerGroup 
{
	virtual void async_connect_from_profile(shared_ptr<ICoinProfile> ctx) = 0;
	virtual void async_connect(shared_ptr<ICoinProfile>, boost::asio::ip::tcp::endpoint ep) = 0;
	virtual shared_ptr<IPeerNode> pop_idle() = 0;
};

