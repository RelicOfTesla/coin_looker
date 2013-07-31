#pragma once
#include "btc_protocol.h"

#include <sdk/binary_stream.hpp>

struct btcnet_istream
{
	btcnet_istream(const binary& bin) : istm(bin)
	{}

	bool has_buffer(size_t request_length)
	{
		return istm.has_buffer(request_length);
	}

	binary_istream istm;
};
struct btcnet_ostream
{
	binary_ostream ostm;
};

template<>
struct pod_archive<btcnet_istream> 
{
	static void invoke(btcnet_istream& stm, pod_buffer& dst)
	{
		stm.istm >> dst; 
	}
};


template<>
struct pod_archive<btcnet_ostream> 
{
	static void invoke(btcnet_ostream& stm, const pod_const_data& src)
	{
		stm.ostm << src;
	}

};

template<typename Stream>
struct serialize_t<Stream, CMessageHeader>
{
	template<typename Archive>
	static void invoke(Archive& ar, CMessageHeader& v)
	{
		typedef std::conditional<Archive::is_read, pod_buffer, pod_const_data>::type pod_type;
		ar & pod_type(v.pchMessageStart, sizeof(v.pchMessageStart));
		ar & pod_type(v.pchCommand, sizeof(v.pchCommand));
		ar & v.nMessageSize;
		ar & v.nChecksum;
	}
};


template<typename Stream>
struct serialize_t<Stream,CompactSize> : serialize_load_save<Stream,CompactSize>
{
	template<typename Archive>
	static void save(Archive& ar, const CompactSize& v)
	{
		const UINT64& n = v.n;
		if (n < 253)
		{
			ar << static_cast<UINT8>(n);
		}
		else if (n < (UINT16)-1)
		{
			ar << static_cast<UINT8>(253);
			ar << static_cast<UINT16>(n);
		}
		else if (n < (UINT32)-1)
		{
			ar << static_cast<UINT8>(254);
			ar << static_cast<UINT32>(n);
		}
		else
		{
			ar << static_cast<UINT8>(255);
			ar << static_cast<UINT64>(n);
		}
	}

	template<typename Archive>
	static void load(Archive& stm, CompactSize& v)
	{
		v.n = 0;
		BYTE hdr = 0;
		stm >> hdr;
		switch (hdr)
		{
		case 253:
			{
				UINT16 tmp = 0;
				stm >> tmp;
				v.n = tmp;
			}
			break;
		case 254:
			{
				UINT32 tmp = 0;
				stm >> tmp;
				v.n = tmp;
			}
			break;
		case 255:
			{
				UINT64 tmp = 0;
				stm >> tmp;
				v.n = tmp;
			}
			break;
		default:
			{
				v.n = hdr;
			}
			break;
		}
	}
};


template<typename Stream>
struct serialize_t<Stream, std::string> : serialize_load_save<Stream,std::string>
{
	template<typename Archive>
	static void save(Archive& ar, const std::string& v)
	{
		assert( v.size() <= 0xffff );
		ar << CompactSize(v.size());
		ar << pod_const_data(v.c_str(), v.size());
	}

	template<typename Archive>
	static void load(Archive& ar, std::string& v)
	{
		CompactSize nsize;
		ar >> nsize;
		assert( nsize.size() <= 0xffff );
		v.resize(nsize.size());
		ar >> pod_buffer((void*)v.c_str(), v.size());
	}
};


template<typename Stream, typename T>
struct base_serialize_t : serialize_load_save<Stream,std::vector<T> >
{
	template<typename Archive>
	static void load(Archive& ar, std::vector<T>& v)
	{
		CompactSize nsize;
		ar >> nsize;
		assert( nsize.size() <= 0xffff );
		v.resize(nsize.size());
		for (std::vector<T>::iterator it = v.begin(); it != v.end(); ++it)
		{
			T& r = *it;
			ar >> r;
		}
	}

	template<typename Archive>
	static void save(Archive& ar, const std::vector<T>& v)
	{
		assert( v.size() <= 0xffff );
		ar << CompactSize(v.size());
		for (std::vector<T>::const_iterator it = v.begin(); it != v.end(); ++it)
		{
			const T& r = *it;
			ar << r;
		}
	}
};

template<typename T>
struct serialize_t<btcnet_ostream, std::vector<T> >  : base_serialize_t<btcnet_ostream, T>
{
protected:
	template<typename Archive>
	static void load(Archive& ar, const std::vector<T>& v);
};

template<typename T>
struct serialize_t<btcnet_istream, std::vector<T> >  : base_serialize_t<btcnet_istream, T>
{
protected:
	template<typename Archive>
	static void save(Archive& ar, const std::vector<T>& v);
};

template<typename T>
struct serialize_t<struct win32_fstream, std::vector<T> >  : base_serialize_t<win32_fstream, T>
{
};

template<typename Stream>
struct serialize_t<Stream, CBlock>
{
	template<typename Archive>
	static void invoke(Archive& ar, CBlock& v)
	{
		ar & *static_cast<CBlockHeader*>( &v );
		ar & v.vtx;
	}
};

template<typename Stream>
struct serialize_t<Stream, CTransaction>
{
	template<typename Archive>
	static void invoke(Archive& ar, CTransaction& v)
	{
		ar & v.nVersion;
		ar & v.vin;
		ar & v.vout;
		ar & v.nLockTime;
	}
};

template<typename Stream>
struct serialize_t<Stream, CTxIn>
{
	template<typename Archive>
	static void invoke(Archive& ar, CTxIn& v)
	{
		ar & v.prevout;
		ar & v.scriptSig;
		ar & v.nSequence;
	}
};

template<typename Stream>
struct serialize_t<Stream, CTxOut>
{
	template<typename Archive>
	static void invoke(Archive& ar, CTxOut& v)
	{
		ar & v.nValue;
		ar & v.scriptPubKey;
	}
};

template<typename Stream>
struct serialize_t<Stream, CScript>
{
	template<typename Archive>
	static void invoke(Archive& ar, CScript& v)
	{
		ar & v.data;
	}
};

template<typename Stream>
struct serialize_t<Stream, CVersionCmdHead>
{
	template<typename Archive>
	static void invoke(Archive& ar, CVersionCmdHead& v)
	{
		ar & v.ProtocolVersion;
		ar & v.nService;
		ar & v.AdjustTime;
		ar & v.remote;
		ar & v.local;
		ar & v.nNonce;
		ar & v.ClientVersion;
		ar & v.nBestHeight;
	}
};

//////////////////////////////////////////////////////////////////////////
