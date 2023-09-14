#ifndef __CYTHON_IMAGE__
#define __CYTHON_IMAGE__
#include <typeinfo>
#include <math.h>
#include <pcl/image.h>
#include <pcl/image_io.h>
#include <pcl/ibis/RoiHelper.h>
#include <pcl/filter.h>
#include <pcl/filter2.h>
#include <pcl/filter/glcm/GlcmFeatures.h>
#include <pcl/filter/glcm/GlcmHelper.h>
#include <pcl/statistics.h>

#include <pcl/measurement/GeometricalMeasurementHelper.h>
#include <boost/unordered_map.hpp>

#define callFunctionType(type,func) \
	if (type==typeid(long long)) func<long long>(); \
	else if (type==typeid(int)) func<int>(); \
	else if (type==typeid(short)) func<short>(); \
	else if (type==typeid(char)) func<char>(); \
	else if (type==typeid(float)) func<float>(); \
	else if (type==typeid(double)) func<double>(); \
	else if (type==typeid(unsigned char)) func<unsigned char>(); \
	else if (type==typeid(unsigned short)) func<unsigned short>(); \
	else if (type==typeid(unsigned int)) func<unsigned int>(); \
	else if (type==typeid(unsigned long long)) func<unsigned long long>(); \
	else { \
		std::cout << "Invalid type encountered!" << std::endl; \
		pcl_ThrowException(pcl::Exception(), "Invalid type encountered!"); \
	}

#define callFunctionParamType(type,func,param) \
	if (type==typeid(long long)) func<long long>(param); \
	else if (type==typeid(int)) func<int>(param); \
	else if (type==typeid(short)) func<short>(param); \
	else if (type==typeid(char)) func<char>(param); \
	else if (type==typeid(float)) func<float>(param); \
	else if (type==typeid(double)) func<double>(param); \
	else if (type==typeid(unsigned char)) func<unsigned char>(param); \
	else if (type==typeid(unsigned short)) func<unsigned short>(param); \
	else if (type==typeid(unsigned int)) func<unsigned int>(param); \
	else if (type==typeid(unsigned long long)) func<unsigned long long>(param); \
	else { \
		std::cout << "Invalid type encountered!" << std::endl; \
		pcl_ThrowException(pcl::Exception(), "Invalid type encountered!"); \
	}

#define callFunctionParamTypeWithReturn(type,func,param) \
	if (type==typeid(long long)) return func<long long>(param); \
	else if (type==typeid(int)) return func<int>(param); \
	else if (type==typeid(short)) return func<short>(param); \
	else if (type==typeid(char)) return func<char>(param); \
	else if (type==typeid(float)) return func<float>(param); \
	else if (type==typeid(double)) return func<double>(param); \
	else if (type==typeid(unsigned char)) return func<unsigned char>(param); \
	else if (type==typeid(unsigned short)) return func<unsigned short>(param); \
	else if (type==typeid(unsigned int)) return func<unsigned int>(param); \
	else if (type==typeid(unsigned long long)) return func<unsigned long long>(param); \
	else { \
		std::cout << "Invalid type encountered!" << std::endl; \
		pcl_ThrowException(pcl::Exception(), "Invalid type encountered!"); \
	}


#define DeclareBinaryOperatorFunc(opname, op) \
	public: \
	void inplace##opname(double val) { callFunctionParamType(*m_Type, actualInplaceVal##opname, val); }\
	void inplace##opname(const ImageObject& obj, double out_val) { \
		auto param = boost::tuple<const ImageObject&, double>(obj, out_val); \
		callFunctionParamType(*m_Type, actualInplaceImg##opname, param); \
	} \
	ImageObject* ret##opname(double val) { callFunctionParamTypeWithReturn(*m_Type, actualVal##opname, val); }\
	ImageObject* ret##opname(const ImageObject& obj, double out_val, double src_out_val) { \
		auto param = boost::tuple<const ImageObject&, double, double>(obj, out_val, src_out_val); \
		callFunctionParamTypeWithReturn(*m_Type, actualImg##opname, param); \
	} \
	protected: \
	template <class T> \
	void actualInplaceVal##opname(double val) \
	{ \
		auto target = boost::static_pointer_cast<pcl::Image<T,true>>(m_Image); \
		pcl::ImageIterator iter(target); \
		pcl_ForIterator(iter) target->set(iter, target->get(iter) op val); \
	} \
	template <class T> \
	void actualInplaceImg##opname(boost::tuple<const ImageObject&, double>& param) \
	{ \
		auto obj = param.get<0>(); \
		auto out_val = param.get<1>(); \
		auto target = boost::static_pointer_cast<pcl::Image<T,true>>(m_Image); \
		pcl::Region3D<int> region(target->getRegion().getIntersect(obj.m_Image->getRegion())); \
		if (!region.empty()) { \
			pcl::ImageIterator target_iter(target), source_iter(obj.m_Image); \
			target_iter.setRegion(region); \
			source_iter.setRegion(region); \
			if (obj.m_Type==m_Type) { \
				auto source = boost::static_pointer_cast<const pcl::Image<T,true>>(obj.m_Image); \
				pcl_ForIterator2(target_iter, source_iter) target->set(target_iter, target->get(target_iter) op source->get(source_iter)); \
			} else { \
				pcl_ForIterator2(target_iter, source_iter) target->set(target_iter, target->get(target_iter) op obj.m_Image->getValue(source_iter)); \
			} \
		} \
		pcl::ImageRegionsIterator<> iter(target); \
		iter.addList(target->getRegion().getRegionsAfterSubtractionBy(obj.m_Image->getRegion())); \
		pcl_ForIterator(iter) target->set(iter, target->get(iter) op out_val); \
	} \
	template <class T> \
	ImageObject* actualVal##opname(double val) \
	{ \
		auto image = boost::static_pointer_cast<pcl::Image<T,true>>(m_Image); \
		auto result = pcl::Image<T,true>::New(image); \
		pcl::ImageIterator iter(image); \
		pcl_ForIterator(iter) result->set(iter, image->get(iter) op val); \
		return new ImageObject(boost::static_pointer_cast<pcl::ImagePhysicalLayer<true>>(result), typeid(T)); \
	} \
	template <class T> \
	ImageObject* actualImg##opname(boost::tuple<const ImageObject&, double, double>& param) \
	{ \
		auto obj = param.get<0>(); \
		auto out_val = param.get<1>(); \
		auto src_out_val = param.get<2>(); \
		auto self_image = boost::static_pointer_cast<pcl::Image<T,true>>(m_Image); \
		auto result_region = self_image->getRegion(); \
		result_region.add(obj.m_Image->getRegion()); \
		auto result = pcl::ImageHelper::GetCroppedAuto(self_image, result_region, src_out_val); \
		pcl::Region3D<int> region(result->getRegion().getIntersect(obj.m_Image->getRegion())); \
		if (!region.empty()) { \
			pcl::ImageIterator image_iter(result), source_iter(obj.m_Image); \
			image_iter.setRegion(region); \
			source_iter.setRegion(region); \
			if (obj.m_Type==m_Type) { \
				auto source = boost::static_pointer_cast<const pcl::Image<T,true>>(obj.m_Image); \
				pcl_ForIterator2(image_iter, source_iter) result->set(image_iter, result->get(image_iter) op source->get(source_iter)); \
			} else { \
				pcl_ForIterator2(image_iter, source_iter) result->set(image_iter, result->get(image_iter) op obj.m_Image->getValue(source_iter)); \
			} \
		} \
		pcl::ImageRegionsIterator<> iter(result); \
		iter.addList(result->getRegion().getRegionsAfterSubtractionBy(obj.m_Image->getRegion())); \
		pcl_ForIterator(iter) result->set(iter, result->get(iter) op out_val); \
		return new ImageObject(boost::static_pointer_cast<pcl::ImagePhysicalLayer<true>>(result), typeid(T)); \
	} \
	public:

