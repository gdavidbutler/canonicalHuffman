# canonicalHuffman
A C language implementation of a Canonical Huffman Encoder and Decoder pair

No dependencies. No dynamic memory allocation. No recursion. Small and Fast.

There are two (handled) exceptions to huffman encoding:

* If the input is already "dense", the ouput is flagged as such (and is larger as a result).
* If the input contains a single value, the encoding is, effectively, run-length encoded.

Otherwise, huffman encoding should be smaller than the input.

Some examples of using this:

* test/encode.c - encodes file on command line to stdout
* test/decode.c - decodes file on command line to stdout
