# fizzbuzz
My take on FizzBuzz

The current state of affairs is:

1. fizzbuzz.avx2.S is currently the top submission at stackoverflow/codechef

2. the avx2 solution has a buffer overwrite issue that I caught with testread.cpp. The competition uses "pv" which is a generic reader and only counts bytes. 

3. My solution (fizzbuzz.cpp) generates about 4GB/s per thread and scales linearly up to around 30 Gb/s when vmsplice() issues arise

4. I tried another approach in fbthread.cpp but that's not compiling so commented out in CMakeLists.txt


# Install

Typical cmake build:

```bash
make build
cmake -DCMAKE_BUILD_TYPE=Release ..
make 
```
