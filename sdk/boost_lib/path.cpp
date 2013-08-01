#include "stdafx.h"
#include "path.h"

//#define USE_BOOST_DIRECTORY_ITERATOR 1

#if USE_BOOST_DIRECTORY_ITERATOR
#include <boost/filesystem/operations.hpp>
#endif

bf::path GetAppPath()
{
	TCHAR path[MAX_PATH];
	GetModuleFileName(0, path, MAX_PATH);
	return path;
}

bf::path GetAppDir()
{
	bf::path r = GetAppPath();
	r.remove_filename();
	return r;
}

bf::path GetAppFile(const bf::path& file)
{
	return GetAppDir() / file;
}

void EnumFile(const bf::path& dir_path, bool childs, const std::function<bool(const bf::path&)>& pfn)
{
#if USE_BOOST_DIRECTORY_ITERATOR
	for (bf::directory_iterator it = bf::directory_iterator(dir_path), it_end;
		it != it_end; ++it)
	{
		if( bf::is_regular(it->status()) )
		{
			if( !pfn(it->path()) )
			{
				break;
			}
		}
		else if (childs)
		{
			if (bf::is_directory(it->status()))
			{
				EnumFile(it->path(), childs, pfn);
			}
		}
	}
#else
	EnumWinFile(dir_path, childs, pfn);
#endif
}

void EnumWinFile(const bf::path& dir_path, bool childs, const std::function<bool(const bf::path&)>& pfn)
{
	WIN32_FIND_DATA FindInfo = {0};
	HANDLE hFind = FindFirstFile( GetTPath(dir_path / TEXT("*")).c_str(), &FindInfo);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do 
		{
			if (FindInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && childs)
			{
				switch (_tcslen(FindInfo.cFileName))
				{
				case 1:
					if (FindInfo.cFileName[0] == TEXT('.'))
					{
						break;
					}
				case 2:
					if (FindInfo.cFileName[0] == TEXT('.') && FindInfo.cFileName[1] == TEXT('.'))
					{
						break;
					}
				default:
					EnumWinFile(dir_path / FindInfo.cFileName, childs, pfn);
				}
			}
			else if ((int)FindInfo.dwFileAttributes != -1)
			{
				if ( !pfn( dir_path / FindInfo.cFileName ) )
				{
					break;
				}
			}
		} while (FindNextFile(hFind, &FindInfo));
		FindClose(hFind);
	}
}