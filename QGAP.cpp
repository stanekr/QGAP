#include "QGAP.h"
#include <Eigen/Core>
#include <Eigen/LU>         // for the determinant
#include <SymEigsSolver.h>  // Also includes <MatOp/DenseSymMatProd.h>

using Eigen::MatrixXd;
using namespace Spectra;

QuadraticGAP::QuadraticGAP()
{
   conf = new Config();
}

QuadraticGAP::~QuadraticGAP()
{
   //dtor
}

int QuadraticGAP::Qopt (void)
{
   // data which define the LP problem. setproblemdata() allocates space for the problem data.  */
   char     *probname = NULL;
   int      numcols;
   int      numrows;
   int      objsen;
   double   *obj = NULL;
   double   *rhs = NULL;
   char     *sense = NULL;
   int      *matbeg = NULL;
   int      *matcnt = NULL;
   int      *matind = NULL;
   double   *matval = NULL;
   double   *lb = NULL;
   double   *ub = NULL;
   int      *qmatbeg = NULL;
   int      *qmatcnt = NULL;
   int      *qmatind = NULL;
   double   *qmatval = NULL;
   char     *ctype = NULL;

   // optimization results
   int      solstat;
   double   objval;
   double   *x  = NULL;
   //double   *pi = NULL;
   double   *slack = NULL;
   //double   *dj = NULL;

   CPXENVptr     env = NULL;
   CPXLPptr      lp = NULL;
   int           status;
   int           i, j;
   int           cur_numrows, cur_numcols;

   vector < vector <double>> q(m, vector<double>(n)); // for debug

   optimality_target = conf->opt_target;

   // Initialize the CPLEX environment
   env = CPXopenCPLEX(&status);
   if (env == NULL) 
   {  char  errmsg[CPXMESSAGEBUFSIZE];
      fprintf(stderr, "Could not open CPLEX environment.\n");
      CPXgeterrorstring(env, status, errmsg);
      fprintf(stderr, "%s", errmsg);
      goto TERMINATE;
   }

   // Turn on output to the screen
   status = CPXsetintparam(env, CPXPARAM_ScreenOutput, CPX_ON);
   if (status) 
   {  fprintf(stderr, "Failure to turn on screen indicator, error %d.\n", status);
      goto TERMINATE;
   }

   // Fill in the data for the problem.
   status = setproblemdata(&probname, &numcols, &numrows, &objsen, &obj,
      &rhs, &sense, &matbeg, &matcnt, &matind,
      &matval, &lb, &ub, &ctype, &qmatbeg, &qmatcnt,
      &qmatind, &qmatval);

   if (status) 
   {  fprintf(stderr, "Failed to build problem data arrays.\n");
      goto TERMINATE;
   }

   // Create the problem.
   lp = CPXcreateprob(env, &status, probname);
   if (lp == NULL) 
   {  fprintf(stderr, "Failed to create problem.\n");
      goto TERMINATE;
   }

   // Now copy the LP part of the problem data into the lp
   status = CPXcopylp(env, lp, numcols, numrows, objsen, obj, rhs,
      sense, matbeg, matcnt, matind, matval, lb, ub, NULL);

   if (status) 
   {  fprintf(stderr, "Failed to copy problem data.\n");
      goto TERMINATE;
   }

   status = CPXcopyquad(env, lp, qmatbeg, qmatcnt, qmatind, qmatval);
   if (status) 
   {  fprintf(stderr, "Failed to copy quadratic matrix.\n");
      goto TERMINATE;
   }

   // Copy the ctype array, make problem integer
   if(optimality_target != 2)
   {  status = CPXcopyctype(env, lp, ctype);
      if (status) 
      {  fprintf(stderr, "Failed to copy ctype\n");
         goto TERMINATE;
      }
   }

   // Write a copy of the problem to a file. 
   status = CPXwriteprob(env, lp, "qgap.lp", NULL);
   if (status)
   {  fprintf(stderr, "Failed to write LP to disk.\n");
      goto TERMINATE;
   }

   // 1: assumes that the model is convex and searches for a globally optimal solution.
   // 2: Searches for a solution that satisfies first-order optimality conditions, but is not necessarily globally optimal.
   // 3: Searches for a globally optimal solution to a nonconvex model; changes problem type to MIQP if necessary.
   cout << "Optimality target set to "<< optimality_target << endl;
   status = CPXsetintparam(env, CPXPARAM_OptimalityTarget, optimality_target);
   if (status)
   {  fprintf(stderr, "Failure to reset optimality target, error %d.\n", status);
      goto TERMINATE;
   }
   else
      switch (optimality_target)
      {
         case 1:  cout << "MIP solution, convex function" << endl;
            break;
         case 2:  cout << "Continuous solution, any function" << endl;
            break;
         case 3:  cout << "Trying heuristic MIP solution, any function" << endl;
            break;
         default:
            break;
      }

   // Optimize the problem and obtain solution.
   if(optimality_target > 1)
      status = CPXqpopt(env, lp); // in case of non convex function (opt target > 1)
   else
   {  status = CPXchgprobtype(env, lp, CPXPROB_MIQCP); // in case of convex functions
      status = CPXmipopt(env, lp);                     // in case of convex functions
   }
   if (status) 
   {  fprintf(stderr, "Failed to optimize QP.\n");
      goto TERMINATE;
   }

   cur_numrows = CPXgetnumrows(env, lp);
   cur_numcols = CPXgetnumcols(env, lp);

   x     = (double *)malloc(cur_numcols * sizeof(double));
   slack = (double *)malloc(cur_numrows * sizeof(double));
   //dj = (double *)malloc(cur_numcols * sizeof(double));
   //pi = (double *)malloc(cur_numrows * sizeof(double));

   status = CPXsolution(env, lp, &solstat, &objval, x, NULL, slack, NULL);
   if (status) 
   {  fprintf(stderr, "Failed to obtain solution.\n");
      goto TERMINATE;
   }

   // Write the output to the screen.
   cout << "\nSolution status = " << solstat << endl;
   cout << "Solution value  = " << objval << endl;
   cout << "Solution:" << endl;
   for(i=0;i<cur_numcols;i++)
      cout << x[i] << " ";
   cout << endl;

TERMINATE:
   // Free up the problem as allocated by CPXcreateprob, if necessary
   if (lp != NULL) {
      status = CPXfreeprob(env, &lp);
      if (status) {
         fprintf(stderr, "CPXfreeprob failed, error code %d.\n", status);
      }
   }

   // Free up the CPLEX environment, if necessary 
   if (env != NULL) {
      status = CPXcloseCPLEX(&env);
      if (status) {
         char  errmsg[CPXMESSAGEBUFSIZE];
         fprintf(stderr, "Could not close CPLEX environment.\n");
         CPXgeterrorstring(env, status, errmsg);
         fprintf(stderr, "%s", errmsg);
      }
   }

   // Free up the problem data arrays, if necessary.
   // free_and_null((char **)&probname);  // gives an error, don't know why
   free_and_null((char **)&obj);
   free_and_null((char **)&rhs);
   free_and_null((char **)&sense);
   free_and_null((char **)&matbeg);
   free_and_null((char **)&matcnt);
   free_and_null((char **)&matind);
   free_and_null((char **)&matval);
   free_and_null((char **)&lb);
   free_and_null((char **)&ub);
   free_and_null((char **)&ctype);
   free_and_null((char **)&qmatbeg);
   free_and_null((char **)&qmatcnt);
   free_and_null((char **)&qmatind);
   free_and_null((char **)&qmatval);

   free_and_null((char **)&x);
   free_and_null((char **)&slack);
   //free_and_null((char **)&dj);
   //free_and_null((char **)&pi);
   return (status);
}  

