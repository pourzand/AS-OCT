namespace pcl
{
	namespace gui
	{

		class PieceWiseLookupTableWrapper
			{
			protected:
				vtkLookupTable* m_LookupTable;

			public:
				ContinuousLookupTableWrapper()
				{
					m_LookupTable = NULL;
				}

				ContinuousLookupTableWrapper(vtkLookupTable* l)
				{
					m_LookupTable = l;
				}

				ContinuousLookupTableWrapper(vtkScalarsToColors* l)
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

				//TODO!!!
			};

	}
}