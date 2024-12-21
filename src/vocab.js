// run in quick javascript
import * as std from "std";
import * as os  from "os";

"use strict";

let min_freq   = 2;
let max_vocab  = 255;

let max_file_size = 4 * 1024 * 1024; // 4MB

class BPE{
    /**
     * @param { String } input_bin
     */
    constructor(input_bin){
        this.bin_list = new Array();
        for(let i = 0; i < input_bin.length; i++){
            this.bin_list.push(input_bin[i]);
        }
        this.vocab = null;
        this.pairs = null;
        this.pair_pos = null;
        this.token = null;
        this.blanklist = new Array();
    }

    /**
     * @function deal vocab
     */
    deal_vocab(){
        this.vocab = new Map();
        for(let seq of this.bin_list){
            this.vocab.set(seq, (this.vocab.get(seq) || 0) + 1);
        }
    }
    
    /**
     * @function deal token
     */
    deal_token(){
        // deep copy
        this.token = new Map(this.vocab); 
    }

    /**
     * @function deal pairs
     */
    deal_pairs(){
        this.pairs    = new Map();
        this.pair_pos = new Map();
        let idx = 0;
        for(let [seq, freq] of this.vocab){
            for(let i = 0; i < seq.length - 1; i++){
                let pair = seq.slice(i, i+2);
                this.pairs.set(pair, (this.pairs.get(pair) || 0) + freq);
                if(!this.pair_pos.has(pair)){
                    this.pair_pos.set(pair, new Array());
                }
                this.pair_pos.get(pair).push([idx, i]);
            }
            idx++;
        }
    }
};

var hfc_tmp = std.open(".hfc.tmp", "r");
var input_file_name = hfc_tmp.getline();
hfc_tmp.close();

console.log(input_file_name);


var test = new BPE("0000001200120000");

test.deal_vocab();
test.deal_token();
console.log(test.vocab);