int QuadraticGAP::setproblemdata(char **probname_p, int *numcols_p, int *numrows_p,
   int *objsen_p, double **obj_p, double **rhs_p,
   char **sense_p, int **matbeg_p, int **matcnt_p,
   int **matind_p, double **matval_p, double **lb_p,
   double **ub_p, char **ctype_p, int **qmatbeg_p, int **qmatcnt_p,
   int **qmatind_p, double **qmatval_p)
{
   int i,j,h,k,ij,hk,idRow,idCol;

   char     *zprobname = NULL;   
   double   *zobj = NULL;
   double   *zrhs = NULL;
   char     *zsense = NULL;
   int      *zmatbeg = NULL;
   int      *zmatcnt = NULL;
   int      *zmatind = NULL;
   double   *zmatval = NULL;
   double   *zlb = NULL;
   double   *zub = NULL;
   char     *zctype = NULL;
   int      *zqmatbeg = NULL;
   int      *zqmatcnt = NULL;
   int      *zqmatind = NULL;
   double   *zqmatval = NULL;
   int      status = 0;

   int numCols = n*m;
   int numRows = n+m;
   int numNZ = n*m+n*m;  // nonzeros in the linear coefficient matrix
   int numQNZ = n*n*m*m; // nonzeros in the quadratic coefficient matrix

   zprobname = (char *)malloc(16 * sizeof(char));
   zobj = (double *)malloc(numCols * sizeof(double));
   zrhs = (double *)malloc(numRows * sizeof(double));
   zsense = (char *)malloc(numRows * sizeof(char));
   zmatbeg = (int *)malloc(numCols * sizeof(int));
   zmatcnt = (int *)malloc(numCols * sizeof(int));
   zmatind = (int *)malloc(numNZ * sizeof(int));
   zmatval = (double *)malloc(numNZ * sizeof(double));
   zlb = (double *)malloc(numCols * sizeof(double));
   zub = (double *)malloc(numCols * sizeof(double));
   zctype = (char *)malloc(numCols * sizeof(char));
   zqmatbeg = (int *)malloc(numCols * sizeof(int));
   zqmatcnt = (int *)malloc(numCols * sizeof(int));
   zqmatind = (int *)malloc(numQNZ * sizeof(int));
   zqmatval = (double *)malloc(numQNZ * sizeof(double));

   if (zprobname == NULL || zobj == NULL ||
      zrhs == NULL || zsense == NULL ||
      zmatbeg == NULL || zmatcnt == NULL ||
      zmatind == NULL || zmatval == NULL ||
      zlb == NULL || zub == NULL ||
      zqmatbeg == NULL || zqmatcnt == NULL ||
      zqmatind == NULL || zqmatval == NULL) {
      status = 1;
      goto TERMINATE;
   }

   zprobname = (char*) name.c_str();

   // -------------------------- linear objective costs
   ij = 0;
   for(i=0;i<m;i++)
      for(j=0;j<n;j++)
      {
         zobj[ij] = cl[i][j];
         zlb[ij] = 0;
         zub[ij] = 1;
         zctype[ij] = 'I';
         ij++;
      }

   // -------------------------- quadratic cost matrix
   ij = 0;
   numNZ = 0;
   for (i = 0; i<m; i++)
      for (j = 0; j<n; j++)
      {
         hk = 0;
         zqmatbeg[ij] = numNZ;
         for (h = 0; h<m; h++)
            for (k = 0; k<n; k++)
            {
               zqmatind[numNZ] = hk;
               if (cqf != NULL)
                  zqmatval[numNZ] = cqd[i][h] * cqf[j][k]; // d_ih f_jk input format
               else
                  zqmatval[numNZ] = cqd[ij][hk];           // c_ijhk input format
               numNZ++;
               hk++;
            }
         zqmatcnt[ij] = numNZ - zqmatbeg[ij];
         ij++;
      }


   // -------------------------- find eigenvalues
   double evalue = eigenValues(zqmatval,i*j);
   cout << "Eigenvalue: " << evalue << endl;

   // -------------------------- constraints section
   idCol = 0;
   numNZ = 0;  
   for (i = 0; i<m; i++)
      for (j = 0; j<n; j++)
      {
         zmatbeg[idCol] = numNZ;

         zmatind[numNZ] = j;           // Assignment constraint
         zmatval[numNZ] = 1.0; 
         numNZ++;

         zmatind[numNZ] = n + i;       // Capacity constraint
         zmatval[numNZ] = req[i][j];
         numNZ++;

         zmatcnt[idCol] = numNZ - zmatbeg[idCol];
         idCol++;
      }

   // -------------------------- rhs
   for(j=0;j<n;j++)
   {  zsense[j] = 'E';
      zrhs[j]   = 1.0;
   }

   for(i=0;i<m;i++)
   {  zsense[n+i] = 'L';
      zrhs[n+i]   = cap[i];
   }

TERMINATE:
   if (status) 
   {
      free_and_null((char **)&zprobname);
      free_and_null((char **)&zobj);
      free_and_null((char **)&zrhs);
      free_and_null((char **)&zsense);
      free_and_null((char **)&zmatbeg);
      free_and_null((char **)&zmatcnt);
      free_and_null((char **)&zmatind);
      free_and_null((char **)&zmatval);
      free_and_null((char **)&zlb);
      free_and_null((char **)&zub);
      free_and_null((char **)&zctype);
      free_and_null((char **)&zqmatbeg);
      free_and_null((char **)&zqmatcnt);
      free_and_null((char **)&zqmatind);
      free_and_null((char **)&zqmatval);
   }
   else 
   {
      *numcols_p = numCols;
      *numrows_p = numRows;
      *objsen_p = CPX_MIN;   

      *probname_p = zprobname;
      *obj_p = zobj;
      *rhs_p = zrhs;
      *sense_p = zsense;
      *matbeg_p = zmatbeg;
      *matcnt_p = zmatcnt;
      *matind_p = zmatind;
      *matval_p = zmatval;
      *lb_p = zlb;
      *ub_p = zub;
      *ctype_p = zctype;
      *qmatbeg_p = zqmatbeg;
      *qmatcnt_p = zqmatcnt;
      *qmatind_p = zqmatind;
      *qmatval_p = zqmatval;
   }
   return (status);

}  // END setproblemdata

