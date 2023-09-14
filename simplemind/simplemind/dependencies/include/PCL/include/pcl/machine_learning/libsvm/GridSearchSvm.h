#ifndef PCL_GRID_SEARCH_SVM
#define PCL_GRID_SEARCH_SVM

//// I N C L U D E S /////////////////////////////////////////////////////////////
#include <pcl/machine_learning/libsvm/Svm.h>
#include <pcl/machine_learning/Evaluation.h>

#include <cmath>
#include <ostream>
//////////////////////////////////////////////////////////////////////////////////

namespace pcl {
namespace svm
{
    //////////////////////////////////////////////////////////////////////////////////
    /**
     * Encapsulates a finite geometric sequence.
     */
    struct GeometricSequence
    {
        double start, step, end;

        GeometricSequence()
        {
            start = 1.0;
            step = 1.0;
            end = 1.0;
        }

        GeometricSequence(double start_in, double step_in, double end_in)
        {
            start = start_in;
            step = step_in;
            end = end_in;
        }

        friend std::ostream &operator<<(std::ostream &out, const GeometricSequence &geo)
        {
            out << geo.start << ", " << geo.step << ", " << geo.end;
            return out;
        }
    };
    //////////////////////////////////////////////////////////////////////////////////


    //// C L A S S ///////////////////////////////////////////////////////////////////
    /**
     * Helper class for performing gridsearch on SVM parameters Cost and Gamma.
     * Makes use of the Svm class for performing cross validation and the Evaluation
     * class for assessing classifier performance.
     */
    class GridSearchSvm
    {
    public:
        typedef pcl::Evaluation<int>        EvaluationType;

        //// C O N S T R U C T O R ///////////////////////////////////////////////////
        /**
         * Constructor. Instantiates a GridSearchSvm object with the following search parameters:
         *   Cost:  1e-2 --> 1e10, step 10
         *   Gamma: 1e-16 --> 1e2, step 10
         */
        GridSearchSvm()
        {
            // Default values
            initialize(1.0e-2, 10, 1.0e10, 1.0e-16, 10, 1.0e2);
        }
        //////////////////////////////////////////////////////////////////////////////

        //// M E T H O D S ///////////////////////////////////////////////////////////
        /**
         * Specifies the sequence of values to evaluate for the Cost parameter.
         */
        void setCostSequence(double start, double step, double end)
        {
            setCostSequence(GeometricSequence(start, step, end));
        }

        /**
         * Specifies the sequence of values to evaluate for the Cost parameter.
         */
        void setCostSequence(const GeometricSequence &seq)
        {
            m_costSeq = seq;
        }

        /**
         * Specifies the sequence of values to evaluate for the gamma parameter.
         */
        void setGammaSequence(double start, double step, double end)
        {
            setGammaSequence(GeometricSequence(start, step, end));
        }

        /**
         * Specifies the sequence of values to evaluate for the gamma parameter.
         */
        void setGammaSequence(const GeometricSequence &seq)
        {
            m_gammaSeq = seq;
        }

        /**
         * Performs gridseach on svm parameters.
         * Parameters
         *   features   - A list of lists specifying feature vectors for the training data.
         *                The outer indexes instances and the inner list indexes features.
         *                The list must be doubly iterable, and elements must be castable to double.
         *   labels     - A list of labels indicating class membership of the training instances.
         *                The list must be iterable, and elements must be castable to double.
         *   numFolds   - The number of folds to use for cross validation. Must be 2 or greater.
         *   parameters - An SvmParameters object specifying the parameters to use for the Svm model.
         *                The values of Cost and Gamma parameters are disregarded.
         */
        template <class FeatureVectorListType, class LabelListType>
        void search(const FeatureVectorListType &features, const LabelListType &labels, int numFolds, const SvmParameters &parameters)
        {
            searchPrint(features, labels, numFolds, parameters, std::cout, false);
        }

        /**
         * Performs gridseach on svm parameters.
         * Parameters
         *   features   - A list of lists specifying feature vectors for the training data.
         *                The outer indexes instances and the inner list indexes features.
         *                The list must be doubly iterable, and elements must be castable to double.
         *   labels     - A list of labels indicating class membership of the training instances.
         *                The list must be iterable, and elements must be castable to double.
         *   numFolds   - The number of folds to use for cross validation. Must be 2 or greater.
         *   parameters - An SvmParameters object specifying the parameters to use for the Svm model.
         *                The values of Cost and Gamma parameters are disregarded.
         *   out        - ostream object to which to print gridsearch results.
         */
        template <class FeatureVectorListType, class LabelListType>
        void search(const FeatureVectorListType &features, const LabelListType &labels, int numFolds, const SvmParameters &parameters, std::ostream &out)
        {
            searchPrint(features, labels, numFolds, parameters, out, true);
        }

        /**
         * Returns the value of the performance metric associated with the optimal cost and gamma.
         * Undefined if search(...) has not been called.
         */
        double getPerformance() const
        {
            return m_performance;
        }

        /**
         * Returns the cost associated with the highest performance.
         * Undefined if search(...) has not been called.
         */
        double getCost() const
        {
            return m_cost;
        }

        /**
         * Returns the gamma associated with the highest performance.
         * Undefined if search(...) has not been called.
         */
        double getGamma() const
        {
            return m_gamma;
        }
        //////////////////////////////////////////////////////////////////////////////