#define DeclareBinaryOperatorFuncWithStaticCast(opname, op) \
	public: \
	void inplace##opname(double val) { callFunctionParamType(*m_Type, actualInplaceVal##opname, val); }\
	void inplace##opname(ImageObject& obj, double out_val) { \
		auto param = boost::tuple<const ImageObject&, double>(obj, out_val); \
		callFunctionParamType(*m_Type, actualInplaceImg##opname, param); \
	} \
	ImageObject* ret##opname(double val) { callFunctionParamTypeWithReturn(*m_Type, actualVal##opname, val); }\
	ImageObject* ret##opname(const ImageObject& obj, double out_val, double src_out_val) { \
		auto param = boost::tuple<const ImageObject&, double, double>(obj, out_val, src_out_val); \
		callFunctionParamTypeWithReturn(*m_Type, actualImg##opname, param); \
	} \
	protected: \
	template <class T> \
	void actualInplaceVal##opname(double val) \
	{ \
		T actual_val = static_cast<T>(val); \
		auto target = boost::static_pointer_cast<pcl::Image<T,true>>(m_Image); \
		pcl::ImageIterator iter(target); \
		pcl_ForIterator(iter) target->set(iter, target->get(iter) op actual_val); \
	} \
	template <class T> \
	void actualInplaceImg##opname(boost::tuple<const ImageObject&, double>& param) \
	{ \
		auto obj = param.get<0>(); \
		auto out_val = param.get<1>(); \
		auto target = boost::static_pointer_cast<pcl::Image<T,true>>(m_Image); \
		pcl::Region3D<int> region(target->getRegion().getIntersect(obj.m_Image->getRegion())); \
		if (!region.empty()) { \
			pcl::ImageIterator target_iter(target), source_iter(obj.m_Image); \
			target_iter.setRegion(region); \
			source_iter.setRegion(region); \
			if (obj.m_Type==m_Type) { \
				auto source = boost::static_pointer_cast<const pcl::Image<T,true>>(obj.m_Image); \
				pcl_ForIterator2(target_iter, source_iter) target->set(target_iter, target->get(target_iter) op static_cast<T>(source->get(source_iter))); \
			} else { \
				pcl_ForIterator2(target_iter, source_iter) target->set(target_iter, target->get(target_iter) op static_cast<T>(obj.m_Image->getValue(source_iter))); \
			} \
		} \
		T actual_val = static_cast<T>(out_val); \
		pcl::ImageRegionsIterator<> iter(target); \
		iter.addList(target->getRegion().getRegionsAfterSubtractionBy(obj.m_Image->getRegion())); \
		pcl_ForIterator(iter) target->set(iter, target->get(iter) op actual_val); \
	} \
	template <class T> \
	ImageObject* actualVal##opname(double val) \
	{ \
		T actual_val = static_cast<T>(val); \
		auto image = boost::static_pointer_cast<pcl::Image<T,true>>(m_Image); \
		auto result = pcl::Image<T,true>::New(image); \
		pcl::ImageIterator iter(image); \
		pcl_ForIterator(iter) result->set(iter, image->get(iter) op actual_val); \
		return new ImageObject(boost::static_pointer_cast<pcl::ImagePhysicalLayer<true>>(result), typeid(T)); \
	} \
	template <class T> \
	ImageObject* actualImg##opname(boost::tuple<const ImageObject&, double, double>& param) \
	{ \
		auto obj = param.get<0>(); \
		auto out_val = param.get<1>(); \
		auto src_out_val = param.get<2>(); \
		auto self_image = boost::static_pointer_cast<pcl::Image<T,true>>(m_Image); \
		auto result_region = self_image->getRegion(); \
		result_region.add(obj.m_Image->getRegion()); \
		auto result = pcl::ImageHelper::GetCroppedAuto(self_image, result_region, src_out_val); \
		pcl::Region3D<int> region(result->getRegion().getIntersect(obj.m_Image->getRegion())); \
		if (!region.empty()) { \
			pcl::ImageIterator image_iter(result), source_iter(obj.m_Image); \
			image_iter.setRegion(region); \
			source_iter.setRegion(region); \
			if (obj.m_Type==m_Type) { \
				auto source = boost::static_pointer_cast<const pcl::Image<T,true>>(obj.m_Image); \
				pcl_ForIterator2(image_iter, source_iter) result->set(image_iter, result->get(image_iter) op static_cast<T>(source->get(source_iter))); \
			} else { \
				pcl_ForIterator2(image_iter, source_iter) result->set(image_iter, result->get(image_iter) op static_cast<T>(obj.m_Image->getValue(source_iter))); \
			} \
		} \
		T actual_val = static_cast<T>(out_val); \
		pcl::ImageRegionsIterator<> iter(result); \
		iter.addList(result->getRegion().getRegionsAfterSubtractionBy(obj.m_Image->getRegion())); \
		pcl_ForIterator(iter) result->set(iter, result->get(iter) op actual_val); \
		return new ImageObject(boost::static_pointer_cast<pcl::ImagePhysicalLayer<true>>(result), typeid(T)); \
	} \
	public:

#define DeclareBinaryOperatorFuncForIntegerOnly(opname, op) \
	public: \
	void inplace##opname(double val) { callFunctionParamType(*m_Type, actualInplaceVal##opname, val); }\
	void inplace##opname(ImageObject& obj, double out_val) { \
		auto param = boost::tuple<const ImageObject&, double>(obj, out_val); \
		callFunctionParamType(*m_Type, actualInplaceImg##opname, param); \
	} \
	ImageObject* ret##opname(double val) { callFunctionParamTypeWithReturn(*m_Type, actualVal##opname, val); }\
	ImageObject* ret##opname(const ImageObject& obj, double out_val, double src_out_val) { \
		auto param = boost::tuple<const ImageObject&, double, double>(obj, out_val, src_out_val); \
		callFunctionParamTypeWithReturn(*m_Type, actualImg##opname, param); \
	} \
	protected: \
	template <class T> \
	typename boost::enable_if<boost::is_integral<T>, void>::type actualInplaceVal##opname(double val) \
	{ \
		T actual_val = static_cast<T>(val); \
		auto target = boost::static_pointer_cast<pcl::Image<T,true>>(m_Image); \
		pcl::ImageIterator iter(target); \
		pcl_ForIterator(iter) target->set(iter, target->get(iter) op actual_val); \
	} \
	template <class T> \
	typename boost::enable_if<boost::is_integral<T>, void>::type actualInplaceImg##opname(boost::tuple<const ImageObject&, double>& param) \
	{ \
		auto obj = param.get<0>(); \
		auto out_val = param.get<1>(); \
		auto target = boost::static_pointer_cast<pcl::Image<T,true>>(m_Image); \
		pcl::Region3D<int> region(target->getRegion().getIntersect(obj.m_Image->getRegion())); \
		if (!region.empty()) { \
			pcl::ImageIterator target_iter(target), source_iter(obj.m_Image); \
			target_iter.setRegion(region); \
			source_iter.setRegion(region); \
			if (obj.m_Type==m_Type) { \
				auto source = boost::static_pointer_cast<const pcl::Image<T,true>>(obj.m_Image); \
				pcl_ForIterator2(target_iter, source_iter) target->set(target_iter, target->get(target_iter) op static_cast<T>(source->get(source_iter))); \
			} else { \
				pcl_ForIterator2(target_iter, source_iter) target->set(target_iter, target->get(target_iter) op static_cast<T>(obj.m_Image->getValue(source_iter))); \
			} \
		} \
		T actual_val = static_cast<T>(out_val); \
		pcl::ImageRegionsIterator<> iter(target); \
		iter.addList(target->getRegion().getRegionsAfterSubtractionBy(obj.m_Image->getRegion())); \
		pcl_ForIterator(iter) target->set(iter, target->get(iter) op actual_val); \
	} \
	template <class T> \
	typename boost::disable_if<boost::is_integral<T>, void>::type actualInplaceVal##opname(double val) {} \
	template <class T> \
	typename boost::disable_if<boost::is_integral<T>, void>::type actualInplaceImg##opname(boost::tuple<const ImageObject&, double>& param) {} \
	template <class T> \
	typename boost::enable_if<boost::is_integral<T>, ImageObject*>::type actualVal##opname(double val) \
	{ \
		T actual_val = static_cast<T>(val); \
			auto image = boost::static_pointer_cast<pcl::Image<T, true>>(m_Image); \
			auto result = pcl::Image<T, true>::New(image); \
			pcl::ImageIterator iter(image); \
			pcl_ForIterator(iter) result->set(iter, image->get(iter) op actual_val); \
			return new ImageObject(boost::static_pointer_cast<pcl::ImagePhysicalLayer<true>>(result), typeid(T)); \
		} \
	template <class T> \
	typename boost::enable_if<boost::is_integral<T>, ImageObject*>::type actualImg##opname(boost::tuple<const ImageObject&, double, double>& param) \
	{ \
		auto obj = param.get<0>(); \
		auto out_val = param.get<1>(); \
		auto src_out_val = param.get<2>(); \
		auto self_image= boost::static_pointer_cast<pcl::Image<T,true>>(m_Image); \
		auto result_region = self_image->getRegion(); \
		result_region.add(obj.m_Image->getRegion()); \
		auto result = pcl::ImageHelper::GetCroppedAuto(self_image, result_region, src_out_val); \
		pcl::Region3D<int> region(result->getRegion().getIntersect(obj.m_Image->getRegion())); \
		if (!region.empty()) { \
			pcl::ImageIterator image_iter(result), source_iter(obj.m_Image); \
			image_iter.setRegion(region); \
			source_iter.setRegion(region); \
			if (obj.m_Type==m_Type) { \
				auto source = boost::static_pointer_cast<const pcl::Image<T,true>>(obj.m_Image); \
				pcl_ForIterator2(image_iter, source_iter) result->set(image_iter, result->get(image_iter) op static_cast<T>(source->get(source_iter))); \
			} else { \
				pcl_ForIterator2(image_iter, source_iter) result->set(image_iter, result->get(image_iter) op static_cast<T>(obj.m_Image->getValue(source_iter))); \
			} \
		} \
		T actual_val = static_cast<T>(out_val); \
		pcl::ImageRegionsIterator<> iter(result); \
		iter.addList(result->getRegion().getRegionsAfterSubtractionBy(obj.m_Image->getRegion())); \
		pcl_ForIterator(iter) result->set(iter, result->get(iter) op actual_val); \
		return new ImageObject(boost::static_pointer_cast<pcl::ImagePhysicalLayer<true>>(result), typeid(T)); \
	} \
	template <class T> \
	typename boost::disable_if<boost::is_integral<T>, ImageObject*>::type actualVal##opname(double val) { return NULL; } \
	template <class T> \
	typename boost::disable_if<boost::is_integral<T>, ImageObject*>::type actualImg##opname(boost::tuple<const ImageObject&, double, double>& param) { return NULL; } \
	public:

