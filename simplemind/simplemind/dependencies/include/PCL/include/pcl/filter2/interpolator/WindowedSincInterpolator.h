#pragma once

#include <pcl/image.h>
#include <pcl/constant.h>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits.hpp>
#include <math.h>

namespace pcl
{
	namespace filter2
	{

		//Adapted from itkWindowedSincInterpolateImageFunction.h
		template< unsigned int VRadius, typename TInput = double, typename TOutput = double >
		class LanczosWindowFunction
		{
		public:
			static const unsigned int Radius = VRadius;
		
			inline TOutput operator()(const TInput & A) const
			{
				if ( A == 0.0 ) { return (TOutput)1.0; }
				double z = m_Factor * A;
				return (TOutput)( std::sin(z) / z );
			}

		private:
			/** Equal to \f$ \frac{\pi}{m} \f$ */
			static const double m_Factor;
		};
		template< unsigned int VRadius, typename TInput, typename TOutput >
		const double LanczosWindowFunction<VRadius,TInput,TOutput>::m_Factor = pcl::PI / VRadius;


	
		template <class BdType, class WindowFunction>
		class WindowedSincInterpolator
		{
		public:
			typedef BdType BoundaryType;
			typedef typename BoundaryType::ImageType ImageType;

			WindowedSincInterpolator() {}
			WindowedSincInterpolator(const BoundaryType& bound)
			{
				setBoundary(bound);
			}
			WindowedSincInterpolator(const WindowedSincInterpolator& obj) 
			{
				*this = obj;
			}
			
			WindowedSincInterpolator& operator=(const WindowedSincInterpolator& obj)
			{
				m_Boundary = obj.m_Boundary;
				m_SafeRegion = obj.m_SafeRegion;
				for (int i=0; i<TableSize; ++i) {
					obj.m_PointOffsetTable[i] = m_PointOffsetTable[i];
					obj.m_WeightOffsetTable[i] = m_WeightOffsetTable[i];
					obj.m_OffsetTable[i] = m_OffsetTable[i];
				}
				return *this;
			}
			
			typename ImageType::ConstantPointer getImage() const
			{
				return m_Boundary.getImage();
			}
			
			const BoundaryType& getBoundary() const
			{
				return m_Boundary;
			}

			template <class PointType>
			typename boost::enable_if<boost::is_floating_point<typename PointType::ValueType>, double>::type get(const PointType& p) const
			{
				Point3D<int> base(floor(p.x()), floor(p.y()), floor(p.z()));
				Point3D<double> distance(p.x()-base.x(), p.y()-base.y(), p.z()-base.z());
				
				Point3D<double> x_weight[WindowSize-1];
				for (int d=0; d<3; ++d) {
					if (distance[d]==0.0) {
						for (int i=0; i<WindowSize-1; ++i) x_weight[i][d] = 0;
						x_weight[WindowFunction::Radius-1][d] = 1;
					} else {
						double x = distance[d] + (WindowFunction::Radius-1);
						for (int i=0; i<WindowSize-1; ++i) {
							x_weight[i][d] = m_WindowFunction(x) * Sinc(x);
							x -= 1.0;
						}
					}
				}

				/*if (m_SafeRegion.contain(base)) {
					auto x_index = m_Boundary.getImage()->localToIndex(base);
					for (int i = 0; i < TableSize; ++i) {
						std::cout << i << ": " << base + m_PointOffsetTable[i] << " " << m_Boundary.getImage()->toPoint(x_index + m_OffsetTable[i]) << std::endl;
						for (int d = 0; d < 3; ++d) {
							double x = distance[d] - m_PointOffsetTable[i][d];
							std::cout << "\t" << d << ": " << x_weight[m_WeightOffsetTable[i][d]][d] << " " <<
								m_WindowFunction(x) * Sinc(x) << std::endl;
						}
					}
					exit(1);
				}*/
				
				double x_pixel_value = 0.0;
				auto x_index = m_Boundary.getImage()->localToIndex(base);
				if (m_SafeRegion.contain(base)) {
					for (int i=0; i<TableSize; ++i) {
						double x_val = m_Boundary.getImage()->get(x_index + m_OffsetTable[i]);
						for (int d=0; d<3; ++d) x_val *= x_weight[m_WeightOffsetTable[i][d]][d];
						x_pixel_value += x_val;
					}
				} else {
					for (int i=0; i<TableSize; ++i) {
						double x_val = m_Boundary.get(base+m_PointOffsetTable[i], x_index + m_OffsetTable[i]);
						for (int d=0; d<3; ++d) x_val *= x_weight[m_WeightOffsetTable[i][d]][d];
						x_pixel_value += x_val;
					}
				}
				return x_pixel_value;
			}
			
