"""Semantic network

This script contains a semantic netowork (SN) class in a SimpleMind application.
This SN is a representation of a knowledge base in the SimpleMind application.
"""
from argparse import ArgumentParser
import os, re
from functools import partial
import yaml
from termcolor import colored
import pandas as pd
import numpy as np
import copy
import logging
logging.getLogger('matplotlib').setLevel(logging.ERROR)
logging.getLogger("PIL.PngImagePlugin").setLevel(logging.ERROR) # PIL logs too much.
logging.getLogger("matplotlib.font_manager").setLevel(logging.ERROR) # matplotlib logs too much.

import matplotlib.pyplot as plt
import networkx as nx
from networkx.drawing.nx_agraph import graphviz_layout


class BaseSemanticNetwork(object):
    """SM Base Model Class
    Abstract base class of a class family for the Semantic Network (SN) 
    in the SimpleMind application.

    Parameters
    ----------
    sn_path : str
        path to the entry file of SN.
        Assume that every node exposed in the SN is saved in 
        the same directory that the model file is held.
    log : object
        logging class instance

    Attributes
    ----------
    self.sn_path
        path to the entry file of SN
    self.log : object
        logging class instance
        
    Methods
    -------
    get_dict_nodes(self):
        Return a dictionary of nodes in the SN
        
    get_list_relationship(self):
        Return a list of relationships between nodes in the SNM
    
    def get_dict_chromosome_bits(self):
        Return a dictionary of chromosomes in the SN
    
    get_relationship_summary_graph(self, draw=True, figsize=(7,9), frameon=False,
                    cmap='summer', node_size=500, font_size=8)
        Return a graph instance of the visualization on the SN relationships.
        If you set draw=True, a matplotlob.pyplot.figure instance will be returned together.

    Examples
    --------
    Python API examples
    ```
    import simplemind.goal.semantic_network as semantic_network

    sn_entry_path = f'/radraid/apps/personal/youngwon/debug_miu/cxr_model.9/et_tip_placement_model'
    
    log = logging.getLogger()
    formatter = logging.Formatter('[%(asctime)s|%(name)-10s|%(levelname)-8s|%(filename)-25s:%(lineno)-3s] %(message)s')
    ch = logging.StreamHandler()
    ch.setFormatter(formatter)
    log.addHandler(ch)
    log.setLevel(logging.DEBUG)

    sn = semantic_network.BaseSemanticNetwork(sn_entry_path, log)

    # relationship summary
    print(sn.get_list_relationship())
    G, fig = sn.get_relationship_summary_graph(figsize=(5,10), font_size=12)
    fig.show()

    # text summary
    default_chromosome = sn.get_default_chromosome()
    ```

    """
    def __init__(self, sn_path, log, verbose=2):
        """
        Parameters
        ----------
        sn_path : str
            Path to a semantic network (SN) entry file.
            Assume that every node exposed in the SN is saved in 
            the same directory that the entry file is held.
        log : object
            logging class instance
        """
        self.sn_path = sn_path
        self.log = log
        self.log.info('---------------------------------------------------------------')
        self.log.info('Semantic Network (SN): Simplemind Knowledge Representation')
        self.log.info('---------------------------------------------------------------')
        self.log.info(f'SN entry file path: \n{sn_path}')
        assert os.path.exists(self.sn_path), f"[ERROR] SN entry file path {sn_path} is not available."
        self._read_nodes()
        self.log.info('---------------------------------------------------------------')
        self._get_relationships()
        self.log.info('---------------------------------------------------------------')
        self._find_chrom_bits()
        self.log.info('---------------------------------------------------------------')
    
    def _read_nodes(self):
        with open(self.sn_path) as f:
            nodes = f.readlines()
        start_num = [i for i in range(len(nodes)) if 'Model:' in nodes[i]][0]
        end_num = [i for i in range(len(nodes)) if 'End:' in nodes[i]][0]
        nodes = nodes[start_num+2:end_num]
        nodes = [node.strip() for node in nodes]
        nodes = nodes[::-1]
        self._nodes = nodes
        self.log.info(f'Node list exposed from the model: \n{self._nodes}')
    
    def _get_relationships(self):
        self.log.info('Read relationship')
        self.log.info('---------------------------------------------------------------')
        list_relationship = []
        dict_nodes = {}
        sn_entry_point = os.path.dirname(self.sn_path)
        reverse_nodes = sorted(self._nodes, key=len, reverse=True)
        for child_node in self._nodes:
#             self.log.debug('---------------------------------------------------------------')
            self.log.info(f'Node: {child_node}')
            child_node_path = os.path.join(sn_entry_point, child_node)
            if not os.path.exists(child_node_path):
                self.log.info(f"No node file: {child_node_path}")
            else:
                with open(child_node_path, 'r') as f:
                    raw_contents = f.read()
                with open(child_node_path, 'r') as f:
                    contents = ''.join(f.readlines()[1:]).split('End:')[0]

                parent_list = []
                nodes_except_current = [x for x in reverse_nodes if x is not child_node]
    #             self.log.debug(nodes_except_current)
                contents_check = copy.copy(contents)
                for x in nodes_except_current:
    #                 self.log.debug(f'check {x}')
                    if np.all([x in contents_check,
                            f'{x}_' not in contents_check,
                            f'_{x}' not in contents_check]
                            ):
                        parent_list.append(x)
    #                     self.log.debug(f'include {x} and replace')
                        contents_check = contents_check.replace(x, '')
    #                     self.log.debug(f'\n{contents_check}\n')
                # if not parent_list: 
                #     parent_list = []
                self.log.info(f'\tparents: {parent_list}')
                is_cnn = ('NeuralNetKeras' in contents)
                dict_nodes[child_node] = {'name': child_node, 'contents': ''.join(raw_contents),
                                        'is_cnn': is_cnn, 
                                        'parents':parent_list, 'value': (3 if is_cnn else 2)}
                
        self._dict_nodes = dict_nodes
        
        list_relationship = []
        for child in self._nodes:
            list_relationship.extend([(parent, child) for parent in dict_nodes[child]['parents']])
#             self.log.debug(list_relationship)
        self._list_relationship = list_relationship
        self.log.info('---------------------------------------------------------------')
    
    def _find_chrom_bits(self, verbose=2):
        self.log.info('Read chromosome bits')
        self.log.info('---------------------------------------------------------------')
        self._chr_bits = []
        for child_node in self._nodes:
#             self.log.debug('---------------------------------------------------------------')
            self.log.info(f'Node: {child_node}')
            chr_bits_in_node = self._find_chrom_bits_from_contents(self._dict_nodes[child_node]['contents'], verbose)
            """TODO: cleanup the char bit indicator"""
            self._dict_nodes[child_node]['chr_in_node_dict'] = chr_bits_in_node
            self._chr_bits.extend(chr_bits_in_node)

    def _find_chrom_bits_from_contents(self, contents, verbose=2):
        node_id = None
        chr_bits_in_node = []
        for l, line in enumerate(contents.split("\n")):
            if l==0:
                node_id = line.replace("AnatPathEntity: ", "").strip().strip(";")
                if verbose: self.log.info(f"node id: {node_id}")
            else:
                if line[:4]=="End:":
                    if verbose: self.log.info(f"Finish read {node_id}")
                    break
                node_line_id = f"{node_id}_{l}"
                for interpret_bits in self._interpret_bit_line(line, node_line_id, verbose):
                    default_val, gene_group_id, bit_id, ind_start, ind_end, possible_values, bit_func, default_decimal_index, default_binary_index, colored_line = interpret_bits
                    if default_val is None: continue
                    chr_bits_in_line = dict(node_id=node_id, gene_group_id=gene_group_id, bit_id=bit_id, 
                                possible_values=possible_values,
                                ind_start=ind_start, ind_end=ind_end, 
                                default_val=default_val,
                                default_decimal_index=default_decimal_index, 
                                default_binary_index=default_binary_index,
                                bit_func=bit_func, 
                                line=line, colored_line=colored_line)
                    chr_bits_in_node.append(chr_bits_in_line)
        return chr_bits_in_node
    
    def _interpret_bit_line(self, line, node_line_id, verbose=2):
        # w = re.compile(r"\s\S*\s\{[\S\s]*?\}")  
        w = re.compile(r"[0-9\-.ef]*\s\{[\S\s]*?\}") # searching for "default_val {ind_start, ind_end, val_start, val_end}""
        if w.search(line) is None:
            return None, None, None, None, None, None, None, None, None, None
        genes = re.findall(w, line)
        genes = [gene.strip() for gene in genes]