static void init()
{
	REDIRECT_ITK_TO_CONSOLE;
}

static double get_infinity()
{
	return std::numeric_limits<double>::infinity();
}

static double get_nan()
{
	return std::numeric_limits<double>::quiet_NaN();
}

static const std::type_info& getType(const std::string& type)
{
	if (type.compare("long")==0) return typeid(long long);
	else if (type.compare("int")==0) return typeid(int);
	else if (type.compare("short")==0) return typeid(short);
	else if (type.compare("char")==0) return typeid(char);
	else if (type.compare("float")==0) return typeid(float);
	else if (type.compare("double")==0) return typeid(double);
	else if (type.compare("uchar")==0) return typeid(unsigned char);
	else if (type.compare("ushort")==0) return typeid(unsigned short);
	else if (type.compare("uint")==0) return typeid(unsigned int);
	else if (type.compare("ulong")==0) return typeid(unsigned long long);
	std::cout << "invalid type \"" << type << "\" encountered" << std::endl;
	pcl_ThrowException(pcl::Exception(), "invalid type \""+type+"\" encountered");
}

static std::string getTypeStr(const std::type_info& type)
{
	if (type==typeid(long long)) return "long"; 
	else if (type==typeid(int)) return "int"; 
	else if (type==typeid(short)) return "short"; 
	else if (type==typeid(char)) return "char"; 
	else if (type==typeid(float)) return "float"; 
	else if (type==typeid(double)) return "double"; 
	else if (type==typeid(unsigned char)) return "uchar"; 
	else if (type==typeid(unsigned short)) return "ushort"; 
	else if (type==typeid(unsigned int)) return "uint"; 
	else if (type==typeid(unsigned long long)) return "ulong";
	std::cout << "invalid type encountered" << std::endl;
	pcl_ThrowException(pcl::Exception(), "invalid type encountered");
}

template <class T>
struct ihash:std::unary_function<boost::tuple<T,T>, std::size_t>
{
    std::size_t operator()(boost::tuple<T,T> const& e) const
    {
        std::size_t seed = 0;
        boost::hash_combine( seed, boost::get<0>(e) );
        boost::hash_combine( seed, boost::get<1>(e) );
        return seed;
    }
};

template <class T>
struct iequal_to:std::binary_function<boost::tuple<T,T>, boost::tuple<T,T>, bool>
{
    bool operator()(boost::tuple<T,T> const& x, boost::tuple<T,T> const& y) const
    {
        return ( 
			boost::get<0>(x)==boost::get<0>(y)
			&& boost::get<1>(x)==boost::get<1>(y)
        );
    }
};

template <class T>
T get0(const boost::tuple<T,T>& obj)
{
	return boost::get<0>(obj);
}

template <class T>
T get1(const boost::tuple<T,T>& obj)
{
	return boost::get<1>(obj);
}

struct PointValue
{
	pcl::Point3D<int> point;
	double value;

	PointValue()
	{}

	PointValue(const pcl::Point3D<int>& p, double v)
	{
		point = p;
		value = v;
	}
};

class ImageObject
{
public:
	ImageObject(const std::string& type, const pcl::Point3D<int>& minp, const pcl::Point3D<int>& maxp,
		const pcl::Point3D<double>& spacing, const pcl::Point3D<double>& origin, const std::vector<double>& o, double fill_value)
	{
		pcl::DummyImage::OrientationMatrixType orientation;
		int i = 0;
		for (int r = 0; r < 3; ++r) for (int c = 0; c < 3; ++c) {
			orientation(r, c) = o[i];
			++i;
		}
		m_Image = pcl::DummyImage::New(minp, maxp, spacing, origin, orientation);
		m_TypeStr = type;
		m_Type = NULL;
		if (type.compare("dummy") != 0) {
			m_Type = &getType(m_TypeStr);
			callFunctionType(*m_Type, createActualImage);
			fill(fill_value, m_Image->getMinPoint(), m_Image->getMaxPoint());
		}
	}
	
	ImageObject(void* ptr, const std::string& type, const pcl::Point3D<int>& minp, const pcl::Point3D<int>& maxp,
		const pcl::Point3D<double>& spacing, const pcl::Point3D<double>& origin, const std::vector<double>& o)
	{
		pcl::DummyImage::OrientationMatrixType orientation;
		int i = 0;
		for (int r = 0; r < 3; ++r) for (int c = 0; c < 3; ++c) {
			orientation(r, c) = o[i];
			++i;
		}
		m_Image = pcl::DummyImage::New(minp, maxp, spacing, origin, orientation);
		m_TypeStr = type;
		m_Type = &getType(m_TypeStr);
		callFunctionParamType(*m_Type, createActualImageWithPointer, ptr);
	}

	ImageObject(const std::string& type, const ImageObject& obj, bool copy, double fill_value)
	{
		m_Type = NULL;
		if (type.compare("auto") == 0) {
			m_Type = obj.m_Type;
			m_TypeStr = obj.m_TypeStr;
		}
		else {
			m_TypeStr = type;
			if (type.compare("dummy") != 0) m_Type = &getType(m_TypeStr);
		}
		m_Image = pcl::DummyImage::New(obj.m_Image);
		if (type.compare("dummy") != 0) {
			callFunctionType(*m_Type, createActualImage);
			if (copy) fill(obj, m_Image->getMinPoint(), m_Image->getMaxPoint());
			else fill(fill_value, m_Image->getMinPoint(), m_Image->getMaxPoint());
		}
	}

	ImageObject(const std::string& type, const std::string& file)
	{
		std::string extension = pcl::FileNameTokenizer(file).getExtensionWithoutDot();
		for (int i = 0; i < extension.size(); i++) extension[i] = tolower(extension[i]);
		m_Type = NULL;
		if (type.compare("auto") == 0) {
			if (extension.compare("dummy") == 0) {
				m_TypeStr = "dummy";
			}
			else {
				if (extension.compare("roi") == 0) m_Type = &typeid(char);
				else m_Type = &pcl::ImageIoHelper::GetImageFileInfo(file).type;
				m_TypeStr = getTypeStr(*m_Type);
			}
		}
		else {
			m_TypeStr = type;
			if (type.compare("dummy") != 0) m_Type = &getType(m_TypeStr);
		}
		if (m_TypeStr.compare("dummy") == 0) m_Image = pcl::ImageIoHelper::ReadDummy(file);
		else if (extension.compare("roi") == 0) {
			callFunctionParamType(*m_Type, actualReadRoi, file);
			if (m_TypeStr.compare("dummy") == 0) m_Image = pcl::DummyImage::New(m_Image);
		}
		else {
			if (m_TypeStr.compare("dummy") == 0) m_Image = pcl::ImageIoHelper::ReadDummy(file);
			else callFunctionParamType(*m_Type, actualReadFile, file);
		}
	}

	ImageObject(pcl::ImagePhysicalLayer<true>::Pointer image, const std::type_info& type)
	{
		m_Image = image;
		m_Type = &type;
		m_TypeStr = getTypeStr(type);
	}

	ImageObject(pcl::ImagePhysicalLayer<true>::Pointer image)
	{
		m_Image = image;
		m_Type = NULL;
		m_TypeStr = "dummy";
	}

	const std::string& getTypeString()
	{
		return m_TypeStr;
	}

