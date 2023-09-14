#ifndef PCL_FAST_BUFFER_BASED_IMAGE_ITERATOR
#define PCL_FAST_BUFFER_BASED_IMAGE_ITERATOR

#include <pcl/image.h>

namespace pcl
{
	namespace iterator
	{

		class FastBufferBasedImageIterator
		{
		public:
			FastBufferBasedImageIterator() {}
			FastBufferBasedImageIterator(const ImageBase::ConstantPointer& img) 
			{
				setImage(img);
			}

			void setImage(const ImageBase::ConstantPointer& img)
			{
				m_EndVal = img->getOffsetTable()[3];
			}

			inline void begin() const
			{
				m_Index = 0;
			}

			inline void next() const
			{
				++m_Index;
			}

			inline bool end() const
			{
				return m_Index>=m_EndVal;
			}

			inline long getIndex() const
			{
				return m_Index;
			}
			inline operator long() const
			{
				return m_Index;
			}

		protected:
			mutable long m_Index;
			long m_EndVal;
		};


		class ReverseFastBufferBasedImageIterator
		{
		public:
			ReverseFastBufferBasedImageIterator() {}
			ReverseFastBufferBasedImageIterator(const ImageBase::ConstantPointer& img) 
			{
				setImage(img);
			}

			void setImage(const ImageBase::ConstantPointer& img)
			{
				m_StartVal = img->getOffsetTable()[3];
				--m_StartVal;
			}

			inline void begin() const
			{
				m_Index = m_StartVal;
			}

			inline void next() const
			{
				--m_Index;
			}

			inline bool end() const
			{
				return m_Index<0;
			}

			inline long getIndex() const
			{
				return m_Index;
			}
			inline operator long() const
			{
				return m_Index;
			}

		protected:
			mutable long m_Index;
			long m_StartVal;
		};

	}
}

#endif