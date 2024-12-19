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
    header.append(bormat(max_value, 32))
    # bitnum of vocab value when encoding
    value_bit = int(math.ceil(math.log2(max_value)))
    # add vocab, int: str
    for k, v in vocab.items():
        if v == 0:
            continue
        header.append(bormat(v, value_bit))
        header.append(k)
    # add content length
    header.append(bormat(len(content), 32))
    # add content
    header.append(content)
    return ''.join(header)