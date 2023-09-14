#ifndef GLCM_FEATURES
#define GLCM_FEATURES

#include <pcl/misc/Timing.h>
#include <pcl/geometry/Point.h>

#include <pcl/filter/glcm/GrayLevelCooccurrenceMatrix.h>
#include <algorithm>
#include <vector>
#include <math.h>
#include <vnl/algo/vnl_real_eigensystem.h>

#define JAVA_EPSILON 0.000000001

namespace pcl
{
	namespace filter
	{
        /**
         * Encapsulates the computation of GLCM features from a graylevel cooccurrence matrix.
         */
		class GlcmFeatures
		{
		public:
            /**
             * Copy constructor.
             */
			GlcmFeatures(const GrayLevelCooccurrenceMatrix& glcm)
			{
				m_Num = glcm.getNum();
				glcm.getNormalizedMatrix(m_Glcm);
			}

            /**
             * Returns a reference to the underlying matrix object.
             */
			const vnl_matrix<double>& getMatrix() const
			{
				return m_Glcm;
			}

			long getNum() const
			{
				return m_Num;
			}

            /**
             * Returns the GLCM contrast, also known as "sum of squares variance."
             * Defined as Sum[Pij * (i-j)^2]
             * Recall t hat (i-j) represents neighboring pixels whose graylevel values differ by (i-j).
             * Hence, contrast increases if neighboring pixels are very different from each other.
             */
			double getContrast()
			{
				if (!contrast.computed) computeFeatures();
				return contrast.value;
			}

			/**
			* Returns the dissimilarity for the GLCM.
			* Defined as Sum[Pij * |i-j|]
			* Similar to contrast, but increases linearly with difference rather than quadradically.
			*/
			double getDissimilarity()
			{
				if (!dissimilarity.computed) computeFeatures();
				return dissimilarity.value;
			}

			/**
			* Returns the homogeneity for the GLCM. Also called "inverse difference moment".
			* Defined as Sum[Pij / (1 + (i-j)^2)]
			* Essentially the inverse of contrast, decreasing if neighboring pixels are different.
			*/
			double getHomogeneity()
			{
				if (!homogeneity.computed) computeFeatures();
				return homogeneity.value;
			}

			/**
			* Returns the angular second moment for the GLCM.
			* Defined as Sum[Pij^2]
			* Increases if image is very orderly, i.e. the same neighboring relationship
			* occurs often in the image. This is reflected in the glcm by a few high-value elements.
			*/
			double getAngularSecondMoment()
			{
				if (!angularSecondMoment.computed) computeFeatures();
				return angularSecondMoment.value;
			}

			/**
			* Returns the energy for the GLCM. Also called "uniformity".
			* Equal to sqrt(angular second moment)
			*/
			double getEnergy()
			{
				if (!energy.computed) energy = sqrt(getAngularSecondMoment());
				return energy.value;
			}

			/**
			* Returns the maximum value of the GLCM.
			* This feature is interesting only because the GLCM is normalized.
			* A high value for max occurs if the same neighboring pixel relationship
			* occurs often in the image.
			*/
			double getMax()
			{
				if (!max.computed) computeFeatures();
				return max.value;
			}

			/**
			* Returns the minimum value of the GLCM.
			* Unlike max, this isn't very interesting. Will often have a value of 0
			* because it only takes the absence of one pixel relationship to have
			* a 0 in the GLCM.
			*/
			double getMin()
			{
				if (!min.computed) computeFeatures();
				return min.value;
			}

			/**
			* Returns the entropy for the GLCM.
			* Defined as Sum[-Pij * log[Pij]], taking 0 * log(0) = 0.
			* Increases if image is very chaotic, i.e. many different neighboring relationships
			* occur with similar frequency. This is reflected in GLCM by having many low-value
			* elements with little or no "spiking".
			*/
			double getEntropy()
			{
				if (!entropy.computed) computeFeatures();
				return entropy.value;
			}

			/**
			* Returns the mean gray level value from the GLCM.
			* Defined as Sum[Pij * i].
			* Similar but not identical to the gray value mean from the image. Counts the number
			* of times each pixel value i contributes to a neighboring relationship. Because of
			* the way the GLCM is defined, each pixel is counted once if it is a border pixel
			* (with respect to the direction used to compute the GLCM) and twice if it is
			* an interior pixel.
			*
			* Note that in general, this formula represents the mean i, and a corresponding mean j
			* may be computed as Sum[Pij * j]. However due to the symmetry of the GLCM, these
			* values are identical.
			*/
			double getMean()
			{
				if (!mean.computed) computeFeatures();
				return mean.value;
			}

