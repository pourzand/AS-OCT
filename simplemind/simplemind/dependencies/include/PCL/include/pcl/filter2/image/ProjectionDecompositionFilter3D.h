#pragma once
#include <pcl/filter2/image/ImageFilterBase.h>
#include <pcl/iterator.h>
#include <math.h>

namespace pcl
{
	namespace filter2
	{

		template <class BoundaryType, class OutputImageType>
		class ProjectionDecompositionFilter3D: public ImageFilterBase
		{
		public:
			enum {
				GEOMETRIC,
				TEXTURE,
				NOISE
			};
		
			static std::vector<typename OutputImageType::Pointer> Compute(const BoundaryType& input, double delta, double epsilon, double lambda, int max, double mu, const std::vector<double>& tau, pcl::Region3D<int>& output_region=pcl::Region3D<int>().reset())
			{
				ProjectionDecompositionFilter3D filter;
				filter.setInput(input);
				filter.setDelta(delta);
				filter.setEpsilon(lambda);
				filter.setLambda(epsilon);
				filter.setMax(max); 
				filter.setMu(mu);
				filter.setTau(tau);
				filter.setOutputRegion(output_region);
				filter.update();
				std::vector<typename OutputImageType::Pointer> result(3);
				for (int i=0; i<3; ++i) result[i] = filter.getOutput(i);
				return result;
			}
			
			static std::vector<typename OutputImageType::Pointer> Compute(const BoundaryType& input, double delta, double max=55, pcl::Region3D<int>& output_region=pcl::Region3D<int>().reset())
			{
				ProjectionDecompositionFilter3D filter;
				filter.setInput(input);
				filter.setDelta(delta);
				filter.setMax(max);
				filter.setOutputRegion(output_region);
				filter.update();
				std::vector<typename OutputImageType::Pointer> result(3);
				for (int i=0; i<3; ++i) result[i] = filter.getOutput(i);
				return result;
			}

			ProjectionDecompositionFilter3D()
			{
				setDelta(50);
				setEpsilon(0.01);
				setLambda(1);
				setMax(55);
				setMu(450);
				setTau(std::vector<double>(50, 0.05));
			}
			
			void setDelta(double val)
			{
				m_Delta = val;
			}
			void setEpsilon(double val)
			{
				m_Epsilon = val;
			}
			void setLambda(double val)
			{
				m_Lambda = val;
			}
			void setMax(int val)
			{
				m_Max = val;
			}
			void setMu(double val)
			{
				m_Mu = val;
			}
			void setTau(const std::vector<double>& val)
			{
				m_Tau = val;
			}
			void setTau(std::vector<double>&& val)
			{
				m_Tau = std::move(val);
			}

			void setInput(const typename BoundaryType& input)
			{
				m_Input = input;
			}
			
			void update()
			{
				for (int i=0; i<3; ++i) {
					m_Output[i] = this->createImage<OutputImageType>(m_Input.getImage());
					pcl::ImageHelper::Fill(m_Output[i], 0);
				}
				pcl::ImageRegionsIteratorWithPoint<> input_iter(m_Input.getImage()), output_iter(m_Output[0]);
				{
					auto safe_region = m_Output[0]->getRegion().getIntersect(m_Input.getImage()->getRegion());
					auto unsafe_regions = m_Output[0]->getRegion().getRegionsAfterSubtractionBy(m_Input.getImage()->getRegion());
					input_iter.add(safe_region, 1); output_iter.add(safe_region, 1);
					input_iter.addList(unsafe_regions, 0); output_iter.addList(unsafe_regions, 0);
				}

				auto &u = m_Output[0],
					&v = m_Output[1],
					&w = m_Output[2];
				auto temp = OutputImageType::New(u);
				pcl::ImageIteratorWithPoint iter(u);
				double dif;
				int current = 0;
				do {
					double udif = 0, vdif = 0, wdif = 0;
					//PdeltaBE
					pcl_ForIterator2(input_iter, output_iter) temp->set(
						output_iter,
						(input_iter.getInfo() == 1 ? m_Input.getImage()->get(input_iter) : m_Input.get(input_iter.getPoint(), input_iter)) - u->get(output_iter) - v->get(output_iter)
						);
					temp = createPGd(m_Delta, temp, iter);
					pcl_ForIterator(iter) {
						wdif += pcl::abs(w->get(iter) - temp->get(iter));
						w->set(iter, temp->get(iter));
					}
					//PmuBG
					pcl_ForIterator2(input_iter, output_iter) temp->set(
						output_iter,
						(input_iter.getInfo() == 1 ? m_Input.getImage()->get(input_iter) : m_Input.get(input_iter.getPoint(), input_iter)) - u->get(output_iter) - w->get(output_iter)
						);
					temp = createPGd(m_Mu, temp, iter);
					pcl_ForIterator(iter) {
						vdif += pcl::abs(v->get(iter) - temp->get(iter));
						v->set(iter, temp->get(iter));
					}
					//PlambdaBG
					pcl_ForIterator2(input_iter, output_iter) temp->set(
						output_iter,
						(input_iter.getInfo() == 1 ? m_Input.getImage()->get(input_iter) : m_Input.get(input_iter.getPoint(), input_iter)) - v->get(output_iter) - w->get(output_iter)
						);
					temp = createPGd(m_Lambda, temp, iter);
					pcl_ForIterator2(input_iter, output_iter) {
						double val = (input_iter.getInfo() == 1 ? m_Input.getImage()->get(input_iter) : m_Input.get(input_iter.getPoint(), input_iter)) - v->get(output_iter) - w->get(output_iter) - temp->get(output_iter);
						udif += pcl::abs(u->get(output_iter) - val);
						u->set(output_iter, val);
					}

					//std::cout << udif << " " << vdif << " " << wdif << std::endl;

					//Calculate stopping
					dif = std::max(udif, vdif);
					dif = std::max(dif, wdif);
					++current;
				} while (current < m_Max && dif > m_Epsilon);
			}

