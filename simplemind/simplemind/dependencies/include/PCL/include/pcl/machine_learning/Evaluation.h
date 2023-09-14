#ifndef PCL_EVALUATION
#define PCL_EVALUATION

//// I N C L U D E S /////////////////////////////////////////////////////////////
#include <boost/bimap.hpp>
#include <vnl/vnl_matrix.h>

#include <ios>
//////////////////////////////////////////////////////////////////////////////////


namespace pcl
{
    //// C L A S S ///////////////////////////////////////////////////////////////////
    /**
     * Encapsulates a class which keeps track of the evaluation of a classifier.
     * In a nutshell, stores a confusion matrix of classifier results and computes performance metrics.
     *
     * Behavior of the evaluation class is slightly complicated by the fact that class labels are
     * not guaranteed to be sequential integers starting from 0. In order to account for this,
     * a vector of labels (typically integers) is provided at instance time. The order of this vector
     * determines the internal order of class indices, while the labels themselves describe the
     * classes in a way that is externally meaningful. This vector may be created manually or provided
     * by such interfaces as Svm::getLabels().
     */
    template <class T_LabelType=int>
    class Evaluation
    {
    public:
        typedef boost::shared_ptr<Evaluation>   Pointer;
        typedef T_LabelType                     LabelType;
        typedef boost::bimap<int, LabelType>    MapType;

        //// F A C T O R Y ///////////////////////////////////////////////////////////
        static Pointer New(int numLabels);

        /**
         * Instantiates a new Evaluation object for a classification task with the specified labels.
         */
        static Pointer New(const std::vector<LabelType> &labels)
        {
            Pointer returnthis(new Evaluation);

            returnthis->initializeLabels(labels);
            returnthis->clear();
            return returnthis;
        }
        //////////////////////////////////////////////////////////////////////////////


        //// M E T H O D S ///////////////////////////////////////////////////////////
        /**
         * Returns the number of classes recognized by the Evaluation object.
         */
        int getNumClasses() const
        {
            return m_labels.size();
        }

        /**
         * Evaluates the specified classifier on the provided dataset.
         * Parameters
         *   classifier - A classifier object. Must implement a method vector<double> compute(FeatureVectorType)
         *                where FeatureVectorType is an iterable list of feature values which are castable to double.
         *   features   - A list of lists specifying feature vectors for the evaluation data.
         *                The outer indexes instances and the inner list indexes features.
         *                The list must be doubly iterable, and elements must be castable to double.
         *   labels     - A list of labels indicating class membership of the evaluation instances.
         *                The list must be iterable, and elements must be castable to double.
         * The two lists features and labels are assumed to contain the same number of elements.
         * The labels recognized by the classifier must match the labels recognized by the Evaluation object.
         */
        template <class ClassifierType, class FeatureVectorListType, class LabelListType>
        void evaluate(const ClassifierType &classifier, const FeatureVectorListType &features, const LabelListType &labels)
        {
            auto iterLabel = labels.begin();
            auto iterFeature = features.begin();

            while (iterLabel != labels.end())
            {
                // If this instance has an unrecognized class label, then skip it
                if (hasLabel(*iterLabel))
                {
                    // Determine index of ground truth
                    int truthIndex = labelToIndex(*iterLabel);

                    // Perform classification
                    auto result = classifier.compute(*iterFeature);

                    // Determine index of classifier prediction
                    int predictedIndex = 0;
                    double probMax = 0.0;
                    int i = 0;
                    for (auto iter = result.begin(); iter != result.end(); iter++)
                    {
                        if (*iter > probMax)
                        {
                            probMax = *iter;
                            predictedIndex = i;
                        }
                        i++;
                    }

                    // Record classification result into confusion matrix
                    recordIndex(truthIndex, predictedIndex);
                }

                iterLabel++;
                iterFeature++;
            }
        }