	double getValue(int x, int y, int z)
	{
		return m_Image->getValue(x, y, z);
	}

	void setValue(int x, int y, int z, double val)
	{
		m_Image->setValue(x, y, z, val);
	}

	pcl::Point3D<double> toPhysicalCoordinates(double x, double y, double z)
	{
		return m_Image->toPhysicalCoordinate(x, y, z);
	}

	pcl::Point3D<double> toImageCoordinates(double x, double y, double z)
	{
		return m_Image->toImageCoordinate(x, y, z);
	}

	pcl::Point3D<int> getSize()
	{
		return m_Image->getSize();
	}

	pcl::Point3D<int> getMinPoint()
	{
		return m_Image->getMinPoint();
	}

	pcl::Point3D<int> getMaxPoint()
	{
		return m_Image->getMaxPoint();
	}

	pcl::Point3D<double> getSpacing()
	{
		return m_Image->getSpacing();
	}

	pcl::Point3D<double> getOrigin()
	{
		return m_Image->getOrigin();
	}

	std::vector<double> getOrientation()
	{
		auto orientation = m_Image->getOrientationMatrix();
		std::vector<double> o;
		o.reserve(9);
		for (int r = 0; r < 3; ++r) for (int c = 0; c < 3; ++c) {
			o.push_back(orientation(r, c));
		}
		return o;
	}

	pcl::Region3D<int> getRegion()
	{
		return m_Image->getRegion();
	}

	pcl::Region3D<double> getPhysicalRegion()
	{
		return m_Image->getPhysicalRegion();
	}

	bool contain(const pcl::Point3D<int>& p)
	{
		return m_Image->contain(p);
	}

	bool contain(const pcl::Point3D<int>& minp, const pcl::Point3D<int>& maxp)
	{
		return m_Image->getRegion().contain(pcl::Region3D<int>(minp, maxp));
	}

	std::vector<PointValue> find(double min_val, double max_val)
	{
		auto param = boost::tuple<double, double>(min_val, max_val);
		callFunctionParamTypeWithReturn(*m_Type, actualFind, param);
	}

	pcl::Region3D<int> findRegion(double min_val, double max_val)
	{
		auto param = boost::tuple<double, double>(min_val, max_val);
		callFunctionParamTypeWithReturn(*m_Type, actualFindRegion, param);
	}

	pcl::Region3D<double> findPhysicalRegion(double min_val, double max_val)
	{
		auto param = boost::tuple<double, double>(min_val, max_val);
		callFunctionParamTypeWithReturn(*m_Type, actualFindPhysicalRegion, param);
	}

	boost::tuple<double, double> getMinMax()
	{
		callFunctionParamTypeWithReturn(*m_Type, actualGetMinMax, 1);
	}

	void fill(double fill_value, const pcl::Point3D<int>& minp, const pcl::Point3D<int>& maxp)
	{
		auto param = boost::tuple<double, const pcl::Point3D<int>&, const pcl::Point3D<int>&>(fill_value, minp, maxp);
		callFunctionParamType(*m_Type, actualFillValue, param);
	}

	void fill(const ImageObject& obj, const pcl::Point3D<int>& minp, const pcl::Point3D<int>& maxp)
	{
		auto param = boost::tuple<const ImageObject&, const pcl::Point3D<int>&, const pcl::Point3D<int>&>(obj, minp, maxp);
		callFunctionParamType(*m_Type, actualFillImage, param);
	}

	pcl::Region3D<int> fillRoi(const std::string& file, double value)
	{
		auto param = boost::tuple<const std::string&, double>(file, value);
		callFunctionParamTypeWithReturn(*m_Type, actualFillRoi, param);
	}

	void write(const std::string& file, bool compress)
	{
		pcl::ImageIoHelper::ItkUseCompression = compress;
		callFunctionParamType(*m_Type, actualWriteFile, file);
	}

	DeclareBinaryOperatorFunc(Add, +);
	DeclareBinaryOperatorFunc(Subtract, -);
	DeclareBinaryOperatorFunc(Multiply, *);
	DeclareBinaryOperatorFunc(Divide, / );
	DeclareBinaryOperatorFuncWithStaticCast(Eq, == );
	DeclareBinaryOperatorFunc(Ne, != );
	DeclareBinaryOperatorFunc(Gt, > );
	DeclareBinaryOperatorFunc(Ge, >= );
	DeclareBinaryOperatorFunc(Lt, < );
	DeclareBinaryOperatorFunc(Le, <= );
	DeclareBinaryOperatorFuncForIntegerOnly(BitAnd, &);
	DeclareBinaryOperatorFuncForIntegerOnly(BitOr, | );
	DeclareBinaryOperatorFuncForIntegerOnly(BitXor, ^);

	void negate()
	{
		callFunctionType(*m_Type, actualNegate);
	}

	ImageObject* getNegate()
	{
		callFunctionParamTypeWithReturn(*m_Type, actualGetNegate, 0);
	}

	void abs()
	{
		callFunctionType(*m_Type, actualAbs);
	}

	ImageObject* getAbs()
	{
		callFunctionParamTypeWithReturn(*m_Type, actualGetAbs, 0);
	}
	
	void log()
	{
		callFunctionType(*m_Type, actualLog);
	}

	ImageObject* getLog()
	{
		callFunctionParamTypeWithReturn(*m_Type, actualGetLog, 0);
	}
	
	void exp()
	{
		callFunctionType(*m_Type, actualExp);
	}

	ImageObject* getExp()
	{
		callFunctionParamTypeWithReturn(*m_Type, actualGetExp, 0);
	}
	
	ImageObject* getBinaryDilation(const pcl::iterator::ImageNeighborIterator::OffsetListPointer& offset_list)
	{
		callFunctionParamTypeWithReturn(*m_Type, actualGetBinaryDilation, offset_list);
	}

	ImageObject* getBinaryErosion(const pcl::iterator::ImageNeighborIterator::OffsetListPointer& offset_list)
	{
		callFunctionParamTypeWithReturn(*m_Type, actualGetBinaryErosion, offset_list);
	}

	ImageObject* getBinaryOpening(const pcl::iterator::ImageNeighborIterator::OffsetListPointer& offset_list)
	{
		callFunctionParamTypeWithReturn(*m_Type, actualGetBinaryOpening, offset_list);
	}

	ImageObject* getBinaryClosing(const pcl::iterator::ImageNeighborIterator::OffsetListPointer& offset_list)
	{
		callFunctionParamTypeWithReturn(*m_Type, actualGetBinaryClosing, offset_list);
	}

	ImageObject* getDistanceTransform(bool is_signed, bool use_square, bool use_spacing, const pcl::Region3D<int>& output_region)
	{
		auto param = boost::tuple<bool, bool, bool, const pcl::Region3D<int>>(is_signed, use_square, use_spacing, output_region);
		callFunctionParamTypeWithReturn(*m_Type, actualGetDistanceTransform, param);
	}

	ImageObject* getResampled(const ImageObject& template_image, bool nearest_neighbor, bool use_out_value, double out_value)
	{
		auto param = boost::tuple<const ImageObject&, bool, bool, double>(template_image, nearest_neighbor, use_out_value, out_value);
		callFunctionParamTypeWithReturn(*m_Type, actualGetResampled, param);
	}

	ImageObject* getCrop(const pcl::Point3D<int>& minp, const pcl::Point3D<int>& maxp, double outval)
	{
		boost::tuple<pcl::Point3D<int>, pcl::Point3D<int>, double> param(minp, maxp, outval);
		callFunctionParamTypeWithReturn(*m_Type, actualCrop, param);
	}

	pcl::iterator::ImageNeighborIterator::OffsetListPointer getElement()
	{
		callFunctionParamTypeWithReturn(*m_Type, actualGetElement, 1);
	}

	boost::unordered_map<double, long long> getHistogram(const pcl::Region3D<int>& region)
	{
		callFunctionParamTypeWithReturn(*m_Type, actualGetHistogram, region);
	}

	boost::unordered_map<boost::tuple<double, double>, long long, ihash<double>, iequal_to<double>> getJointHistogram(const ImageObject& obj, const pcl::Region3D<int>& region, double out_value)
	{
		boost::tuple<const ImageObject&, const pcl::Region3D<int>&, double> param(obj, region, out_value);
		callFunctionParamTypeWithReturn(*m_Type, actualGetJointHistogram, param);
	}

	pcl::statistics::StatisticsCalculator* getStatisticsCalculator(const ImageObject& mask)
	{
		callFunctionParamTypeWithReturn(*m_Type, actualGetStatisticsCalculator, mask);
	}

	pcl::statistics::PercentileCalculator<double>* getPercentileCalculator(const ImageObject& mask)
	{
		callFunctionParamTypeWithReturn(*m_Type, actualGetPercentileCalculator, mask);
	}