			template <class PointType>
			typename boost::disable_if<boost::is_floating_point<typename PointType::ValueType>, double>::type get(const PointType& p) const
			{
				Point3D<int> temp; temp.assign(p);
				return m_Boundary.get(p);
			}
			
			inline typename ImageType::IoValueType get(const Point3D<int>& p, long index) const
			{
				return m_Boundary.get(p, index);
			}
			
			template <class Iterator>
			inline typename boost::enable_if<getPoint_exists<Iterator>, typename ImageType::IoValueType>::type get(const Iterator& iter) const
			{
				return m_Boundary.get(iter);
			}

		protected:
			static const unsigned int WindowSize = (WindowFunction::Radius << 1)+1;
			static const unsigned int TableSize = (WindowSize-1)*(WindowSize-1)*(WindowSize-1);
			BoundaryType m_Boundary;
			WindowFunction m_WindowFunction;
			Region3D<int> m_SafeRegion;
			pcl::Point3D<int> m_PointOffsetTable[TableSize], m_WeightOffsetTable[TableSize];
			unsigned int m_OffsetTable[TableSize];
			
			void setBoundary(const BoundaryType& bound)
			{
				m_Boundary = bound;
				m_SafeRegion = m_Boundary.getImage()->getRegion();
				auto image_offset_table = m_Boundary.getImage()->getOffsetTable();
				m_SafeRegion.getMaxPoint() -= WindowFunction::Radius;
				m_SafeRegion.getMinPoint() += (WindowFunction::Radius-1);
				if (m_SafeRegion.empty()) m_SafeRegion.reset();
				int ind = 0, radius = WindowFunction::Radius;
				for (int z=-radius+1; z<=radius; ++z) {
					for (int y=-radius+1; y<=radius; ++y) {
						for (int x=-radius+1; x<=radius; ++x) {
							m_PointOffsetTable[ind].set(x,y,z);
							m_WeightOffsetTable[ind].set(
								x + (radius-1),
								y + (radius-1),
								z + (radius-1)
							);
							m_OffsetTable[ind] = image_offset_table[0]*x + image_offset_table[1]*y + image_offset_table[2]*z;
							++ind;
						}
					}
				}
				//std::cout << ind << " " << TableSize << std::endl;
			}
			
			inline double Sinc(double x) const
			{
				double px = pcl::PI * x;
				return ( x == 0.0 ) ? 1.0 : std::sin(px) / px;
			}
		};



		template <class BdType, unsigned int Dim, class WindowFunction>
		class WindowedSincInterpolator1D
		{
		public:
			typedef BdType BoundaryType;
			typedef typename BoundaryType::ImageType ImageType;

			WindowedSincInterpolator1D() {}
			WindowedSincInterpolator1D(const BoundaryType& bound)
			{
				setBoundary(bound);
			}
			WindowedSincInterpolator1D(const WindowedSincInterpolator1D& obj)
			{
				*this = obj;
			}
			
