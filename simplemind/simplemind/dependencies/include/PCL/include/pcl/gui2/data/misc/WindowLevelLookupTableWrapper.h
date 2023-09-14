#ifndef PCL_WINDOW_LEVEL_LOOKUP_TABLE_WRAPPER
#define PCL_WINDOW_LEVEL_LOOKUP_TABLE_WRAPPER

#include <vtkLookupTable.h>

#define DEFAULT_LOOKUP_TABLE_NUMBER 500

namespace pcl
{
	namespace gui
	{

		class WindowLevelLookupTableWrapper
			{
			protected:
				vtkLookupTable* m_LookupTable;

			public:
				WindowLevelLookupTableWrapper()
				{
					m_LookupTable = NULL;
				}

				WindowLevelLookupTableWrapper(vtkLookupTable* l)
				{
					m_LookupTable = l;
				}

				WindowLevelLookupTableWrapper(vtkScalarsToColors* l)
				{
					setLookupTable(l);
				}

				void setLookupTable(vtkScalarsToColors* l)
				{
					m_LookupTable = dynamic_cast<vtkLookupTable*>(l);
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
				}

				void setWindowLevel(double offset, double width)
				{
					if (width<1) width = 1;
					setRange(offset-width*0.5, offset+width*0.5);
				}

				void setAlpha(double alpha)
				{
					m_LookupTable->SetAlphaRange(alpha, alpha);
				}

				void reset(double min_range, double max_range)
				{
					std::cout << min_range << " " << max_range << std::endl;
					m_LookupTable->SetNumberOfTableValues(DEFAULT_LOOKUP_TABLE_NUMBER);
					m_LookupTable->SetRange(min_range, max_range);
					m_LookupTable->SetValueRange(0, 1);
					m_LookupTable->SetSaturationRange(0,0);
					m_LookupTable->SetRampToLinear();
					m_LookupTable->Build();
				}
			};

	}
}

#endif