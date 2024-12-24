import collections

import hffl

dict = lambda: collections.defaultdict(int)

batch = 8
min_freq = 2
max_vocab = None
div_chuck = 128

def encoding(string: str) -> str:
    result = []
    for b in bytearray(string, 'utf-8'):
        s = bin(b)[2:]
        s = '0' * (8 - len(s)) + s
        result.append(s)
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
        self.code = ''
        self.weight = freq * len(seq)


def create_node(weight):
    n = node(0, '')
    n.weight = weight
    return n

# 用于生成适当长度的二进制片段
class BPE:
    def __init__(self, input_bin):
        self.bin: str = input_bin
        # 每4个字符为一组，用于统计频率
        self.bin_list = [self.bin[i:i+batch] for i in range(0, len(self.bin), batch)]
        assert len(self.bin_list) * batch == len(self.bin), 'must be whole number of batch'
        self.vocab = dict()
        self.pairs = dict()
        self.token = dict()
        self.blacklist = set()

    def train(self, size = None) -> None:
        if size is None:
            if max_vocab is not None:
                size = max_vocab
            else:
                size = len(self.bin) // div_chuck
        assert 0 < size,             "size must be greater than 0"
        assert size < len(self.bin), "size must be less than the length of the input binary string"
        self._deal_vocab()
        self._deal_token()
        for i in range(size):
            self._deal_pairs()
            self._merge_vocab()

    def _deal_vocab(self) -> None:
        for seq in self.bin_list:
            self.vocab[seq] += 1

    def _deal_token(self) -> None:
        self.token = self.vocab.copy()

    def _deal_pairs(self) -> None:
        self.pairs = dict()
        self.pair_pos = collections.defaultdict(list)
        skip_next = False
        for idx, b in enumerate(self.bin_list):
            if(idx == 0 or skip_next):
                skip_next = False
                continue
            pair = (self.bin_list[idx-1], b)
            self.pairs[pair] += 1
            self.pair_pos[pair].append(idx-1)
            if pair[0] == pair[1]:
                skip_next = True

    def _merge_vocab(self) -> None:
        ori_list = self.bin_list.copy()
        # 最多，且不在黑名单中的pair
        best_pair = max(self.pairs, key=self.pairs.get)
        while best_pair in self.blacklist:
            self.pairs.pop(best_pair)
            best_pair = max(self.pairs, key=self.pairs.get)
        if self.pairs[best_pair] <= min_freq:
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
        common_freq = lambda x: x <= 0 or x > min_freq
        if not common_freq(self.token[best_pair[0]]) or not common_freq(self.token[best_pair[1]]):
            # warning: the token is only used once, it is a low-frequency token
            # should reset to the original state
            self.bin_list = ori_list
            self.token[best_bin] = 0
            self.token[best_pair[0]] += freq
            self.token[best_pair[1]] += freq
            self.blacklist.add(best_pair)

    def build(self) -> node:
        self.vocab = dict()
        for seq in self.bin_list:
            self.vocab[seq] += 1
        nodes = [ node(freq, seq) for seq, freq in self.vocab.items() ]
        while len(nodes) > 1:
            nodes.sort(key=lambda x: x.weight)
            left = nodes.pop(0)
            right = nodes.pop(0)
            parent = create_node(left.weight + right.weight)
            parent.left = left
            parent.right = right
            nodes.append(parent)
        return nodes[0]
    

def huffman_generate(n: node, code = '') -> None:
    if n.left is None and n.right is None:
        n.code = code
        return
    huffman_generate(n.left, code + '0')
    huffman_generate(n.right, code + '1')


def huffman_mapping(n: node):
    mapping = {}
    if n.left is None and n.right is None:
        mapping[n.seq] = n.code
    else:
        mapping.update(huffman_mapping(n.left))
        mapping.update(huffman_mapping(n.right))
    return mapping

example = '''红蓝对抗的概念最早来源于20世纪60年代的美国演习，演习是专指军队进行大规模的实兵演习，演习中通常分为红军、蓝军，其中蓝军通常是指在部队模拟对抗演习专门扮演假想敌的部队，与红军(代表我方正面部队)进行针对性的训练，这种方式也被称作Red Teaming。

网络安全红蓝对抗的概念就源自于此。红军作为企业防守方，通过安全加固、攻击监测、应急处置等手段来保障企业安全。而蓝军作为攻击方，以发现安全漏洞，获取业务权限或数据为目标，利用各种攻击手段，试图绕过红军层层防护，达成既定目标。可能会造成混淆的是，在欧美一般采用红队代表攻击方，蓝队代表防守方，颜色代表正好相反'''

bin_str = encoding(example)
#print(bin_str)


bpe = BPE(bin_str)
bpe.train()
root = bpe.build()



#print(bpe.bin_list)
#print(bpe.vocab)

huffman_generate(root)
mapping = huffman_mapping(root)
#print(mapping)

result = ''
for b in bpe.bin_list:
    result += mapping[b]

whole = hffl.header_generate(bpe.vocab, result)

#print(result)
#print(len(result))
print(f'batch {batch}')
print(f'net compression ratio: {len(result) / len(bin_str)}')
print(f'whole compression ratio: {len(whole) / len(bin_str)}')