#pragma once

#include <pcl/image.h>
#include <vtkObjectFactory.h>
#include <vtkImageImport.h>
#include <pcl/vtk/ItkToVtkHelper.h>

template <class T>
class PclToVtkImport: public vtkImageImport
{
public:
	typedef T ImageType;
	vtkTypeMacro(PclToVtkImport, vtkImageImport);

	static PclToVtkImport* New()
	{
		VTK_STANDARD_NEW_BODY(PclToVtkImport);
	}

	void setInput(typename ImageType::Pointer image)
	{
		m_Source = image;
		m_Modified = true;

		auto const &region = m_Source->getRegion();
		for (int i = 0; i < 3; ++i) {
			m_WholeExtent[i * 2] = region.getMinPoint()[i];
			m_WholeExtent[i * 2 + 1] = region.getMaxPoint()[i];
			m_DataSpacing[i] = m_Source->getSpacing()[i];
			m_DataOrigin[i] = m_Source->getOrigin()[i];
		}
	}

protected:
	typename ImageType::Pointer m_Source;
	std::string m_ScalarTypeName;
	int m_WholeExtent[6];
	double m_DataSpacing[3];
	double m_DataOrigin[3];
	bool m_Modified;

	PclToVtkImport(): vtkImageImport()
	{
		typedef typename ImageType::ValueType ScalarType;
		if (typeid(ScalarType) == typeid(double))
		{
			m_ScalarTypeName = "double";
		}
		else if (typeid(ScalarType) == typeid(float))
		{
			m_ScalarTypeName = "float";
		}
		else if (typeid(ScalarType) == typeid(long))
		{
			m_ScalarTypeName = "long";
		}
		else if (typeid(ScalarType) == typeid(unsigned long))
		{
			m_ScalarTypeName = "unsigned long";
		}
		else if (typeid(ScalarType) == typeid(int))
		{
			m_ScalarTypeName = "int";
		}
		else if (typeid(ScalarType) == typeid(unsigned int))
		{
			m_ScalarTypeName = "unsigned int";
		}
		else if (typeid(ScalarType) == typeid(short))
		{
			m_ScalarTypeName = "short";
		}
		else if (typeid(ScalarType) == typeid(unsigned short))
		{
			m_ScalarTypeName = "unsigned short";
		}
		else if (typeid(ScalarType) == typeid(char))
		{
			m_ScalarTypeName = "char";
		}
		else if (typeid(ScalarType) == typeid(unsigned char))
		{
			m_ScalarTypeName = "unsigned char";
		}
		else if (typeid(ScalarType) == typeid(signed char))
		{
			m_ScalarTypeName = "signed char";
		}
		else
		{
			pcl_ThrowException(pcl::Exception(),"Type currently not supported");
		}
		
		this->SetUpdateInformationCallback(&PclToVtkImport::UpdateInformationCallback);
		this->SetPipelineModifiedCallback(&PclToVtkImport::PipelineModifiedCallback);
		this->SetWholeExtentCallback(&PclToVtkImport::WholeExtentCallback);
		this->SetSpacingCallback(&PclToVtkImport::SpacingCallback);
		this->SetOriginCallback(&PclToVtkImport::OriginCallback);
		this->SetScalarTypeCallback(&PclToVtkImport::ScalarTypeCallback);
		this->SetNumberOfComponentsCallback(&PclToVtkImport::NumberOfComponentsCallback);
		this->SetPropagateUpdateExtentCallback(&PclToVtkImport::PropagateUpdateExtentCallback);
		this->SetUpdateDataCallback(&PclToVtkImport::UpdateDataCallback);
		this->SetDataExtentCallback(&PclToVtkImport::DataExtentCallback);
		this->SetBufferPointerCallback(&PclToVtkImport::BufferPointerCallback);
		this->SetCallbackUserData(this);
	}

	void UpdateOutputInformationFunction()
	{}
	
	int PipelineModifiedFunction()
	{
		if (m_Modified) {
			m_Modified = false;
			return 1;
		} else {
			return 0;
		}
	}

	void UpdateDataFunction()
	{}
	
	int* WholeExtentFunction()
	{
		return m_WholeExtent;
	}

	double* SpacingFunction()
	{
		return m_DataSpacing;
	}

	double* OriginFunction()
	{
		return m_DataOrigin;
	}

	const char* ScalarTypeFunction()
	{
		return m_ScalarTypeName.c_str();
	}

	int NumberOfComponentsFunction()
	{
		return 1;
	}

	void PropagateUpdateExtentFunction(int *extent)
	{}

	int* DataExtentFunction()
	{
		return m_WholeExtent;
	}

	void* BufferPointerFunction()
	{
		return m_Source->getBuffer()->getPointer();
	}



	static void UpdateInformationCallback(void* userdata)
	{
		static_cast<PclToVtkImport*>(userdata)->UpdateOutputInformationFunction();
	}

	static int PipelineModifiedCallback(void* userdata)
	{
		return static_cast<PclToVtkImport*>(userdata)->PipelineModifiedFunction();
	}

	static int* WholeExtentCallback(void* userdata)
	{
		return static_cast<PclToVtkImport*>(userdata)->WholeExtentFunction();
	}

	static double* SpacingCallback(void* userdata)
	{
		return static_cast<PclToVtkImport*>(userdata)->SpacingFunction();
	}

	static double* OriginCallback(void* userdata)
	{
		return static_cast<PclToVtkImport*>(userdata)->OriginFunction();
	}

	static const char* ScalarTypeCallback(void* userdata)
	{
		return static_cast<PclToVtkImport*>(userdata)->ScalarTypeFunction();
	}

	static int NumberOfComponentsCallback(void* userdata)
	{
		return static_cast<PclToVtkImport*>(userdata)->NumberOfComponentsFunction();
	}

	static void PropagateUpdateExtentCallback(void* userdata, int *extent)
	{
		static_cast<PclToVtkImport*>(userdata)->PropagateUpdateExtentFunction(extent);
	}

	static void UpdateDataCallback(void* userdata)
	{
		static_cast<PclToVtkImport*>(userdata)->UpdateDataFunction();
	}

	static int* DataExtentCallback(void* userdata)
	{
		return static_cast<PclToVtkImport*>(userdata)->DataExtentFunction();
	}

	static void* BufferPointerCallback(void* userdata)
	{
		return static_cast<PclToVtkImport*>(userdata)->BufferPointerFunction();
	}
};