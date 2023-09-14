#ifndef PCL_GUI_IMAGE_CROSS_HAIRS
#define PCL_GUI_IMAGE_CROSS_HAIRS

#include <pcl/geometry.h>
#include <vtkMatrix4x4.h>
#include <vtkLineSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkProperty.h>

namespace pcl
{
	namespace gui
	{

		class ImageCrossHairs
		{
		public:
			typedef ImageCrossHairs Self;
			typedef boost::shared_ptr<Self> Pointer;

			enum Color
			{
				AXIAL, CORONAL, SAGITTAL
			};

			static Pointer New(vtkMatrix4x4* actor_stack, double* normal_multiplier=NULL, double length_ratio=10, double offset=0.001)
			{
				return Pointer(new Self(actor_stack, normal_multiplier, length_ratio, offset));
			}

			void updateLineGeometry(const pcl::Region3D<double>& region)
			{
				auto corner_points = region.getCornerPoints();
				pcl::Point3D<double> center = region.getMinPoint() + (region.getMaxPoint()-region.getMinPoint())/2;
				for (int i=0; i<2; ++i) {
					pcl::Point3D<double> direction(m_Axes->GetElement(0,i), m_Axes->GetElement(1,i), m_Axes->GetElement(2,i));
					double min_offset=std::numeric_limits<double>::infinity(), 
						max_offset=-std::numeric_limits<double>::infinity();
					pcl_ForEach(corner_points, point) {
						double offset = (*point-center).getDotProduct(direction);
						min_offset = std::min(min_offset, offset);
						max_offset = std::max(max_offset, offset);
					}
					double extra_length = (max_offset - min_offset)*(m_LengthRatio-1);
					pcl::Point3D<double> minp, maxp;
					minp = center + direction*(min_offset-extra_length);
					maxp = center + direction*(max_offset+extra_length);
					m_LineSource[i]->SetPoint1(&minp[0]);
					m_LineSource[i]->SetPoint2(&maxp[0]);
				}
				setPoint(m_Point);
			}

			void setLineActorColor(int i, double r, double g, double b)
			{
				m_LineActor[i]->GetProperty()->SetColor(r,g,b);
			}

			void setLineActorColor(int i, Color col)
			{
				switch (col) {
				case AXIAL:
					setLineActorColor(i, 1, 0, 0);
					break;
				case CORONAL:
					setLineActorColor(i, 0, 1, 0);
					break;
				case SAGITTAL:
					setLineActorColor(i, 0, 0, 1);
					break;
				};
			}

			void setLineActorColor(Color plane)
			{
				switch (plane) {
				case AXIAL:
					setLineActorColor(0, CORONAL);
					setLineActorColor(1, SAGITTAL);
					break;
				case CORONAL:
					setLineActorColor(0, AXIAL);
					setLineActorColor(1, SAGITTAL);
					break;
				case SAGITTAL:
					setLineActorColor(0, AXIAL);
					setLineActorColor(1, CORONAL);
					break;
				};
			}

			vtkActor* getLineActor(int i)
			{
				return m_LineActor[i];
			}

			void setPoint(const pcl::Point3D<double>& p)
			{
				pcl::Point3D<double> plane_direction(m_Axes->GetElement(0,2), m_Axes->GetElement(1,2), m_Axes->GetElement(2,2));
				for (int i=0; i<2; ++i) {
					pcl::Point3D<double> line_point[2];
					line_point[0] = m_LineSource[i]->GetPoint1();
					line_point[1] = m_LineSource[i]->GetPoint2();
					pcl::Point3D<double> ortho_direction;
					if (i==0) ortho_direction.set(m_Axes->GetElement(0,1), m_Axes->GetElement(1,1), m_Axes->GetElement(2,1));
					else ortho_direction.set(m_Axes->GetElement(0,0), m_Axes->GetElement(1,0), m_Axes->GetElement(2,0));
					auto motion = p-line_point[0];
					double ortho_offset = motion.getDotProduct(ortho_direction),
						plane_offset = motion.getDotProduct(plane_direction)+(m_NormalMultiplier==NULL?1:-m_NormalMultiplier[0])*m_Offset;
					for (int j=0; j<2; ++j) {
						line_point[j] += ortho_direction*ortho_offset;
						line_point[j] += plane_direction*plane_offset;
					}
					m_LineSource[i]->SetPoint1(&line_point[0][0]);
					m_LineSource[i]->SetPoint2(&line_point[1][0]);
					m_LineActor[i]->Modified();
				}
				m_Point = p;
			}
			void setPoint()
			{
				setPoint(m_Point);
			}

			void setVisibility(bool en)
			{
				m_LineActor[0]->SetVisibility(en);
				m_LineActor[1]->SetVisibility(en);
			}

			void modified()
			{
				m_LineActor[0]->Modified();
				m_LineActor[1]->Modified();
			}

		protected:
			vtkSmartPointer<vtkMatrix4x4> m_Axes;
			double m_LengthRatio, m_Offset;
			vtkSmartPointer<vtkLineSource> m_LineSource[2];
			vtkSmartPointer<vtkActor> m_LineActor[2];
			pcl::Point3D<double> m_Point;
			double *m_NormalMultiplier;

			ImageCrossHairs(vtkMatrix4x4* axes, double *normal_multiplier, double length_ratio, double offset)
			{
				m_NormalMultiplier = normal_multiplier;
				m_Axes = axes;
				m_LengthRatio = length_ratio;
				m_Offset = offset;
				for (int i=0; i<2; ++i) {
					m_LineSource[i] = vtkSmartPointer<vtkLineSource>::New();
					auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
					mapper->SetInput(m_LineSource[i]->GetOutput());
					m_LineActor[i] = vtkSmartPointer<vtkActor>::New();
					m_LineActor[i]->SetMapper(mapper);
					m_LineActor[i]->PickableOff();
					auto property = m_LineActor[i]->GetProperty();
					property->SetOpacity(0.5);
				}
				m_Point.set(0,0,0);
			}
		};

	}
}

#endif