	ImageObject* getFlip(int flip_axis)
	{
		callFunctionParamTypeWithReturn(*m_Type, actualFlip, flip_axis);
	}

	std::vector<double> computeOriginalLongestAxialDiameter()
	{
		callFunctionParamTypeWithReturn(*m_Type, actualComputeOriginalLongestAxialDiameter, 1);
	}

	std::vector<double> computeLongestAxialDiameter()
	{
		callFunctionParamTypeWithReturn(*m_Type, actualComputeLongestAxialDiameter, 1);
	}

	std::vector<double> computeLongestDiameter()
	{
		callFunctionParamTypeWithReturn(*m_Type, actualComputeLongestDiameter, 1);
	}
	/* //DISABLED, cmake compilation failed 
	pcl::filter::GlcmFeatures* getGlcmFeatures(const ImageObject& mask, const std::vector<pcl::Point3D<int>>& offsets, int level, bool symmetric)
	{
		boost::tuple<const ImageObject&, const std::vector<pcl::Point3D<int>>&, int, bool> param(mask, offsets, level, symmetric);
		callFunctionParamTypeWithReturn(*m_Type, actualGetGlcmFeatures, param);
	}
	*/
	void modifyImage(const pcl::Point3D<int>& minp, bool fix_physical)
	{
		pcl::Point3D<double> origin;
		if (fix_physical) origin = m_Image->toPhysicalCoordinate(m_Image->getMinPoint()) - m_Image->toPhysicalVector(minp);
		else origin = m_Image->getOrigin();
		if (m_Type == NULL) {
			m_Image = pcl::DummyImage::New(minp, minp + m_Image->getSize() - 1, m_Image->getSpacing(),
				origin, m_Image->getOrientationMatrix());
		}
		else {
			auto param = boost::tuple<pcl::Point3D<int>, pcl::Point3D<double>, pcl::Point3D<double>, pcl::ImagePhysicalLayer<true>::OrientationMatrixType>(
				minp, m_Image->getSpacing(), origin, m_Image->getOrientationMatrix()
				);
			callFunctionParamType(*m_Type, actualModifyImage, param);
		}
	}

	void modifyImage(const pcl::Point3D<int>& minp, const pcl::Point3D<double>& spacing, const pcl::Point3D<double>& origin, const std::vector<double>& o)
	{
		pcl::ImagePhysicalLayer<true>::OrientationMatrixType orientation;
		int i = 0;
		for (int r = 0; r < 3; ++r) for (int c = 0; c < 3; ++c) {
			orientation(r, c) = o[i];
			++i;
		}
		if (m_Type == NULL) {
			m_Image = pcl::DummyImage::New(minp, m_Image->getMaxPoint(), spacing, origin, orientation);
		}
		else {
			auto param = boost::tuple<pcl::Point3D<int>, pcl::Point3D<double>, pcl::Point3D<double>, pcl::ImagePhysicalLayer<true>::OrientationMatrixType>(
				minp, spacing, origin, orientation
				);
			callFunctionParamType(*m_Type, actualModifyImage, param);
		}
	}

	ImageObject* getAlias(const pcl::Point3D<int>& minp, bool fix_physical)
	{
		pcl::Point3D<double> origin;
		if (fix_physical) origin = m_Image->toPhysicalCoordinate(m_Image->getMinPoint()) - m_Image->toPhysicalVector(minp);
		else origin = m_Image->getOrigin();
		if (m_Type == NULL) {
			auto result = pcl::DummyImage::New(minp, minp + m_Image->getSize() - 1, m_Image->getSpacing(),
				origin, m_Image->getOrientationMatrix());
			return new ImageObject(boost::static_pointer_cast<pcl::ImagePhysicalLayer<true>>(result));
		}
		else {
			auto param = boost::tuple<pcl::Point3D<int>, pcl::Point3D<double>, pcl::Point3D<double>, pcl::ImagePhysicalLayer<true>::OrientationMatrixType>(
				minp, m_Image->getSpacing(), origin, m_Image->getOrientationMatrix()
				);
			callFunctionParamTypeWithReturn(*m_Type, actualGetAlias, param);
		}
	}

	ImageObject* getAlias(const pcl::Point3D<int>& minp, const pcl::Point3D<double>& spacing, const pcl::Point3D<double>& origin, const std::vector<double>& o)
	{
		pcl::ImagePhysicalLayer<true>::OrientationMatrixType orientation;
		int i = 0;
		for (int r = 0; r < 3; ++r) for (int c = 0; c < 3; ++c) {
			orientation(r, c) = o[i];
			++i;
		}
		if (m_Type == NULL) {
			auto result = pcl::DummyImage::New(minp, m_Image->getMaxPoint(), spacing, origin, orientation);
			return new ImageObject(boost::static_pointer_cast<pcl::ImagePhysicalLayer<true>>(result));
		}
		else {
			auto param = boost::tuple<pcl::Point3D<int>, pcl::Point3D<double>, pcl::Point3D<double>, pcl::ImagePhysicalLayer<true>::OrientationMatrixType>(
				minp, spacing, origin, orientation
				);
			callFunctionParamTypeWithReturn(*m_Type, actualGetAlias, param);
		}
	}

	ImageObject* getSubImage(const pcl::Point3D<int>& minp, const pcl::Point3D<int>& maxp)
	{
		if (!m_Image->contain(minp) || !m_Image->contain(maxp)) pcl_ThrowException(pcl::Exception(), "Invalid region provided!");
		if (m_Type == NULL) {
			auto result = pcl::DummyImage::New(minp, maxp, m_Image->getSpacing(), m_Image->getOrigin(), m_Image->getOrientationMatrix());
			return new ImageObject(boost::static_pointer_cast<pcl::ImagePhysicalLayer<true>>(result));
		} else {
			auto param = boost::tuple<pcl::Point3D<int>, pcl::Point3D<int>>(minp, maxp);
			callFunctionParamTypeWithReturn(*m_Type, actualGetSubImage, param);
		}
	}

	ImageObject* getWholeImage()
	{
		if (m_Type == NULL) {
			auto result = pcl::DummyImage::New(m_Image->getMinPoint(), m_Image->getMaxPoint(), m_Image->getSpacing(), m_Image->getOrigin(), m_Image->getOrientationMatrix());
			return new ImageObject(boost::static_pointer_cast<pcl::ImagePhysicalLayer<true>>(result));
		}
		else {
			callFunctionParamTypeWithReturn(*m_Type, actualGetWholeImage, 1);
		}
	}

	bool isSubImage()
	{
		return m_Image->getSize() != m_Image->getBufferSize();
	}

	const std::type_info& type() const
	{
		return *m_Type;
	}

	const pcl::ImagePhysicalLayer<true>::Pointer& image() const
	{
		return m_Image;
	}
	
	void* getBuffer()
	{
		callFunctionParamTypeWithReturn(*m_Type, actualGetBuffer, 1);
	}

	unsigned long long getBufferSize()
	{		
		callFunctionParamTypeWithReturn(*m_Type, actualGetBufferSize, 1);
	}
	
	unsigned int getVoxelSize()
	{
		callFunctionParamTypeWithReturn(*m_Type, actualGetVoxelSize, 1);
	}

protected:
	pcl::ImagePhysicalLayer<true>::Pointer m_Image;
	const std::type_info* m_Type;
	std::string m_TypeStr;

	template <class T>
	void createActualImage()
	{
		m_Image = pcl::Image<T,true>::New(m_Image);
	}
	
	template <class T>
	void createActualImageWithPointer(void* ptr)
	{
		m_Image = pcl::Image<T,true>::New(
			static_cast<T*>(ptr), false, 
			m_Image->getSize(), m_Image->getMinPoint(), m_Image->getSpacing(), 
			m_Image->getOrigin(), m_Image->getOrientationMatrix()
		);
	}

	template <class T>
	boost::tuple<double,double> actualGetMinMax(int)
	{
		auto image = boost::static_pointer_cast<pcl::Image<T,true>>(m_Image);
		double min_val = std::numeric_limits<double>::infinity(),
			max_val = -std::numeric_limits<double>::infinity();
		pcl::ImageIterator iter(image);
		pcl_ForIterator(iter) {
			double val = image->get(iter);
			min_val = std::min(min_val, val);
			max_val = std::max(max_val, val);
		}
		return boost::tuple<double,double>(min_val, max_val);
	}

	template <class T>
	void actualReadFile(const std::string& file) 
	{
		m_Image = pcl::ImageIoHelper::Read<pcl::Image<T,true>>(file);
	}

