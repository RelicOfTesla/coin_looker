#include "stdafx.h"

#include "util.h"
#include <openssl/bn.h>
#include "bignum.h"

static const char* pszBase58 = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

std::string EncodeBase58(const unsigned char* pbegin, const unsigned char* pend)
{
	CAutoBN_CTX pctx;
	CBigNum bn58 = 58;
	CBigNum bn0 = 0;

	// Convert big endian data to little endian
	// Extra zero at the end make sure bignum will interpret as a positive number
	std::vector<unsigned char> vchTmp(pend-pbegin+1, 0);
	reverse_copy(pbegin, pend, vchTmp.begin());

	// Convert little endian data to bignum
	CBigNum bn;
	bn.setvch(vchTmp);

	// Convert bignum to std::string
	std::string str;
	// Expected size increase from base58 conversion is approximately 137%
	// use 138% to be safe
	str.reserve((pend - pbegin) * 138 / 100 + 1);
	CBigNum dv;
	CBigNum rem;
	while (bn > bn0)
	{
		if (!BN_div(&dv, &rem, &bn, &bn58, pctx))
			throw bignum_error("EncodeBase58 : BN_div failed");
		bn = dv;
		unsigned int c = rem.getulong();
		str += pszBase58[c];
	}

	// Leading zeroes encoded as base58 zeros
	for (const unsigned char* p = pbegin; p < pend && *p == 0; p++)
		str += pszBase58[0];

	// Convert little endian std::string to big endian
	reverse(str.begin(), str.end());
	return str;
}
// Decode a base58-encoded string psz into byte vector vchRet
// returns true if decoding is successful
inline bool DecodeBase58(const char* psz, std::vector<unsigned char>& vchRet)
{
	CAutoBN_CTX pctx;
	vchRet.clear();
	CBigNum bn58 = 58;
	CBigNum bn = 0;
	CBigNum bnChar;
	while (isspace(*psz))
		psz++;

	// Convert big endian string to bignum
	for (const char* p = psz; *p; p++)
	{
		const char* p1 = strchr(pszBase58, *p);
		if (p1 == NULL)
		{
			while (isspace(*p))
				p++;
			if (*p != '\0')
				return false;
			break;
		}
		bnChar.setulong(p1 - pszBase58);
		if (!BN_mul(&bn, &bn, &bn58, pctx))
			throw bignum_error("DecodeBase58 : BN_mul failed");
		bn += bnChar;
	}

	// Get bignum as little endian data
	std::vector<unsigned char> vchTmp = bn.getvch();

	// Trim off sign byte if present
	if (vchTmp.size() >= 2 && vchTmp.end()[-1] == 0 && vchTmp.end()[-2] >= 0x80)
		vchTmp.erase(vchTmp.end()-1);

	// Restore leading zeros
	int nLeadingZeros = 0;
	for (const char* p = psz; *p == pszBase58[0]; p++)
		nLeadingZeros++;
	vchRet.assign(nLeadingZeros + vchTmp.size(), 0);

	// Convert little endian data to big endian
	reverse_copy(vchTmp.begin(), vchTmp.end(), vchRet.end() - vchTmp.size());
	return true;
}

std::string base58_encode(const void* pd, size_t len)
{
	PBYTE p = (PBYTE)pd;
	return EncodeBase58(p, &p[len]);
}

binary base58_decode(const char* s)
{
	binary bin;
	DecodeBase58(s, bin);
	return bin;
}
//////////////////////////////////////////////////////////////////////////
std::string to_hex(const void* src, size_t len)
{
	PBYTE pSrc = (PBYTE)src;
	std::string buf;
	buf.resize(len*2);
	for (size_t i = 0; i < len; ++i)
	{
		sprintf(&buf[i*2], "%02x", pSrc[i]);
	}
	return buf;
}

binary from_hex(const char* src)
{
	size_t len = strlen(src);
	if (len & 1)
	{
		return binary();
	}
	binary r;
	r.reserve(len);
	char hex[3]={0};
	for (size_t i = 0; i < len; i+=2)
	{
		hex[0] = src[i];
		hex[1] = src[i+1];
		r.push_back( strtol(hex, nullptr, 16) );
	}
	return r;
}

std::string uint256_to_rstr(const uint256& src) // uint256 to reverse string
{
	const int maxsize = sizeof(src);
	char buf[maxsize*2+1];
	for (int i = 0; i < maxsize; ++i)
	{
		sprintf(&buf[i*2], "%02x", src.data[maxsize-i-1]);
	}
	return buf;
}
// reverse string to uint256
uint256 rstr_to_uint256(const std::string& s)
{
	uint256 r;
	const int maxsize = sizeof(uint256);
	assert(s.size() == maxsize*2);
	char hex[3]={0};
	for (int i = 0; i < maxsize; ++i)
	{
		hex[0] = s[i*2];
		hex[1] = s[i*2+1];
		r.data[maxsize-i-1] = static_cast<BYTE>( strtol(hex, nullptr, 16) );
	}
	return r;
}


uint256 double_sha256(const void* src, size_t len)
{
	uint256 tmp;
	SHA256((PBYTE)src, len, tmp.data);
	uint256 r;
	SHA256(tmp.data, sizeof(tmp), r.data);
	return r;
}


std::string GetAppDir()
{
	std::string s;
	s.resize(MAX_PATH);
	s.resize( GetModuleFileNameA(0, (char*)s.c_str(), MAX_PATH) );
	size_t pos = s.rfind('\\');
	if (pos != std::string::npos)
	{
		s.erase( pos+1, std::string::npos );
	}
	return s;
}

std::string GetAppFile(const std::string& name)
{
	return GetAppDir() + name;
}
