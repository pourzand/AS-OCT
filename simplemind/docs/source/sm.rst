.. highlight:: shell

######################################
SimpleMind
######################################

*A virtual data scientist built using Cognitive AI*

******************************************************
Cognitive AI Framework
******************************************************

SimpleMind (SM) is an open source software framework that supports deep neural networks (DNNs) with higher level machine reasoning and automatic parameter tuning. The framework can be used to add reasoning to pre-existing DNNs and/or to train DNNs with pre/post processing and end-to-end parameter optimization. It makes it easier for a data scientist to develop knowledge-based AI applications and can achieve better segmentation accuracy and reliability in clinical practice. The current applications are in medical image segmentation and analysis.

Deep learning segmentation algorithms are data driven, but rely on human (data scientist) knowledge for hand tuning of deep neural network (DNN) architectures and learning hyper parameters and applying knowledge ad hoc in heuristic pre/post processing algorithms. This knowledge is coded ad hoc in scripts, with limited application of common sense reasoning and limited hand tuning of parameters (both in learning and pre/post processing). The result is application-specific Narrow AI, that is typically suboptimal in terms of parameter search and limited as to the level of knowledge and reasoning applied. These shortfalls impact the performance of AI systems and leaves them vulnerable to errors that are obvious to a human, resulting in a loss of trust and limiting real-world clinical application and adoption.

Cognitive AI can address the limitations of Narrow AI by melding high-level conceptual knowledge with pattern recognition. Cognitive AI is broadly defined as enabling human-level reasoning and intelligence. It can provide a layer of machine reasoning atop DNNs, applying knowledge where representative training data may be limited and enabling conceptual decision making (e.g., whether a device is properly positioned relative to an anatomical landmark). Crucially, reasoning can be used to avoid common sense (“dumb”) mistakes that violate basic high-level concepts (e.g., anatomical constraints).

The SM framework brings transparency and automation to the tasks of the data scientist. A high level model represents knowledge usually incorporated ad hoc by the data scientist, including descriptions of scene content and concepts and meta knowledge about learning and image processing. Using this model, SM can automatically chain together a series of general-purpose agents for image pre/post processing DNNs, and machine reasoning. It provides end-to-end automatic parameter tuning (APT) to intelligently explore combinations of agent parameters, enabling a more extensive, unbiased search than manual tuning by a human data scientist and resulting in a more optional solution. The increased level of intelligence brings two major benefits: 

#) making it easier for a data scientist to teach, train, and optimize the AI, and
#) better segmentation accuracy and reliability, with common sense reasoning to avoid obvious mistakes.

In development at our research center since 2016, the SM framework has improved the performance of 
many medical imaging applications, and we believe that there is strong potential utility 
for other research and commercial groups.

.. tip:: 

    *We sometimes think that deep learning models are simply fed data and learn....*

    .. image:: ./data/Current_DL_Layers_1.jpg
        :width: 350
        :align: center


    |
    *...but the reality is that extensive ad hoc human knowledge is applied implicitly in the form of pre-processing of images, post-processing of DNN outputs, and in hand tuning of the associated parameters as well as the learning hyper parameters....*

    .. image:: ./data/Current_DL_Layers_2.jpg
        :width: 350
        :align: center


    |
    *...SimpleMind provides a true Cognitive Layer that explicitly describes the pre- and post- processing and allows machine reasoning from multiple related DNNs, as well as providing automated parameter tuning.*

    .. image:: ./data/Cognitive_Layer.jpg
        :width: 350
        :align: center

    |

******************************************************
Technology Overview
******************************************************

The SimpleMind framework provides both completed image analysis solutions for several public datasets, 
as well as a software development toolkit (SDK) to build a custom model. The new Cognitive AI framework is general-purpose and extensible, supporting AI development tasks that currently require human intelligence, to accomplish them more efficiently and optimally by:

