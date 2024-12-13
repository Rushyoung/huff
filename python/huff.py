import collections

dict = lambda: collections.defaultdict(int)


def encoding(string: str) -> str:
    result = []
    for b in bytearray(string, 'utf-8'):
        result.append(bin(b)[2:])
    return ''.join(result)

def decoding(string: str) -> str:
    result = bytearray()
    for i in range(0, len(string), 8):
        result.append(int(string[i:i+8], 2))
    return result.decode('utf-8')


class node:
    def __init__(self, freq, seq):
        self.freq = freq
        self.seq = seq
        self.left = None
        self.right = None


# 用于生成适当长度的二进制片段
class BPE:
    def __init__(self, input_bin):
        self.bin: str = input_bin
        self.bin_list = list(self.bin)
        self.vocab = dict()
        self.pairs = dict()
        self.token = dict()

    def train(self, size = None) -> None:
        if size == None:
            size = len(self.bin) // 4
        assert 0 < size,             "size must be greater than 0"
        assert size < len(self.bin), "size must be less than the length of the input binary string"
        self._deal_vocab()
        self._deal_token()
        for i in range(size):
            self._deal_pairs()
            self._merge_vocab()

    def _deal_vocab(self) -> None:
        self.vocab['0'] = self.bin.count('0')
        self.vocab['1'] = len(self.bin) - self.vocab[0]

    def _deal_token(self) -> None:
        self.token = self.vocab.copy()

    def _deal_pairs(self) -> None:
        self.pairs = dict()
        self.pair_pos = collections.defaultdict(list)
        for idx, b in enumerate(self.bin_list):
            if(idx == 0):
                continue
            pair = (self.bin_list[idx-1], b)
            self.pairs[pair] += 1
            self.pair_pos[pair].append(idx-1)

    def _merge_vocab(self) -> None:
        best_pair = max(self.pairs, key=self.pairs.get)
        if self.pairs[best_pair] == 1:
            return
        best_bin  = best_pair[0] + best_pair[1]
        length = len(self.bin_list)
        offset = 0
        for pos in self.pair_pos[best_pair]:
            idx = pos - offset
            if idx < 0 or idx >= length:
                continue
            if self.bin_list[idx] != best_pair[0] or self.bin_list[idx+1] != best_pair[1]:
                continue
            self.bin_list[idx] = best_bin
            self.bin_list.pop(idx+1)
            offset += 1
        freq = self.pairs[best_pair]
        self.token[best_bin] = freq
        self.token[best_pair[0]] -= freq
        self.token[best_pair[1]] -= freq

    def build(self) -> node:
        self.vocab = dict()
        for seq in self.bin_list:
            self.vocab[seq] += 1
        nodes = [ node(freq, seq) for seq, freq in self.vocab.items() ]
        while len(nodes) > 1:
            nodes.sort(key=lambda x: x.freq)
            left = nodes.pop(0)
            right = nodes.pop(0)
            parent = node(left.freq + right.freq, left.seq + right.seq)
            parent.left = left
            parent.right = right
            nodes.append(parent)
        return nodes[0]


example = '这是一个测试句子，用于测试哈夫曼编码'

bin_str = encoding(example)
print(bin_str)


bpe = BPE(bin_str)
bpe.train()
bpe.build()



print(bpe.bin_list)
print(bpe.vocab)