#ifndef PCL_SVM
#define PCL_SVM

//// I N C L U D E S /////////////////////////////////////////////////////////////
#include <pcl/machine_learning/Evaluation.h>
#include <pcl/machine_learning/libsvm/SvmParameters.h>

#include <libsvm/svm.h>
#include <ios>
#include <unordered_set>
//////////////////////////////////////////////////////////////////////////////////

namespace pcl {
namespace svm
{
    /**
     * Wrapper for LibSVM. Encapsulates training, cross validating, and evaluating an SVM classifier.
     * Use the method train(...) to build an SVM model. Alternatively, an existing model can be loaded from file with read(...).
     * After building the model, a novel instance can be classified using the methods compute(...), computeMap(...), or computeMax(...).
     * Detailed test set evaluation can be done through the Evaluation class.
     */
    class Svm
    {
    public:
        //// C O N S T R U C T O R S /////////////////////////////////////////////////////
        /**
         * Instantiates an Svm object using default parameters.
         * The model must be built using train(...) or read(...) before it can be used.
         */
        Svm()
        {
            initialize(SvmParameters::prepareDefaultParameters());
        }

        /**
         * Instantiates an Svm object using the specified parameters.
         * The model must be built using train(...) or read(...) before it can be used.
         */
        Svm(const SvmParameters &parameters)
        {
            initialize(parameters);
        }

        /**
         * Instantiates an Svm object using an existing model found in the specified file.
         * Equivalent to instantiating with Svm() then calling read(filename).
         */
        Svm(const std::string &filename)
        {
            initialize();
            read(filename);
        }

        /**
         * Destructor. Frees the memory allocated to the Svm object.
         */
        ~Svm()
        {
            free();
        }
        //////////////////////////////////////////////////////////////////////////////////

        //// S T A T I C  M E T H O D S //////////////////////////////////////////////////
        /**
         * Static method. Returns true if LibSVM is operating in quiet mode, false otherwise.
         */
        static bool getQuiet()
        {
            return s_quietMode;
        }

        /**
         * Static method. Instructs LibSVM to operate in quiet mode, which suppresses detailed output during classifier training.
         */
        static void setQuiet()
        {
            svm_set_print_string_function(Svm::printQuiet);
            s_quietMode = true;
        }

        /**
         * Static method. Instructs LibSVM to operate in verbose mode, which results in additional output during classifier training.
         */
        static void setVerbose()
        {
            svm_set_print_string_function(NULL);
            s_quietMode = false;
        }

        /**
         * Performs cross validation on the provided dataset based on default Svm parameters.
         * The number of folds specified must be greater than 1.
         * Cross-validation results are returned as an Evaluation object, which stores the confusion matrix as well as performance metrics.
         */
        template <class FeatureVectorListType, class LabelListType>
        static Evaluation<int>::Pointer crossValidate(const FeatureVectorListType &features, const LabelListType &labels, int numFolds)
        {
            return crossValidate(features, labels, numFolds, SvmParameters::prepareDefaultParameters());
        }

        /**
         * Performs n-fold cross validation on the provided dataset based on the specified Svm parameters.
         * The number of folds specified must be greater than 1.
         * Cross-validation results are returned as an Evaluation object, which stores the confusion matrix as well as performance metrics.
         */
        template <class FeatureVectorListType, class LabelListType>
        static Evaluation<int>::Pointer crossValidate(const FeatureVectorListType &features, const LabelListType &labels, int numFolds, const SvmParameters &parameters)
        {
            svm_problem *problem;
            Evaluation<int>::Pointer eval;

            problem = prepareSvmProblem(features, labels);
            const char *error = svm_check_parameter(problem, &parameters.getParameters());

            if (error == NULL)
            {
                std::vector<int> predicted;
                double *results = new double[problem->l];

                svm_cross_validation(problem, &parameters.getParameters(), numFolds, results);

                for (int i = 0; i < problem->l; i++)
                    predicted.push_back((int)results[i]);

                eval = Evaluation<int>::New(getLabels(problem));
                eval->record(labels, predicted);

                delete[] results;
            }
            else
            {
                std::cout << error << std::endl;
                std::cout << "Classifier has NOT been trained!" << std::endl;

                delete[] error;
            }

            return eval;
        }
        //////////////////////////////////////////////////////////////////////////////////


