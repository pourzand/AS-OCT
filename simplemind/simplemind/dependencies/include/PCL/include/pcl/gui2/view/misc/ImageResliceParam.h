#ifndef PCL_IMAGE_RESLICE_PARAM
#define PCL_IMAGE_RESLICE_PARAM

#include <pcl/gui2/data/ImageData.h>
#include <pcl/vtk/VtkModificationTracker.h>
#include <boost/unordered_set.hpp>

namespace pcl
{
	namespace gui 
	{

		class ImageResliceParam: public ModifiableObject
		{
		public:
			typedef ImageResliceParam Self;
			typedef boost::shared_ptr<Self> Pointer;

			enum AxesType {
				AXIAL,
				CORONAL,
				SAGITTAL
			};

			static Pointer New()
			{
				return Pointer(new ImageResliceParam);
			}

			void add(const ImageDataBase::Pointer& image, bool init)
			{
				if (m_Region.empty()) init = true;
				auto dummy = image->getDummyImage();
				if (init) {
					m_CornerPoints.clear();
					m_Origin = dummy->getOrigin();
					m_Region = dummy->getPhysicalRegion();
					m_RegionSpacing = dummy->getSpacing();
				} else {
					m_Region.add(dummy->getPhysicalRegion());
					m_RegionSpacing.min(dummy->getSpacing());
				}
				auto corner_points = dummy->getRegion().getCornerPoints();
				pcl_ForEach(corner_points, item) m_CornerPoints.insert(dummy->toPhysicalCoordinate(*item));
				update(true);
			}

			void setAxesTo(AxesType type)
			{
				switch(type) {
				case AXIAL:
					m_Axes->SetElement(0,0, 1); m_Axes->SetElement(0,1, 0); m_Axes->SetElement(0,2, 0);
					m_Axes->SetElement(1,0, 0); m_Axes->SetElement(1,1, -1); m_Axes->SetElement(1,2, 0);
					m_Axes->SetElement(2,0, 0); m_Axes->SetElement(2,1, 0); m_Axes->SetElement(2,2, 1);
					break;
				case CORONAL:
					m_Axes->SetElement(0,0, 1); m_Axes->SetElement(0,1, 0); m_Axes->SetElement(0,2, 0);
					m_Axes->SetElement(1,0, 0); m_Axes->SetElement(1,1, 0); m_Axes->SetElement(1,2, 1);
					m_Axes->SetElement(2,0, 0); m_Axes->SetElement(2,1, 1); m_Axes->SetElement(2,2, 0);
					break;
				case SAGITTAL:
					m_Axes->SetElement(0,0, 0); m_Axes->SetElement(0,1, 0); m_Axes->SetElement(0,2, 1);
					m_Axes->SetElement(1,0, 1); m_Axes->SetElement(1,1, 0); m_Axes->SetElement(1,2, 0);
					m_Axes->SetElement(2,0, 0); m_Axes->SetElement(2,1, 1); m_Axes->SetElement(2,2, 0);
					break;
				}
				update();
			}

			/******************************** Get methods ********************************/

			const double* getSpacing() const
			{
				return m_Spacing;
			}

			const int* getExtent() const
			{
				return m_Extent;
			}

			vtkMatrix4x4* getAxes()
			{
				return m_Axes;
			}

			const pcl::Region3D<double>& getRegion() const
			{
				return m_Region;
			}

			const pcl::Point3D<double>& getXAxis() const
			{
				return m_XAxis;
			}

			const pcl::Point3D<double>& getYAxis() const
			{
				return m_YAxis;
			}

			/******************************** Misc methods ********************************/

			void update(bool force_update = false)
			{
				if (!m_Region.empty() && (m_AxesTracker.isModified() || force_update)) {
					this->modified();
					Point3D<double> xaxis, yaxis;
					for (int i=0; i<3; ++i) {
						m_XAxis[i] = m_Axes->GetElement(i, 0);
						m_YAxis[i] = m_Axes->GetElement(i, 1);
						m_NormalStep[i] = m_Axes->GetElement(i, 2);
					}
					m_XAxis = computeOptimalSamplingVector(m_XAxis, m_RegionSpacing);
					m_YAxis = computeOptimalSamplingVector(m_YAxis, m_RegionSpacing);
					m_NormalStep = computeOptimalSamplingVector(m_NormalStep, m_RegionSpacing);
					m_MinChangeOffset = m_NormalStep.getNorm()*0.1;
					m_Spacing[0] = m_XAxis.getNorm(); m_XAxis /= m_Spacing[0];
					m_Spacing[1] = m_YAxis.getNorm(); m_YAxis /= m_Spacing[1];
					updateExtent();
					m_AxesTracker.updated();
				}
			}

			bool moveSliceToContain()
			{
				return moveSliceToContain(m_Region.getMinPoint() + (m_Region.getMaxPoint()-m_Region.getMinPoint())/2);
			}

