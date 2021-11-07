# fizzbuzz
My take on FizzBuzz

The current state of affairs is:

1. fizzbuzz.avx2.S is currently the top submission at stackoverflow/codechef. This solution has a buffer overwrite issue that I caught with testread.cpp. The competition uses "pv" which is a generic reader and only counts bytes. Check out with:

```bash
./fizzbuzz.avx2 | ./testread
```

2. My solution (fizzbuzz.cpp) generates about 4GB/s per thread and scales linearly up to around 30 Gb/s when vmsplice() issues arise

3. I tried another approach in fbthread.cpp but that's not compiling so commented out in CMakeLists.txt


# Install

Typical cmake build:

```bash
make build
cmake -DCMAKE_BUILD_TYPE=Release ..
make 
```
