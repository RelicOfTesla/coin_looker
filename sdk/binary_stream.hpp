#pragma once

#include "binary.h"
#include "stream_t.hpp"
#include <boost/noncopyable.hpp>

struct binary_istream : boost::noncopyable 
{
	binary_istream(const binary& bin) : m_read_offset(0), m_rdata(bin)
	{}

	void pop_buffer(BYTE* buf, size_t len)
	{
		if (m_read_offset+len> max_size())
		{
			throw std::out_of_range("out binary size");
		}
		memcpy(buf, &m_rdata[m_read_offset], len);
		m_read_offset += len;
	}

	bool eof()
	{
		return m_read_offset+1 > max_size();
	}
	bool has_buffer(size_t request_length)
	{
		return m_read_offset+request_length <= max_size();
	}

	size_t max_size()
	{
		return m_rdata.size();
	}

	const binary& m_rdata;
	size_t m_read_offset;
};


template<>
struct pod_archive< binary_istream > 
{
	static void invoke(binary_istream& stm, pod_buffer& dst)
	{
		stm.pop_buffer((PBYTE)dst.buffer, dst.buflen);
	}
	// 	static void invoke(binary_istream& stm, const pod_const_data& pb);
	// 	{
	// 		static_assert(0, "not support write");
	// 	}
};

struct binary_istream_dynamic : binary_istream 
{
	binary_istream_dynamic() : binary_istream(m_buffer)
	{}

	void clear()
	{
		m_buffer.clear();
		m_read_offset = 0;
	}

	void* resize_buffer(size_t n)
	{
		m_buffer.resize(n);
		return &m_buffer[0];
	}

	binary m_buffer;
};
template<>
struct pod_archive< binary_istream_dynamic >
{
	static void invoke(binary_istream_dynamic& stm, pod_buffer& dst)
	{
		stm.pop_buffer((PBYTE)dst.buffer, dst.buflen);
	}
	// 	static void invoke(binary_istream& stm, const pod_const_data& pb);
	// 	{
	// 		static_assert(0, "not support write");
	// 	}
};

struct binary_ostream : boost::noncopyable
{
	binary_ostream() : m_write_offset(0)
	{}

	void append(const BYTE* pd, size_t len)
	{
		m_data.insert(m_data.end(), pd, &pd[len]);
		m_write_offset += len;
	}

	binary m_data;
	size_t m_write_offset;
};

template<>
struct pod_archive< binary_ostream >
{
// 	static void invoke(binary_ostream& stm, pod_buffer& pb);
// 	{
// 		static_assert(0, "not support read");
// 	}
	static void invoke(binary_ostream& stm, const pod_const_data& src)
	{
		stm.append((PBYTE)src.data, src.len);
	}
};


//////////////////////////////////////////////////////////////////////////
