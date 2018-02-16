#ifndef QGAP_H
#define QGAP_H

#include "c:\IBM\ILOG\CPLEX_Studio1263\cplex\include\ilcplex/cplex.h"
#include <iostream>
#include <fstream>   // ifstream
#include <string>

using namespace std;

class Config
{
public:
   string datafile;  // the instance to solve
   int isverbose;    // console output detail
};

class QuadraticGAP
{
public:
   QuadraticGAP();
   ~QuadraticGAP();

   string name;  // instance name
   int n;        // number of clients
   int m;        // number of servers
   double** cl;  // linear assignment costs
   double** cqf; // quadratic assignment costs, flows
   double** cqd; // quadratic assignment costs, distances
   int** req;    // client requests
   int*  cap;    // server capacities

   Config* conf;

   int QuadraticGAP::Qopt(void);

protected:
private:
   int QuadraticGAP::setproblemdata(char **probname_p, int *numcols_p, int *numrows_p,
      int *objsen_p, double **obj_p, double **rhs_p,
      char **sense_p, int **matbeg_p, int **matcnt_p,
      int **matind_p, double **matval_p, double **lb_p,
      double **ub_p, int **qmatbeg_p, int **qmatcnt_p,
      int **qmatind_p, double **qmatval_p);
};

// free
void free_and_null(char **ptr);

#endif // QGAP_H