			/**
			* Returns the variance of gray level values from the GLCM.
			* Defined as Sum[Pij * (i-mean)^2]
			* See the discussion under getMean() for how this value differs from the variance
			* of gray level values from the image.
			*
			* Similarly, this formula represents variance in i, which is identical to the variance
			* in j due to the symmetry of GLCM.
			*/
			double getVariance()
			{
				if (!variance.computed) computeVariance();
				return variance.value;
			}

			double getStandardDeviation()
			{
				if (!standardDeviation.computed) standardDeviation = sqrt(getVariance());
				return standardDeviation.value;
			}

			/**
			* Returns the correlation for the GLCM. If the image is uniform, then returns 1.0.
			* Defined as Sum[Pij * (i - mean_i) * (j - mean_j) / Sqrt[variance_i * variance_j]
			* Due to symmetry, this formula collapses to
			*   Sum[Pij * (i - mean) * (j - mean) / variance
			* Measures the linear dependency of gray level values on those of neighboring pixels.
			*/
			double getCorrelation()
			{
				if (!correlation.computed) computeCorrelation();
				return correlation.value;
			}


			// Miscellaneous
			double getSumAverage()
			{
				if (!sumAverage.computed) computeSumFeatures();
				return sumAverage.value;
			}

			double getSumVariance()
			{
				if (!sumVariance.computed) computeSumFeatures();
				return sumVariance.value;
			}

			double getSumEntropy()
			{
				if (!sumEntropy.computed) computeSumFeatures();
				return sumEntropy.value;
			}

			double getDiffAverage()
			{
				if (!diffAverage.computed) computeDiffFeatures();
				return diffAverage.value;
			}

			double getDiffVariance()
			{
				if (!diffVariance.computed) computeDiffFeatures();
				return diffVariance.value;
			}

			double getDiffEntropy()
			{
				if (!diffEntropy.computed) computeDiffFeatures();
				return diffEntropy.value;
			}

			double getInfoCorrelationA()
			{
				if (!infoCorrelationA.computed) computeInfoCorrelations();
				return infoCorrelationA.value;
			}

			double getInfoCorrelationB()
			{
				if (!infoCorrelationB.computed) computeInfoCorrelations();
				return infoCorrelationB.value;
			}

			double getMaximalCorrelationCoefficient()
			{
				if (!maximalCorrelationCoefficient.computed) computeMaximalCorrelationCoefficient();
				return maximalCorrelationCoefficient.value;
			}
			
			double getInverseDifferenceMoment()
			{
				if (!idm.computed) computeInverseDifferenceMoment();
				return idm.value;
			}
			
			double getJavaContrast()
			{
				if (!javaContrast.computed) computeJavaFeatures();
				return javaContrast.value;
			}
			double getJavaEntropy()
			{
				if (!javaEntropy.computed) computeJavaFeatures();
				return javaEntropy.value;
			}
			double getJavaSumAverage()
			{
				if (!javaSumAverage.computed) computeJavaFeatures();
				return javaEntropy.value;
			}
			double getJavaSumVariance()
			{
				if (!javaSumVariance.computed) computeJavaFeatures();
				return javaSumVariance.value;
			}
			double getJavaSumEntropy()
			{
				if (!javaSumEntropy.computed) computeJavaFeatures();
				return javaSumEntropy.value;
			}
			double getJavaDiffVariance()
			{
				if (!javaDiffVariance.computed) computeJavaFeatures();
				return javaDiffVariance.value;
			}
			double getJavaDiffEntropy()
			{
				if (!javaDiffEntropy.computed) computeJavaFeatures();
				return javaDiffEntropy.value;
			}
			double getJavaInfoCorrelationA()
			{
				if (!javaInfoCorrelationA.computed) computeJavaFeatures();
				return javaInfoCorrelationA.value;
			}
			double getJavaInfoCorrelationB()
			{
				if (!javaInfoCorrelationB.computed) computeJavaFeatures();
				return javaInfoCorrelationB.value;
			}

		protected:
			long m_Num;
			template <class T>
			struct Result
			{
				T value;
				bool computed;

				Result()
				{
					computed = false;
				}

				Result& operator=(const T& v)
				{
					value = v;
					computed = true;
				}

				Result& operator=(T&& v)
				{
					value = std::move(v);
					computed = true;
					return *this;
				}
			};

			vnl_matrix<double> m_Glcm;

			//// Contrast features //////////////////////////////////////
			Result<double> contrast;              // aka sum of squares variance
			Result<double> javaContrast;
			Result<double> dissimilarity;
			Result<double> homogeneity;           // aka inverse difference moment

