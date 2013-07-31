#pragma once

#include "btc_def.h"

struct ICoinProfile
{
	std::string prev_name;

	// ./src/net.cpp	strMainNetDNSSeed
	std::vector< std::string > DNSSeed;
	// ./src/main.cpp	pchMessageStart[4]
	std::array<BYTE,4> pchMessageStart;
	// ./src/protocol.h	GetDefaultPort()
	int port;
	// ./src/version.h	PROTOCOL_VERSION
	int protocol_version;
	// hashGenesisBlock
	uint256 first_blockhash;

	UINT32 first_blocktime;

	BYTE PubkeyStart;

	inline std::string GetRandomDns()
	{
		if (DNSSeed.size())
		{
			int r = rand() % DNSSeed.size();
			return DNSSeed[r];
		}
		return "";
	}
};