#) adding a domain knowledge base (model) and methods for reasoning about scene content such as spatial inferencing and conditional reasoning to check neural network outputs,
#) adding general-purpose process knowledge, in the form of software agents, that can be chained together to accomplish image pre-processing, outputs, neural network prediction, and result post-processing, and
#) performing automatic end-to-end automatic optimization of all agent parameters and learning hyper parameters to specific medical image segmentation problems.

SM models are semantic networks that embed functional processing elements (e.g. neural networks) within a relational knowledge base that allows higher level machine reasoning to be applied to both guide the neural network processing and check the validity of their outputs. Model nodes support existing processing tools/agents (e.g. Tensorflow) and cognitive elements (e.g. anatomical rules); agents can be added to incorporate more tools (e.g. PyTorch, NVIDIA Clara, nnUNet) or custom algorithms.  

During training, an SM optimizer (e.g., a genetic algorithm) manages the training process, providing end-to-end automatic parameter tuning of each node in the SM model and supporting parallelized GPU/CPU computing. The tuning includes DNN learning hyperparameters and automates the work of the data scientist. Training and executing a SM model is initiated by an easy-to-use SM runner, accessible by command-line or by python API. A toolkit with summary and visualization tools is also provided to easily track intermediate results of SM model optimization (parameter search progression, node-wise outputs, optimized reasoning rules, computational resource management) to understand model optimization and the final output. Altogether, the SM framework enables a comprehensive Cognitive AI approach to better address complex problems requiring both data-driven pattern-recognition and knowledge-driven conceptual rules, with end-to-end parameter optimization.


******************************************************
Embedding DNNs
******************************************************

The technology is based on a previously developed Blackboard architecture where agents contribute to the matching of image regions to entities (nodes) in the semantic network. The SM framework allows embedding DNNs within a Cognitive AI framework, including both image content knowledge (e.g., structural and spatial relationships) and processing agent knowledge (e.g., image enhancement and morphological operations). We have currently developed two embedding methods:

#. as a “segmentation agent” that segments image regions as candidates for matching to an anatomic/pathologic entity in the network; and
#. as a “matching agent” that matches (classifies) candidates as an anatomic/pathologic entity based on its characteristic features.


DNN as Segmentation Agent
============================================
The semantic network describes spatial relationships between anatomic/pathologic entities and provides a hierarchical order for segmenting each entity. When a particular entity is scheduled for segmentation an image search region is computed based on its spatial relationships to previously segmented entities. A segmentation agent is then invoked to generate candidates, one or more of which are then matched (classified) as the entity based on characteristic numerical features. A trained CNN pixel/voxel classifier is embedded as an agent and regions are generated by the agent as candidates.


DNN as Matching Agent
============================================
Once candidates have been generated for an entity (by one or more Segmentation agents that may or may not be DNN based) a trained DNN may be used as the agent to classify each candidate as being matched or not matched to the entity. For example, a cubic ROI can be formed around the candidate ROI and the pixel/voxel intensities used as inputs to the DNN which outputs a binary (yes/no) classification result.


Benefits
============================================
This embedding of the DNN in the semantic network confers two primary advantages:

1. It allows explicit higher level knowledge and post processing to be applied:
   
   1. Search area in which to apply the segmentation CNN (via spatial relationships in the semantic network).
   2. ROI labeling using an explicit flexible vocabulary of higher entity level relational features.
   3. Additional segmentation agents can be applied by the system for CNN segmentation result (ROI) refinement, e.g., using morphological operators etc

2. It allows initial segmentation results to be generated, without CNNs, using very limited training data (since the initial semantic network can be constructed using declarative knowledge engineering, rather than machine learned). This training data may be used for either a segmentation or matching CNN.
   
   1. Segmentation agents based on simpler intuitive methods can be used initially such as intensity thresholding.
   2. These results can be used to generate the necessary training sets for CNN learning (assuming the results of the simpler methods are good enough to make manually editing feasible for large training set generation).



.. _SM Model Attributes: ./attributes.rst