			//// Orderliness features ///////////////////////////////////
			Result<double> angularSecondMoment;
			Result<double> energy;                // aka uniformity
			Result<double> max;
			Result<double> min;
			Result<double> entropy;
			Result<double> javaEntropy;

			//// Descriptive features ///////////////////////////////////
			Result<double> mean;
			Result<double> variance;
			Result<double> standardDeviation;
			Result<double> correlation;

			//// Other features from Haralick 1973 //////////////////////
			Result<double> sumAverage;
			Result<double> javaSumAverage;
			Result<double> sumVariance;
			Result<double> javaSumVariance;
			Result<double> sumEntropy;
			Result<double> javaSumEntropy;
			Result<double> diffAverage;
			Result<double> idm;
			Result<double> diffVariance;
			Result<double> javaDiffVariance;
			Result<double> diffEntropy;
			Result<double> javaDiffEntropy;
			Result<double> infoCorrelationA;
			Result<double> infoCorrelationB;
			Result<double> javaInfoCorrelationA;
			Result<double> javaInfoCorrelationB;
			Result<double> maximalCorrelationCoefficient;

			Result<std::vector<double>> marginalX;
			Result<std::vector<double>> marginalY;
			Result<std::vector<double>> marginalXplusY;
			Result<std::vector<double>> marginalXminusY;

			std::vector<double>& getMarginalX()
			{
				if (!marginalX.computed) computeMarginals();
				return marginalX.value;
			}

			std::vector<double>& getMarginalY()
			{
				if (!marginalY.computed) computeMarginals();
				return marginalY.value;
			}

			std::vector<double>& getMarginalXplusY()
			{
				if (!marginalXplusY.computed) computeMarginals();
				return marginalXplusY.value;
			}

			std::vector<double>& getMarginalXminusY()
			{
				if (!marginalXminusY.computed) computeMarginals();
				return marginalXminusY.value;
			}

			/**
			* Source: http://www.fp.ucalgary.ca/mhallbey/correlation.htm
			*/
            void computeFeatures()
			{
				int i, j;
				double value;

				int imj;            // i - j
				int imj2;           // (i - j)^2
				double value2;      // value^2

				contrast = 0.0;
				dissimilarity = 0.0;
				homogeneity = 0.0;
				angularSecondMoment = 0.0;
                mean = 0.0;
				max = -std::numeric_limits<double>::infinity();
				min = std::numeric_limits<double>::infinity();
				entropy = 0.0;


				for (i = 0; i < m_Glcm.rows(); i++)
				{
					for (j = 0; j < m_Glcm.columns(); j++)
					{
						value = m_Glcm(i, j);

						// Helper variables
						imj = i - j;
						imj2 = imj * imj;
						value2 = value * value;

						contrast.value += value * imj2;
						dissimilarity.value += value * pcl::abs(imj);
						homogeneity.value += value / (1 + imj2);
						angularSecondMoment.value += value2;
                        mean.value += i * value;
						if (value > max.value) max.value = value;
						if (value < min.value) min.value = value;
						if (value != 0.0) entropy.value -= value * std::log(value);
					}
				}

				energy = sqrt(angularSecondMoment.value);        // move this to getEnergy()
			}
			
			void computeJavaFeatures() 
			{
				int N = m_Glcm.rows();
				{
					double entr = 0;
					for(int i = 0; i < N; i++)
						for(int j = 0; j < N; j++)
							entr += m_Glcm(i,j) * log(m_Glcm(i,j) + JAVA_EPSILON);
					javaEntropy.value = -entr;
				}
				
				{
					auto &p = getMarginalXplusY();
					double savg = 0, svar = 0, entr = 0;
					for(int i = 0; i < N; i++) {
						savg += (i + 2) * p[i];
						entr += p[i] * log(p[i] + JAVA_EPSILON);
					}
					for(int i = 0; i < N; i++) svar += pcl::square(i + 2 - savg) * p[i];
					javaSumAverage.value = savg;
					javaSumEntropy.value = -entr;
					javaSumVariance.value = svar;
				}
				
				{
					double con = 0;
					for(int k = 0; k < (N - 1); k++)
						for(int i = 0; i < N; i++)
							for(int j = 0; j < i; j++)
								if(pcl::abs(i - j) == k) con += k*k * m_Glcm(i,j);
					con *= 2;
					javaContrast.value = con;
				}
				
				{
					double ave = getDiffAverage();
					auto &p = getMarginalXminusY();
					double var = 0, s = 0, e = 0;
					double entr = 0;
					for(int i = 0; i < N; i++) {
						s = p[i] - ave;
						e += s;
						var += s*s;
						
						entr += p[i] * std::log(p[i] + JAVA_EPSILON);
					}
					javaDiffVariance.value = (var - (e*e)/N) / N;
					javaDiffEntropy.value = -entr;
				}
				
				{
					auto &px = getMarginalX();
					auto &py = getMarginalY();
					double infocor= 0;
					double hx = 0, hxy = 0, hxy1 = 0, hxy2 = 0;
					for(int i = 0; i < N; i++) {
						hx -= px[i] * std::log(px[i] + JAVA_EPSILON);
						for(int j = 0; j < N; j++) {
							hxy -= m_Glcm(i,j) * std::log(m_Glcm(i,j) + JAVA_EPSILON);
							hxy1 -= m_Glcm(i,j) * std::log(px[i] * px[j] + JAVA_EPSILON);
							hxy2 -= px[i] * px[j] * std::log(px[i] * px[j] + JAVA_EPSILON);
						}
					}
					javaInfoCorrelationA.value = (hxy - hxy1) / hx;
					javaInfoCorrelationB.value = sqrt(pcl::abs(1 - exp(-2.0 * (hxy2 - hxy))));
				}
			}
			
