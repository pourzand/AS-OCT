#ifndef PCL_COD_IF_ELSE
#define PCL_COD_IF_ELSE

#include <pcl/cod/CodBase.h>

namespace pcl
{
	namespace cod
	{
		using namespace pcl;

		template <class CondType, class IfType, class ElseType, class RType>
		class CodIfElse: public CodBase<CondType::ReferableViaIndex && IfType::ReferableViaIndex && ElseType::ReferableViaIndex>
		{
		public:
			typedef CodIfElse Self;
			typedef boost::shared_ptr<Self> Pointer;
			typedef RType ReturnType;

			static Pointer New(const typename CondType::Pointer& cond, const typename IfType::Pointer& if_obj, const typename ElseType::Pointer& else_obj)
			{
				if (!cond->isCompatible(if_obj) || !cond->isCompatible(else_obj) || !if_obj->isCompatible(else_obj)) {
					pcl_ThrowException(IncompatibleCodException(), "Incompatible COD objects provided");
				}
				Pointer result(new Self);
				result->m_Cond = cond;
				result->m_If = if_obj;
				result->m_Else = else_obj;
				result->setRegionInfo();
				if (!cond->isUnbounded()) result->setRegionInfo(cond);
				else if (!if_obj->isUnbounded()) result->setRegionInfo(if_obj);
				else if (!else_obj->isUnbounded()) result->setRegionInfo(else_obj);
				return result;
			}

			template <class IteratorType>
			ReturnType get(const IteratorType& iter) const 
			{
				if (m_Cond->get(iter)) return m_If->get(iter);
				else return m_Else->get(iter);
			}
			
			ReturnType get(const Point3D<int>& p) const 
			{
				if (m_Cond->get(p)) return m_If->get(p);
				else return m_Else->get(p);
			}

			ReturnType get(const Point3D<int>& p, long i) const
			{
				if (m_Cond->get(p,i)) return m_If->get(p,i);
				else return m_Else->get(p,i);
			}
			
			DummyImage::ConstantPointer getTemplateImage() const
			{
				DummyImage::ConstantPointer template_image;
				//Prioritized bounded object first
				if (!m_Cond->isUnbounded()) {
					template_image = m_Cond->getTemplateImage();
				} else if (!m_If->isUnbounded()) {
					template_image = m_If->getTemplateImage();
				} else if (!m_Else->isUnbounded()) {
					template_image = m_Else->getTemplateImage();
				}
				if (template_image) return template_image;
				//Prioritized object that are referable via index
				if (CondType::ReferableViaIndex) {
					template_image = m_Cond->getTemplateImage();
				} else if (IfType::ReferableViaIndex) {
					template_image = m_If->getTemplateImage();
				} else if (ElseType::ReferableViaIndex) {
					template_image = m_Else->getTemplateImage();
				}
				if (template_image) return template_image;
				//Last resort
				template_image = m_Cond->getTemplateImage();
				if (template_image) return template_image;
				template_image = m_If->getTemplateImage();
				if (template_image) return template_image;
				template_image = m_Else->getTemplateImage();
				return template_image;
			}

		protected:
			typename CondType::Pointer m_Cond;
			typename IfType::Pointer m_If;
			typename ElseType::Pointer m_Else;

			CodIfElse() {};
		};

	}
}

#endif