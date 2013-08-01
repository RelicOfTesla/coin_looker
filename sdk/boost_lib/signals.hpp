#pragma once

#pragma warning(disable:4996)

#include <boost/signals2.hpp>
namespace boost
{
	template<typename Visitor, typename R1, typename R2, typename B1>
	inline void visit_each( Visitor & v, const std::tr1::_Bind<R1, R2, B1>& t )
	{
		static_assert(false, "Don't use std::bind, Please use boost::bind!");
	}
}
namespace signals = boost::signals2;
