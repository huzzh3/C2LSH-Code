# C2LSH-Code for the following paper

Junhao Gan, Jianlin Feng, Qiong Fang, and Wilfred Ng. Locality-Sensitive Hashing Scheme Based on Dynamic Collision Counting. In: Proceedings of the ACM SIGMOD International Conference on Management of Data (acceptance rate: 48/289, 16.61%), SIGMOD 2012, pp. 541-552., Scottsdale, USA, May 20-24, 2012. Download: DataProcessor Source Code; C2LSH Source Code.

# How to compile the program

```bash
cd build

# compile the program
cmake ..
make

# execute
cd bin
./C2lSH
```

# How to run the data set (Take sift for example)

1. create the folders (Make sure you are in the root directory)

```bash
mkdir data/sift
mkdir proj/sift
mkdir res/sift
```

2. copy the sift.q and sift.ds to the 'data/sift' folder

3. execute the program

```bash
cd build/bin

# create the index
./C2LSH -n 1000000 -d 128 -ds ../../data/Sift/Sift.ds -pf ../../proj/Sift -c 2

# create the ground truth
./C2LSH -n 1000000 -d 128 -ds ../../data/Sift/Sift.ds -qn 100 -qs ../../data/Sift/Sift.q -gt ../../data/Sift/Sift.gt

# query
./C2LSH -n 1000000 -d 128 -ds ../../data/Sift/Sift.ds -qn 100 -qs ../../data/Sift/Sift.q -pf ../../proj/Sift  -gt ../../data/Sift/Sift.gt -b 4096 -k 100 -c 2 -useCt 0 -rf ../../res/Sift
```

