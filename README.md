# splay-trees_c
Collection of splay trees implementations, ready for user applications programming. Written in C, requires the GNU C Library.

As splay trees are a particular kind of _binary search trees_, the work in this repository is derived from my other work [avl-trees_c](https://github.com/robmasocco/avl-trees_c).
Splay trees do not account for *balance*, instead they replace the tree's root with the latest modified node, thus working as a sort of *cache*, exploiting temporal locality assumptions to speed up following accesses to the last modified nodes. Depending on your workload, this might make a tree degenerate into a linked list with linear access times. An amortized analysis shows logarithmic access times in an average sequence of operations, but with some caveats in multithreaded scenarios (see below). In a sequence of random accesses and operations, it's been proven that this structure performs better than its balanced counterparts.

They work as a dictionary, storing values paired with keys and rearranging records in memory to make binary searches (by keys) more efficient. Data stored can be anything that fits into a _void *_ (so 64 bits at most on x86_64 systems). They support insertion, deletion, record search, total structure deletion, and various kinds of _breadth-first_ and _depth-first_ searches. Since they extensively use dynamic memory (heap), options are provided to specify if keys or data are to be free'd when calling deletions, to make things faster. I plan to develop multiple flavours, depending on the type of the key (which influences comparisons and memory usage). Those currently available are:

- Integer keys (int).

**NOTE:** This library is currently under development, as you can tell from the _dev_ branch. The _main_ branch will be updated as soon as a first version is available.

## Splay trees and multithreading

Splay trees were invented by D. Sleator and R. Tarjan as a form of self-adjusting data structure that could work as a dictionary, in a more efficient way than known alternatives like AVL trees. For one, they do not store any kind of balancing or bookkeeping information, nor do they perform computations along a path from the root to a given node to assess the status of the structure and perform management operations. They just apply the **splay heuristic**: once any node is accessed, as a result of any operation among *search*, *insert* and *delete*, a series of rotations that follows a specific set of [rules](https://en.wikipedia.org/wiki/Splay_tree#Splaying) is performed to move the node up to the root of the tree. This implies that **any** operation on the tree potentially modifies it.
Problem is, the year was 1985, and nobody was thinking about parallel systems and concurrent accesses.
In such a scenario, these structures are hardly useful: *insert*/*delete* operations require locking (not explicitly dealt with here), and with *full splaying* (the one we've described up to this point) also *searches* do. Alternatives include *read-copy-update* schemes, usually heavy on main memory because of many copies of the structure that have to be generated, or more elaborate schemes that add management information to nodes and more computation, also relying on non-trivial assumptions.
Here, we keep things simple: full splaying (i.e. up to the root) can be performed each time or only after an *insert*/*delete*. This behavior can be set using options for the *search* routine (see the header file). There are different consequences on access times:

- If splaying is performed in searches too, the amortized analysis results apply and access times are logarithmic in the max number of nodes the structure holds in a sequence of operations, but **all operations must be performed atomically**.
- If splaying is not performed during searches, the access time to search a node is linear in the worst case, but many concurrent "readers" can access the structure, while locking is still required to perform *insertions* and *deletions*. If no option is passed to the search routine, this is the default behavior, resulting in a compromise between access times, concurrent access and the *cache-like* features of a splay tree.

Choose accordingly to your usage scenario, if this structure is applicable.

## Can I use this?

If you stumbled upon here and find this suitable for your project, or think this might save you some work, sure! Just credit the use of this code, and although I used this stuff in projects and tested it, there's no 100% guarantee that it's bug-free. Feel free to open an issue if you see something wrong.

Also, this code is protected by the MIT license as per the attached LICENSE file.
