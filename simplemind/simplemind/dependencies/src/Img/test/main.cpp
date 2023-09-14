#include <pcl/image_io.h>
#include <pcl/macro.h>
#include <pcl/filter2.h>
#include <functional>
#include <pcl/misc/ColorImageGenerator.h>
#include <pcl/misc/CommandLineParser.h>


template <class ImageType>
class Test
{
public:
	Test()
	{}

	template <class FuncType>
	void set(FuncType& func)
	{
		m_Func = func;
	}

	typename ImageType::Pointer getResult()
	{
		std::cout << "Calling getResult()" << std::endl;
		if (!m_Result) m_Result = m_Func();
		return m_Result;
	}

protected:
	std::function<typename ImageType::Pointer()> m_Func;
	typename ImageType::Pointer m_Result;
};

void func(Test<pcl::Image<char>>& a) 
{
	auto temp = pcl::Image<char>::New(pcl::Point3D<int>(0,0,0), pcl::Point3D<int>(10,10,10));
	a.set([=]()->pcl::Image<char>::Pointer
	{
		std::cout << "Calling lambda function" << std::endl;
		return temp;
	});
}

int main(int argc, char* argv[]) pcl_MainStart
{
	pcl::CommandLineParser parser(argc, argv);
	parser.addArgument<std::string>("in_ap", "IN_AP", "Input AP image");
	parser.addArgument<std::string>("in_pa", "IN_PA", "Input PA image");
	parser.addArgument<std::string>("out_ap", "OUT_AP", "Resulting preprocessed AP image");
	parser.addArgument<std::string>("out_pa", "OUT_PA", "Resulting preprocessed PA image");
	parser.addArgument<std::string>("out_mask", "OUT_MASK", "Resulting soft tissue mask");
	parser.addOption<double>(3, "-v", "-v X Y Z", "Custom voxel spacing when reading input images");
	parser.addOption<std::string>(1, "--pa_lesion", "--pa_lesion PA_LESION", "PA lesion filename to flip about vertical axis");
	parser.update();


	std::cout << "Start" << std::endl;
	auto temp = pcl::ImageIoHelper::Read<pcl::Image<char,true>>("C:\\temp\\lungseg\\1.2.392.200036.9116.2.5.1.37.2426676844.1403584527.658449\\dicom\\1.dcm");
	std::cout << temp->getSpacing() << std::endl;
	std::cout << "All done" << std::endl;
} pcl_MainEnd(std::cout);