CXX = g++
CXXFLAGS = -g -Wall -O
LDADD =

CPX_INCDIR  = /IBM/ILOG\CPLEX_Studio1263/cplex/include
CPX_LDPATH  = /IBM/ILOG/CPLEX_Studio1263/cplex/lib/x86_windows_vs2013/stat_mda
CPX_LDADD = -lm

OBJ = QGAP.o 

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -I$(CPX_INCDIR) -c $^ -o $@

main: $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o main -L$(CPX_LDPATH) $(CPX_LDADD)

clean:
	rm -rf $(OBJ) QGAP

.PHONY: clean