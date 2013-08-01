#include "stdafx.h"
#include "btc_peer.hpp"
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include "btc_stream.hpp"
#include "btc_helper.h"
#include "ICoinOption.h"

//////////////////////////////////////////////////////////////////////////

static CAddress caddress_from_endpoint(tcp::endpoint& ep, BOOL net)
{
	CAddress r;
	r.nServices = net;
	r.Addr = *ep.data();
	static_assert(sizeof(sockaddr)==16, "error CAddress size");
	r.Port = ep.port();
	return r;
}


std::string FormatSubVersion(const std::string& name, const std::string ClientVersion)
{
	return "/*" + name + ":" + ClientVersion + "/";
}

std::string g_ClientVersion = FormatSubVersion("my-looker","1.0");
//////////////////////////////////////////////////////////////////////////
CPeerNode::CPeerNode(boost::asio::io_service& service, shared_ptr<ICoinOption> pCoinOption) 
	: m_sk(service) , m_connect_start_tick(GetTickCount()), m_ver_finish_tick(0), m_pCoinOption(pCoinOption)
{}

void CPeerNode::send_package(const char* command, binary& bin)
{
	assert(strlen(command) <= 12);
	CMessageHeader hdr(m_pCoinOption->pchMessageStart, command, bin.size());
	uint256 hash = double_sha256(bin.data(), bin.size());
	hdr.nChecksum = *(UINT*)&hash;
	assert(m_sk.is_open());
	m_sk.send(asio::buffer(&hdr, sizeof(hdr)));
	m_sk.send(asio::buffer(bin));
}

void CPeerNode::send_version(int nBestHeight, const std::string& SubVer)
{
	CVersionCmdHead ver;
	ver.ProtocolVersion = m_pCoinOption->protocol_version;
	ver.nService = 0;
	ver.AdjustTime = _time64(0);
	ver.remote = caddress_from_endpoint(m_sk.remote_endpoint(), TRUE);
	ver.local = caddress_from_endpoint(m_sk.local_endpoint(), FALSE);
	memset(&ver.local, 0, sizeof(ver.local));
	ver.nNonce = 1; // rand();
	ver.ClientVersion = SubVer;
	ver.nBestHeight = nBestHeight;
	btcnet_ostream stm;
	stm << ver;
	send_package("version", stm.ostm.m_data);
}

void CPeerNode::send_getheaders(const uint256& start_block, const uint256& stop_block)
{
	btcnet_ostream stm;
	UINT32 nVersion = 0;
	stm << UINT32(nVersion)
		<< CompactSize(1)
		<< start_block
		<< stop_block
		;
	send_package("getheaders", stm.ostm.m_data);
}

enum {
	MAX_SEND_INV_COUNT = 500
};
void CPeerNode::_send_getblockdata_n(const uint256* phashs, size_t n)
{
	assert(n <= MAX_SEND_INV_COUNT);
	std::vector<CInv> invs;
	invs.resize(n);
	for (size_t i = 0; i < n; ++i)
	{
		invs[i].type = MSG_BLOCK;
		invs[i].hash = phashs[i];
	}
	btcnet_ostream stm;
	stm << invs;
	send_package("getdata", stm.ostm.m_data);
}


void CPeerNode::send_getblockdatas(const std::vector<uint256>& hashs)
{
	int offset = 0;
	size_t size = hashs.size();
	for (; size > MAX_SEND_INV_COUNT; offset+=MAX_SEND_INV_COUNT)
	{
		_send_getblockdata_n(&hashs[offset], MAX_SEND_INV_COUNT);
		size -= MAX_SEND_INV_COUNT;
	}
	if (size>0)
	{
		_send_getblockdata_n(&hashs[offset], size);
	}
}
void CPeerNode::send_getblockdata(const uint256& hash)
{
	std::vector<uint256> hashs;
	hashs.push_back(hash);
	send_getblockdatas(hashs);
}
void CPeerNode::send_getblock(const uint256& start_block, const uint256& stop_block)
{
	btcnet_ostream stm;
	UINT32 nVersion = 0;
	stm << UINT32(nVersion)
		<< CompactSize(1)
		<< start_block
		<< stop_block
		;
	send_package("getblocks", stm.ostm.m_data);

}