			WindowedSincInterpolator1D& operator=(const WindowedSincInterpolator1D& obj)
			{
				m_Boundary = obj.m_Boundary;
				m_SafeRegion = obj.m_SafeRegion;
				for (int i=0; i<TableSize; ++i) {
					obj.m_PointOffsetTable[i] = m_PointOffsetTable[i];
					obj.m_WeightOffsetTable[i] = m_WeightOffsetTable[i];
					obj.m_OffsetTable[i] = m_OffsetTable[i];
				}
				return *this;
			}
			
			typename ImageType::ConstantPointer getImage() const
			{
				return m_Boundary.getImage();
			}
			
			const BoundaryType& getBoundary() const
			{
				return m_Boundary;
			}

			template <class PointType>
			typename boost::enable_if<boost::is_floating_point<typename PointType::ValueType>, double>::type get(const PointType& p) const
			{
				Point3D<int> base(floor(p.x()), floor(p.y()), floor(p.z()));
				double distance = p[Dim] - base[Dim];
				if (distance == 0.0) return m_Boundary.get(p);

				double x = distance + (WindowFunction::Radius - 1);
				double x_pixel_value = 0.0;
				auto x_index = m_Boundary.getImage()->localToIndex(base);
				if (m_SafeRegion.contain(base)) {
					for (int i = 0; i<TableSize; ++i) {
						double x_val = m_Boundary.getImage()->get(x_index + m_OffsetTable[i]);
						x_val *= (m_WindowFunction(x) * Sinc(x));
						x_pixel_value += x_val;
						x -= 1.0;
					}
				} else {
					Point3D<int> x_pos = base;
					for (int i = 0; i<TableSize; ++i) {
						x_pos[Dim] = base[Dim] + m_PointOffsetTable[i];
						double x_val = m_Boundary.get(x_pos, x_index + m_OffsetTable[i]);
						x_val *= (m_WindowFunction(x) * Sinc(x));
						x_pixel_value += x_val;
						x -= 1.0;
					}
				}

				return x_pixel_value;
			}
			
			template <class PointType>
			typename boost::disable_if<boost::is_floating_point<typename PointType::ValueType>, double>::type get(const PointType& p) const
			{
				Point3D<int> temp; temp.assign(p);
				return m_Boundary.get(p);
			}
			
			inline typename ImageType::IoValueType get(const Point3D<int>& p, long index) const
			{
				return m_Boundary.get(p, index);
			}
			
			template <class Iterator>
			inline typename boost::enable_if<getPoint_exists<Iterator>, typename ImageType::IoValueType>::type get(const Iterator& iter) const
			{
				return m_Boundary.get(iter);
			}

		protected:
			static const unsigned int WindowSize = (WindowFunction::Radius << 1)+1;
			static const unsigned int TableSize = WindowSize-1;
			BoundaryType m_Boundary;
			WindowFunction m_WindowFunction;
			Region3D<int> m_SafeRegion;
			int m_PointOffsetTable[TableSize], m_WeightOffsetTable[TableSize];
			unsigned int m_OffsetTable[TableSize];
			
			void setBoundary(const BoundaryType& bound)
			{
				m_Boundary = bound;
				m_SafeRegion = m_Boundary.getImage()->getRegion();
				auto image_offset_table = m_Boundary.getImage()->getOffsetTable();
				m_SafeRegion.getMaxPoint()[Dim] -= WindowFunction::Radius;
				m_SafeRegion.getMinPoint()[Dim] += (WindowFunction::Radius-1);
				if (m_SafeRegion.empty()) m_SafeRegion.reset();
				int ind = 0, radius = WindowFunction::Radius;
				for (int i=-radius+1; i<=radius; ++i) {
					m_PointOffsetTable[ind] = i;
					m_WeightOffsetTable[ind] = i + (radius - 1);
					m_OffsetTable[ind] = image_offset_table[Dim]*i;
					++ind;
				}
			}
			
			inline double Sinc(double x) const
			{
				double px = pcl::PI * x;
				return ( x == 0.0 ) ? 1.0 : std::sin(px) / px;
			}
		};

	}
}
