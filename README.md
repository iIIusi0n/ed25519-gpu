# Ed25519 key on GPU
Generate Ed25519 key in GPU for hi---------gh performance!!

## Using
1. Set CUDA_DIR env variable to cuda directory. (C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v10.1)
2. Set BOOST_DIR env variable to boost directory. (C:\boost)
3. Build boost library.

## Performance
- 32bit
```
key count: 1048576
device: NVIDIA GeForce GTX 1660 Ti
time: 131300 ns
```
8key / 1ns
- 64bit
```
key count: 1048576
device: NVIDIA GeForce GTX 1660 Ti
time: 227400 ns
```
4key / 1ns

## Purpose
for tor v3 vanity address!

## Reference
- https://www.boost.org/doc/libs/1_66_0/libs/compute/doc/html/index.html
- https://github.com/orlp/ed25519
- https://github.com/bkerler/opencl_brute