        /**
         * Record classification results into confusion matrix.
         * truthList and predictedList are parallel lists containing ground truth labels and classification labels, respectively.
         * The labels must match the labels recognized by the Evaluation object; otherwise the classification result is disregarded.
         */
        template <class LabelListType>
        void record(const LabelListType &truthList, const LabelListType &predictedList)
        {
            for (auto iter1 = truthList.begin(), iter2 = predictedList.begin(); iter1 != truthList.end() && iter2 != predictedList.end(); iter1++, iter2++)
            {
                if (!hasLabel(*iter1) || !hasLabel(*iter2)) continue;

                int truthIndex = labelToIndex(*iter1);
                int predictedIndex = labelToIndex(*iter2);
                recordIndex(truthIndex, predictedIndex);
            }
        }

        /**
         * Record classification result into confusion matrix.
         * The provided ground truth and classification labels must match the labels recognized by the Evaluation object;
         * otherwise they will be disregarded.
         */
        void record(const LabelType &truthLabel, const LabelType &predictedLabel)
        {
            if (!hasLabel(truthLabel) || !hasLabel(predictedLabel)) return;

            int truthIndex = labelToIndex(truthLabel);
            int predictedIndex = labelToIndex(predictedLabel);
            recordIndex(truthIndex, predictedIndex);
        }

        /**
         * Zeroes out the confusion matrix.
         */
        void clear()
        {
            m_matrix.fill(0);

            for (auto iter = m_marginalTruth.begin(); iter != m_marginalTruth.end(); iter++)
                *iter = 0;

            for (auto iter = m_marginalPredicted.begin(); iter != m_marginalPredicted.end(); iter++)
                *iter = 0;

            m_numInstances = 0;
        }

        
        //// C O N F U S I O N  M A T R I X  S T A T S ///////////////////////////////
        /**********************
        | Counting statistics |
        **********************/
        /**
         * Returns the total number of observations recorded in the confusion matrix.
         */
        int getNumInstances() const
        {
            return m_numInstances;
        }

        /**
         * Returns the number of observations with the given truth and predicted labels.
         */
        int get(const LabelType &truthLabel, const LabelType &predictedLabel) const
        {
            if (!isDefined()) return -1;
            if (!hasLabel(truthLabel) || !hasLabel(predictedLabel)) return -1;

            return getFast(truthIndex, predictedIndex);
        }

        /**
         * Returns the marginal number of observations for the given truth label.
         */
        int getMarginalTruth(const LabelType &truthLabel) const
        {
            if (!isDefined()) return -1;
            if (!hasLabel(truthLabel)) return -1;

            return m_marginalTruth[truthIndex];
        }

        /**
         * Returns the marginal number of observations for the given predicted label.
         */
        int getMarginalPredicted(const LabelType &predictedLabel) const
        {
            if (!isDefined()) return -1;
            if (!hasLabel(predictedLabel)) return -1;

            return m_marginalPredicted[predictedIndex];
        }

        /****************************
        | Class-specific statistics |
        ****************************/
        /**
         * Returns the number of true positive observations with respect to the specified label.
         */
        int getTruePositiveCount(const LabelType &label) const
        {
            if (!isDefined() || !hasLabel(label)) return 0;

            int index = labelToIndex(label);
            return getFast(index, index);
        }

        /**
         * Returns the number of false positive observations with respect to the specified label.
         */
        int getFalsePositiveCount(const LabelType &label) const
        {
            if (!isDefined() || !hasLabel(label)) return 0;

            return getMarginalPredictedFast(labelToIndex(label)) - getTruePositiveCount(label);
        }

        /**
         * Returns the number of true negative observations with respect to the specified label.
         */
        int getTrueNegativeCount(const LabelType &label) const
        {
            if (!isDefined() || !hasLabel(label)) return 0;

            return getNumInstances() - getTruePositiveCount(label) - getFalsePositiveCount(label) - getFalseNegativeCount(label);
        }

