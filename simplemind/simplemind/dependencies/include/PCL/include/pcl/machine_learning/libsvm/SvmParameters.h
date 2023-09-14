#ifndef PCL_SVM_PARAMETERS
#define PCL_SVM_PARAMETERS


//// I N C L U D E S /////////////////////////////////////////////////////////////
#include <libsvm/svm.h>
//////////////////////////////////////////////////////////////////////////////////


namespace pcl {
namespace svm
{
    //// C L A S S ///////////////////////////////////////////////////////////////////
    /**
     * Encapsulates parameters used by the libsvm classifier.
     * Parameters are as follows:
     *   int svm_type;              // One of C_SVC, NU_SVC, ONE_CLASS, EPSILON_SVR, NU_SVR
     *   int kernel_type;           // One of LINEAR, POLY, RBF, SIGMOID, PRECMOPUTED
     *   int degree;                // For POLY kernel
     *   double gamma;              // For POLY/RBF/SIGMOID kernels
     *   double coef0;              // For POLY/SIGMOID kernels
     *
     *   // For training only
     *   double cache_size;         // In MB
     *   double eps;                // Stopping criterion
     *   double C;                  // For C_SVC, EPSILON_SVR, and NU_SVR
     *   int nr_weight;             // For C_SVC
     *   int *weight_label;         // For C_SVC
     *   double *weight;            // For C_SVC
     *   double nu;                 // For NU_SVC, ONE_CLASS, and NU_SVR
     *   double p;                  // For EPSILON_SVR
     *   int shrinking;             // Use the shrinking heuristics
     *   int probability;           // Do probability estimates
     */
    class SvmParameters
    {
    public:
        //////////////////////////////////////////////////////////////////////////////////
        // Constants defined in global scope in libsvm/svm.h
        enum SvmType
        {
            C_SVC = C_SVC,
            NU_SVC = NU_SVC,
            ONE_CLASS = ONE_CLASS,
            EPSILON_SVR = EPSILON_SVR,
            NU_SVR = NU_SVR
        };

        // Constants defined in global scope in libsvm/svm.h
        enum KernelType
        {
            LINEAR = LINEAR,
            POLY = POLY,
            RBF = RBF,
            SIGMOID = SIGMOID,
            PRECOMPUTED = PRECOMPUTED
        };
        //////////////////////////////////////////////////////////////////////////////////

        //// F A C T O R Y ///////////////////////////////////////////////////////////////
        /** 
         * Prepares a default set of parameters.
         * Default values taken from the default parameters used for LibSVM by Weka.
         */
        static SvmParameters prepareDefaultParameters()
        {
            SvmParameters param;

            param.setSvmType(C_SVC);
            param.setKernelType(RBF);
            param.setCacheSize(40.0);
            param.setCoef0(0.0);
            param.setC(1.0);
            param.setDegree(3);
            param.setEps(0.001);
            param.setGamma(0.0);
            param.setNu(0.5);
            param.setUseProbability(false);
            param.setUseShrinking(true);
            param.setNrWeight(0);

            return param;
        }
        //////////////////////////////////////////////////////////////////////////////////

        //// A C C E S S O R S ///////////////////////////////////////////////////////////
        int getSvmType() const
        {
            return m_parameters.svm_type;
        }

        void setSvmType(SvmType svmType)
        {
            m_parameters.svm_type = svmType;
        }

        int getKernelType() const
        {
            return m_parameters.kernel_type;
        }

        void setKernelType(KernelType kernelType)
        {
            m_parameters.kernel_type = kernelType;
        }

        int getDegree() const
        {
            return m_parameters.degree;
        }

        void setDegree(int degree)
        {
            m_parameters.degree = degree;
        }

        double getGamma() const
        {
            return m_parameters.gamma;
        }

        void setGamma(double gamma)
        {
            m_parameters.gamma = gamma;
        }

        double getCoef0() const
        {
            return m_parameters.coef0;
        }

        void setCoef0(double coef0)
        {
            m_parameters.coef0 = coef0;
        }

        double getCacheSize() const
        {
            return m_parameters.cache_size;
        }

        void setCacheSize(double cacheSize)
        {
            m_parameters.cache_size = cacheSize;
        }

        double getEps() const
        {
            return m_parameters.eps;
        }

        void setEps(double eps)
        {
            m_parameters.eps = eps;
        }

        double getC() const
        {
            return m_parameters.C;
        }

        void setC(double C)
        {
            m_parameters.C = C;
        }

        int getNrWeight() const
        {
            return m_parameters.nr_weight;
        }

        void setNrWeight(int nrWeight)
        {
            m_parameters.nr_weight = nrWeight;
        }

        int *getWeightLabel() const
        {
            return m_parameters.weight_label;
        }

        void setWeightLabel(int *weightLabel)
        {
            m_parameters.weight_label = weightLabel;
        }

        double *getWeight() const
        {
            return m_parameters.weight;
        }

        void setWeight(double *weight)
        {
            m_parameters.weight = weight;
        }

        double getNu() const
        {
            return m_parameters.nu;
        }

        void setNu(double nu)
        {
            m_parameters.nu = nu;
        }

        double getP() const
        {
            return m_parameters.p;
        }

        void setP(double p)
        {
            m_parameters.p = p;
        }

        bool getUseShrinking() const
        {
            return m_parameters.shrinking == 1;
        }

        void setUseShrinking(bool shrinking)
        {
            m_parameters.shrinking = (shrinking ? 1 : 0);
        }

        bool getUseProbability() const
        {
            return m_parameters.probability == 1;
        }

        void setUseProbability(bool probability)
        {
            m_parameters.probability = (probability ? 1 : 0);
        }


        const svm_parameter &getParameters() const
        {
            return m_parameters;
        }
        //////////////////////////////////////////////////////////////////////////////////


    private:
        //// C O N S T R U C T O R ///////////////////////////////////////////////////////
        SvmParameters() {}
        //////////////////////////////////////////////////////////////////////////////////

        svm_parameter m_parameters;
    };
    //////////////////////////////////////////////////////////////////////////////////
}}


#endif