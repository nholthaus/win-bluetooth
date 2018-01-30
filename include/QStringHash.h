//--------------------------------------------------------------------------------------------------
// 
//	QSTRING HASH
//
//--------------------------------------------------------------------------------------------------
//
// The MIT License (MIT)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software 
// and associated documentation files (the "Software"), to deal in the Software without 
// restriction, including without limitation the rights to use, copy, modify, merge, publish, 
// distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the 
// Software is furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all copies or 
// substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING 
// BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//--------------------------------------------------------------------------------------------------
//
// Copyright (c) 2017 Nic Holthaus
// 
//--------------------------------------------------------------------------------------------------
//
// ATTRIBUTION:
// http://www.cse.yorku.ca/~oz/hash.html
// http://en.cppreference.com/w/cpp/utility/hash
//
//--------------------------------------------------------------------------------------------------
//
/// @file	QStringHash.h
/// @brief	hash function for std::unordered_set and std::unordered_map
//
//--------------------------------------------------------------------------------------------------

#pragma once
#ifndef QStringHash_h__
#define QStringHash_h__

//-------------------------
//	INCLUDES
//-------------------------

#include <QString>
#include <unordered_map>

//------------------------------
//	HASH FUNCTION
//------------------------------
namespace std
{
	template<> struct hash<QString>
	{
		std::size_t operator()(const QString& s) const noexcept
		{
			const QChar* str = s.data();
			std::size_t hash = 5381;
			
			for (int i = 0; i < s.size(); ++i)
				hash = ((hash << 5) + hash) + ((str->row() << 8) | (str++)->cell());

			return hash;
		}
	};
}
#endif // QStringHash_h__