	template <class T>
	void actualReadRoi(const std::string& file) 
	{
		m_Image = pcl::ibis::RoiHelper::Read<pcl::Image<T,true>>(file);
		if (!m_Image) pcl_ThrowException(pcl::Exception(), "Empty ROI encountered when reading "+file);
	}

	template <class T>
	std::vector<PointValue> actualFind(boost::tuple<double, double>& param)
	{
		double min_val = param.get<0>(),
			max_val = param.get<1>();
		auto image = boost::static_pointer_cast<pcl::Image<T,true>>(m_Image);
		std::vector<PointValue> result;
		pcl::ImageIteratorWithPoint iter(image);
		pcl_ForIterator(iter) {
			double val = image->get(iter);
			if (val>=min_val && val<=max_val) result.push_back(PointValue(iter.getPoint(), val));
		}
		return result;
	}

	template <class T>
	pcl::Region3D<int> actualFindRegion(boost::tuple<double, double>& param)
	{
		double min_val = param.get<0>(),
			max_val = param.get<1>();
		auto image = boost::static_pointer_cast<pcl::Image<T,true>>(m_Image);
		pcl::Region3D<int> result; result.reset();
		pcl::ImageIteratorWithPoint iter(image);
		pcl_ForIterator(iter) {
			double val = image->get(iter);
			if (val>=min_val && val<=max_val) result.add(iter.getPoint());
		}
		return result;
	}

	template <class T>
	pcl::Region3D<double> actualFindPhysicalRegion(boost::tuple<double, double>& param)
	{
		double min_val = param.get<0>(),
			max_val = param.get<1>();
		auto image = boost::static_pointer_cast<pcl::Image<T, true>>(m_Image);
		pcl::Region3D<double> result; result.reset();
		pcl::ImageIteratorWithPoint iter(image);
		pcl_ForIterator(iter) {
			double val = image->get(iter);
			if (val >= min_val && val <= max_val) result.add(image->toPhysicalCoordinate(iter.getPoint()));
		}
		return result;
	}

	template <class T>
	void actualWriteFile(const std::string& file)
	{
		auto image = boost::static_pointer_cast<pcl::Image<T,true>>(m_Image);
		if (boost::iequals(pcl::FileNameTokenizer(file).getExtensionWithoutDot(), "roi")) {
			pcl::ibis::RoiHelper::Write(file, image, [&](const pcl::PointIndexObject& pi)->bool {
				return image->get(pi)>0;
			});
		} else {
			pcl::ImageIoHelper::Write(file, image);
		}
	}

	template <class T>
	void actualFillValue(boost::tuple<double, const pcl::Point3D<int>&, const pcl::Point3D<int>&>& param)
	{
		pcl::ImageHelper::Fill(
			boost::static_pointer_cast<pcl::Image<T,true>>(m_Image),
			param.get<1>(),
			param.get<2>(),
			param.get<0>()
		);
	}

	template <class T>
	void actualFillImage(boost::tuple<const ImageObject&, const pcl::Point3D<int>&, const pcl::Point3D<int>&>& param) 
	{
		const ImageObject& obj = param.get<0>();
		auto target = boost::static_pointer_cast<pcl::Image<T,true>>(m_Image);
		if (obj.m_Type==m_Type) {
			auto source = boost::static_pointer_cast<const pcl::Image<T,true>>(obj.m_Image);
			pcl::ImageHelper::Copy(source, target, param.get<1>(), param.get<2>());
		} else {
			pcl::Region3D<int> region(param.get<1>(), param.get<2>());
			region.setIntersect(m_Image->getRegion());
			region.setIntersect(obj.m_Image->getRegion());
			pcl::ImageIteratorWithPoint iter(target);
			iter.setRegion(region);
			pcl_ForIterator(iter) target->set(iter, obj.m_Image->getValue(iter.getPoint()));
		}
	}

	template <class T>
	pcl::Region3D<int> actualFillRoi(boost::tuple<const std::string&,double>& param)
	{
		return pcl::ibis::RoiHelper::Fill(
			boost::static_pointer_cast<pcl::Image<T,true>>(m_Image),
			param.get<0>(),
			param.get<1>()
		);
	}

	template <class T>
	ImageObject* actualCrop(boost::tuple<pcl::Point3D<int>, pcl::Point3D<int>, double>& param)
	{
		auto target = boost::static_pointer_cast<pcl::Image<T,true>>(m_Image);
		return new ImageObject(
			boost::static_pointer_cast<pcl::ImagePhysicalLayer<true>>(pcl::ImageHelper::GetCroppedAuto(target, pcl::Region3D<int>(param.get<0>(), param.get<1>()), static_cast<T>(param.get<2>()))), 
			*m_Type
		);
	}

	template <class T>
	void actualNegate()
	{
		auto target = boost::static_pointer_cast<pcl::Image<T,true>>(m_Image);
		pcl::ImageIterator iter(target);
		pcl_ForIterator(iter) target->set(iter, -target->get(iter));
	}

	template <class T>
	void actualAbs()
	{
		auto target = boost::static_pointer_cast<pcl::Image<T,true>>(m_Image);
		pcl::ImageIterator iter(target);
		pcl_ForIterator(iter) target->set(iter, pcl::abs(target->get(iter)));
	}	
	
	template <class T>
	void actualExp()
	{
		auto target = boost::static_pointer_cast<pcl::Image<T, true>>(m_Image);
		pcl::ImageIterator iter(target);
		pcl_ForIterator(iter) target->set(iter, std::exp(target->get(iter)));
	}	
	
	template <class T>
	ImageObject* actualGetExp(int)
	{
		auto image = boost::static_pointer_cast<pcl::Image<T, true>>(m_Image);
		auto result = pcl::Image<T, true>::New(image);
		pcl::ImageIterator iter(image);
		pcl_ForIterator(iter) result->set(iter, std::exp(image->get(iter)));
		return new ImageObject(boost::static_pointer_cast<pcl::ImagePhysicalLayer<true>>(result), *m_Type);
	}
	
	template <class T>
	ImageObject* actualGetNegate(int)
	{
		auto image = boost::static_pointer_cast<pcl::Image<T, true>>(m_Image);
		auto result = pcl::Image<T, true>::New(image);
		pcl::ImageIterator iter(image);
		pcl_ForIterator(iter) result->set(iter, -image->get(iter));
		return new ImageObject(boost::static_pointer_cast<pcl::ImagePhysicalLayer<true>>(result), *m_Type);
	}

	template <class T>
	ImageObject* actualGetAbs(int)
	{
		auto image = boost::static_pointer_cast<pcl::Image<T, true>>(m_Image);
		auto result = pcl::Image<T, true>::New(image);
		pcl::ImageIterator iter(image);
		pcl_ForIterator(iter) result->set(iter, pcl::abs(image->get(iter)));
		return new ImageObject(boost::static_pointer_cast<pcl::ImagePhysicalLayer<true>>(result), *m_Type);
	}
	
	template <class T>
	void actualLog()
	{
		auto target = boost::static_pointer_cast<pcl::Image<T, true>>(m_Image);
		pcl::ImageIterator iter(target);
		pcl_ForIterator(iter) {			
			auto val = target->get(iter);			
			if (boost::math::isnormal(val)) {				
				// calculate log for values >0 and is normal (not zero, infinite, NaN, or denormalised).
				if (val>0) target->set(iter, std::log(val));
				else target->set(iter, NAN);
			}else {
				auto fp_type = boost::math::fpclassify(val);
				if (fp_type == FP_ZERO) target->set(iter, -INFINITY);
				else target->set(iter, NAN);
			}
		}
	}

	template <class T>
	ImageObject* actualGetLog(int)
	{
		auto image = boost::static_pointer_cast<pcl::Image<T, true>>(m_Image);
		auto result = pcl::Image<T, true>::New(image);
		pcl::ImageIterator iter(image);
		pcl_ForIterator(iter) {
			auto val = image->get(iter);
			if (boost::math::isnormal(val)) {
				// calculate log for values >0 and is normal (not zero, infinite, NaN, or denormalised).
				if (val>0) result->set(iter, std::log(val));
				else result->set(iter, NAN);
			}
			else {
				auto fp_type = boost::math::fpclassify(val);
				if (fp_type == FP_ZERO) result->set(iter, -INFINITY);
				// infinite,NaN denormalized values all gets set to NAN.
				else result->set(iter, NAN); 
			}
		}
		return new ImageObject(boost::static_pointer_cast<pcl::ImagePhysicalLayer<true>>(result), *m_Type);
	}
	
