# canonicalHuffman
A C language implementation of a Canonical Huffman Encoder and Decoder pair

No dependencies. No dynamic memory allocation. No recursion. Small and Fast.

There are two (handled) exceptions to huffman encoding:

* If the input is "dense", the encoding is a little larger.
* If the input contains a single value, the encoding is trivially small.

Otherwise, Huffman encoding should be smaller than the input.

On inputs, where Run-Length encoding is effective, Huffman encoding is more effective on Run-Length encoded inputs.
For these inputs, an implementation of a Run-Length Encoder and Decoder pair is also provided.

Examples:

* test/encode.c - encodes file on command line to stdout
* test/decode.c - decodes file on command line to stdout
