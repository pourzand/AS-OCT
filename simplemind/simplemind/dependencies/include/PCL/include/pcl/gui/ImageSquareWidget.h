#ifndef PCL_GUI_IMAGE_SQUARE_WIDGET
#define PCL_GUI_IMAGE_SQUARE_WIDGET

#include <pcl/geometry.h>
#include <vtkLineSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkAssembly.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <pcl/gui/ImageWidget.h>

namespace pcl
{
	namespace gui
	{

		class ImageSquareWidget
		{
		public:
			typedef ImageSquareWidget Self;
			typedef boost::shared_ptr<Self> Pointer;

			enum Color
			{
				AXIAL, CORONAL, SAGITTAL
			};

			static Pointer New(ImageWidget *widget, double size_in_mm, double r, double g, double b, double normal_multiplier=2)
			{
				return Pointer(new Self(widget, size_in_mm, r, g, b, normal_multiplier));
			}

			~ImageSquareWidget()
			{
				m_ImageWidget->getRenderer()->RemoveActor(m_Actor);
				render();
			}

			void setPoint(const pcl::Point3D<double>& p)
			{
				m_Actor->SetPosition(p.x(), p.y(), p.z());
			}

			void setVisibility(bool en)
			{
				m_Actor->SetVisibility(en);
			}

			bool belongTo(ImageWidget *widget)
			{
				return m_ImageWidget==widget;
			}

			void render()
			{
				m_ImageWidget->render();
			}

		protected:
			vtkSmartPointer<vtkAssembly> m_Actor;
			double m_NormalMultiplier;
			ImageWidget *m_ImageWidget;

			ImageSquareWidget(ImageWidget *widget, double size_in_mm, double r, double g, double b, double normal_multiplier)
			{
				m_NormalMultiplier = normal_multiplier;
				m_Actor = vtkSmartPointer<vtkAssembly>::New();
				m_ImageWidget = widget;
				m_ImageWidget->getRenderer()->AddActor(m_Actor);
				auto reslice_axes = m_ImageWidget->getResliceActorStack()->getResliceAxes();
				double half_size = size_in_mm/2;
				for (int a=0; a<2; ++a) {
					int pa = a==0?1:0;
					pcl::Point3D<double> main_axis(
						reslice_axes->GetElement(0, a),
						reslice_axes->GetElement(1, a),
						reslice_axes->GetElement(2, a)
						);
					pcl::Point3D<double> perp_axis(
						reslice_axes->GetElement(0, pa),
						reslice_axes->GetElement(1, pa),
						reslice_axes->GetElement(2, pa)
						);
					for (int i=0; i<2; ++i) {
						double mul = i==0?half_size:-half_size;
						pcl::Point3D<double> start_p, end_p;
						start_p = end_p = (perp_axis*mul);
						start_p += main_axis*(-half_size);
						end_p += main_axis*(half_size);

						auto line_source = vtkSmartPointer<vtkLineSource>::New();
						line_source->SetPoint1(&start_p[0]);
						line_source->SetPoint2(&end_p[0]);

						auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
						mapper->SetInput(line_source->GetOutput());
						auto actor = vtkSmartPointer<vtkActor>::New();
						actor->SetMapper(mapper);
						actor->PickableOff();
						auto prop = actor->GetProperty();
						prop->SetColor(r,g,b);
						m_Actor->AddPart(actor);
					}
				}
			}
		};

	}
}

#endif