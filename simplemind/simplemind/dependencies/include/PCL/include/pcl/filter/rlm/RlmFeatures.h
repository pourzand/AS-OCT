#ifndef RLM_FEATURES
#define RLM_FEATURES

#include <pcl/filter/rlm/RunLengthMatrix.h>
#include <algorithm>
#include <vector>
#include <math.h>
//#include <vnl/algo/vnl_real_eigensystem.h>

namespace pcl
{
	namespace filter
	{
        /**
         * Encapsulates the computation of RLM texture features from a run length matrix.
         */
		class RlmFeatures
		{
		public:
            /**
             * Copy constructor.
             */
			RlmFeatures(const RunLengthMatrix& rlm)
			{
				m_rlm = rlm.getMatrix();
			}

            /**
             * Returns a reference to the underlying matrix.
             */
			const vnl_matrix<double>& getMatrix() const
			{
				return m_rlm;
			}

            /**
             * Returns the number of graylevel bins used to generate the RLM.
             * Equal to the number of rows in the RLM.
             */
            int getNumGraylevels() const
            {
                return m_rlm.rows();
            }

            /**
             * Returns the maximum runlength encountered while building the RLM.
             * Equal to one minus the number of columns in the RLM (since column 0 is a dummy column).
             */
            int getMaxRunLength() const
            {
                return m_rlm.cols() - 1;
            }

            //// F E A T U R E  A C C E S S O R S ///////////////////////////////////////////
            /**
             * Returns the short run emphasis (SRE) for the RLM.
             * According to Galloway1975, SRE is defined as Sum[Pij / j^2] / N,
             *   where i = gray level (bin number), j = run length, and N = total number of runs (normalizing factor).
             * The weighting factor 1/j^2 gives decreasing weight as the length of the run increases.
             */
            double getShortRunEmphasis()
            {
                if (!shortRunEmphasis.computed) computeFeatures();
                return shortRunEmphasis.value;
            }

            /**
             * Returns the long run emphasis (LRE) for the RLM.
             * According to Galloway1975, LRE is defined as Sum[Pij * j^2] / N,
             *   where i = gray level (bin number), j = run length, and N = total number of runs (normalizing factor).
             * The weighting factor j^2 gives increasing weight as the length of the run increaess.
             */
            double getLongRunEmphasis()
            {
                if (!longRunEmphasis.computed) computeFeatures();
                return longRunEmphasis.value;
            }

            /**
             * Returns the low graylevel run emphasis (LGRE) for the RLM.
             * According Chu1990, LGRE is defined as Sum[Pij / i^2] / N,
             *   where i = gray level (bin number), j = run length, and N = total number of runs (normalizing factor).
             * The weighting factor 1/i^2 gives decreasing weight with increasing graylevel value.
             */
            double getLowGraylevelRunEmphasis()
            {
                if (!lowGraylevelRunEmphasis.computed) computeFeatures();
                return lowGraylevelRunEmphasis.value;
            }

            /**
             * Returns the high graylevel run emphasis (HGRE) for the RLM.
             * According Chu1990, HGRE is defined as Sum[Pij * i^2] / N,
             *   where i = gray level (bin number), j = run length, and N = total number of runs (normalizing factor).
             * The weighting factor i^2 gives increasing weight with increasing graylevel value.
             */
            double getHighGraylevelRunEmphasis()
            {
                if (!highGraylevelRunEmphasis.computed) computeFeatures();
                return highGraylevelRunEmphasis.value;
            }

            /**
             * Returns the short run low graylevel emphasis (SRLGE) for the RLM
             * According to Dasarathy1991, SRLGE is defined as Sum[Pij / i^2 / j^2] / N,
             *   where i = gray level (bin number), j = run length, and N = total number of runs (normalizing factor).
             * The weighting factors 1/i^2 and 1/j^2 give decreasing weight to higher graylevel values and
             * long runlengths, respectively.
             */
            double getShortRunLowGraylevelEmphasis()
            {
                if (!shortRunLowGraylevelEmphasis.computed) computeFeatures();
                return shortRunLowGraylevelEmphasis.value;
            }

            /**
             * Returns the short run high graylevel emphasis (SRHGE) for the RLM
             * According to Dasarathy1991, SRHGE is defined as Sum[Pij * i^2 / j^2] / N,
             *   where i = gray level (bin number), j = run length, and N = total number of runs (normalizing factor).
             * The weighting factors i^2 and 1/j^2 give increasing weight to higher graylevel values and decreasing
             * weight to long runlengths, respectively.
             */
            double getShortRunHighGraylevelEmphasis()
            {
                if (!shortRunHighGraylevelEmphasis.computed) computeFeatures();
                return shortRunHighGraylevelEmphasis.value;
            }