        /**
         * Returns the number of false negative observations with respect to the specified label.
         */
        int getFalseNegativeCount(const LabelType &label) const
        {
            if (!isDefined() || !hasLabel(label)) return 0;

            return getMarginalTruthFast(labelToIndex(label)) - getTruePositiveCount(label);
        }

        /**
         * Equal to TP / (TP + FN).
         * Equivalent to sensitivity, recall.
         */
        double getTruePositiveRate(const LabelType &label) const
        {
            if (!isDefined() || !hasLabel(label)) return 0.0;

            return (double)getTruePositiveCount(label) / getMarginalTruthFast(labelToIndex(label));
        }

        /**
         * Equal to FP / (FP + TN) = 1 - Specificity.
         */
        double getFalsePositiveRate(const LabelType &label) const
        {
            if (!isDefined() || !hasLabel(label)) return 0.0;

            return (double)getFalsePositiveCount(label) / (getNumInstances() - getMarginalTruthFast(labelToIndex(label)));
        }

        /**
         * Equal to TN / (TN + FP).
         * Equivalent to specificity.
         */
        double getTrueNegativeRate(const LabelType &label) const
        {
            if (!isDefined() || !hasLabel(label)) return 0.0;

            return (double)getTrueNegativeCount(label) / (getNumInstances() - getMarginalTruthFast(labelToIndex(label)));
        }

        /**
         * Equal to 1 - TruePositiveRate.
         */
        double getFalseNegativeRate(const LabelType &label) const
        {
            if (!isDefined() || !hasLabel(label)) return 0.0;
            return 1 - getTruePositiveRate(label);
        }

        /**
         * Equal to TP / (TP + FN).
         * Equivalent to TruePositiveRate, Recall.
         */
        double getSensitivity(const LabelType &label) const
        {
            if (!isDefined() || !hasLabel(label)) return 0.0;
            return getTruePositiveRate(label);
        }

        /**
         * Equal to TN / (TN + FP).
         */
        double getSpecificity(const LabelType &label) const
        {
            if (!isDefined() || !hasLabel(label)) return 0.0;
            return getTrueNegativeRate(label);
        }

        /**
         * Equal to TP / (TP + FN).
         * Equivalent to TruePositiveRate, Sensitivity.
         */
        double getRecall(const LabelType &label) const
        {
            if (!isDefined() || !hasLabel(label)) return 0.0;
            return getTruePositiveRate(label);
        }

        /**
         * Equal to TP / (TP + FP).
         */
        double getPrecision(const LabelType &label) const
        {
            if (!isDefined() || !hasLabel(label)) return 0.0;
            return (double)getTruePositiveCount(label) / getMarginalPredictedFast(labelToIndex(label));
        }

        /**
         * Equal to the harmonic mean of recall and precision.
         * F1 = 2 * REC * PREC / (REC + PREC).
         */
        double getFMeasure(const LabelType &label) const
        {
            if (!isDefined() || !hasLabel(label)) return 0.0;
            
            double rec = getRecall(label);
            double prec = getPrecision(label);

            return 2 * rec * prec / (rec + prec);
        }


        /***********************
        | Aggregate statistics |
        ***********************/
        /**
         * Get total number of correctly classified observations.
         */
        int getCorrect() const
        {
            if (!isDefined()) return 0;

            int sum = 0;
            for (int i = 0; i < getNumClasses(); i++)
                sum += getFast(i, i);

            return sum;
        }
        
        /**
         * Get total number of incorrectly classified observations.
         */
        int getIncorrect() const
        {
            if (!isDefined()) return 0;

            return getNumInstances() - getCorrect();
        }

        /**
         * Returns the accuracy of the matrix.
         *   Accuracy = # of correctly classified instances / total # of instances
         */
        double getAccuracy() const
        {
            if (!isDefined()) return 0.0;

            return (double)getCorrect()/ getNumInstances();
        }

