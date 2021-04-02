# splay-trees_c
Collection of splay trees implementations, ready for user applications programming. Written in C, requires the GNU C Library.

As splay trees are a particular kind of _binary search trees_, the work in this repository is derived from my other work [avl-trees_c](https://github.com/robmasocco/avl-trees_c).
Splay trees do not account for *balance*, instead they replace the tree's root with the latest modified node, thus working as a sort of cache, exploiting temporal locality assumptions to speed up following accesses to the last modified nodes. Depending on your workload, this might make a tree degenerate into a linked list with linear access times. An amortized analysis shows logarithmic access times in an average sequence of operations.

They work as a dictionary, storing values paired with keys and rearranging records in memory to make binary searches (by keys) more efficient. Data stored can be anything that fits into a _void *_ (so 64 bits at most on x86_64 systems). They support insertion, deletion, record search, total structure deletion, and various kinds of _breadth-first_ and _depth-first_ searches. Since they extensively use dynamic memory (heap), options are provided to specify if keys or data are to be free'd when calling deletions, to make things faster. I plan to develop multiple flavours, depending on the type of the key (which influences comparisons and memory usage). Those currently available are:

- Integer keys (int).

**NOTE:** This library is currently under development, as you can tell from the _dev_ branch. The _main_ branch will be updated as soon as a first version is available.

## Can I use this?
If you stumbled upon here and find this suitable for your project, or think this might save you some work, sure! Just credit the use of this code, and although I used this stuff in projects and tested it, there's no 100% guarantee that it's bug-free. Feel free to open an issue if you see something wrong.

Also, this code is protected by the MIT license as per the attached LICENSE file.