        //// A C C E S S O R S ///////////////////////////////////////////////////////////
        /**
         * Returns true if the Svm model has been built.
         * This is true iff one of the following is true:
         *   The model has been trained by successfully calling the train( ... ) method.
         *   The model has been loaded from a valid libsvm model file by calling the constructor Svm(string)
         *   The model has been loaded from a valid libsvm model file by calling read(string).
         * Returns false otherwise.
         */
        bool built() const
        {
            return m_model != NULL;
        }

        /**
         * Returns true if the SVM model is configured to compute probability estimates for each class.
         * This functionality is specified by the method setUseProbability(bool) in the SvmParameters
         * object used to instantiate the Svm object.
         * Returns false if the model has not been built.
         */
        bool isProbabilityModel() const
        {
            return built() ? svm_check_probability_model(m_model) == 1 : false;
        }

        /**
         * Returns the number of classes recognized by the Svm model.
         * Returns 0 if the model has not been built.
         */
        int getNumClasses() const
        {
            return built() ? svm_get_nr_class(m_model) : 0;
        }

        /**
         * Returns a vector describing the class labels in internal index order.
         * This order coincides with the order in which the labels are encountered in the training data.
         */
        const std::vector<int> &getLabels() const
        {
            return m_labels;
        }
        //////////////////////////////////////////////////////////////////////////////////


        //// S V M  M E T H O D S ////////////////////////////////////////////////////////
        /**
         * Builds an Svm model based on the provided training data and the parameters specified at instantiation time.
         * Parameters
         *   features - A list of lists specifying feature vectors for the training data.
         *              The outer indexes instances and the inner list indexes features.
         *              The list must be doubly iterable, and elements must be castable to double.
         *   labels   - A list of labels indicating class membership of the training instances.
         *              The list must be iterable, and elements must be castable to double.
         * The two lists features and labels are assumed to contain the same number of elements.
         *
         * Returns true if model is successfully trained; false otherwise.
         */
        template <class FeatureVectorListType, class LabelListType>
        bool train(const FeatureVectorListType &features, const LabelListType &labels)
        {
            free();

            // Convert training data to internal (libsvm-compatible) format
            m_problem = prepareSvmProblem(features, labels);

            // Validate SvmParameters against training data
            const char *error = svm_check_parameter(m_problem, &m_parameters);
            
            if (error != NULL)
            {
                // Encountered a problem
                std::cout << error << std::endl;
                std::cout << "Classifier has NOT been trained!" << std::endl;

                // Deallocate all memory
                delete[] error;
                free();
                
                // Return error
                return false;
            }

            // Train svm model
            m_model = svm_train(m_problem, &m_parameters);

            // Update list of recognized class labels
            updateLabels();

            return true;
        }

        /**
         * Applies the classifier to the specified input feature vector.
         * Returns a vector of doubles indicating the probability estimates for each class.
         * The vector is indexed in internal order; to convert this to class labels, use the methods getLabels() or indexToLabel(int).
         * If the svm model has not been built, returns an empty vector.
         */
        template <class FeatureVectorType>
        std::vector<double> compute(const FeatureVectorType &input) const
        {
            double *results = new double[getNumClasses()];

            // Perform classification
            computeForInput(input, results);

            // Convert probability estimates to vector
            std::vector<double> returnthis(results, results + getNumClasses());

            // Free array
            delete[] results;

            return returnthis;
        }

        /**
         * Applies the classifier to the specified input feature vector.
         * Returns a map associating class labels with probability estimates.
         * If the svm model has not been built, returns an empty map.
         */
        template <class FeatureVectorType>
        std::unordered_map<int, double> computeMap(const FeatureVectorType &input) const
        {
            std::unordered_map<int, double> returnthis;
            double *results = new double[getNumClasses()];

            // Perform classification
            computeForInput(input, results);

            // Convert probability estimates to map
            std::vector<int> labels = getLabels();
            for (int i = 0; i < getNumClasses(); i++)
            {
                returnthis[labels[i]] = results[i];
            }

            // Free array
            delete[] results;

            return returnthis;
        }

