#include <obexOptionalHeaders.h>
#include <exception>

//--------------------------------------------------------------------------------------------------
//	contains (public ) []
//--------------------------------------------------------------------------------------------------
std::tuple<bool,OBEXOptionalHeaders::Base::iterator> OBEXOptionalHeaders::contains(OBEXHeader::HeaderIdentifier id)
{
	if (auto itr = std::find(begin(), end(), id); itr != end())
		return std::make_tuple(true, itr);
	else
		return std::make_tuple(false, end());
}

//--------------------------------------------------------------------------------------------------
//	operator[] (public ) []
//--------------------------------------------------------------------------------------------------
OBEXHeader& OBEXOptionalHeaders::operator[](OBEXHeader::HeaderIdentifier id)
{
	if (auto[hasId, itr] = contains(id); hasId)
		return *itr;
	else
		throw std::out_of_range("Optional headers does not contain ID");
}
