#ifndef PCL_GUI_IMAGE_WIDGET_GROUP_CONTROLLER
#define PCL_GUI_IMAGE_WIDGET_GROUP_CONTROLLER

#include <pcl/gui/ImageWidget.h>

namespace pcl
{
	namespace gui
	{

		class ImageWidgetGroupController
		{
		public:
			typedef ImageWidgetGroupController Self;
			typedef boost::shared_ptr<Self> Pointer;

			static Pointer New()
			{
				return Pointer(new Self);
			}

			void addWidget(ImageWidget* widget)
			{
				pcl_ForEach(m_Widget, item) if (widget==*item) return;
				m_Widget.push_back(widget);
			}

			void removeWidget(ImageWidget* widget)
			{
				std::vector<ImageWidget*> temp;
				temp.reserve(m_Widget.size());
				pcl_ForEach(m_Widget, item) {
					if (*item!=widget) temp.push_back(*item);
				}
				m_Widget = std::move(temp);
			}

			const Point3D<double>& getCenter() const
			{
				return m_Center;
			}

			void setCenter(const pcl::Point3D<double>& center)
			{
				m_Center = center;
				pcl_ForEach(m_Widget, item) {
					if ((*item)->getResliceActorStack()->setSliceToContain(m_Center)) (*item)->render(true);
					else (*item)->render();
				}
				//std::cout << m_Center << std::endl;
			}

			void render()
			{
				pcl_ForEach(m_Widget, item) (*item)->render();
			}
			
			void keyReleaseEvent(QKeyEvent* event)
			{
				pcl_ForEach(m_Widget, item) {
					if ((*item)->underMouse()) {
						(*item)->keyReleaseEvent(event);
						break;
					}
				}
			}
			
			void keyPressEvent(QKeyEvent* event)
			{
				pcl_ForEach(m_Widget, item) {
					if ((*item)->underMouse()) {
						(*item)->keyPressEvent(event);
						break;
					}
				}
			}

		protected:
			pcl::Point3D<double> m_Center;
			std::vector<ImageWidget*> m_Widget;

			ImageWidgetGroupController() {}
		};

	}
}

#endif