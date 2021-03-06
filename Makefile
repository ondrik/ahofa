CXX=g++
CXX_VERSION=$(shell $(CXX) -dumpversion)
STD=-std=c++14
ifeq ($(shell expr $(CXX_VERSION) '>=' 5.1), 1)
STD=-std=c++17
endif

SRCDIR=src
COMMON=$(SRCDIR)/common
EXE=$(SRCDIR)/exe

CXXFLAGS=$(STD) -Wall -Wextra -pedantic  -I $(COMMON) -O3 #-Wfatal-errors #-DNDEBUG
LIBS=-lpcap -lpthread -lboost_system -lboost_filesystem

PROG=nfa_eval state_frequency prefix_labeling
all: $(PROG)

SRC=$(wildcard $(COMMON)/*.cpp)
HDR=$(wildcard $(COMMON)/*.hpp)
OBJ=$(patsubst %.cpp, %.o, $(SRC))

.PHONY: clean all

nfa_eval: $(EXE)/nfa_eval.o $(OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS)

$(EXE)/nfa_eval.o: $(EXE)/nfa_eval.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(LIBS)

state_frequency: $(EXE)/state_frequency.o $(OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS)

$(EXE)/state_frequency.o: $(EXE)/state_frequency.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(LIBS)

# merging inspired by predicate logic
prefix_labeling: $(EXE)/prefix_labeling.o $(OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS)

$(EXE)/prefix_labeling.o: $(EXE)/prefix_labeling.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(LIBS)

%.o: %.cpp %.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(LIBS)

no-data:
	rm -f *.fa *.dot *.jpg *.json *.jsn tmp*
	rm -f obs* tmp* *.fsm *.fa *.pa *.ba *csv

pack:
	rm -f ahofa.zip
	zip -r ahofa.zip src/*/*.hpp src/*/*.cpp Makefile *.py \
	README.md experiments automata

clean:
	rm -f $(COMMON)/*.o $(EXE)/*.o $(PROG)
