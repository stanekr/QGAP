#ifndef QGAP_H
#define QGAP_H

#include "c:/IBM/ILOG/CPLEX_Studio1263/cplex/include/ilcplex/cplex.h"
#include <iostream>
#include <fstream>   // ifstream
#include <string>
#include <vector>

using namespace std;

class Config
{
public:
   string datapath;  // path to instances directory
   string datafile;  // the instance to solve
   int mode;         // usage mode: 1 optimize, 2 transcode
   int opt_target;   // optimality target
   int isverbose;    // console output detail
   int maxnodes;     // max num of nodes expanded in the tree search
};

class QuadraticGAP
{
public:
   QuadraticGAP();
   ~QuadraticGAP();

   string name;  // instance name
   int n;        // number of clients
   int m;        // number of servers
   //int v;        // unit traffic cost    <- factorize this into cqd
   double** cl;  // linear assignment costs
   double** cqf; // quadratic assignment costs, flows
   double** cqd; // quadratic assignment costs, distances
   int** req;    // client requests
   int*  cap;    // server capacities

   Config* conf;

   int QuadraticGAP::Qopt(void);
   int QuadraticGAP::checkfeas(double* x, double cost);

protected:
private:
   int QuadraticGAP::setproblemdata(char **probname_p, int *numcols_p, int *numrows_p,
      int *objsen_p, double **obj_p, double **rhs_p,
      char **sense_p, int **matbeg_p, int **matcnt_p,
      int **matind_p, double **matval_p, double **lb_p,
      double **ub_p, char **ctype_p, int **qmatbeg_p, int **qmatcnt_p,
      int **qmatind_p, double **qmatval_p);
   double eigenValues(double *qmatval, int n);

   int optimality_target;  // convex function or not
   double EPS = 0.0001;
};

// free
void free_and_null(char **ptr);

#endif // QGAP_H