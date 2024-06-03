# btok

BPE tokenizer with full binary and unicode support

50x faster than huggingface `ByteLevelBPETokenizer`

12% better compression than `ByteLevelBPETokenizer`

Lean c++ backend, with slim python wrapper

Decoding is strictly concat of token strings, enabling portability to embedded runtimes.

Headers are installed with python packge, can be found with `python -m btok.includes`