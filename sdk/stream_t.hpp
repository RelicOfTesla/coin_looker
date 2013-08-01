#pragma once

template<typename Stream>
struct pod_archive;

template<typename Stream, typename T>
struct serialize_t;

// template<>
// struct pod_archive< Stream >
// {
// 	static void invoke(Stream& stm, pod_buffer& pb);
// 	static void invoke(Stream& stm, const pod_const_data& pcd);
// };

// template<typename Stream>
// struct serialize_t<Stream, ctest>
// {
// 	template<typename Archive>
// 	static void invoke(Archive& ar, ctest&);
// };

namespace detail
{
	template<typename Stream>
	struct basic_archive_t
	{
		basic_archive_t(Stream& stm) : m_stm(stm)
		{}

		inline Stream& get_stream()
		{
			return m_stm;
		}
		Stream& m_stm;
	private:
		basic_archive_t(const basic_archive_t&){}
		void operator =(const basic_archive_t&){}
	};


	template<typename Stream>
	struct iarchive_t : basic_archive_t<Stream>
	{
		enum {
			is_read = 1, is_load = 1,
			is_write = 0, is_save = 0,
		};

		iarchive_t(Stream& stm) : basic_archive_t(stm)
		{}

		template<typename T>
		friend inline iarchive_t<Stream>& operator & (iarchive_t<Stream>& stm, T& data)
		{
			stm >> data;
			return stm;
		};

		template<typename T>
		friend inline iarchive_t<Stream>& operator >> (iarchive_t<Stream>& stm, T& data)
		{
			serialize_t<Stream, T>::invoke(stm, data);
			return stm;
		};

	};

	template<typename Stream>
	struct oarchive_t : basic_archive_t<Stream>
	{
		enum {
			is_read = 0, is_load = 0,
			is_write = 1, is_save = 1,
		};

		oarchive_t(Stream& stm) : basic_archive_t(stm)
		{}

		template<typename T>
		friend inline oarchive_t<Stream>&  operator & (oarchive_t<Stream>& stm, const T& data)
		{
			stm << data;
			return stm;
		}

		template<typename T>
		friend inline oarchive_t<Stream>& operator << (oarchive_t<Stream>& stm, const T& data)
		{
			serialize_t<Stream, T>::invoke(stm, *(T*)&data);
			return stm;
		}
	};

	struct pod_const_data
	{
		pod_const_data(const void* pv, size_t _len) : data(pv), len(_len)
		{}

		const void* data;
		size_t len;
	};

	struct pod_buffer
	{
		pod_buffer(void* pv, size_t _len) : buffer(pv), buflen(_len)
		{}

		void* buffer;
		size_t buflen;
	};


	template<typename Stream, typename T>
	struct serialize_load_save
	{
		struct call_read 
		{
			template<typename Archive>
			static inline void invoke(Archive& ar, T& v)
			{
				serialize_t<Stream,T>::load(ar, v);
			}
		};
		struct call_write
		{
			template<typename Archive>
			static inline void invoke(Archive& ar, T& v)
			{
				serialize_t<Stream,T>::save(ar, v);
			}
		};

		template<typename Archive>
		static inline void invoke(Archive& ar, T& v)
		{
			typedef typename std::conditional<Archive::is_read, call_read, call_write>::type caller;
			caller::invoke(ar, v);
		}
	};
};

typedef detail::pod_buffer		pod_buffer;
typedef detail::pod_const_data	pod_const_data;

using detail::serialize_load_save;

template<typename Stream>
struct serialize_t<Stream, pod_buffer>
{
	template<typename Archive>
	static inline void invoke(Archive& ar, pod_buffer& v)
	{
		pod_archive<Stream>::invoke(ar.get_stream(), v);
	}
};

template<typename Stream>
struct serialize_t<Stream, pod_const_data>
{
	template<typename Archive>
	static inline void invoke(Archive& ar, pod_const_data& v)
	{
		pod_archive<Stream>::invoke(ar.get_stream(), v);
	}
};


template<typename Stream, typename T>
struct serialize_t
{
	template<typename Archive>
	static inline void invoke(Archive& ar, T& v)
	{
		static_assert( std::is_pod<T>::value, "only support pod type. please define template<> serialize_t<T>" );
		static_assert( !std::is_pointer<T>::value, "not support pointer" );
		typedef typename std::conditional<Archive::is_read, pod_buffer, pod_const_data>::type pod_type;
		pod_type pt(&v,sizeof(T));
#if !_DEBUG
		ar & pt;
#else
		serialize_t<Stream, pod_type>::invoke(ar, pt);
#endif
	}

};

template<typename Stream, typename T>
struct serialize_t<Stream, T*>;
template<typename Stream, typename T>
struct serialize_t<Stream, const T*>;

template<typename Stream, int n>
struct serialize_t<Stream, char[n]>;
template<typename Stream, int n>
struct serialize_t<Stream, wchar_t[n]>;

template<typename Stream, int n>
struct serialize_t<Stream, const char[n]>;
template<typename Stream, int n>
struct serialize_t<Stream, const wchar_t[n]>;

namespace
{
	template<typename Stream, typename T>
	inline Stream& operator << (Stream& stm, const T& v)
	{
		typedef detail::oarchive_t<Stream> Archive;

		static_cast< void(*)(Archive&,T&) >( ::serialize_t<Stream,T>::invoke );
		static_cast< void(*)(Stream&,const pod_const_data&) >( ::pod_archive<Stream>::invoke );

		Archive ar(stm);
		ar << v;
		return stm;
	}

	template<typename Stream, typename T>
	inline Stream& operator >> (Stream& stm, T& v)
	{
		typedef detail::iarchive_t<Stream> Archive;

		static_cast< void(*)(Archive&,T&) >( ::serialize_t<Stream,T>::invoke );
		static_cast< void(*)(Stream&,pod_buffer&) >( ::pod_archive<Stream>::invoke );

		Archive ar(stm);
		ar>> v;
		return stm;
	}
};

