#ifndef HEUMAG_H
#define HEUMAG_H

#include "c:/IBM/ILOG/CPLEX_Studio1263/cplex/include/ilcplex/cplex.h"
#include <iostream>
#include <string>
#include <vector>
#include "QGAP.h"

using namespace std;

class HeuMagnify
{
public:
   QuadraticGAP* QGAP;

   HeuMagnify(QuadraticGAP*);
   ~HeuMagnify();
   void MagniGlass(CPXENVptr env, CPXLPptr lp, int maxnodes, int optimality_target);
};


#endif // HEUMAG_H
