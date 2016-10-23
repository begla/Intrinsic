/// @brief Include to use filter enum, to select filtering methods.
/// @file gli/filter.hpp

#pragma once

namespace gli{
namespace detail
{
	enum dimension
	{
		DIMENSION_1D,
		DIMENSION_2D,
		DIMENSION_3D
	};
}//namespace detail

	/// Texture filtring modes
	enum filter
	{
		FILTER_NONE = 0,
		FILTER_NEAREST, FILTER_FIRST = FILTER_NEAREST,
		FILTER_LINEAR, FILTER_LAST = FILTER_LINEAR
	};

	enum
	{
		FILTER_COUNT = FILTER_LAST - FILTER_FIRST + 1,
		FILTER_INVALID = -1
	};
}//namespace gli

#include "./core/filter.inl"