double QuadraticGAP::eigenValues(double *qmatval, int n)
{  int i,j,k;

   Eigen::MatrixXd m = Eigen::MatrixXd(n, n);
   m(0, 0) = 1; m(0, 1) = 4; m(0, 2) = 6; m(0, 3) = 4; m(0, 4) = 8; m(0, 5) = 12;
   m(1, 0) = 4; m(1, 1) = 1; m(1, 2) = 4; m(1, 3) = 8; m(1, 4) = 4; m(1, 5) = 8;
   m(2, 0) = 6; m(2, 1) = 4; m(2, 2) = 1; m(2, 3) = 12; m(2, 4) = 8; m(2, 5) = 4;
   m(3, 0) = 4; m(3, 1) = 8; m(3, 2) = 12; m(3, 3) = 1; m(3, 4) = 4; m(3, 5) = 6;
   m(4, 0) = 8; m(4, 1) = 4; m(4, 2) = 8; m(4, 3) = 4; m(4, 4) = 1; m(4, 5) = 4;
   m(5, 0) = 12; m(5, 1) = 8; m(5, 2) = 4; m(5, 3) = 6; m(5, 4) = 4; m(5, 5) = 1;

   Eigen::MatrixXd mat = Eigen::MatrixXd(n, n);
   k = 0;
   for(i=0;i<n;i++)
      for(j=0;j<n;j++)
      {  mat(i,j) = qmatval[k];
         //if(mat(i,j) != m(i,j)/2)
         //   cout << "ouch" << endl;
         k++;
      }

   double det = mat.determinant();
   cout << "Determinant: " << det << endl;

   Eigen::MatrixXd M = m + m.transpose();
   // Construct matrix operation object 
   DenseSymMatProd<double> op(M);

   /*
   Construct eigen solver object, requesting the smallest / largest eigenvalues SMALLEST_MAGN / LARGEST_ALGE
   Parameters
   op_  Pointer to the matrix operation object, which should implement the matrix - vector multiplication operation of A: calculating Av for any vector v. Users could either create the object from the wrapper class such as DenseSymMatProd, or define their own that implements all the public member functions as in DenseSymMatProd.
   nev_ Number of eigenvalues requested. This should satisfy 1<=nev<=n-1, where n is the size of matrix.
   ncv_ Parameter that controls the convergence speed of the algorithm. Typically a larger ncv_ means faster convergence, but it may also result in greater memory use and more matrix operations in each iteration.This parameter must satisfy nev<ncv<=n, and is advised to take ncv>=2nev.
   */

   SymEigsSolver< double, SMALLEST_ALGE, DenseSymMatProd<double> > eigs(&op, 1, n);
   // Initialize and compute
   eigs.init();
   int nconv = eigs.compute();
   // Retrieve results
   Eigen::VectorXd evalues;
   if (eigs.info() == SUCCESSFUL)
      evalues = eigs.eigenvalues();
   std::cout << "Smallest eigenvalues:" << evalues << std::endl;

   for (i = 0; i<n; i++)
      for (j = 0; j<n; j++)
         if (i == j) 
            if(evalues[0] < 0 && optimality_target == 1)
               qmatval[i*n+j] -= evalues[0];  // convexification
            else
               qmatval[i*n + j] *= 2;  // all other coefficients will be doubled

   return evalues[0];
}

// This simple routine frees up the pointer *ptr, and sets *ptr to NULL
void free_and_null(char **ptr)
{
   if (*ptr != NULL) {
      free(*ptr);
      *ptr = NULL;
   }
} // END free_and_null