            /**
             * Returns the long run low graylevel emphasis (LRLGE) for the RLM
             * According to Dasarathy1991, LRLGE is defined as Sum[Pij / i^2 * j^2] / N,
             *   where i = gray level (bin number), j = run length, and N = total number of runs (normalizing factor).
             * The weighting factors 1/i^2 and j^2 give decreasing weight to higher graylevel values and increasing
             * weight to long runlengths, respectively.
             */
            double getLongRunLowGraylevelEmphasis()
            {
                if (!longRunLowGraylevelEmphasis.computed) computeFeatures();
                return longRunLowGraylevelEmphasis.value;
            }

            /**
             * Returns the long run high graylevel emphasis (LRHGE) for the RLM
             * According to Dasarathy1991, LRHGE is defined as Sum[Pij * i^2 * j^2] / N,
             *   where i = gray level (bin number), j = run length, and N = total number of runs (normalizing factor).
             * The weighting factors i^2 and j^2 give increasing weight to higher graylevel values and
             * high runlengths, respectively.
             */
            double getLongRunHighGraylevelEmphasis()
            {
                if (!longRunHighGraylevelEmphasis.computed) computeFeatures();
                return longRunHighGraylevelEmphasis.value;
            }


            /**
             * Returns the gray level nonuniformity (GLN) for the RLM.
             * According to Galloway1975, GLN is defined as
             *   Sum[(Sum[Pij, j])^2, i] / N
             *   where i = gray level (bin number), j = run length, and N = total number of runs (normalizing factor).
             * The inner sum represents the total number of runs present in the image for a given gray level i. If the
             * gray levels are distributed evenly, this number should be relatively small (and similar) across all gray
             * levels i. If the opposite is true, then the inner sum will be quite large for some values of i, which
             * will in turn be magnified by the square.
             */
            double getGraylevelNonuniformity()
            {
                if (!grayLevelNonuniformity.computed) computeFeatures();
                return grayLevelNonuniformity.value;
            }

            /**
             * Returns the run length nonuniformity (RLN) for the RLM.
             * According to Galloway1975, RLN is defined as
             *   Sum[(Sum[Pij, i])^2, j] / N
             *   where i = gray level (bin number), j = run length, and N = total number of runs (normalizing factor).
             * The inner sum represents the total number of runs present in the image for a given runlength j. If the
             * runlengths are distributed evenly, this number should be relatively small (and similar) across all
             * runlengths j. If the opposite is true, then the inner sum will be quite large for some values of j,
             * which will in turn be magnified by the square.
             */
            double getRunlengthNonuniformity()
            {
                if (!runLengthNonuniformity.computed) computeFeatures();
                return runLengthNonuniformity.value;
            }

            /**
             * Returns the runlength percentage (RP) for the RLM.
             * According to Galloway1975, RP is defined as Sum[Pij] / P,
             *   where i = gray level (bin number), j = run length, and P = total number of voxels in the image
             * RP has a maximum value of 1, which is achieved in an image consisting entirely of runs of length 1.
             */
            double getRunPercentage()
            {
                if (!runPercentage.computed) computeFeatures();
                return runPercentage.value;
            }

            /**
             * Returns the total number of runs present in the image. Since the runlength matrix
             * consists of discrete counts for each graylevel and runlength, this method simply
             * returns the sum over all the values in the RLM.
             */
            int getNumRuns()
            {
                if (!numRuns.computed) computeFeatures();
                return numRuns.value;
            }

            /**
             * Returns the total number of voxels in the image used to create the RLM. This
             * value is equal to getNumRuns() if and only if all runs in the image are of
             * length 1.
             */
            int getNumVoxels()
            {
                if (!numVoxels.computed) computeFeatures();
                return numVoxels.value;
            }
            /////////////////////////////////////////////////////////////////////////////////

		protected:
            //// R E S U L T  S T R U C T ///////////////////////////////////////////////////
            /**
             * Helper struct for lazy instantiation.
             */
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
            /////////////////////////////////////////////////////////////////////////////////


            //// M E M B E R S //////////////////////////////////////////////////////////////
			vnl_matrix<double> m_rlm;

