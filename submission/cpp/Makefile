CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall

ifndef CUDA_HOME
  ifdef CUDA_PATH
    CUDA_HOME := $(CUDA_PATH)
  else
    CUDA_HOME := /usr/local/cuda
  endif
endif
CUDA_INCDIR = $(CUDA_HOME)/include
CUDA_LIBDIR = $(CUDA_HOME)/lib64

INCLUDES = -Iheterosta3d.v1.0.20260107/include -I$(CUDA_INCDIR)

HETEROSTA_LIB_PATH = heterosta3d.v1.0.20260107/lib
LDFLAGS = -L$(HETEROSTA_LIB_PATH) -L$(CUDA_LIBDIR) \
          -Wl,-rpath,'$$ORIGIN/heterosta3d.v1.0.20260107/lib' \
          -Wl,-rpath,$(CUDA_LIBDIR) \
          -Wl,-rpath-link,$(HETEROSTA_LIB_PATH) \
          -Wl,--unresolved-symbols=ignore-in-shared-libs
LIBS = -lheterosta3d -lcuda -lcudart -lcudadevrt -lgomp -ldl -lpthread -lrt -ljansson

# ── CPU targets (no CUDA needed) ──
gcd_cpu: run_gcd_cpu.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $< $(LDFLAGS) $(LIBS) -Wl,--no-as-needed

aes_cpu: run_aes_cpu.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $< $(LDFLAGS) $(LIBS) -Wl,--no-as-needed

# ── GPU targets (need CUDA) ──
gcd_gpu: run_gcd_gpu2.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $< $(LDFLAGS) $(LIBS) -Wl,--no-as-needed

aes_gpu: run_aes_gpu.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $< $(LDFLAGS) $(LIBS) -Wl,--no-as-needed

# ── Benchmarks ──
gcd_bench: bench_cpu_vs_gpu.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $< $(LDFLAGS) $(LIBS) -Wl,--no-as-needed

aes_bench: bench_aes.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $< $(LDFLAGS) $(LIBS) -Wl,--no-as-needed

# ── Convenience ──
all: gcd_cpu gcd_gpu aes_cpu aes_gpu
bench: gcd_bench aes_bench

# ── Run targets ──
LD_RUN = LD_LIBRARY_PATH=heterosta3d.v1.0.20260107/lib:$$LD_LIBRARY_PATH

run_gcd_cpu: gcd_cpu
	$(LD_RUN) ./gcd_cpu

run_gcd_gpu: gcd_gpu
	$(LD_RUN) ./gcd_gpu

run_aes_cpu: aes_cpu
	$(LD_RUN) ./aes_cpu

run_aes_gpu: aes_gpu
	$(LD_RUN) ./aes_gpu

run_gcd_bench: gcd_bench
	$(LD_RUN) ./gcd_bench

run_aes_bench: aes_bench
	$(LD_RUN) ./aes_bench

clean:
	rm -f gcd_cpu gcd_gpu aes_cpu aes_gpu gcd_bench aes_bench
	rm -f *.rpt *.log

.PHONY: all clean bench gcd_cpu gcd_gpu aes_cpu aes_gpu gcd_bench aes_bench \
        run_gcd_cpu run_gcd_gpu run_aes_cpu run_aes_gpu run_gcd_bench run_aes_bench
