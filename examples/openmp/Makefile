BINS = nesting_par nesting_seq critical_sections_par critical_sections_seq
CPP=clang++
LIBS=-lm
NESTING_INPUTS=9000000 20 20
CS_INPUTS=9000000

all: $(BINS)

nesting_par: nesting.cpp
	$(CPP) $(LIBS) -lomp $^ -O3 -fopenmp -o $@

nesting_seq: nesting.cpp
	$(CPP) $(LIBS) $^ -O3 -o $@

critical_sections_par: critical_sections.cpp
	$(CPP) $(LIBS) -lomp $^ -O3 -fopenmp -o $@

critical_sections_seq: critical_sections.cpp
	$(CPP) $(LIBS) $^ -O3 -o $@

run_nesting_par: nesting_par
	perf stat ./$^ $(NESTING_INPUTS)

run_nesting_seq: nesting_seq
	perf stat ./$^ $(NESTING_INPUTS)

run_critical_sections_par: critical_sections_par
	perf stat ./$^ $(CS_INPUTS)

run_critical_sections_seq: critical_sections_seq
	perf stat ./$^ $(CS_INPUTS)

clean:
	rm -f $(BINS) *.o