            // Features from Galloway1975
            Result<double> shortRunEmphasis;
            Result<double> longRunEmphasis;
            Result<double> lowGraylevelRunEmphasis;
            Result<double> highGraylevelRunEmphasis;
            Result<double> shortRunLowGraylevelEmphasis;
            Result<double> shortRunHighGraylevelEmphasis;
            Result<double> longRunLowGraylevelEmphasis;
            Result<double> longRunHighGraylevelEmphasis;
            Result<double> grayLevelNonuniformity;
            Result<double> runLengthNonuniformity;
            Result<double> runPercentage;

            // Helper variables
            Result<int> numRuns;                                    // The sum over all the elements of the RLM, also equal to the total number of runs in the image
            Result<int> numVoxels;                                  // The number of voxels in the image = Sum[Pij * j, {i, j}]. Equal to numRuns iff all runs are of length 1.

            Result<std::vector<double>> marginalGraylevels;         // Marginal distribution of graylevels, i.e. Sum[Pij, j]
            Result<std::vector<double>> marginalRunLengths;         // Marginal distribution of runlengths, i.e. Sum[Pij, i]
            /////////////////////////////////////////////////////////////////////////////////


            /**
             * All features supported by the RlmFeatures class are computed in this method.
             *
             * Source: Galloway1975 - Texture Analysis Using Gray Level Run Lengths
             */
            void computeFeatures()
            {
                int i ,j, i2, j2;
                double pij;

                numRuns = 0;
                numVoxels = 0;

                shortRunEmphasis = 0.0;
                longRunEmphasis = 0.0;
                lowGraylevelRunEmphasis = 0.0;
                highGraylevelRunEmphasis = 0.0;
                shortRunLowGraylevelEmphasis = 0.0;
                shortRunHighGraylevelEmphasis = 0.0;
                longRunLowGraylevelEmphasis = 0.0;
                longRunHighGraylevelEmphasis = 0.0;
                grayLevelNonuniformity = 0.0;
                runLengthNonuniformity = 0.0;
                runPercentage = 0.0;

                marginalGraylevels = std::vector<double>(getNumGraylevels(), 0);
                marginalRunLengths = std::vector<double>(getMaxRunLength() + 1, 0);

                // Loop over image to compute features and marginal distributions
                for (i = 0; i < getNumGraylevels(); i++)
                {
                    i2 = (i+1) * (i+1);     // Don't want to do i^2, otherwise we'd end up dividing by zero

                    // We can skip j = 0 since by definition there are no runs of length 0
                    for (j = 1; j <= getMaxRunLength(); j++)   
                    {
                        pij = m_rlm(i, j);

                        if (pij != 0)
                        {
                            j2 = j * j;

                            shortRunEmphasis.value += pij / j2;
                            longRunEmphasis.value += pij * j2;
                            lowGraylevelRunEmphasis.value += pij / i2;
                            highGraylevelRunEmphasis.value += pij * i2;
                            shortRunLowGraylevelEmphasis.value += pij / j2 / i2;
                            shortRunHighGraylevelEmphasis.value += pij / j2 * i2;
                            longRunLowGraylevelEmphasis.value += pij * j2 / i2;
                            longRunHighGraylevelEmphasis.value += pij * j2 * i2;

                            numRuns.value += pij;
                            marginalGraylevels.value[i] += pij;
                            marginalRunLengths.value[j] += pij;

                            numVoxels.value += pij * j;     // Each run represents a number of voxels equal to its length
                        }
                    }

                    grayLevelNonuniformity.value += marginalGraylevels.value[i] * marginalGraylevels.value[i];
                }

                // Loop columns one more time to compute RLN
                for (j = 1; j <= getMaxRunLength(); j++)
                {
                    runLengthNonuniformity.value += marginalRunLengths.value[j] * marginalRunLengths.value[j];
                }

                shortRunEmphasis.value /= numRuns.value;
                longRunEmphasis.value /= numRuns.value;
                lowGraylevelRunEmphasis.value /= numRuns.value;
                highGraylevelRunEmphasis.value /= numRuns.value;
                shortRunLowGraylevelEmphasis.value /= numRuns.value;
                shortRunHighGraylevelEmphasis.value /= numRuns.value;
                longRunLowGraylevelEmphasis.value /= numRuns.value;
                longRunHighGraylevelEmphasis.value /= numRuns.value;
                grayLevelNonuniformity.value /= numRuns.value;
                runLengthNonuniformity.value /= numRuns.value;
                runPercentage.value = (double)numRuns.value / numVoxels.value;
            }
		};
	}
}

#endif