			bool moveSliceToContain(const pcl::Point3D<double>& p)
			{
				update();
				pcl::Point3D<double> slice_origin = m_Origin;
				pcl::Point3D<double> normal(m_Axes->GetElement(0,2), m_Axes->GetElement(1,2), m_Axes->GetElement(2,2));
				double old_offset = normal.getDotProduct(pcl::Point3D<double>(m_Axes->GetElement(0,3), m_Axes->GetElement(1,3), m_Axes->GetElement(2,3))-m_Origin);
				double offset = normal.getDotProduct(p-m_Origin);
				if (pcl::abs(offset-old_offset)<m_MinChangeOffset) return false;
				slice_origin += normal*offset;
				double max_change = 0;
				for (int i=0; i<3; ++i) {
					m_Axes->SetElement(i,3, slice_origin[i]);
				}
				this->modified();
				return true;
			}

			bool moveSlice(int normal_offset)
			{
				update();
				pcl::Point3D<double> normal(m_Axes->GetElement(0,2), m_Axes->GetElement(1,2), m_Axes->GetElement(2,2));
				pcl::Point3D<double> slice_origin;
				for (int i=0; i<3; ++i) slice_origin[i] = m_Axes->GetElement(i,3);
				slice_origin += (m_NormalStep*normal_offset);

				double offset = (slice_origin-m_Origin).getDotProduct(normal);
				if (offset<m_ZRange[0] || offset>m_ZRange[1]) return false;

				for (int i=0; i<3; ++i) {
					m_Axes->SetElement(i,3, slice_origin[i]);
				}
				this->modified();
				return true;
			}

		protected:
			boost::unordered_set<pcl::Point3D<double>, pcl::PointHash> m_CornerPoints;
			pcl::Point3D<double> m_Origin, m_RegionSpacing, m_NormalStep;
			pcl::Point3D<double> m_XAxis, m_YAxis;
			double m_MinChangeOffset;
			int m_Extent[4];
			double m_Spacing[2];
			vtkSmartPointer<vtkMatrix4x4> m_Axes;
			pcl::Region3D<double> m_Region;
			double m_ZRange[2];
			pcl::vtk::VtkModificationTracker m_AxesTracker;

			ImageResliceParam() 
			{
				m_Axes = vtkSmartPointer<vtkMatrix4x4>::New();
				m_Axes->Identity();
				m_AxesTracker.reset(m_Axes);
				m_Region.reset();
			}

			pcl::Point3D<double> computeOptimalSamplingVector(const pcl::Point3D<double>& vec, const pcl::Point3D<double>& spc) const
			{
				pcl::Point3D<double> result;
				double result_norm_sqr = 0;
				for (int i=0; i<3; ++i) {
					pcl::Point3D<double> v = vec*spc[i];
					bool valid = true;
					for (int j=0; j<3; ++j) if (v[j]>spc[j]) {
						valid = false;
						break;
					}
					if (valid) {
						double norm_sqr = v.getNormSqr();
						if (norm_sqr>result_norm_sqr) {
							result = v;
							result_norm_sqr = norm_sqr;
						}
					}
				}
				if (result_norm_sqr==0) pcl_ThrowException(pcl::Exception(), "Norm of resulting vector is zero");
				return result;
			}

			void updateExtent()
			{
				pcl::Point3D<double> normal(m_Axes->GetElement(0,2), m_Axes->GetElement(1,2), m_Axes->GetElement(2,2));
				bool init = true;
				pcl_ForEach(m_CornerPoints, point) {
					pcl::Point3D<double> vec = *point-m_Origin;
					double x = vec.getDotProduct(m_XAxis)/m_Spacing[0], 
						y = vec.getDotProduct(m_YAxis)/m_Spacing[1],
						z = vec.getDotProduct(normal);
					if (init) {
						m_Extent[0] = std::floor(x);
						m_Extent[1] = std::ceil(x);
						m_Extent[2] = std::floor(y);
						m_Extent[3] = std::ceil(y);
						m_ZRange[0] = z;
						m_ZRange[1] = z;
						init = false;
					} else {
						m_Extent[0] = std::min<int>(std::floor(x), m_Extent[0]);
						m_Extent[1] = std::max<int>(std::ceil(x), m_Extent[1]);
						m_Extent[2] = std::min<int>(std::floor(y), m_Extent[2]);
						m_Extent[3] = std::max<int>(std::ceil(y), m_Extent[3]);
						m_ZRange[0] = std::min(m_ZRange[0], z);
						m_ZRange[1] = std::max(m_ZRange[1], z);
					}
				}
				double allowance = m_NormalStep.getNorm() * 0.5;
				m_ZRange[0] -= allowance;
				m_ZRange[1] += allowance;
			}
		};

	}
}

#endif