			void computeInverseDifferenceMoment() 
			{
				idm = 0.0;
				for(int i = 0; i < m_Glcm.rows(); i++)
				  for(int j = 0; j < m_Glcm.columns(); j++)
					idm.value += m_Glcm(i, j) / (1 + std::pow(double(i - j), 2));
			}

			/**
			* Source: http://www.fp.ucalgary.ca/mhallbey/correlation.htm
			*/
			void computeVariance()
			{
				standardDeviation.computed = false;
				variance = 0.0;
				double mean = getMean();
				for (int i = 0; i < m_Glcm.rows(); i++)
					for (int j = 0; j < m_Glcm.columns(); j++)
						variance.value += (i - mean) * (i - mean) * m_Glcm(i, j);
			}

			/**
			* Source: http://www.fp.ucalgary.ca/mhallbey/correlation.htm
			*/
			void computeCorrelation()
			{
				// Actual formula is Sum[wij * Pij], where
				//   wij = (i - mean_i) * (j - mean_j) / Sqrt[variance_i * variance_j]
				// But due to symmetry of GLCM, mean_i = mean_j and variance_i = variance_j

				// In Haralick 1973, this formula is presented as
				//   (Sum[i * j * p(i,j)] - mean_x * mean_y) / (stdev_x  * stdev_y)
				// It's trivial to show that this is equivalent to above.
				correlation = 0.0;
				double mean = getMean(),
					variance = getVariance();
				for (int i = 0; i < m_Glcm.rows(); i++)
					for (int j = 0; j < m_Glcm.columns(); j++)
						correlation.value += m_Glcm(i, j) * (i - mean) * (j - mean) / variance;
			}

			/**
			* Computes the horizontal, vertical, and diagonal marginal probability vectors.
			*   p_x(i) = Sum[p(i,j), {j: 0->N-1}]
			*   p_y(j) = Sum[p(i,j), {i: 0->N-1}]
			*   p_x+y(k) = Sum[p(i,j), {i,j: 1->N, i+j=k}]
			*   p_x-y(k) = Sum[p(i,j), {i,j: 1->N, |i-j|=k}]
			* where p(i,j) is the GLCM
			*       N is the number of gray-levels (equal to the number of rows/columns of the GLCM)
			*       k refers to a diagonal
			*
			* Source: Haralick 1973
			*/
			void computeMarginals()
			{
				int i, j;
				int N;

				N = m_Glcm.rows();           // Should be equal to getNumColumns()

				marginalX = std::vector<double>(N,0);
				marginalY = std::vector<double>(N,0);
				marginalXplusY = std::vector<double>(2 * N - 1, 0);
				marginalXminusY = std::vector<double>(N,0);

				for (i = 0; i < N; i++)
					for (j = 0; j < N; j++) {
						double val = m_Glcm(i,j);
						marginalX.value[i] += val;
						marginalY.value[j] += val;         // Due to symmetry, marginalX = marginalY
						marginalXplusY.value[i + j] += val;
						marginalXminusY.value[pcl::abs(i - j)] += val;
					}
			}