#         w = re.compile(r"[\w\s]+[\[|\-|\d]")
        w = re.compile(r"^([^\s]+)") #.+?(?=(\s\S*\s\{))
        gene_group_id = w.search(line).group().strip()
        if verbose:
            self.log.info("----------------------------------------------------------")
            self.log.info(f"From line: {line}")
            self.log.info(f"gene_group_id: {gene_group_id}")
            self.log.info(f"genes: {genes}")
            self.log.info("----------------------------------------------------------")
        
        for i, gene in enumerate(genes):
            default_val, ind_start, ind_end, val_start, val_end = [x.strip() for x in re.split('[\{\},]', gene)[:-1]]
            default_val, val_start, val_end = float(default_val), float(val_start), float(val_end)
            ind_start, ind_end = int(ind_start), int(ind_end)
            bit_id = f"{node_line_id}_{gene_group_id}_{i}"
            start_num = line.find(gene)
            colored_line = line[:start_num] + colored(gene, 'red') + line[start_num+len(gene):]

            bit_func = partial(self._bit_function, start_bit=ind_start, stop_bit=ind_end, low_value=val_start, high_value=val_end)
            possible_values = self._get_possible_values(ind_start, ind_end, val_start, val_end)
            default_decimal_index = round(self._reverse_bit_function(default_val, ind_start, ind_end, possible_values[0], possible_values[-1]))
            default_binary_index = str(format(default_decimal_index, '0%sb'%str(ind_end - ind_start+1)))[::-1]

            if verbose:
                self.log.info(f"Parsing {i}th gene:'{gene}' from line")
                self.log.info(f"{colored_line}")
                self.log.info(f"bit id: {bit_id}")
                self.log.info(f"ind [start, end]: [{ind_start}, {ind_end}]")
                # self.log.info(f"val [start, end]: [{val_start}, {val_end}]")
                self.log.info(f"possible_values: {possible_values}")
                self.log.info(f"default_val: {default_val}")
                self.log.info(f"default_index: decimal = {default_decimal_index}, binary =  {default_binary_index}")
                self.log.info("----------------------------------------------------------")
            yield default_val, gene_group_id, bit_id, ind_start, ind_end, possible_values, bit_func, default_decimal_index, default_binary_index, colored_line
            
    def _get_possible_values(self, start_bit, stop_bit, low_value, high_value):
        gene_max_value = int(2.0**(stop_bit-start_bit+1) - 1)
        possible_values = [low_value + ((high_value-low_value)*gene_value/gene_max_value) for gene_value in range(gene_max_value+1)]
        return possible_values
    
    def _reverse_bit_function(self, default_val, start_bit, stop_bit, low_value, high_value):
        default_val = float(default_val)
        gene_max_value = 2.0**(stop_bit-start_bit+1) - 1
        gene_value = ( default_val - low_value )* gene_max_value/(high_value-low_value)
        return gene_value

    def _bit_function(self, binary_chr, start_bit, stop_bit, low_value, high_value):
        bit_section = binary_chr[start_bit:stop_bit+1]
        try:
            gene_value = int(bit_section,2)
            gene_max_value = 2.0**(stop_bit-start_bit+1) - 1
            v = low_value + ((high_value-low_value)*gene_value/gene_max_value)
            return v
        except:
            pass

    def _get_parameters_from_chromosome(self, binary_chr):
        for bit_dict in self._chr_bits:
            value = bit_dict["bit_func"](binary_chr)
            bit_dict["value"] = value
            yield bit_dict

    def get_list_nodes(self):
        return self._nodes

    def get_list_relationship(self):
        return self._list_relationship
    
    def get_dict_nodes(self):
        return self._dict_nodes

    def get_dict_chromosome_bits(self):
        return self._chr_bits
        
    def get_default_chromosome(self, verbose=2):
        self.log.info(f'Start calculation the default chromosome')
        self.log.info("----------------------------------------------------------")
        default_chromosome = ""
        colored_previous_chromosome = default_chromosome
        if verbose:
            self.log.info("----------------------------------------------------------")
            self.log.info("Start calculating default chromosome")
        df_chr_bits = pd.DataFrame(self._chr_bits)
        # self.log.debug(df_chr_bits)
        df_chr_bits = df_chr_bits[~df_chr_bits[['ind_start','ind_end','default_val']].duplicated()]
        df_chr_bits = df_chr_bits.sort_values(by="ind_start")
        ordered_chr_bits = df_chr_bits.to_dict("records")
        
        for bit_segment in ordered_chr_bits:
            if verbose:
                self.log.info("----------------------------------------------------------")
                self.log.info(f"bit_id: {bit_segment['bit_id']}")
            decimal_index = bit_segment['default_decimal_index']
            binary_index = bit_segment['default_binary_index']
            default_chromosome+=binary_index
            colored_previous_chromosome = default_chromosome[:-len(binary_index)]
            colored_addended_index = colored(default_chromosome[-len(binary_index):], "red")
            if verbose:
                self.log.info(f"decimal_index {decimal_index} --> binary_index {binary_index}")
                ind_start = bit_segment['ind_start']
                ind_end = bit_segment['ind_end']
                self.log.info(f"bit length: {ind_end} - {ind_start+1} from {ind_start} to {ind_end}")
                self.log.info(f"default_val: {bit_segment['default_val']} out of {bit_segment['possible_values']}")
                self.log.info(f"line: {bit_segment['colored_line']}")
                self.log.info(f"Appended default_chromosome: {colored_previous_chromosome}{colored_addended_index}")
        if verbose:
            self.log.info("----------------------------------------------------------")
            self.log.info("Finished calculating default_chromosome")
            self.log.info("----------------------------------------------------------")
            self.log.info(f"Result default_chromosome: {default_chromosome}")
            self.log.info(f"Length of default_chromosome: {len(default_chromosome)}")
            self.log.info("----------------------------------------------------------")
            
        return default_chromosome
    
    def get_parameters_from_chromosome(self, binary_chr=None, summary_path=None, verbose=2):
        if binary_chr:
            self.log.info(f'Start interpreting the binary chromosome {binary_chr}')
            self.log.info("----------------------------------------------------------")
        else:
            """TODO: use default chromosome"""
            self.log.info(f'Chromosome is Not given. Assume to interpret with the default chromsome.')
            self.log.info("----------------------------------------------------------")
            binary_chr = self.get_default_chromosome(verbose=verbose)
            self.log.info(f'Start interpreting with the default chromosome {binary_chr}')
            self.log.info("----------------------------------------------------------")

        node_dict = dict()
        for params in self._get_parameters_from_chromosome(binary_chr):
            # print(params)
            # for params in model_reader.output_parameters(binary_chr):
            # k = ",".join((params["node_id"][:-1], params["bit_id"]))
            k = "_".join((params["node_id"], params["bit_id"]))
            # print(k)
            node_dict[k] = params
        if summary_path:
            with open(summary_path, 'w') as f:
                f.write(yaml.dump(dict(nodes=node_dict, node_list=self._nodes[1:], sn_entry_path=self.sn_path)))
        return node_dict
    
    def get_relationship_summary_graph(self, figure_path=None, 
                        # figsize=(7,9), frameon=False, #hover=False, # not used for graphviz style
                        #cmap='summer', node_size=500, font_size=8 # not used for graphviz style
                        ):
        """get graph summary of the relationshipts in Semantic Network
        
        Parameters
        ----------
        figure_path : str
            path to save the relational links graph (default=None)
            By default, it will not save the figure but return the graph instance.
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
        self.log.info('Generating graph visualization of the Semantic Network')
        self.log.info('---------------------------------------------------------------')

        G = nx.DiGraph(directed=True)
        G.add_nodes_from(self._nodes)
        nx.set_node_attributes(G, self._dict_nodes)
        G.add_edges_from(self._list_relationship)

        A = nx.nx_agraph.to_agraph(G)  # convert to a graphviz graph
        # X1 = nx.nx_agraph.from_agraph(A)  # convert back to networkx (but as Graph)
        # X2 = nx.Graph(A)  # fancy way to do conversion
        # G1 = nx.Graph(X1)  # now make it a Graph
        
        if figure_path is not None:
            # fig = plt.figure(figsize=figsize, frameon=frameon)
            # ax = fig.add_axes([0, 0, 1, 1])
            # ax.axis('off')
            # pos=graphviz_layout(G, prog='dot')
            # # pos=graphviz_layout(G, prog='neato')
            # node_values = [self._dict_nodes[node]['value'] for node in self._nodes]
            # annot_font_size = int(font_size*0.8)
            # nx.draw_networkx(G, pos, 
            #                 cmap=plt.get_cmap(cmap), node_color = node_values, 
            #                 node_size = node_size, font_size=font_size,
            #                 horizontalalignment='center', verticalalignment='center', 
            #                 arrows=True, arrowsize=20,
            #                 )
            # annot = ax.annotate("", xy=(0,0), xytext=(annot_font_size,annot_font_size),textcoords="offset points",
            #                     bbox=dict(boxstyle="round", fc="w"),
            #                     arrowprops=dict(arrowstyle="->"))
            # annot.set_visible(False)
            # fig.show()
            # plt.savefig(figure_path)

#             idx_to_node_dict = {}
#             for idx, node in enumerate(G.nodes):
#                 idx_to_node_dict[idx] = node

#             def _update_annot(self, ind):
#                 node_idx = ind["ind"][0]
#                 node = idx_to_node_dict[node_idx]

#             def _hover(self, event, fig, G):
#                 vis = annot.get_visible()
#                 if event.inaxes == ax:
#                     cont, ind = draw_nodes.contains(event)
#                     if cont:
#                         sm_model._update_annot(ind)
#                         annot.set_visible(True)
#                         fig.canvas.draw_idle()
#                     else:
#                         if vis:
#                             annot.set_visible(False)
#                             fig.canvas.draw_idle()
#             fig.canvas.mpl_connect("motion_notify_event", _hover)

            summary_dot_path = figure_path.replace('.png','.dot')
            A.write(summary_dot_path) 
            # X3 = nx.nx_agraph.read_dot(summary_dot_path)
            A.draw(figure_path, args='-Gsize=20 -Gratio=1.', prog="dot")
        return G

if __name__=='__main__':
    parser = ArgumentParser(description='Semantic Network summary')
    parser.add_argument('--tool', type=str, dest='tool', default='chromosome_text_summary', 
                        help="two type of summary tools are supported: \
1) chromosome_text_summary: summary tool for parsing model and getting default chromosome, \
2) relationships_graph_summary: summary tool for showing the node relationship in a graph view. The default is text_summary.")
    parser.add_argument('--sn_path', type=str, dest='sn_path', 
                        help="Path to the entry file of Semantic Network to summarize")
    parser.add_argument('--summary_path', type=str, dest='summary_path', default='none', 
                        help="path to save text summary file from text summmary tool. if none, summry file will not be saved.")
    parser.add_argument('--binary_chromosome', type=str, dest='binary_chromosome', default='none',
                        help="binary_chromosome for text summary tool. if none, default chromosome will be used to summarize the SN")
    args = parser.parse_args()

    log = logging.getLogger()
    formatter = logging.Formatter('[%(asctime)s] [%(name)-6s|%(levelname)-6s|%(filename)-20s:%(lineno)-3s] %(message)s',
                        '%Y-%m-%d %H:%M:%S')
    ch = logging.StreamHandler()
    ch.setFormatter(formatter)
    log.addHandler(ch)
    log.setLevel(logging.DEBUG)

    sn = BaseSemanticNetwork(args.sn_path, log)
    
    if args.tool == 'relationships_graph_summary':
        G = sn.get_relationship_summary_graph(args.summary_path)
        if args.summary_path:
            log.info('Please check the output summary figure file: %s' % args.summary_path)
            log.info('---------------------------------------------------------------')
    elif args.tool == 'chromosome_text_summary':
        default_chromosome = sn.get_default_chromosome()
        if args.binary_chromosome.strip().lower() == "none":
            binary_chr = default_chromosome
        else:
            binary_chr = args.binary_chromosome.strip()
        sn.get_parameters_from_chromosome(binary_chr = binary_chr, 
                                summary_path = args.summary_path)
        log.info('Please check the output summary file: %s' % args.summary_path)
        log.info('---------------------------------------------------------------')
    else:
        raise ValueError(f'Tool name {args.tool} is not supported.')