    private:
        //// M E M B E R S ///////////////////////////////////////////////////////////
        GeometricSequence m_costSeq;            // Geometric sequence describing search space for cost
        GeometricSequence m_gammaSeq;           // Geometric sequence describing search space for gamma

        // Values of optimized parameters
        double m_performance;                   // Optimal value of performance
        double m_cost, m_gamma;                 // Optimal value of cost, gamma
        //////////////////////////////////////////////////////////////////////////////

        //// M E T H O D S ///////////////////////////////////////////////////////////
        /**
         * Initializes a gridsearch with the specified cost and gamma sequences.
         */
        void initialize(double costStart, double costStep, double costEnd, double gammaStart, double gammaStep, double gammaEnd)
        {
            setCostSequence(costStart, costStep, costEnd);
            setGammaSequence(gammaStart, gammaStep, gammaEnd);
            
            m_cost = std::numeric_limits<double>::quiet_NaN();
            m_gamma = std::numeric_limits<double>::quiet_NaN();
        }

        /**
         * Implementation of gridsearch algorithm.
         * Parameters
         *   features   - A list of lists specifying feature vectors for the training data.
         *                The outer indexes instances and the inner list indexes features.
         *                The list must be doubly iterable, and elements must be castable to double.
         *   labels     - A list of labels indicating class membership of the training instances.
         *                The list must be iterable, and elements must be castable to double.
         *   numFolds   - The number of folds to use for cross validation. Must be 2 or greater.
         *   parameters - An SvmParameters object specifying the parameters to use for the Svm model.
         *                The values of Cost and Gamma parameters are disregarded.
         *   out        - ostream object to which to print gridsearch results.
         *   doPrint    - Flag indicating whether intermediate output should be printed.
         */
        template <class FeatureVectorListType, class LabelListType>
        void searchPrint(const FeatureVectorListType &features, const LabelListType &labels, int numFolds, const SvmParameters &parameters, std::ostream &out, bool doPrint)
        {
            double cost, gamma;
            SvmParameters param = parameters;

            double bestPerformance = 0.0;
            double bestCost = m_costSeq.start;
            double bestGamma = m_gammaSeq.start;

            // Check to make sure search space is valid
            if (!validateSearch())
            {
                std::cout << "Invalid search space specified. Grid search has not been performed." << std::endl;

                m_performance = bestPerformance;
                m_cost = bestCost;
                m_gamma = bestGamma;
                return;
            }
            
            if (doPrint)
            {
                // Print header row
                out << "C \\\\ g";
                for (gamma = m_gammaSeq.start; gamma <= m_gammaSeq.end; gamma *= m_gammaSeq.step)
                    out << "," << gamma;
                out << endl;
            }

            // Iterate over all costs
            for (cost = m_costSeq.start; cost <= m_costSeq.end; cost *= m_costSeq.step)
            {
                if (doPrint) out << cost;

                // Iterate over all gammas
                for (gamma = m_gammaSeq.start; gamma <= m_gammaSeq.end; gamma *= m_gammaSeq.step)
                {
                    // Start clock
                    std::cout << "  Evaluating (" << std::scientific << std::setprecision(1) << cost << ", " << gamma << ")... " << std::flush;
                    pcl::Timing clock;
                    clock.tic();

                    // Set cost and gamma svm parameters
                    param.setC(cost);
                    param.setGamma(gamma);

                    // Cross-evaluate svm classifier
                    Svm svm(param);
                    EvaluationType::Pointer eval = svm.crossValidate(features, labels, numFolds, param);

                    // Examine performance
                    double perf = getPerformance(eval);
                    if (perf > bestPerformance)
                    {
                        bestPerformance = perf;
                        bestCost = cost;
                        bestGamma = gamma;
                    }

                    // Update gridfile
                    if (doPrint) out << "," << perf << std::flush;

                    // Stop clock
                    clock.toc();
                    std::cout << std::fixed << std::setprecision(3) << perf << " (" << clock.getClockInSeconds() << " s)" << std::endl;
                }

                if (doPrint) out << endl;
            }

            // Record best performance
            m_performance = bestPerformance;
            m_cost = bestCost;
            m_gamma = bestGamma;
        }

        /**
         * Checks cost and gamma sequences to see if they describe a valid search space.
         */
        bool validateSearch() const
        {
            return validateSequence(m_costSeq) && validateSequence(m_gammaSeq);
        }

        /**
         * Checks the specified geometric sequence to see whether it is valid for search.
         * A sequence is valid iff it is finite and strictly increasing. In order to satisfy this, the following conditions must hold:
         *   Start != 0.0
         *   End != 0.0
         *   Start < End
         *   Step > 1.0
         * Returns true if valid, false otherwise.
         */
        static bool validateSequence(const GeometricSequence &geo)
        {
            if (geo.start == 0.0) return false;
            if (geo.end == 0.0) return false;
            if (geo.start > geo.end) return false;
            if (geo.step <= 1) return false;

            return true;
        }

        /**
         * Returns the performance measure for a given Evaluation.
         * The performance metric is hardcoded to EGM.
         */
        static double getPerformance(const EvaluationType::Pointer eval)
        {
            return eval->getEGM();
        }
        //////////////////////////////////////////////////////////////////////////////
    };
    //////////////////////////////////////////////////////////////////////////////////
}}

#endif
