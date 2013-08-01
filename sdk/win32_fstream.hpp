#pragma once
#include "stream_t.hpp"
#include <boost/noncopyable.hpp>

struct win32_fstream : boost::noncopyable
{
	win32_fstream()
	{
		m_hFile = INVALID_HANDLE_VALUE;
	}
	win32_fstream(const std::string& filepath, DWORD OpenType)
	{
		BOOL isopen = Open(filepath, OpenType);
		assert( isopen || OpenType==OPEN_EXISTING );
	}
	~win32_fstream()
	{
		CloseHandle(m_hFile);
	}

	BOOL Open(const std::string& filepath, DWORD OpenType)
	{
		m_hFile = CreateFileA(filepath.c_str(), GENERIC_ALL, FILE_SHARE_READ, 0, OpenType, 0, 0);
		return m_hFile != INVALID_HANDLE_VALUE;
	}

	BOOL IsOpen()const
	{
		return m_hFile != INVALID_HANDLE_VALUE;
	}
	void Read(void* pv, size_t len)
	{
		assert(IsOpen());
		DWORD bytes = 0;
		ReadFile(m_hFile, pv, len, &bytes, 0);
	}
	void Write(const void* pv, size_t len)
	{
		assert(IsOpen());
		DWORD bytes = 0;
		WriteFile(m_hFile, pv, len, &bytes, 0);
	}
	size_t GetFileSize()
	{
		size_t n = ::GetFileSize(m_hFile, 0);
		if (n == (size_t)-1)
		{
			n = 0;
		}
		return n;
	}
	void SetFilePointer( DWORD dwMoveMethod, DWORD lDistanceToMove )
	{
		::SetFilePointer(m_hFile, lDistanceToMove, 0, dwMoveMethod);
	}
	typedef struct win32fstream_archive ArchiveType;
protected:
	HANDLE m_hFile;
};

template<>
struct pod_archive<win32_fstream>
{
	static void invoke(win32_fstream& stm, const pod_const_data& src)
	{
		stm.Write(src.data, src.len);
	}
	static void invoke(win32_fstream& stm, pod_buffer& dst)
	{
		stm.Read(dst.buffer, dst.buflen);
	}
};
