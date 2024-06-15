# btok

BPE tokenizer with full binary and unicode support

50x faster than huggingface `ByteLevelBPETokenizer`

Lean c++ backend, with slim python wrapper

Decoding is strictly concat of token strings, enabling portability to embedded runtimes.

C++ headers are installed with python packge, and can be found with `python -m btok.includes`
You can use that command in makefiles to compile against the C++ backend directly.