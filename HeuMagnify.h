#ifndef HEUMAG_H
#define HEUMAG_H

#include "c:/IBM/ILOG/CPLEX_Studio1263/cplex/include/ilcplex/cplex.h"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm> // sort
#include "QGAP.h"

using namespace std;

class HeuMagnify
{
public:
   QuadraticGAP* QGAP;

   HeuMagnify(QuadraticGAP*);
   ~HeuMagnify();
   void MagniGlass(CPXENVptr env, CPXLPptr lp, int maxnodes, int optimality_target);

private:
   double simpleContruct(double* x);
   void HeuMagnify::computeRegrets(double** c, int n, int m, vector<int> & regrets);
   double HeuMagnify::computeCost(double* x, int n, int m);
};


#endif // HEUMAG_H