	template <class T>
	pcl::iterator::ImageNeighborIterator::OffsetListPointer actualGetElement(int)
	{
		pcl::iterator::ImageNeighborIterator::OffsetListPointer list(new pcl::iterator::ImageNeighborIterator::OffsetListType());
		auto img = boost::static_pointer_cast<pcl::Image<T,true>>(m_Image);
		pcl::ImageIteratorWithPoint iter(img);
		pcl_ForIterator(iter) {
			if (img->get(iter)>0) list->push_back(iter.getPoint());
		}
		return list;
	}

	template <class T>
	ImageObject* actualGetBinaryDilation(const pcl::iterator::ImageNeighborIterator::OffsetListPointer& offset_list)
	{
		auto input = boost::static_pointer_cast<pcl::Image<T,true>>(m_Image);
		return new ImageObject(
			boost::static_pointer_cast<pcl::ImagePhysicalLayer<true>>(pcl::filter2::BinaryMorphology::Dilate<pcl::Image<char,true>>(input, offset_list)),
			typeid(char)
		);
	}

	template <class T>
	ImageObject* actualGetBinaryErosion(const pcl::iterator::ImageNeighborIterator::OffsetListPointer& offset_list)
	{
		auto input = boost::static_pointer_cast<pcl::Image<T,true>>(m_Image);
		return new ImageObject(
			boost::static_pointer_cast<pcl::ImagePhysicalLayer<true>>(pcl::filter2::BinaryMorphology::Erode<pcl::Image<char,true>>(input, offset_list)),
			typeid(char)
		);
	}

	template <class T>
	ImageObject* actualGetBinaryClosing(const pcl::iterator::ImageNeighborIterator::OffsetListPointer& offset_list)
	{
		auto input = boost::static_pointer_cast<pcl::Image<T,true>>(m_Image);
		return new ImageObject(
			boost::static_pointer_cast<pcl::ImagePhysicalLayer<true>>(pcl::filter2::BinaryMorphology::Close<pcl::Image<char,true>>(input, offset_list)),
			typeid(char)
		);
	}

	template <class T>
	ImageObject* actualGetBinaryOpening(const pcl::iterator::ImageNeighborIterator::OffsetListPointer& offset_list)
	{
		auto input = boost::static_pointer_cast<pcl::Image<T,true>>(m_Image);
		return new ImageObject(
			boost::static_pointer_cast<pcl::ImagePhysicalLayer<true>>(pcl::filter2::BinaryMorphology::Open<pcl::Image<char,true>>(input, offset_list)),
			typeid(char)
		);
	}

	template <class T>
	ImageObject* actualGetDistanceTransform(boost::tuple<bool, bool, bool, const pcl::Region3D<int>>& param)
	{
		bool is_signed = param.get<0>();
		bool use_square = param.get<1>();
		bool use_spacing = param.get<2>();
		const pcl::Region3D<int>& output_region = param.get<3>();
		auto image = boost::static_pointer_cast<pcl::Image<T,true>>(m_Image);
		auto result = pcl::filter2::DistanceTransform::Compute<pcl::Image<float,true>>(image, is_signed, use_square, use_spacing, output_region);
		return new ImageObject(boost::static_pointer_cast<pcl::ImagePhysicalLayer<true>>(result), typeid(float));
	}

	template <class T>
	ImageObject* actualGetResampled(boost::tuple<const ImageObject&, bool, bool, double>& param)
	{
		auto template_image = param.get<0>().image();
		bool use_nearest_neighbor = param.get<1>(),
			use_out_value = param.get<2>();
		T out_value = static_cast<T>(param.get<3>());
		auto source_image = boost::static_pointer_cast<pcl::Image<T,true>>(m_Image);
		auto transform = pcl::ImageHelper::GetImageCoordinateTransformation(template_image, source_image);
		typename pcl::Image<T,true>::Pointer result;
		if (use_nearest_neighbor) {
			if (use_out_value) {
				auto interpolator = pcl::filter2::Helper::GetNearestNeighborInterpolator(
					pcl::filter2::Helper::GetFixedValueBoundary(source_image, out_value)
					);
				result = pcl::filter2::ImageResampleHelper::Resample<pcl::Image<T,true>>(interpolator, transform, false, template_image->getRegion());
			} else {
				auto interpolator = pcl::filter2::Helper::GetNearestNeighborInterpolator(
					pcl::filter2::Helper::GetZeroFluxBoundary(source_image)
					);
				result = pcl::filter2::ImageResampleHelper::Resample<pcl::Image<T,true>>(interpolator, transform, false, template_image->getRegion());
			}
		} else {
			if (use_out_value) {
				auto interpolator = pcl::filter2::Helper::GetTrilinearInterpolator(
					pcl::filter2::Helper::GetFixedValueBoundary(source_image, out_value)
					);
				result = pcl::filter2::ImageResampleHelper::Resample<pcl::Image<T,true>>(interpolator, transform, false, template_image->getRegion());
			} else {
				auto interpolator = pcl::filter2::Helper::GetTrilinearInterpolator(
					pcl::filter2::Helper::GetZeroFluxBoundary(source_image)
					);
				result = pcl::filter2::ImageResampleHelper::Resample<pcl::Image<T,true>>(interpolator, transform, false, template_image->getRegion());
			}
		}
		return new ImageObject(boost::static_pointer_cast<pcl::ImagePhysicalLayer<true>>(result), typeid(T));
	}

	template <class MapType, class KeyEqual=std::equal_to<typename MapType::key_type>>
	struct UnorderedMapInserter
	{
		MapType map;
		typename MapType::iterator iter;
		typename MapType::key_type prev_key;
		KeyEqual equal;

		UnorderedMapInserter()
		{}
		void init(const typename MapType::key_type& key)
		{
			iter = map.insert(std::make_pair(key,0)).first;
			prev_key = key;
		}
		void add(const typename MapType::key_type& key)
		{
			if (equal(key,prev_key)) iter->second += 1;
			else {
				iter = map.find(key);
				if (iter==map.end()) iter = map.insert(std::make_pair(key,1)).first;
				else iter->second += 1;
				prev_key = key;
			}
		}

	};

	template <class T>
	boost::unordered_map<double,long long> actualGetHistogram(const pcl::Region3D<int>& region)
	{
		UnorderedMapInserter<boost::unordered_map<double,long long>> result;
		auto img = boost::static_pointer_cast<pcl::Image<T,true>>(m_Image);
		pcl::ImageIterator iter(img);
		if (!region.empty()) iter.setRegion(region.getIntersect(img->getRegion()));
		iter.begin();
		result.init(img->get(iter));
		pcl_ForIterator(iter) result.add(img->get(iter));
		return result.map;
	}

	template <class T>
	boost::unordered_map<boost::tuple<double,double>,long long,ihash<double>,iequal_to<double>> actualGetJointHistogram(boost::tuple<const ImageObject&, const pcl::Region3D<int>&, double>& param)
	{
		auto obj = param.get<0>().m_Image;
		pcl::Region3D<int> needed_region = param.get<1>();
		if (needed_region.empty()) needed_region = m_Image->getRegion();
		double fill_value = param.get<2>();
		UnorderedMapInserter<boost::unordered_map<boost::tuple<double,double>,long long,ihash<double>,iequal_to<double>>,iequal_to<double>> result;
		result.init(boost::tuple<double,double>(fill_value, fill_value));

		auto self_img = boost::static_pointer_cast<pcl::Image<T,true>>(m_Image);
		pcl::Region3D<int> region(needed_region.getIntersect(obj->getRegion()));
		auto self_region = needed_region.getRegionsAfterSubtractionBy(region);
		auto obj_region = obj->getRegion().getRegionsAfterSubtractionBy(region);
		pcl::ImageIterator self_iter(self_img), obj_iter(obj);
		if (param.get<0>().m_Type==m_Type) {
			auto obj_img = boost::static_pointer_cast<const pcl::Image<T,true>>(obj);
			if (!region.empty()) {
				self_iter.setRegion(region);
				obj_iter.setRegion(region);
				pcl_ForIterator2(self_iter, obj_iter) {
					double val1 = self_img->get(self_iter),
						val2 = obj_img->get(obj_iter);
					result.add(boost::tuple<double,double>(val1, val2));
				}
			}
			pcl_ForEach(self_region, item) {
				self_iter.setRegion(*item);
				pcl_ForIterator(self_iter) {
					double val = self_img->get(self_iter);
					result.add(boost::tuple<double,double>(val, fill_value));
				}
			}
			pcl_ForEach(obj_region, item) {
				obj_iter.setRegion(*item);
				pcl_ForIterator(obj_iter) {
					double val = obj_img->get(obj_iter);
					result.add(boost::tuple<double,double>(fill_value, val));
				}
			}
		} else {
			if (!region.empty()) {
				self_iter.setRegion(region);
				obj_iter.setRegion(region);
				pcl_ForIterator2(self_iter, obj_iter) {
					double val1 = self_img->get(self_iter),
						val2 = obj->getValue(obj_iter);
					result.add(boost::tuple<double,double>(val1, val2));
				}
			}
			pcl_ForEach(self_region, item) {
				self_iter.setRegion(*item);
				pcl_ForIterator(self_iter) {
					double val = self_img->get(self_iter);
					result.add(boost::tuple<double,double>(val, fill_value));
				}
			}
			pcl_ForEach(obj_region, item) {
				obj_iter.setRegion(*item);
				pcl_ForIterator(obj_iter) {
					double val = obj->getValue(obj_iter);
					result.add(boost::tuple<double,double>(fill_value, val));
				}
			}
		}
		return result.map;
	}

