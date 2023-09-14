"""Semantic network summary tools

This script contains tools for summarizing a semantic netowork (SN)
in a SimpleMind application.
This SN is a representation of a knowledge base in the SimpleMind application.
"""

from argparse import ArgumentParser
import os, re
import logging
import yaml
from simplemind.goal import semantic_network
import matplotlib.pyplot as plt

def summary_sn_relationships_graph(sn_path, summary_path=None, draw=True, 
                    # figsize=(7,9), frameon=False,
                    # cmap='summer', node_size=500, font_size=8
                    ):
    """get relationships_graph_summary of SN
    Generating relationships graph summary of the SN

    Parameters
    ----------
    sn_path : str
        path to the entry file of the semantic network to summary
    summary_path : str
        path to save text summary file from text summmary tool. (default=None)
        If None, summry file will not be saved.
    draw : bool
        whether to draw the graph visualizatoin
    figure size: tuple of int
        size of figure (default = (7,9))
    frameon : bool
        whether to add frame in the graph visualization or not (default=False)
    cmap : str
        matplotlib.plt color map to draw the graph visualization (default='summer')
    node_size : int
        node size in the graph visualization (default=500)
    font_size : int
        font size in the graph visualization (default=8)

    """
    log = logging.getLogger()
    formatter = logging.Formatter('[%(asctime)s|%(levelname)-5s|%(filename)-19s:%(lineno)-3s] %(message)s')
    ch = logging.StreamHandler()
    ch.setFormatter(formatter)
    log.addHandler(ch)
    log.setLevel(logging.DEBUG)

    sn = semantic_network.BaseSemanticNetwork(sn_path, log)

    log.info('Generating relationships graph summary of the SM model')
    log.info(': showing the node relationship in a graph view')
    log.info('---------------------------------------------------------------')
    G = sn.get_relationship_summary_graph(summary_path, 
                    # figsize=figsize, frameon=frameon, 
                    # cmap=cmap, node_size=node_size, font_size=font_size
                    )
    if summary_path:
        log.info('Please check the output summary figure file: %s' % summary_path)
        log.info('---------------------------------------------------------------')
    return G

def summary_sn_chromosomes(sn_path, summary_path=None, binary_chromosome=None):
    """get summary of chromosome in SN
    Generating summary of chromosomes in the SN

    Parameters
    ----------
    sn_path : str
        path to the entry file of the semantic network to summary
    summary_path : str
        path to save text summary file from text summmary tool. (default=None)
        If None, summry file will not be saved.
    binary_chromosome : str
        binary chromosome to interpret as the parameters in SN. (default=None)
        If None, default chromosome will be used for summary
    """

    log = logging.getLogger()
    # formatter = logging.Formatter('[%(asctime)s|%(name)-10s|%(levelname)-8s|%(filename)-25s:%(lineno)-3s] %(message)s')
    formatter = logging.Formatter('[%(asctime)s|%(levelname)-5s|%(filename)-19s:%(lineno)-3s] %(message)s')
    ch = logging.StreamHandler()
    ch.setFormatter(formatter)
    log.addHandler(ch)
    log.setLevel(logging.DEBUG)

    sn = semantic_network.BaseSemanticNetwork(sn_path, log)

    log.info('Generating chromosome summary of the SM model')
    log.info('---------------------------------------------------------------')
    default_chromosome = sn.get_default_chromosome()
    if binary_chromosome: binary_chr = binary_chromosome
    else: binary_chr = default_chromosome
    sn_dict = sn.get_parameters_from_chromosome(binary_chr = binary_chr, summary_path = summary_path)
    log.info('Please check the output summary file: %s' % summary_path)
    log.info('---------------------------------------------------------------')

    return sn_dict

if __name__=='__main__':
    parser = ArgumentParser(description='Summarize the Semantic Network')
    parser.add_argument('--tool', type=str, dest='tool', default='text_summary', 
                        help="two type of summary tools are supported: \
1) text_summary: summary tool for parsing model and getting default chromosome, \
2) graph_summary: summary tool for showing the node relationship in a graph view. The default is text_summary.")
    parser.add_argument('--sn_path', type=str, dest='sn_path', 
                        help="Path to entry file of SN")
    parser.add_argument('--summary_path', type=str, dest='summary_path', default='none', 
                        help="path to save text summary file from text summmary tool. if none, summry file will not be saved.")
    parser.add_argument('--binary_chromosome', type=str, dest='binary_chromosome', default='none',
                        help="binary_chromosome for text summary tool. if none, default chromosome will be used to summarize the model")
    args = parser.parse_args()
    
    if args.summary_path != 'none': summary_path = args.summary_path
    else: summary_path = None
    if args.binary_chromosome != 'none': binary_chromosome = args.binary_chromosome
    else: binary_chromosome = None
    if args.tool == 'graph_summary':
        summary_sn_relationships_graph(args.sn_path, summary_path=summary_path)
    elif args.tool == 'text_summary':
        summary_sn_chromosomes(args.sn_path, summary_path=summary_path, binary_chromosome=binary_chromosome)
    else:
        raise ValueError(f'Tool name {args.tool} is not supported.')