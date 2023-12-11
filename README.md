# CSCI596-Final-Project

This is the repository for the final project of CSCI596 Scientific Computing & Visualization at USC. The project is mainly about parallelizing the Locality Sensitive Hashing (LSH) algorithm.

## Introduction to Locality Sensitive Hashing (LSH)

Locality Sensitive Hashing (LSH) is a popular technique used for efficient approximate similarity serach in high-dimensional space. It empowers vector databases that align with Language Model Models (LLMs). Even when comparing a single query to billions of samples, the achieved complexity remains at best O(n). The core idea is to hash data points such that similar items are mapped to the same hash buckets with high probability. There are 3 main steps in tradition LSH: 

1. Shingling: Convert documents to sets of strings of length k. A k-shingle is a sequence of k tokens appears in the document. After we get shingles for all the documents, we union them to create a big vocabulary set. Then we create a sparse vector for each documents. For each shingle in the vocabulary set, we set the corresponding position to 1 if the document has the shingle and set to 0 if not, just like one-hot encoding.
2. MinHashing: The sparse vectors are too large to compare. In this step we will convert them to short signatures, which are dense integer vectors that represent sets and reflect their similarity. Each number in the signature corresponds to a minhash function.  The signature values are generated by first picking a randomly permuted count vector of the rows (from 1 to len(vocab)+1) and finds the minimum number of the row in which is 1 in our sparse vector.
![minhash example](img/alg_design_img/image.png)
3. Banded LSH function: In order to find signature pairs that share even some similarity and further improve the efficiency, we segment the signatures into bands b (each has r rows), and pass each band through a hash function. Mark the ones that are hashed to the same bucket as candidate pairs. Repeat the process for each band.
![band lsh example](img/alg_design_img/image-1.png)

## Implementation Methods
We will parallelize the LSH using MPI and OpenMP target. Both the MinHashing and Banded LSH functions are possible to be parallelized.
* MinHashing: There are n documents which need to generate the signatures. For each document, there are x hash functions to be applied to y rows to get the signature with x numbers. We will split them to 2 MPI nodes, while each node has an OpenMP master thread with league of teams. Each team will deal with one document, with OpenMP pragma directive to parallelize the nested loops regarding the hash functions. Below is the architecture design:
![minhash architecture](img/alg_design_img/minhash.png)

* Banded LSH function：We will get candidate pairs from b bands. For each band, we need to check for each document if its signature pieces are the same as others. If so, they are the candidate pairs. We will split b bands to 2 MPI nodes, while each node has an OpenMP master thread with league of teams. Each team will deal with one band, with OpenMP pragma directive to parallelize the nested loops for the comparisons between every pair of signature pieces. Below is the architecture design:
![Banded LSH architecture](img/alg_design_img/banded_lsh.png)

* Valid pair check: After getting candidate pairs from the banded LSH function. We need to check the real similarity for each pair to filter out valid pairs with a similarity larger than or equal to the threshold. To get the Jaccard similarity bewtween a pair of documents, we need to iterate through each element of the sparse 0-1 matrix of the first step. The process is homework-alike because we want to collect the intersection number and the union number of the vecotr pairs, which can be distribute to different threads and get the reduction results. 

## Code Structure Introduction
* lsh.c: the basic version of c code containing the whole process. Construct a Set structure to store the candidate pair results.
* lsh_opt.c: a optimization version of lsh.c. Due to the race conditions related to Set operations, we found that it's difficult to design an efficient parallelization process since the majority computation cost falls into the Set operations. In order to reduce some omp critical operations, we build this version. By changing the candidate pair set to the valid pair set, we reduce the number of set operations while increasing the checking similarity operations which can be better parallelized.
* lsh.h: the header file that contains the declarations of functions and constants.

## Expected Results
We will test the runtime and efficiency of the algorithm with different numbers of nodes and threads by running the program on the CARC clusters. We are expected to see a similar efficiency pattern with strong scaling with more nodes and threads, while the runtime should decrease.


## Reference
[1] https://www.pinecone.io/learn/series/faiss/locality-sensitive-hashing/