void CPeerNode::start_check()
{
	async_recv();
	send_version(1, g_ClientVersion);
}

void CPeerNode::on_recv(const boost::system::error_code& ec, size_t len)
{
	if (ec || len==0)
	{
		m_sk.close();
		return;
	}
	m_data.insert(m_data.end(), m_buffer.begin(), m_buffer.begin()+len);
	async_recv();

	while(true)
	{
		btcnet_istream stm(m_data);
		if( !stm.has_buffer(sizeof(CMessageHeader)) )
		{
			break;
		}

		CMessageHeader cmd(true);
		stm >> cmd;
		if (!stm.has_buffer(cmd.nMessageSize))
		{
			break;
		}
		if (strcmp(cmd.pchCommand, "version") == 0)
		{
			shared_ptr<CVersionCmdHead> pVer(new CVersionCmdHead);
			stm >> *pVer;
			m_pDestVersion = pVer;
			m_ver_finish_tick = GetTickCount();
		}
		else if (strcmp(cmd.pchCommand, "block") == 0)
		{
			shared_ptr<CBlock> pBlock(new CBlock);
			stm >> *pBlock;
			uint256 hash = calc_block_hash(*pBlock);
			boost::mutex::scoped_lock lock(m_mutex_blocks);
			m_block_pool.push_back( pBlock );
		}
		else if (strcmp(cmd.pchCommand, "headers") == 0)
		{
			shared_ptr< std::vector<CBlock> > pHeaders(new std::vector<CBlock> );
			stm >> *pHeaders;
			if (pHeaders->size())
			{
				boost::mutex::scoped_lock lock(m_mutex_headers);
				m_headers_pool.push_back( pHeaders );
			}
		}
// 		else if (strcmp(cmd.pchCommand, "verack") == 0)
// 		{
// 		}
// 		else if (strcmp(cmd.pchCommand, "inv") == 0)
// 		{
// 			std::vector<CInv> invs;
// 			stm >> invs;
// 			for (auto it = invs.begin(); it != invs.end(); ++it)
// 			{
// 				CInv& v = *it;
// 			}
// 		}
		m_data.erase(m_data.begin(), m_data.begin() + sizeof(cmd) + cmd.nMessageSize);
	}
}

shared_ptr<CVersionCmdHead> CPeerNode::GetDestVersion()
{
	return m_pDestVersion;
}


tcp::socket& CPeerNode::GetObject()
{
	return m_sk;
}

bool CPeerNode::IsCheckTimeout()
{
	if (m_ver_finish_tick > 0)
	{
		return false;
	}
	return (GetTickCount() - m_connect_start_tick) > 60*1000;
}

size_t CPeerNode::GetDelaySpeed()
{
	if (m_ver_finish_tick > 0)
	{
		return m_ver_finish_tick - m_connect_start_tick;
	}
	return (size_t)-1;
}

void CPeerNode::async_download_headers(const uint256& last_block_hash)
{
	send_getheaders(last_block_hash);
}

void CPeerNode::async_download_blocks(const std::vector<uint256>& blocks)
{
	send_getblockdatas(blocks);
}

void CPeerNode::async_recv()
{
	m_sk.async_receive(asio::buffer(m_buffer), 
		boost::bind(&CPeerNode::on_recv, shared_from_this(), boost::asio::placeholders::error ,boost::asio::placeholders::bytes_transferred));

}

void CPeerNode::filter_modal()
{
	std::list< shared_ptr<CBlock> > blocks;
	{
		boost::mutex::scoped_lock lock(m_mutex_blocks);
		blocks = m_block_pool;
		m_block_pool.clear();
	}
	for (auto bit = blocks.begin(); bit != blocks.end(); ++bit)
	{
		shared_ptr<CBlock> p = *bit;
		uint256 hash = calc_block_hash(*p);
		pfn_OnNewBlock(hash, *p);
	}

	std::list< shared_ptr< std::vector<CBlock> > > headers;
	{
		boost::mutex::scoped_lock lock(m_mutex_headers);
		headers = m_headers_pool;
		m_headers_pool.clear();
	}
	for (auto bit = headers.begin(); bit != headers.end(); ++bit)
	{
		shared_ptr< std::vector<CBlock> > p = *bit;
		pfn_OnNewHeaderList(*p);
	}


}

