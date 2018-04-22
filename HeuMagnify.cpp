#include "HeuMagnify.h"

HeuMagnify::HeuMagnify(QuadraticGAP* QGAPinst)
{  QGAP = QGAPinst;
}

HeuMagnify::~HeuMagnify()
{
}

void HeuMagnify::MagniGlass(CPXENVptr env, CPXLPptr lp, int maxnodes, int optimality_target)
{  int    i,j,n,m,status,solstat;
   double objval;
   double *x     = NULL;
   double *slack = NULL;
   cout << "Magnifying glass heuristic. Starting" << endl;
   n = QGAP->n;
   m = QGAP->m;

   status = CPXsetintparam(env, CPXPARAM_MIP_Limits_Nodes, maxnodes);   // max num of nodes
   if (status)
   {  fprintf(stderr, "Failure to reset max tree search nodes, error %d.\n", status);
      goto TERMINATE;
   }

   cout << "Optimality target set to "<< optimality_target << endl;
   status = CPXsetintparam(env, CPXPARAM_OptimalityTarget, optimality_target);
   if (status)
   {  fprintf(stderr, "Failure to reset optimality target, error %d.\n", status);
      goto TERMINATE;
   }

   // Optimize the problem and obtain solution.
   if(optimality_target > 1)
      status = CPXqpopt(env, lp); // in case of non convex function (opt target > 1)
   if (status) 
   {  fprintf(stderr, "Failed to optimize QP.\n");
   //goto TERMINATE;
   }

   int cur_numrows = CPXgetnumrows(env, lp);
   int cur_numcols = CPXgetnumcols(env, lp);

   int nn = CPXgetsolnpoolnumsolns(env, lp);
   if(nn==0)
   {  fprintf(stderr, "Failed to find feasible solutions.\n");
      goto TERMINATE;
   }

   double mincost = DBL_MAX;
   int minind = -1;
   for(i=0;i<nn;i++)
   {  status = CPXgetsolnpoolobjval (env, lp, i, &objval);
      cout << "Solution " << i << " cost " << objval << endl;
      if(objval<mincost)
      {  minind = i;
         mincost = objval;
      }
   }

   x     = (double *)malloc(cur_numcols * sizeof(double));
   slack = (double *)malloc(cur_numrows * sizeof(double));
   status = CPXgetsolnpoolx (env, lp, minind, x, 0, CPXgetnumcols(env, lp)-1);

   status = CPXsolution(env, lp, &solstat, &objval, x, NULL, slack, NULL);
   if (status) 
   {  fprintf(stderr, "Failed to obtain solution.\n");
      goto TERMINATE;
   }

   status = QGAP->checkfeas(x, objval);
   if (status)
   {  fprintf(stderr, "Solution infeasible.\n");
      cout << "Solution infeasible !!! status = " << status << endl;
      //goto TERMINATE;
   }
   else cout << "Solution checked, status " << status << " cost " << objval << endl;

   // Write the output to the screen.
   cout << "\nSolution status = " << solstat << endl;
   cout << "Solution value  = " << objval << endl;
   cout << "Solution:" << endl;
   for(i=0;i<cur_numcols;i++)
      cout << x[i] << " ";
   cout << endl;

   // lu[j] 	= 'L' 	bd[j] is a lower bound
   // lu[j] 	= 'U' 	bd[j] is an upper bound
   // lu[j] 	= 'B' 	bd[j] is the lower and upper bound
   int cnt,k = n/3;
   int * indices = (int *) malloc (k*sizeof(int));
   char * lu = (char *) malloc (k*sizeof(char));
   double * bd = (double *) malloc (k*sizeof(double));	
   cnt = 0;
   for(i=0;i<m;i++)
      for(j=0;j<n;j++)
         if(j>k)
         {
            indices[cnt] = i*n+j;
            lu[i] = 'B';				//both bounds will be set
            bd[i] = x[i*n+j];				
            cnt++;
         }
   status = CPXchgbds (env, lp, cnt, indices, lu, bd);
   status = CPXqpopt(env, lp); // in case of non convex function (opt target > 1)

TERMINATE:
   free_and_null((char **)&x);
   free_and_null((char **)&slack);
   return;
}