        /**
         * Applies the classifier to the specified input feature vector.
         * Returns the maximal probability class label.
         * If the svm model has not been built, returns -1.
         */
        template <class FeatureVectorType>
        int computeMax(const FeatureVectorType &input) const
        {
            if (!built()) return -1;

            double *results = new double[getNumClasses()];
            
            // Perform classification
            computeForInput(input, results);

            // Find max
            int maxIndex = (int)(std::max_element(results, results+getNumClasses()) - results);

            // Free array
            delete[] results;

            return indexToLabel(maxIndex);
        }


        /**
         * Writes the svm model to file in LibSVM format. This model can then be loaded through the read(string) or Svm(string) methods.
         * Returns true on success; false on failure.
         */
        bool write(const std::string &filename) const
        {
            if (!built()) return false;

            // Returns 0 on success, -1 on fail
            int result = svm_save_model(filename.c_str(), m_model);

            return result == 0;
        }

        /**
         * Reads an existing svm model from the specified file. If the read operation fails, then the svm model remains unbuilt.
         * Returns true on success, false on failure.
         */
        bool read(const std::string &filename)
        {
            free();

            m_model = svm_load_model(filename.c_str());

            if (m_model == NULL) return false;

            updateLabels();
            return true;
        }
        //////////////////////////////////////////////////////////////////////////////////


    private:
        //// M E M B E R S ///////////////////////////////////////////////////////////////
        svm_parameter m_parameters;         // Parameters used to build SVM model
        svm_problem *m_problem;             // Training dataset for SVM model, in internal libsvm format
        svm_model *m_model;                 // SVM model, in internal libsvm format
        std::vector<int> m_labels;          // Vector mapping classes from internal index order to labels specified in training dataset

        static bool s_quietMode;            // Static flag indicating whether to suppress libsvm output during model training
        //////////////////////////////////////////////////////////////////////////////////


        //// M E T H O D S ///////////////////////////////////////////////////////////////
        /**
         * Initializes the svm with the given set of parameters.
         * Also sets all internal pointers to null.
         */
        void initialize(const SvmParameters &parameters)
        {
            m_parameters = parameters.getParameters();
            initialize();
        }

        /**
         * Initializes the svm by setting all internal pointers to null.
         */
        void initialize()
        {
            m_model = NULL;
            m_problem = NULL;
        }

        /**
         * Static method. Converts training dataset, which is specified as parallel lists of feature vectors and labels, to internal libsvm format.
         * Explicitly allocates memory to hold the training data. This memory must not be freed until the Svm model is destroyed.
         * The parameters features and labels must be iterable and are assumed to be the same size.
         *
         * Returns a pointer to an svm_problem struct, which is the internal representation of training data used by libsvm.
         */
        template <class FeatureVectorListType, class LabelListType>
        static svm_problem *prepareSvmProblem(const FeatureVectorListType &features, const LabelListType &labels)
        {
            size_t numColumns = labels.size();
            size_t numRows = features.size();

            svm_problem *problem = new svm_problem;

            // Set number of training data
            problem->l = (int)numRows;
            
            // Create vector of labels
            {
                problem->y = new double[numRows];
                double *p = problem->y;
                for (auto iter = labels.begin(); iter != labels.end(); iter++)
                {
                    *p = *iter;         // Assumes castable to double
                    p++;
                }
            }

            // Prepare vector of feature vectors
            {
                problem->x = new svm_node*[numRows];
                svm_node **p = problem->x;
                for (auto iter = features.begin(); iter != features.end(); iter++)                
                {
                    *p = makeSvmNodes(*iter);
                    p++;
                }
            }

            return problem;
        }

