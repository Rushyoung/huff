import math

def bormat(value, bitnum):
    assert type(value) == str or type(value) == int, "value must be str or int"
    if type(value) == str:
        value = ord(value)
    b = bin(value)[2:]
    return '0' * (bitnum - len(b)) + b

def header_generate(vocab, content):
    header = []
    # signature hfc
    header.append(bormat('H', 1))
    header.append(bormat('F', 1))
    header.append(bormat('C', 1))
    # mapping length
    header.append(bormat(len(vocab), 32))
    # max value of vocab {str: int}
    max_value = max(vocab.values())
    max_seq_len = max(len(k) for k in vocab.keys())
    header.append(bormat(max_value, 32))
    # bitnum of vocab value when encoding
    value_bit = int(math.ceil(math.log2(max_value)))
    seql_bit  = int(math.ceil(math.log2(max_seq_len)))
    # add vocab, int: str
    for k, v in vocab.items():
        if v == 0:
            continue
        header.append(bormat(v, value_bit))
        header.append(bormat(len(k), seql_bit))
        header.append(k)
    # add content length
    header.append(bormat(len(content), 32))
    # add content
    header.append(content)
    return ''.join(header)