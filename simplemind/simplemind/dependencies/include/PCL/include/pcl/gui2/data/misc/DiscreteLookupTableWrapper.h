#ifndef PCL_DISCRETE_LOOKUP_TABLE_WRAPPER
#define PCL_DISCRETE_LOOKUP_TABLE_WRAPPER

#include <vtkLookupTable.h>
#include <boost/random.hpp>

#define MAX_LOOKUP_TABLE_NUMBER 500

namespace pcl
{
	namespace gui
	{

		class DiscreteLookupTableWrapper
			{
			protected:
				vtkLookupTable* m_LookupTable;

			public:
				DiscreteLookupTableWrapper()
				{
					m_LookupTable = NULL;
				}

				DiscreteLookupTableWrapper(vtkLookupTable* l)
				{
					setLookupTable(l);
				}

				DiscreteLookupTableWrapper(vtkScalarsToColors* l)
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

				void setRandomColor(double min_range, double max_range, double alpha)
				{
					int num = static_cast<int>(max_range - min_range + 1);
					if (num>MAX_LOOKUP_TABLE_NUMBER) num = MAX_LOOKUP_TABLE_NUMBER;
					setRandomColor(min_range, max_range, num, alpha);
				}

				void setRandomColor(double min_range, double max_range, int label_num, double alpha)
				{
					int zero_ind = -1;
					if (min_range<=0) {
						double step = (max_range-min_range)/(label_num-1);
						double min_val = std::numeric_limits<double>::infinity(),
							cur_val = min_range;
						for (int i=0; i<label_num; ++i) {
							if (abs(cur_val)<min_val) {
								min_val = cur_val;
								zero_ind = i;
							} else break;
							cur_val += step;
						}
					}
					std::cout << "Zero index: " << zero_ind << std::endl;
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
							} while (r+g+b==0);
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

	}
}

#endif