			/**
			* Computes average, variance, and entropy features from positive diagonal vector p_x+y
			*
			* Source: Haralick 1973
			*/
			void computeSumFeatures()
			{
                int N = 2 * m_Glcm.rows() - 1;
				auto &p = getMarginalXplusY();

				// Compute average
				sumAverage = 0.0;
				for (int k = 0; k < N; k++) {
					sumAverage.value += k * p[k];
				}

				// Compute variance, entropy
				sumVariance = 0.0;
				sumEntropy = 0.0;
				for (int k = 0; k < N; k++) {
					sumVariance.value += (k - sumAverage.value) * (k - sumAverage.value) * p[k];
					if (p[k] != 0) sumEntropy.value -= p[k] * log(p[k]);
				}
			}

			/**
			* Computes average, variance, and entropy features from negative diagonal vector p_x-y
			*
			* Source: Haralick 1973
			*/
			void computeDiffFeatures()
			{
				int N = m_Glcm.rows();
				auto &p = getMarginalXminusY();

				// Compute average, entropy
				diffAverage = 0.0;
				diffEntropy = 0.0;
				for (int k = 0; k < N; k++) {
					diffAverage.value += k * p[k];
					if (p[k] != 0) diffEntropy.value -= p[k] * log(p[k]);
				}

				// Compute variance
				double temp = 0.0;
				for (int k = 0; k < N; k++)
					temp += k * k * p[k];
				diffVariance = temp - diffAverage.value * diffAverage.value;
			}

			/*
			* Computes the info correlation coefficients A and B.
			*
			* Source: Haralick 1973
			*/
			void computeInfoCorrelations()
			{
				double h, hxy, hxy1, hxy2;
				double logpxpy;

				auto &px = getMarginalX();
				auto &py = getMarginalY();

				// h = max{entropy of marginal p(x), entropy of marginal p(y)}
				// but p(x) = p(y) so...
				h = 0.0;
				for (int i = 0; i < m_Glcm.rows(); i++)
					if (px[i] != 0.0)
						h -= px[i] * log(px[i]);

				// hxy = entropy of glcm
				hxy = getEntropy();

				// hxy1 = -Sum[p(i,j) * log[px(i) * py(j)]
				// hxy2 = -Sum[px(i) * py(j) * log[px(i) * py(j)]
				hxy1 = 0.0;
				hxy2 = 0.0;
				for (int i = 0; i < m_Glcm.rows(); i++)
					for (int j = 0; j < m_Glcm.columns(); j++) {
						logpxpy = log(px[i] * py[j]);
						if (m_Glcm(i, j) != 0.0) {             // If p(i,j) != 0, then px(i) and py(j) are guaranteed to be nonzero also
							hxy1 -= m_Glcm(i, j) * logpxpy;
						}
						if ((px[i] != 0.0) && (py[j] != 0.0)) {
							hxy2 -= px[i] * py[j] * logpxpy;
						}
					}

					infoCorrelationA = (hxy - hxy1) / h;
					if (hxy > hxy2) std::cout << "HXY > HXY2 IN GlcmFeatures.computeInfoCorrelations()" << std::endl;
					infoCorrelationB = sqrt(1 - exp(-2 * pcl::abs(hxy2 - hxy)));
			}

			/*
			* Computes the maximal correlation coefficient
			*
			* Source: Haralick 1973
			*/
			void computeMaximalCorrelationCoefficient()
			{
				vnl_matrix<double> matrix(m_Glcm.rows(), m_Glcm.columns());
				double value;
				auto &px = getMarginalX();
				auto &py = getMarginalY();

				for (int i = 0; i < m_Glcm.rows(); i++) {
					for (int j = 0; j < m_Glcm.columns(); j++) {
						value = 0;
						for (int k = 0; k < m_Glcm.rows(); k++) {
							if ((px[i] != 0) && (py[k] != 0)) {
								value += m_Glcm(i, k) * m_Glcm(j, k) / (px[i] * py[k]);
							} else {
								value = 0;
							}
						}
						matrix(i, j) = value;
					}
				}

				vnl_real_eigensystem eig(matrix);
				std::vector<double> val(m_Glcm.rows());
				//for (int i=0; i<m_Glcm.rows(); i++) val[i] = eig.Vreal(i,i);          // Vreal returns the wrong results
                for (int i=0; i<m_Glcm.rows(); i++) val[i] = eig.D(i,i).real();         // We return the real components of the eigenvectors
				std::sort(val.begin(), val.end());

                /*
                // Debug text
                std::cout << "computeMaximalCorrelationCoefficient()" << std::endl;
                std::cout << matrix << std::endl << "  ";
                for (int i = 0; i < m_Glcm.rows(); i++)
                    std::cout << (val[i]) << " ";
                std::cout << std::endl;
                */

                maximalCorrelationCoefficient = std::sqrt(val[val.size()-2]);
			}
		};
	}
}

#endif