        /**
         * Static method. Converts a feature vector, which is specified as a list of feature values, to internal libsvm format.
         * The feature values must be castable to double.
         * Returns a pointer to an svm_node struct, which is the internal representation of a feature vector used by libsvm.
         */
        template <class FeatureVectorType>
        static svm_node *makeSvmNodes(const FeatureVectorType &featureVector)
        {
            svm_node *nodes = new svm_node[featureVector.size() + 1];
            int j;

            j  = 0;
            for (auto iter = featureVector.begin(); iter != featureVector.end(); iter++)
            {
                nodes[j].index = j;
                nodes[j].value = *iter;     // Assumes castable to double
                j++;
            }
            nodes[featureVector.size()].index = -1;

            return nodes;
        }

        /**
         * Updates the list of class labels recognized by the svm model.
         */
        void updateLabels()
        {
            if (!built()) return;

            int *labels = new int[getNumClasses()];

            svm_get_labels(m_model, labels);

            m_labels.clear();
            for (int i = 0; i < getNumClasses(); i++)
                m_labels.push_back(labels[i]);

            delete[] labels;
        }

        /**
         * Applies the svm classifier to the given input vector, then stores the results to the result array.
         * The results array is indexed in internal order.
         * Assumes that sufficient memory has been allocated to *results.
         * Does nothing if the model does not exist.
         */
        template <class FeatureVectorType>
        void computeForInput(const FeatureVectorType &input, double *results) const
        {
            if (!built()) return;

            // Prepare feature vector in libsvm format
            svm_node *nodes = makeSvmNodes(input);

            if (isProbabilityModel())                   // Probability estimates
            {
                // Perform classification
                svm_predict_probability(m_model, nodes, results);
            }
            else                                        // Discrete classification only
            {
                double result;

                // Perform classification
                result = svm_predict(m_model, nodes);       // result is returned as a label

                // Prepare return vector
                for (int i = 0; i < getNumClasses(); i++)
                    results[i] = 0.0;
                results[labelToIndex((int)result)] = 1.0;
            }

            delete[] nodes;
        }

        /**
         * Frees the memory allocated to the svm object.
         * This method should not be called until the current Svm model is ready to be destroyed.
         */
        void free()
        {
            if (m_model != NULL)
            {
                svm_free_and_destroy_model(&m_model);               // Defined in libsvm/svm.h
                m_model = NULL;
            }

            if (m_problem != NULL)
            {
                if (m_problem->x != NULL)
                {
                    for (int i = 0; i < m_problem->l; i++)
                    {
                        if (m_problem->x[i] != NULL) delete[] m_problem->x[i];
                    }
                    delete[] m_problem->x;
                    m_problem->x = NULL;
                }
            
                if (m_problem->y != NULL)
                {
                    delete[] m_problem->y;
                    m_problem->y = NULL;
                }

                delete m_problem;
            }

            m_labels.clear();
        }

        /**
         * Returns the internal index associated with the specified class label.
         * Returns -1 if the specified label cannot be found.
         */
        int labelToIndex(int label) const
        {
            for (int i = 0; i < m_labels.size(); i++)
                if (m_labels[i] == label) return i;
            return -1;
        }

        /**
         * Returns the class label associated with the given internal index number.
         * Performs no bounds checking.
         */
        int indexToLabel(int index) const
        {
            return m_labels[index];
        }
        //////////////////////////////////////////////////////////////////////////////////


        //////////////////////////////////////////////////////////////////////////////////
        /**
         * Static method. Returns a vector associating class labels to internal index order for the given
         * training data in internal libsvm format.
         */
        static const std::vector<int> getLabels(const svm_problem *problem)
        {
            std::vector<int> labels;
            std::unordered_set<int> uniqueLabels;

            if (problem == NULL) return labels;
            for (int i = 0; i < problem->l; i++)
            {
                int label = (int)problem->y[i];

                if (uniqueLabels.count(label) == 0)
                {
                    labels.push_back(label);
                    uniqueLabels.insert(label);
                }
            }

            return labels;
        }
        //////////////////////////////////////////////////////////////////////////////////

        /**
         * Dummy method that does nothing. This method is registered using the libsvm function svm_set_print_string_function
         * in order to suppress output during classifier training.
         */
        static void printQuiet(const char *string) {}
    };

    bool Svm::s_quietMode = false;
}}

#endif