	template <class CalcType, class SourceImagePointer, class MaskImagePointer>
	void populateCalculatorFast(CalcType& calc, SourceImagePointer src, MaskImagePointer mask)
	{
		auto region = src->getRegion().getIntersect(mask->getRegion());
		pcl::ImageIterator s_iter(src), m_iter(mask);
		s_iter.setRegion(region);
		m_iter.setRegion(region);
		pcl_ForIterator2(s_iter, m_iter) if (mask->get(m_iter)>0) {
			calc.addValue(src->get(s_iter));
		}
	}

	template <class CalcType, class SourceImagePointer, class MaskImagePointer>
	void populateCalculatorSlow(CalcType& calc, SourceImagePointer src, MaskImagePointer mask)
	{
		auto region = src->getRegion().getIntersect(mask->getRegion());
		pcl::ImageIterator s_iter(src), m_iter(mask);
		s_iter.setRegion(region);
		m_iter.setRegion(region);
		pcl_ForIterator2(s_iter, m_iter) if (mask->getValue(m_iter)>0) {
			calc.addValue(src->get(s_iter));
		}
	}

	template <class T>
	pcl::statistics::StatisticsCalculator* actualGetStatisticsCalculator(const ImageObject& mask)
	{
		auto calc = new pcl::statistics::StatisticsCalculator();
		auto img = boost::static_pointer_cast<pcl::Image<T,true>>(m_Image);
		if (mask.type()==typeid(char)) {
			auto mask_img = boost::static_pointer_cast<pcl::Image<char,true>>(mask.m_Image);
			populateCalculatorFast(*calc, img, mask_img);
		} else populateCalculatorSlow(*calc, img, mask.m_Image);
		return calc;
	}

	template <class T>
	pcl::statistics::PercentileCalculator<double>* actualGetPercentileCalculator(const ImageObject& mask)
	{
		auto calc = new pcl::statistics::PercentileCalculator<double>();
		auto img = boost::static_pointer_cast<pcl::Image<T,true>>(m_Image);
		if (mask.type()==typeid(char)) {
			auto mask_img = boost::static_pointer_cast<pcl::Image<char,true>>(mask.m_Image);
			populateCalculatorFast(*calc, img, mask_img);
		} else populateCalculatorSlow(*calc, img, mask.m_Image);
		return calc;
	}

	template <class T>
	ImageObject* actualFlip(int flip_axis)
	{
		auto img = boost::static_pointer_cast<pcl::Image<T,true>>(m_Image);
		auto result = pcl::Image<T,true>::New(img);
		pcl::ImageIterator iter(img), r_iter;
		switch(flip_axis) {
		case 0:
			r_iter.setImage(result, pcl::ImageIterator::RX, pcl::ImageIterator::Y, pcl::ImageIterator::Z);
			break;
		case 1:
			r_iter.setImage(result, pcl::ImageIterator::X, pcl::ImageIterator::RY, pcl::ImageIterator::Z);
			break;
		case 2:
			r_iter.setImage(result, pcl::ImageIterator::X, pcl::ImageIterator::Y, pcl::ImageIterator::RZ);
			break;
		default:
			pcl_ThrowException(pcl::Exception(), "Invalid flip axis "+boost::lexical_cast<std::string>(flip_axis)+" provided!");
		}

		pcl_ForIterator2(iter, r_iter) {
			result->set(r_iter, img->get(iter));
		}

		return new ImageObject(boost::static_pointer_cast<pcl::ImagePhysicalLayer<true>>(result), typeid(T));
	}

	template <class T>
	std::vector<double> actualComputeOriginalLongestAxialDiameter(char)
	{
		auto img = boost::static_pointer_cast<pcl::Image<T,true>>(m_Image);
		return pcl::measurement::GeometricalMeasurementHelper::ComputeOriginalLongestAxialDiameter(img);
	}

	template <class T>
	std::vector<double> actualComputeLongestAxialDiameter(char)
	{
		auto img = boost::static_pointer_cast<pcl::Image<T,true>>(m_Image);
		return pcl::measurement::GeometricalMeasurementHelper::ComputeLongestAxialDiameter(img);
	}

	template <class T>
	std::vector<double> actualComputeLongestDiameter(char)
	{
		auto img = boost::static_pointer_cast<pcl::Image<T,true>>(m_Image);
		return pcl::measurement::GeometricalMeasurementHelper::ComputeLongestDiameter(img);
	}
	/* //DISABLED, cmake compilation failed 
	template <class T>
	pcl::filter::GlcmFeatures* actualGetGlcmFeatures(boost::tuple<const ImageObject&, const std::vector<pcl::Point3D<int>>&, int, bool>& param)
	{
		auto obj = param.get<0>().m_Image;
		auto offsets = param.get<1>();
		auto level = param.get<2>();
		auto symmetric = param.get<3>();
		auto img = boost::static_pointer_cast<pcl::Image<T,true>>(m_Image);
		pcl::filter::GrayLevelCooccurrenceMatrix glcm(level, symmetric);
		pcl::ImageIteratorWithPoint iter(img);
		pcl_ForEach(offsets, item) {
			pcl::PointIndexObject pi((*item), img->localToIndex(*item)-img->localToIndex(0,0,0));
			pcl::filter::GlcmHelper::ComputeFromIteratorWithDynamicQuantization(glcm, img, iter, pi, 
				[&](const pcl::Point3D<int>& p, long i)->bool{
				return obj->getValue(p)>0;},
				[&](const pcl::Point3D<int>& p, long i)->bool{
				return obj->getValue(p)>0;}
			);
		}
		return new pcl::filter::GlcmFeatures(glcm);
	}
	*/
	template <class T>
	void actualModifyImage(boost::tuple<pcl::Point3D<int>, pcl::Point3D<double>, pcl::Point3D<double>, pcl::ImagePhysicalLayer<true>::OrientationMatrixType>& param)
	{
		m_Image = boost::static_pointer_cast<pcl::Image<T, true>>(m_Image)->getAlias(
			param.get<0>(), param.get<1>(), param.get<2>(), param.get<3>()
			);
	}

	template <class T>
	ImageObject* actualGetAlias(boost::tuple<pcl::Point3D<int>, pcl::Point3D<double>, pcl::Point3D<double>, pcl::ImagePhysicalLayer<true>::OrientationMatrixType>& param)
	{
		auto result = boost::static_pointer_cast<pcl::Image<T, true>>(m_Image)->getAlias(
			param.get<0>(), param.get<1>(), param.get<2>(), param.get<3>()
			);
		return new ImageObject(boost::static_pointer_cast<pcl::ImagePhysicalLayer<true>>(result), typeid(T));
	}

	template <class T>
	ImageObject* actualGetSubImage(boost::tuple<pcl::Point3D<int>, pcl::Point3D<int>>& param)
	{
		auto result = boost::static_pointer_cast<pcl::Image<T, true>>(m_Image)->getSubImage(
			param.get<0>(), param.get<1>()
			);
		return new ImageObject(boost::static_pointer_cast<pcl::ImagePhysicalLayer<true>>(result), typeid(T));
	}

	template <class T>
	ImageObject* actualGetWholeImage(int)
	{
		auto result = boost::static_pointer_cast<pcl::Image<T, true>>(m_Image)->getWholeImage();
		return new ImageObject(boost::static_pointer_cast<pcl::ImagePhysicalLayer<true>>(result), typeid(T));
	}
    
	template <class T>
	void* actualGetBuffer(int)
	{
		return (void*)(boost::static_pointer_cast<pcl::Image<T, true>>(m_Image)->getBuffer()->getPointer());
	}

	template <class T>
	unsigned long long actualGetBufferSize(int)
	{		
		long long byteSize = sizeof(T);
        auto image = boost::static_pointer_cast<pcl::Image<T, true>>(m_Image);
		return byteSize*image->getBufferSize().x() * image->getBufferSize().y() * image->getBufferSize().z();
	}
	
	template <class T>
	unsigned int actualGetVoxelSize(int)
	{
		return sizeof(T);
	}

};

#endif