        /**
         * Returns the Cohen's kappa measure for the matrix.
         *   Kappa = (O - E) / (1 - E),
         *     where O = observed agreement rate (i.e. Accuracy)
         *           E = expected chance agreement rate (based on marginal probabilities)
         */
        double getKappa() const
        {
            if (!isDefined()) return 0.0;

            double observed = getAccuracy();
            double expected = 0.0;

            for (int i = 0; i < getNumClasses(); i++)
            {
                expected += getMarginalProbTruthFast(i) * getMarginalProbPredictedFast(i);
            }


            return (observed - expected) / (1 - expected);
        }

        /** 
         * Returns the extended G-mean of the matrix.
         *   EGM = geometric mean of class-specific recalls
         */
        double getEGM() const
        {
            if (!isDefined()) return 0.0;

            double egm = 1.0;
            for (int i = 0; i < getNumClasses(); i++)
                egm *= getRecall(indexToLabel(i));

            return pow(egm, (double)1 / getNumClasses());
        }

        double getWeightedRecall() const
        {
            return getWeightedAverage(&Evaluation::getRecall);
        }

        double getWeightedPrecision() const
        {
            return getWeightedAverage(&Evaluation::getPrecision);
        }

        double getWeightedSpecificity() const
        {
            return getWeightedAverage(&Evaluation::getSpecificity);
        }

        double getWeightedSensitivity() const
        {
            return getWeightedAverage(&Evaluation::getSensitivity);
        }

        double getWeightedFMeasure() const
        {
            return getWeightedAverage(&Evaluation::getFMeasure);
        }

        /**
         * Prints text summarizing classification performance to the provided ostream.
         */
        void summarize(std::ostream &out) const
        {
            using std::endl;
            using std::setw;

            // Set leading width to be at least max(length("TRUTH"), maxlength(classLabels))+1
            // Set cellwidth to be at least max(maxlength(classLabels), maxlength(matrixCount))+1

            out << "*****************************************************" << endl;
            out << "Evaluation summary" << endl;
            out << endl;

            // Output confusion matrix
            summarizeConfusionMatrix(out);
            out << endl;
            summarizeStats(out);
            out << endl;
            summarizeClassStats(out);
        }

        void summarizeConfusionMatrix(std::ostream &out) const
        {
            using std::endl;
            using std::setw;

            //          PREDICTED -->
            //              0    1    2    3 
            //  TRUTH +----------------------+
            //      0 |    11    4    0    2 |
            //      1 |     0   14    0    1 |
            //      2 |     3    8   10    0 |
            //      3 |     1    2    4   20 |
            //        +----------------------+
            //
            //!     !@ @#   ##   ##   ##   #@ @
            //
            // ! leadingWidth
            // @ gutterWidth
            // # cellWidth
            // $ trailingWidth

            int leadingWidth = 8;
            int gutterWidth = 3;
            int cellWidth = 4;
            int trailingWidth = 3;

            out << "=== Confusion matrix" << endl;
            out << std::string(leadingWidth + gutterWidth, ' ') << "PREDICTED -->" << endl;

            out << std::string(leadingWidth + gutterWidth, ' ');
            for (int j = 0; j< getNumClasses(); j++)
                out << setw(cellWidth) << indexToLabel(j);
            out << endl;

            out << setw(leadingWidth) << " TRUTH";
            out << " +-" << std::string(cellWidth * getNumClasses(), '-') << "-+" << endl;

            for (int i = 0; i < getNumClasses(); i++)
            {
                out << setw(leadingWidth) << indexToLabel(i);
                out << setw(gutterWidth) << " | ";
                for (int j = 0; j < getNumClasses(); j++)
                {
                    if (getFast(i, j) > 0)
                        out << setw(cellWidth) << getFast(i, j);
                    else
                        out << setw(cellWidth) << '.';
                }
                out << setw(gutterWidth) << " | ";
                out << setw(cellWidth) << getMarginalTruthFast(i);
                out << endl;
            }

            out << setw(leadingWidth) << "";
            out << " +-" << std::string(cellWidth * getNumClasses(), '-') << "-+" << endl;

            out << setw(leadingWidth + gutterWidth) << "";
            for (int j = 0; j < getNumClasses(); j++)
            {
                out << setw(cellWidth) << getMarginalPredictedFast(j);
            }
            out << endl;
        }