			typename OutputImageType::Pointer getOutput(int i)
			{
				return m_Output[i];
			}

		protected:
			BoundaryType m_Input;
			typename OutputImageType::Pointer m_Output[3];
			std::vector<double> m_Tau;
			double m_Delta, m_Epsilon, m_Lambda, m_Mu;
			int m_Max;
			
			typename OutputImageType::Pointer createPGd(double param, typename OutputImageType::Pointer f, pcl::ImageIteratorWithPoint& iter)
			{
				auto p1 = OutputImageType::New(f),
					p2 = OutputImageType::New(f),
					p3 = OutputImageType::New(f),
					p = OutputImageType::New(f);
					
				pcl_ForIterator(iter) {
					f->set(iter, -1*f->get(iter)/param);
					p1->set(iter, 0);
					p2->set(iter, 0);
					p3->set(iter, 0);
				}

				auto divp = OutputImageType::New(f);
				pcl_ForEach(m_Tau, tau) {
					auto prevp1 = pcl::ImageHelper::GetCopyAuto(p1),
						prevp2 = pcl::ImageHelper::GetCopyAuto(p2),
						prevp3 = pcl::ImageHelper::GetCopyAuto(p3);
					div(p1, p2, p3, divp, iter);

					pcl_ForIterator(iter) divp->set(iter, divp->get(iter)+f->get(iter));
					gradient(divp, p1, p2, p3);

					pcl_ForIterator(iter) {
						double p1val = p1->get(iter);
						double p2val = p2->get(iter);
						double p3val = p3->get(iter);
						double denom = 1 + *tau * sqrt(p1val*p1val + p2val*p2val + p3val*p3val);

						p1val = (prevp1->get(iter) + *tau * p1val) / denom;
						p2val = (prevp2->get(iter) + *tau * p2val) / denom;
						p3val = (prevp3->get(iter) + *tau * p3val) / denom;
						p1->set(iter, p1val);
						p2->set(iter, p2val);
						p3->set(iter, p3val);
					}

					div(p1, p2, p3, p, iter);
					pcl_ForIterator(iter) p->set(iter, param*p->get(iter));
				}

				return p;
			}
			
			void div(typename OutputImageType::Pointer& in1, typename OutputImageType::Pointer& in2, typename OutputImageType::Pointer& in3, typename OutputImageType::Pointer& out, pcl::ImageIteratorWithPoint& iter)
			{
				//std::cout << "div start" << std::endl;
				auto minp = in1->getMinPoint(),
					maxp = in1->getMaxPoint();
				double a, b, c;
				long x_offset = -in1->getOffsetTable()[0], 
					y_offset = -in1->getOffsetTable()[1],
					z_offset = -in1->getOffsetTable()[2];
				double z_step = m_Input.getImage()->getSpacing()[0] / m_Input.getImage()->getSpacing()[2];
				pcl_ForIterator(iter) {
					if (iter.getPoint()[0]==minp[0]) a = in1->get(iter);
					else if (iter.getPoint()[0]==maxp[0]) a = -in1->get(iter + x_offset);
					else a = in1->get(iter) - in1->get(iter + y_offset);
					
					if (iter.getPoint()[1]==minp[1]) b = in2->get(iter);
					else if (iter.getPoint()[1]==maxp[1]) b = -in2->get(iter + y_offset);
					else b = in2->get(iter) - in2->get(iter + y_offset);

					if (iter.getPoint()[2]==minp[2]) c = in3->get(iter);
					else if (iter.getPoint()[2]==maxp[2]) c = -in3->get(iter + z_offset);
					else c = in2->get(iter) - in3->get(iter + z_offset);
					
					out->set(iter, a + b + c/(z_step));
				}
				//std::cout << "div end" << std::endl;
			}
			
			void gradient(typename OutputImageType::Pointer& in, typename OutputImageType::Pointer& out1, typename OutputImageType::Pointer& out2, typename OutputImageType::Pointer& out3)
			{
				//std::cout << "gradient start" << std::endl;
				long x_offset = in->getOffsetTable()[0],
					y_offset = in->getOffsetTable()[1],
					z_offset = in->getOffsetTable()[2];
					
					
				pcl::ImageHelper::Fill(out1, 0);
				pcl::ImageHelper::Fill(out2, 0);
				pcl::ImageHelper::Fill(out3, 0);
				pcl::ImageIterator iter(in);
				{
					auto region = in->getRegion();
					region.getMaxPoint()[0] -= 1;
					iter.setRegion(region);
					pcl_ForIterator(iter) out1->set(iter, in->get(iter+x_offset) - in->get(iter));
				}
				{
					auto region = in->getRegion();
					region.getMaxPoint()[1] -= 1;
					iter.setRegion(region);
					pcl_ForIterator(iter) out2->set(iter, in->get(iter+y_offset) - in->get(iter));
				}
				{
					auto region = in->getRegion();
					region.getMaxPoint()[2] -= 1;
					iter.setRegion(region);
					pcl_ForIterator(iter) out3->set(iter, in->get(iter+z_offset) - in->get(iter));
				}
				//std::cout << "gradient end" << std::endl;
			}
		};

	}
}
