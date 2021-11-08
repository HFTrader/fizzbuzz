
#include "PipeWriter.h"
#include "Generator.h"

#include <cstdint>
#include <cstdlib>
#include <memory>
#include <vector>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: fizzbuzz.hb <numthreads> <numblocks> \n");
        return 0;
    }

    uint32_t nthreads = std::atoi(argv[1]);
    uint32_t numblocks = std::atoi(argv[2]);
    uint32_t bufsize = numblocks * (8 * 20 + 7 * 5);
    uint32_t jump = (nthreads - 1) * numblocks * 15;
    uint32_t stride = numblocks * 15;
    using GeneratorPtr = std::shared_ptr<Generator>;
    std::vector<GeneratorPtr> loops;
    PipeWriter writer(nthreads, bufsize);
    for (uint32_t j = 0; j < nthreads; ++j) {
        GeneratorPtr loop(new Generator(writer, 1 + j * stride, numblocks, jump));
        loops.push_back(loop);
    }
    writer.run();
}