        void summarizeStats(std::ostream &out) const
        {
            using std::endl;
            using std::setw;
            using std::setprecision;
            using std::fixed;

            int leadingWidth = 25;
            int precision = 3;

            out << fixed;
            out << "=== Performance summary" << endl;
            out << setw(leadingWidth) << "# of instances: " << getNumInstances() << endl;
            out << setw(leadingWidth) << "Correctly classified: " << getCorrect() << endl;
            out << setw(leadingWidth) << "Incorrectly classified: " << getIncorrect() << endl;
            out << setw(leadingWidth) << "Accuracy: " << setprecision(precision) << getAccuracy() << endl;
            out << setw(leadingWidth) << "Extended g-mean: " << getEGM() << endl;
            out << setw(leadingWidth) << "Cohen's kappa: " << setprecision(precision) << getKappa() << endl;
        }

        void summarizeClassStats(std::ostream &out) const
        {
            using std::endl;
            using std::setw;
            using std::setprecision;
            using std::fixed;

            int labelWidth = 8;
            int statWidth = 7;
            int precision = 3;

            out << fixed;
            out << "=== Class summary" << endl;

            // Print header line
            out << setw(labelWidth) << "Class";
            out << setw(statWidth) << "SENS";
            out << setw(statWidth) << "SPEC";
            out << setw(statWidth) << "REC";
            out << setw(statWidth) << "PREC";
            out << setw(statWidth) << "F";
            out << endl;

            for (int i = 0; i < getNumClasses(); i++)
            {
                LabelType label = indexToLabel(i);

                out << setw(labelWidth) << label;
                out << setw(statWidth) << setprecision(precision) << getSensitivity(label);
                out << setw(statWidth) << setprecision(precision) << getSpecificity(label);
                out << setw(statWidth) << setprecision(precision) << getRecall(label);
                out << setw(statWidth) << setprecision(precision) << getPrecision(label);
                out << setw(statWidth) << setprecision(precision) << getFMeasure(label);
                out << endl;
            }

            out << setw(labelWidth) << "Weighted";
            out << setw(statWidth) << setprecision(precision) << getWeightedSensitivity();
            out << setw(statWidth) << setprecision(precision) << getWeightedSpecificity();
            out << setw(statWidth) << setprecision(precision) << getWeightedRecall();
            out << setw(statWidth) << setprecision(precision) << getWeightedPrecision();
            out << setw(statWidth) << setprecision(precision) << getWeightedFMeasure();
            out << endl;

        }
        //////////////////////////////////////////////////////////////////////////////

    private:
        MapType m_labels;
        vnl_matrix<int> m_matrix;
        
        std::vector<int> m_marginalTruth;
        std::vector<int> m_marginalPredicted;
        unsigned m_numInstances;

        //// C O N S T R U C T O R ///////////////////////////////////////////////////
        Evaluation()
        {
        }
        //////////////////////////////////////////////////////////////////////////////


        //// M E T H O D S ///////////////////////////////////////////////////////////
        /**
         * Initialize evaluation object based on the provided ordered list of class labels.
         */
        void initializeLabels(const std::vector<LabelType> &labels)
        {
            m_labels.clear();

            // Store association between internal index number and class label as a Boost bimap
            for (int i = 0; i < labels.size(); i++)
            {
                m_labels.insert(MapType::value_type(i, labels[i]));
            }

            // Initialize size of confusion matrix, marginal matrices
            m_matrix.set_size(labels.size(), labels.size());
            m_marginalTruth.resize(labels.size());
            m_marginalPredicted.resize(labels.size());
        }

