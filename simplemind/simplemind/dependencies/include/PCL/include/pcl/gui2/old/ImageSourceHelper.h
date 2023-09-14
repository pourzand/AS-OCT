#ifndef PCL_GUI_IMAGE_SOURCE_HELPER
#define PCL_GUI_IMAGE_SOURCE_HELPER

#include <pcl/gui2/ImageSource.h>
#include <vtkLookupTable.h>
#include <boost/random.hpp>

#define DEFAULT_LOOKUP_TABLE_NUMBER 500
#define MAX_LOOKUP_TABLE_NUMBER 500

namespace pcl
{
	namespace gui
	{

		class ImageSourceHelper
		{
		public:
			class GrayscaleVtkLookupTableWrapper
			{
			protected:
				vtkLookupTable* m_LookupTable;

			public:
				GrayscaleVtkLookupTableWrapper()
				{
					m_LookupTable = NULL;
				}

				GrayscaleVtkLookupTableWrapper(vtkLookupTable* l)
				{
					m_LookupTable = l;
				}

				void setLookupTable(vtkLookupTable* l)
				{
					m_LookupTable = l;
				}

				bool isNull()
				{
					return m_LookupTable==NULL;
				}

				double* getRange()
				{
					return m_LookupTable->GetRange();
				}

				void getWindowLevel(double& offset, double& width)
				{
					double* range = getRange();
					width = range[1]-range[0];
					offset = range[0]+ width*0.5;
				}

				void setRange(double min_val, double max_val)
				{
					m_LookupTable->SetRange(min_val, max_val);
					m_LookupTable->Build();
				}

				void setWindowLevel(double offset, double width)
				{
					if (width<1) width = 1;
					setRange(offset-width*0.5, offset+width*0.5);
					m_LookupTable->Build();
				}

				void setAlpha(double alpha)
				{
					m_LookupTable->SetAlphaRange(alpha, alpha);
					m_LookupTable->Build();
				}

				void reset(double min_range, double max_range)
				{
					m_LookupTable->SetNumberOfTableValues(DEFAULT_LOOKUP_TABLE_NUMBER);
					m_LookupTable->SetRange(min_range, max_range);
					m_LookupTable->SetValueRange(0, 1);
					m_LookupTable->SetSaturationRange(0,0);
					m_LookupTable->SetRampToLinear();
					m_LookupTable->Build();
				}
			};


			class DiscreteVtkLookupTableWrapper
			{
			protected:
				vtkLookupTable* m_LookupTable;

			public:
				DiscreteVtkLookupTableWrapper()
				{
					m_LookupTable = NULL;
				}

				DiscreteVtkLookupTableWrapper(vtkLookupTable* l)
				{
					m_LookupTable = l;
				}

				void setLookupTable(vtkLookupTable* l)
				{
					m_LookupTable = l;
				}

				bool isNull()
				{
					return m_LookupTable==NULL;
				}

				void setRandomColor(double min_range, double max_range, int label_num, double alpha)
				{
					int zero_ind = -1;
					if (min_range<=0) {
						double step = (max_range-min_range)/(label_num-1);
						double min_val = max_range,
							cur_val = min_val;
						for (int i=0; i<label_num; ++i) {
							if (abs(cur_val)<min_val) {
								min_val = cur_val;
								zero_ind = i;
							} else break;
						}
					}
					m_LookupTable->SetNumberOfTableValues(label_num);
					m_LookupTable->SetRange(min_range, max_range);
					boost::random::mt19937 rng;
					boost::random::uniform_int_distribution<> rnd_col(0, 256);
					for (int i=0; i<label_num; ++i) {
						if (i==zero_ind) m_LookupTable->SetTableValue(i, 0,0,0,0);
						else {
							int r,g,b;
							do {
								r = rnd_col(rng),
								g = rnd_col(rng),
								b = rnd_col(rng);
								m_LookupTable->SetTableValue(i, double(rnd_col(rng))/256, double(rnd_col(rng))/256, double(rnd_col(rng))/256, alpha);
							} while (r+g+b>0);
						}
					}
				}

				void setAlpha(double alpha)
				{
					for (int i=0; i<m_LookupTable->GetNumberOfTableValues(); ++i) {
						double *val = m_LookupTable->GetTableValue(i);
						if (val[0]+val[1]+val[2]+val[3]>0) m_LookupTable->SetTableValue(i, val[0], val[1], val[2], alpha);
					}
				}
			};


			template <class ImagePointerType>
			static typename ImageSource<typename pcl::ptr_base_type<ImagePointerType>::type>::Pointer Create(const ImagePointerType& image, bool* is_alias=NULL)
			{
				return ImageSource<typename pcl::ptr_base_type<ImagePointerType>::type>::New(image, is_alias);
			}

			static void SetDefaultImageLookup(const ImageSourceBase::Pointer& image_source)
			{
				auto range = image_source->getMinMax();
				vtkLookupTable *lookup_table = image_source->getColorLookup<vtkLookupTable>();
				if (lookup_table==NULL) lookup_table = vtkLookupTable::New();
				GrayscaleVtkLookupTableWrapper wrapper(lookup_table);
				wrapper.reset(range.get<0>(), range.get<1>());
				image_source->setColorLookup(lookup_table);
			}

			static void SetDefaultLabelLookup(const ImageSourceBase::Pointer& image_source, double alpha)
			{
				auto range = image_source->getMinMax();
				int label_num = ceil(range.get<1>()-range.get<0>())+1;
				if (label_num>MAX_LOOKUP_TABLE_NUMBER) label_num = MAX_LOOKUP_TABLE_NUMBER;
				vtkLookupTable *lookup_table = image_source->getColorLookup<vtkLookupTable>();
				if (lookup_table==NULL) lookup_table = vtkLookupTable::New();
				DiscreteVtkLookupTableWrapper wrapper(lookup_table);
				wrapper.setRandomColor(range.get<0>(), range.get<1>(), label_num, alpha);
				image_source->setColorLookup(lookup_table);
			}
		};

	}
}

#endif