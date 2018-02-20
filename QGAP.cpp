#include "QGAP.h"

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

   // optimization results
   int      solstat;
   double   objval;
   double   x  = NULL;
   double   pi = NULL;
   double   slack = NULL;
   double   dj = NULL;

   CPXENVptr     env = NULL;
   CPXLPptr      lp = NULL;
   int           status;
   int           i, j;
   int           cur_numrows, cur_numcols;

   vector < vector <double>> q(m, vector<double>(n)); // for debug

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
      &matval, &lb, &ub, &qmatbeg, &qmatcnt,
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

   // Write a copy of the problem to a file. 
   status = CPXwriteprob(env, lp, "qgap.lp", NULL);
   if (status)
   {
      fprintf(stderr, "Failed to write LP to disk.\n");
      goto TERMINATE;
   }

   // Optimize the problem and obtain solution.
   status = CPXqpopt(env, lp);
   if (status) 
   {  fprintf(stderr, "Failed to optimize QP.\n");
      goto TERMINATE;
   }

   status = CPXsolution(env, lp, &solstat, &objval, &x, &pi, &slack, &dj);
   if (status) 
   {  fprintf(stderr, "Failed to obtain solution.\n");
      goto TERMINATE;
   }

   // Write the output to the screen.
   cout << "\nSolution status = " << solstat << endl;
   cout << "Solution value  = " << objval << endl;

   cur_numrows = CPXgetnumrows(env, lp);
   cur_numcols = CPXgetnumcols(env, lp);

   /*
   for (i = 0; i < cur_numrows; i++) 
      printf("Row %d:  Slack = %10f  Pi = %10f\n", i, slack[i], pi[i]);

   for (j = 0; j < cur_numcols; j++) 
      printf("Column %d:  Value = %10f  Reduced cost = %10f\n",
         j, x[j], dj[j]);
    */

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
   free_and_null((char **)&qmatbeg);
   free_and_null((char **)&qmatcnt);
   free_and_null((char **)&qmatind);
   free_and_null((char **)&qmatval);

   return (status);
}  

int QuadraticGAP::setproblemdata(char **probname_p, int *numcols_p, int *numrows_p,
   int *objsen_p, double **obj_p, double **rhs_p,
   char **sense_p, int **matbeg_p, int **matcnt_p,
   int **matind_p, double **matval_p, double **lb_p,
   double **ub_p, int **qmatbeg_p, int **qmatcnt_p,
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
               zqmatval[numNZ] = cqd[i][h] * cqf[j][k];
               numNZ++;
               hk++;
            }
         zqmatcnt[ij] = numNZ - zqmatbeg[ij];
         ij++;
      }

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
      *qmatbeg_p = zqmatbeg;
      *qmatcnt_p = zqmatcnt;
      *qmatind_p = zqmatind;
      *qmatval_p = zqmatval;
   }
   return (status);

}  // END setproblemdata

// This simple routine frees up the pointer *ptr, and sets *ptr to NULL
void free_and_null(char **ptr)
{
   if (*ptr != NULL) {
      free(*ptr);
      *ptr = NULL;
   }
} // END free_and_null
