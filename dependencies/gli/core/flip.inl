namespace gli{
namespace detail
{
	inline void flip(image ImageDst, image ImageSrc, size_t BlockSize)
	{
		size_t const LineSize = BlockSize * ImageDst.extent().x;

		for(int y = 0; y < ImageDst.extent().y; ++y)
		{
			size_t OffsetDst = LineSize * y;
			size_t OffsetSrc = ImageSrc.size() - (LineSize * (y + 1));

			memcpy(
				ImageDst.data<glm::byte>() + OffsetDst,
				ImageSrc.data<glm::byte>() + OffsetSrc,
				LineSize);
		}
	}

}//namespace detail

/*
template <>
inline image flip(image const & Image)
{

}
*/

template <>
inline texture2d flip(texture2d const& Texture)
{
	GLI_ASSERT(!gli::is_compressed(Texture.format()));

	texture2d Flip(Texture.format(), Texture.extent(), Texture.levels());

	texture2d::size_type const BlockSize = block_size(Texture.format());

	for(texture2d::size_type Level = 0; Level < Flip.levels(); ++Level)
		detail::flip(Flip[Level], Texture[Level], BlockSize);

	return Flip;
}

template <>
inline texture2d_array flip(texture2d_array const& Texture)
{
	GLI_ASSERT(!gli::is_compressed(Texture.format()));

	texture2d_array Flip(Texture.format(), Texture.extent(), Texture.layers(), Texture.levels());

	texture2d_array::size_type const BlockSize = block_size(Texture.format());

	for(texture2d_array::size_type Layer = 0; Layer < Flip.layers(); ++Layer)
	for(texture2d_array::size_type Level = 0; Level < Flip.levels(); ++Level)
		detail::flip(Flip[Layer][Level], Texture[Layer][Level], BlockSize);

	return Flip;
}

}//namespace gli