        /**
         * Returns whether the confusion matrix has been defined
         * (i.e. whether it is correctly sized and contains at least one observation).
         */
        bool isDefined() const
        {
            return !m_matrix.empty();
        }

        /**
         * Returns whether the given internal index number is recognized by the Evaluation object.
         */
        bool hasIndex(int index) const
        {
            return m_labels.left.count(index) > 0;
        }

        /**
         * Returns whether the given class label is recognized by the Evaluation object.
         */
        bool hasLabel(const LabelType &label) const
        {
            return m_labels.right.count(label) > 0;
        }

        /**
         * Converts the specified internal index number to a class label.
         * Performs no bounds checking.
         */
        const LabelType &indexToLabel(int index) const
        {
            return m_labels.left.at(index);
        }

        /**
         * Converts the specified class label to the associated internal index number.
         * Performs no bounds checking.
         */
        int labelToIndex(const LabelType &label) const
        {
            return m_labels.right.at(label);
        }

        /**
         * Records the specified observation (provided in index numbers) into the confusion matrix.
         * Performs no bounds checking.
         */
        void recordIndex(int truthIndex, int predictedIndex)
        {
            m_matrix(truthIndex, predictedIndex)++;
            m_marginalTruth[truthIndex]++;
            m_marginalPredicted[predictedIndex]++;
            m_numInstances++;
        }

        /**
         * Retrieves the number of observations associated with the specified truth and predicted indices.
         * Performs no bounds checking.
         */
        int getFast(int truthIndex, int predictedIndex) const
        {
            return m_matrix(truthIndex, predictedIndex);
        }

        /**
         * Returns the marginal number of observations associated with the specified truth index.
         * Performs no bounds checking.
         */
        int getMarginalTruthFast(int truthIndex) const
        {
            return m_marginalTruth[truthIndex];
        }

        /**
         * Returns the normalized marginal probability associated with the specified truth index.
         * Performs no bounds checking.
         */
        double getMarginalProbTruthFast(int truthIndex) const
        {
            return (double)getMarginalTruthFast(truthIndex) / getNumInstances();
        }

        /**
         * Returns the marginal number of observations associated with the specified predicted index.
         * Performs no bounds checking.
         */
        int getMarginalPredictedFast(int predictedIndex) const
        {
            return m_marginalPredicted[predictedIndex];
        }

        /**
         * Returns the normalized marginal probability associated with the specified predicted index.
         * Performs no bounds checking.
         */
        double getMarginalProbPredictedFast(int predictedIndex) const
        {
            return (double)getMarginalPredictedFast(predictedIndex) / getNumInstances();
        }

        /**
         * Returns the average value of the specified metric, weighted by the marginal number of truth observations for each class.
         */
        double getWeightedAverage(double (Evaluation::*fn)(const LabelType &label) const) const
        {
            if (!isDefined()) return 0.0;

            double sum = 0.0;
            for (int i = 0; i < getNumClasses(); i++)
                sum += (this->*fn)(indexToLabel(i)) * getMarginalTruthFast(i);
            
            return sum / getNumInstances();
        }
        //////////////////////////////////////////////////////////////////////////////
    };


    /**
     * Class specialization for factory method. Instantiates a new Evaluation object based on a
     * sequential set of class labels, going from 0 to numLabels - 1.
     */
    Evaluation<int>::Pointer Evaluation<int>::New(int numLabels)
    {
        Pointer returnthis(new Evaluation);

        std::vector<int> labels(numLabels);
        for (int i = 0; i < numLabels; i++)
            labels.push_back(i);

        returnthis->initializeLabels(labels);
        returnthis->clear();
        return returnthis;
    }
    //////////////////////////////////////////////////////////////////////////////////
}


#endif
