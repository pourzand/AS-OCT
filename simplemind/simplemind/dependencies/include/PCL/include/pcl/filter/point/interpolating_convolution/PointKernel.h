
namespace pcl
{
	namespace filter
	{

		template <class T=double>
		class PointKernel
		{
		public:
			typedef ValueType T;

			PointKernel()
			{}

			PointKernel(int capacity)
			{
				m_Kernel.reserve(capacity);
			}

			PointKernel(const PointKernel& obj)
			{
				m_Kernel = obj.m_Kernel;
			}

			int size() const
			{
				return m_Kernel.size();
			}

			void add(const Point3D<double>& p, const ValueType& v)
			{
				m_Kernel.push_back(PointValue(v,p));
			}

			const ValueType& getValue(int i) const
			{
				return m_Kernel[i].value;
			}

			const Point3D<double>& getPoint(int i) const
			{
				return m_Kernel[i].point;
			}

			const std::vector<PointValue>& get() const
			{
				return m_Kernel;
			}

			PointKernel& operator=(const PointKernel& obj)
			{
				m_Kernel = obj.m_Kernel;
				return *this;
			}

			template<class ImagePointer>
			void toPhysicalCoordinate(const ImagePointer& image)
			{
				pcl_ForEach(m_Kernel, item) {
					item->point = image->toPhysicalCoordinate(item->point);
				}
			}

			template<class ImagePointer>
			void toImageCoordinate(const ImagePointer& image)
			{
				pcl_ForEach(m_Kernel, item) {
					item->point = image->toImageCoordinate(item->point);
				}
			}

			template<class TransformationPointer>
			void applyTransformation(const TransformationPointer& trans)
			{
				pcl_ForEach(m_Kernel, item) {
					item->point = trans->toTransformed(item->point);
				}
			}

		protected:
			struct PointValue
			{
				ValueType value;
				Point3D<double> point;

				PointValue(ValueType v, const Point3D<double>& p)
				{
					value = v;
					point = p;
				}
			};
			std::vector<PointValue> m_Kernel;
		};

	}
}