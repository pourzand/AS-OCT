import numpy as np, math


def binlist_to_hexstr(bin_input):
    """Returns a hexdecimal string from a given list of 0s and 1s.
    Below is an example:
        Command: binlist_to_hexstr([1,1,0,0,1,0,1,1,0])
        Expected ouput: 3D0
    """
    # return (('%%0%sX' % math.ceil(len(str(input))/4)) % int("".join((str(i) for i in reversed(str(input)))),2))[::-1]
    if bin_input is None:
        print("Warning: Conversion binary was None.")
        return None
    bin_input = "".join((str(i) for i in reversed(bin_input)))
    return (('%%0%sX' % math.ceil(len(str(bin_input))/4)) % int(bin_input,2))[::-1]


def Hamming_dist(ind1, ind2):
    diffs = 0
    for bit1, bit2 in zip(str(ind1), str(ind2)):
        if not bit1==bit2:
            diffs+=1
    return np.sqrt(diffs)


def compute_diversity(population, alpha=1):
    diversity = []
    _diversity = []
    n = len(population)
    for i, ind1 in enumerate(population):
        try:
            fitness = ind1[1]
            # if ind1.fitness.values:
            #     fitness = ind1.fitness.values[0]
            # else:
            #     fitness = ind1.fitness.parent_value[0]
        except:
            fitness = 1.0
        ind_div = 0
        for k, ind2 in enumerate(population):
            if i == k:
                continue
            dist = Hamming_dist(ind1[0], ind2[0])
            ind_div += dist
        div = ind_div/(n-1)
        f_norm_div = ind_div/(n-1) + alpha * fitness  # fitness normalized diversity
        # print(i, f_norm_div, fitness, binlist_to_hexstr(ind1))
        _diversity.append([f_norm_div, div, fitness, " ", binlist_to_hexstr(ind1[0])])
        diversity.append(f_norm_div)
    overall_diversity = np.mean(diversity)
    sorted_diversity = sorted(zip(diversity, population), reverse=True, key=lambda t: t[0])
    _diversity = sorted(_diversity, reverse=True, key=lambda t: t[2])
    for i, v in enumerate(sorted_diversity):
        d, ind = v
        for item in _diversity:
            if binlist_to_hexstr(ind[0])==item[4]:
                item[3]=i+1
                break

    return overall_diversity, sorted_diversity, _diversity


# E891C75AB2F8E2A 1.6949999999999998. - 011100011001100000111110101001011101010011110001011101000101
# E891C71AB3F8E2A 1.6949999999999998 - 011100011001100000111110100001011101110011110001011101000101
# E890C71BB389E0A 1.685 - 011100011001000000111110100011011101110000011001011100000101
# E8D0C71AB3F8E2A 1.685 - 
# E890C71AB399E2A 1.685 - 
# E890D71B9389E2A 1.685 - 
# E891C75AB3F8E2A 1.685 - 
# E890D71AB3F8E2A 1.685 - 
# E891C71AA3F8E2A 1.68 - 
# D341C51B97D264C 1.68 - 101111000010100000111010100011011001111010110100011000100011
if __name__=="__main__":

    pop =  [    ("011100011001100000111110101001011101010011110001011101000101", 1.695),
                ("011100011001100000111110100001011101110011110001011101000101", 1.695),
                ("011100011001000000111110100011011101110000011001011100000101", 1.685),
                ("011100011011000000111110100001011101110011110001011101000101", 1.685),
                ("011100011001000000111110100001011101110010011001011101000101", 1.685),
                ("011100011001000010111110100011011001110000011001011101000101", 1.685),
                ("011100011001100000111110101001011101110011110001011101000101", 1.685),
                ("011100011001000010111110100001011101110011110001011101000101", 1.685),
                ("011100011001100000111110100001010101110011110001011101000101", 1.68),
                ("101111000010100000111010100011011001111010110100011000100011", 1.68),
                ("101110001000001110110110100011011001111010110100011000100011", 1.68),
                ("011100011001000010111110100001011101110011110001011101000101", 1.68),
    ]

    # print(compute_diversity(pop)[1])

    [print(v) for v in compute_diversity(pop)[2]]
    [print(v) for v in compute_diversity(pop)[1]]