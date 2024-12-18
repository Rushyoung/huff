import collections

dict = lambda: collections.defaultdict(int)

batch = 8


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
        if size == None:
            size = len(self.bin) // 6
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
        ori_list = self.bin_list.copy()
        # 最多，且不在黑名单中的pair
        best_pair = max(self.pairs, key=self.pairs.get)
        while best_pair in self.blacklist:
            self.pairs.pop(best_pair)
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
        if self.token[best_pair[0]] == 1 or self.token[best_pair[1]] == 1:
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
            nodes.sort(key=lambda x: x.freq)
            left = nodes.pop(0)
            right = nodes.pop(0)
            parent = node(left.freq + right.freq, 'no the leaf')
            parent.left = left
            parent.right = right
            nodes.append(parent)
        return nodes[0]
    

def huffman_generate(n: node, code = '') -> None:
    if n.left == None and n.right == None:
        n.code = code
        return
    huffman_generate(n.left, code + '0')
    huffman_generate(n.right, code + '1')


def huffman_mapping(n: node):
    mapping = {}
    if n.left == None and n.right == None:
        mapping[n.seq] = n.code
    else:
        mapping.update(huffman_mapping(n.left))
        mapping.update(huffman_mapping(n.right))
    return mapping

example = '''Licensing
openpilot is released under the MIT license. Some parts of the software are released under other licenses as specified.

Any user of this software shall indemnify and hold harmless Comma.ai, Inc. and its directors, officers, employees, agents, stockholders, affiliates, subcontractors and customers from and against all allegations, claims, actions, suits, demands, damages, liabilities, obligations, losses, settlements, judgments, costs and expenses (including without limitation attorneys’ fees and costs) which arise out of, relate to or result from any use of this software by user.

THIS IS ALPHA QUALITY SOFTWARE FOR RESEARCH PURPOSES ONLY. THIS IS NOT A PRODUCT. YOU ARE RESPONSIBLE FOR COMPLYING WITH LOCAL LAWS AND REGULATIONS. NO WARRANTY EXPRESSED OR IMPLIED.

User Data and comma Account
By default, openpilot uploads the driving data to our servers. You can also access your data through comma connect. We use your data to train better models and improve openpilot for everyone.

openpilot is open source software: the user is free to disable data collection if they wish to do so.

openpilot logs the road-facing cameras, CAN, GPS, IMU, magnetometer, thermal sensors, crashes, and operating system logs. The driver-facing camera is only logged if you explicitly opt-in in settings. The microphone is not recorded.

By using openpilot, you agree to our Privacy Policy. You understand that use of this software or its related services will generate certain types of user data, which may be logged and stored at the sole discretion of comma. By accepting this agreement, you grant an irrevocable, perpetual, worldwide right to comma for the use of this data.'''

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

#print(result)
#print(len(result))
print(f'batch {batch} : compression ratio: {len(result) / len(bin_str)}')