void CPeerNode::modal()
{
	filter_modal();
}
//////////////////////////////////////////////////////////////////////////
void CPeerGroup::async_connect_from_profile(shared_ptr<ICoinOption> pCoinOption)
{
	for (auto dns_it = pCoinOption->DNSSeed.begin(); dns_it != pCoinOption->DNSSeed.end(); ++dns_it)
	{
		std::list<tcp::endpoint> eps = asio_query_all_endpoint(m_service, *dns_it, "");
		for (auto eit = eps.begin(); eit != eps.end(); ++eit)
		{
			tcp::endpoint ep = *eit;
			ep.port(pCoinOption->port);
			async_connect(pCoinOption, ep);
		}
	}
}

void CPeerGroup::async_connect(shared_ptr<ICoinOption> pCoinOption, boost::asio::ip::tcp::endpoint ep)
{
	shared_ptr<CPeerNode> peer(new CPeerNode(m_service, pCoinOption));
	tcp::socket& sk = peer->GetObject();
	sk.open(ep.protocol());
	sk.async_connect(ep, 
		std::bind(&CPeerGroup::on_connect, shared_from_this(), peer, std::asio::placeholders::error));
	InterlockedIncrement(&m_busy_count);
	check_state();
}

void CPeerGroup::on_connect(shared_ptr<CPeerNode> peer, const boost::system::error_code& ec)
{
	if (!ec)
	{
		try
		{
			peer->start_check();
			boost::mutex::scoped_lock lock(m_mutex);
			m_checking_list.push_back(peer);
			return;
		}
		catch(std::exception& )
		{
		}
	}
	InterlockedDecrement(&m_busy_count);
}

shared_ptr<IPeerNode> CPeerGroup::pop_idle()
{
	boost::mutex::scoped_lock lock(m_mutex);
	for (auto it = m_checking_list.begin(); it != m_checking_list.end(); )
	{
		shared_ptr<CPeerNode> p = *it;
		if (p->GetDestVersion())
		{
			m_idle.push_back(p);
			InterlockedDecrement(&m_busy_count);
			it = m_checking_list.erase(it);
			continue;
		}
		else if (p->IsCheckTimeout())
		{
			InterlockedDecrement(&m_busy_count);
			it = m_checking_list.erase(it);
			p->GetObject().close();
			continue;
		}
		++it;
	}

	shared_ptr<CPeerNode> min_peer;
	auto min_it = m_idle.end();
	for (auto it = m_idle.begin(); it != m_idle.end(); ++it)
	{
		shared_ptr<CPeerNode> p = *it;
		size_t delay = p->GetDelaySpeed();
		if (!min_peer)
		{
			min_peer = p;
			min_it = it;
		}
		else if (delay < min_peer->GetDelaySpeed())
		{
			min_peer = p;
			min_it = it;
		}
	}
	if (min_peer)
	{
		m_working.push_back(min_peer);
		m_idle.erase(min_it);
	}
	return min_peer;
}

void CPeerGroup::domodal()
{
	assert(m_busy_count > 0);
	while(true)
	{
		try
		{
			while( m_service.poll_one() )
			{

			}
			for (auto it = m_working.begin(); it != m_working.end(); ++it)
			{
				shared_ptr<CPeerNode>& p = *it;
				p->modal();
			}
			boost::this_thread::sleep(boost::posix_time::seconds(1));
		}
		catch(std::exception& e)
		{
			printf(e.what());
		}
		Sleep(1);
	}
}

void CPeerGroup::check_state()
{
	if (!m_is_start)
	{
		assert(m_busy_count > 0);
		boost::thread th(
			std::bind(&CPeerGroup::domodal, shared_from_this())
			);
		m_is_start = true;
	}
}

//////////////////////////////////////////////////////////////////////////
shared_ptr<IPeerGroup> CreatePeerGroup()
{
	shared_ptr<IPeerGroup> p(new CPeerGroup);
	return p;
}
