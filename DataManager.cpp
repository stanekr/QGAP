#include "DataManager.h"
#include <stdexcept>

DataManager::DataManager()
{
   //ctor
}

DataManager::~DataManager()
{
   //dtor
}

// loading parameters of all supported algorithms
Config* DataManager::loadConfig()
{
   string infile, line;

   infile = "config.json";
   cout << "Opening " << infile << endl;

   ifstream jData(infile.c_str());
   std::stringstream buffer;
   buffer << jData.rdbuf();
   line = buffer.str();
   //std::getline(jData,line);
   jData.close();

   json::Value JSV = json::Deserialize(line);

   QGAP->conf->datapath = JSV["datapath"];
   QGAP->conf->datafile = JSV["datafile"];
   QGAP->conf->mode     = JSV["mode"];
   QGAP->conf->opt_target = JSV["opt_target"];
   QGAP->conf->isverbose  = JSV["isverbose"];
   return QGAP->conf;
}

// reads instance data from json formatted files
void DataManager::readJSONdata(string infile)
{
   string line;
   size_t i, j, cont;
   int format;

   try
   {
      cout << "Reading " << infile << endl;
      ifstream jData;
      jData.open(infile, std::ifstream::in);
      jData.exceptions(ifstream::eofbit | ifstream::failbit | ifstream::badbit);
      getline(jData, line);
      //cout << "line:" << line << endl;
      jData.close();
   }
   catch (std::exception const& e)
   {
      cout << "Error: " << e.what() << endl;
      return;
   }

   json::Value JSV = json::Deserialize(line);
   QGAP->name = JSV["name"];
   QGAP->n = JSV["numcli"];
   QGAP->m = JSV["numserv"];
   format  = JSV["format"];   // 1: single quad matrix, 2 two matrices to multiply
   QGAP->cap = (int*)malloc(QGAP->m * sizeof(int));
   for (i = 0; i<JSV["cap"].size(); i++)
      QGAP->cap[i] = JSV["cap"][i];

   QGAP->cl = (double**)malloc(QGAP->m * sizeof(double *));
   for (i = 0; i<JSV["costlin"].size(); i++)
   {
      QGAP->cl[i] = (double*)malloc(QGAP->n * sizeof(double));
      for (j = 0; j<JSV["costlin"][i].size(); j++)
         QGAP->cl[i][j] = JSV["costlin"][i][j];
   }

   if (format > 1)   // 2 matrices, distances and flows, d_ij, f_hk
   {
      QGAP->cqd = (double**)malloc(QGAP->m * sizeof(double *));
      for (i = 0; i<JSV["costqd"].size(); i++)
      {
         QGAP->cqd[i] = (double*)malloc(QGAP->m * sizeof(double));
         for (j = 0; j<JSV["costqd"][i].size(); j++)
            QGAP->cqd[i][j] = JSV["costqd"][i][j];
      }

      QGAP->cqf = (double**)malloc(QGAP->n * sizeof(double *));
      for (i = 0; i<JSV["costqf"].size(); i++)
      {
         QGAP->cqf[i] = (double*)malloc(QGAP->n * sizeof(double));
         for (j = 0; j<JSV["costqf"][i].size(); j++)
            QGAP->cqf[i][j] = JSV["costqf"][i][j];
      }
   }
   else     // 1 matrix, c_ijhk
   {
      QGAP->cqd = (double**)malloc(QGAP->m*QGAP->n * sizeof(double *));
      for (i = 0; i<JSV["costq"].size(); i++)
      {
         QGAP->cqd[i] = (double*)malloc(QGAP->m*QGAP->n * sizeof(double));
         for (j = 0; j<JSV["costq"][i].size(); j++)
            QGAP->cqd[i][j] = JSV["costq"][i][j];
      }
      QGAP->cqf = NULL;
   }

   QGAP->req = (int**)malloc(QGAP->m * sizeof(int *));
   for (i = 0; i<JSV["req"].size(); i++)
   {
      QGAP->req[i] = (int*)malloc(QGAP->n * sizeof(int));
      for (j = 0; j<JSV["req"][i].size(); j++)
         QGAP->req[i][j] = JSV["req"][i][j];
   }

   cout << "JSON data read" << endl;;
}

bool endsWith(const string& s, const string& suffix)
{
   return s.rfind(suffix) == (s.size() - suffix.size());
}


// trim, toglie spazi bianchi a inizio e fine stringa
static std::string Trim(const std::string& str)
{
   std::string s = str;

   // remove white space in front
   s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));

   // remove trailing white space
   s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());

   return s;
}

// split di una stringa in un array di elementi delimitati da separatori
vector<string> split(string str, string sep)
{
   char* cstr = const_cast<char*>(str.c_str());
   char* current;
   vector<string> elem;
   current = strtok(cstr, sep.c_str());
   while (current != NULL)
   {
      elem.push_back(current);
      current = strtok(NULL, sep.c_str());
   }
   return elem;
}

void DataManager::transcode(string infile)
{  
   if(endsWith(infile,"json"))
   {  cout << "Transcoding json -> ampl" << endl;
      json2ampl(infile);
   }
   else
   {  cout << "Transcoding ampl -> json" << endl;
      ampl2json(infile);
   }
}

void DataManager::json2ampl(string infile)
{  int i,j,h,k,n,m;

   readJSONdata(infile);
   if(QGAP->cqf == NULL)
   {  cout << "Unacceptable input format, I need the d and f matrices" << endl;
      return;
   }

   string str = infile, str2 = "json", str3 = "dat";
   str.replace(str.find(str2), str2.length(), str3);

   ofstream amplFile;
   amplFile.open(str);
   m = QGAP->m;
   n = QGAP->n;
   str = str.substr(str.find_last_of("/") + 1);
   cout << "### QGAP data-file " << str << " ###" << endl;
   amplFile << "### QGAP data-file " << str << " ###" << endl;
   amplFile << endl;
   amplFile << "param: I: a:= " << endl;

   for(i=0;i<m;i++)
   {  amplFile << i+1 << " " << QGAP->cap[i] << endl;
   }

   amplFile << ";" << endl;
   amplFile << endl;
   amplFile << "set J := ";
   for(i=0;i<n;i++) amplFile << i+1 <<" ";
   amplFile << ";" << endl;
   amplFile << endl;
   amplFile << "set R := "; 
   for (i = 0; i<m; i++) amplFile << i+1 << " ";
   amplFile << ";" << endl;
   amplFile << endl;
   amplFile << "set S := "; 
   for (i = 0; i<n; i++) amplFile << i+1 << " ";
   amplFile << ";" << endl;

   // requests
   amplFile << " \nparam w :\n  ";
   for (i = 0; i<n; i++) 
      amplFile << setw(4) << (i + 1);
   amplFile << " := " << endl;
   for(i=0;i<m;i++)
   {  amplFile << setw(2) << i+1;
      for(j=0;j<n;j++) amplFile << setw(4) << QGAP->req[i][j];
      amplFile << (i==m-1 ? " ;" : "") << endl;
   }

   // linear costs
   amplFile << " \nparam p :\n  ";
   for (i = 0; i<n; i++)
      amplFile << setw(4) << (i + 1);
   amplFile << " := " << endl;
   for (i = 0; i<m; i++)
   {
      amplFile << setw(2) << i + 1;
      for (j = 0; j<n; j++) amplFile << setw(4) << QGAP->cl[i][j];
      amplFile << (i == m - 1 ? " ;" : "") << endl;
   }

   // distance matrix
   amplFile << " \nparam d :\n  ";
   for (i = 0; i<m; i++)
      amplFile << setw(4) << (i + 1);
   amplFile << " := " << endl;
   for (i = 0; i<m; i++)
   {
      amplFile << setw(2) << i + 1;
      for (j = 0; j<m; j++) amplFile << setw(4) << QGAP->cqd[i][j];
      amplFile << (i == m - 1 ? " ;" : "") << endl;
   }

   // flow matrix
   amplFile << " \nparam f :\n  ";
   for (i = 0; i<n; i++)
      amplFile << setw(4) << (i + 1);
   amplFile << " := " << endl;
   for (i = 0; i<n; i++)
   {
      amplFile << setw(2) << i + 1;
      for (j = 0; j<n; j++) amplFile << setw(4) << QGAP->cqf[i][j];
      amplFile << (i == n - 1 ? " ;" : "") << endl;
   }

   amplFile.close();
}

void DataManager::ampl2json(string infile)
{
   string line;
   int i,j,m,n;
   vector<string> elem;
   vector<vector<int>> clin,req,cqd,cqf;

   // data reading section
   ifstream amplFile;
   amplFile.open(infile);
   amplFile.exceptions(ifstream::eofbit | ifstream::failbit | ifstream::badbit);
   
   do
   {
      getline(amplFile, line);
      line = Trim(line);
      cout << line << endl;
   }
   while(line.substr(0,1) != "p");

   // reading capacities
   vector<int> cap;
   getline(amplFile, line);
   do
   {
      line = Trim(line);
      elem = split(line," ");
      cap.push_back(atoi(elem[1].c_str() ));
      cout << line << endl;
      getline(amplFile, line);
   } while (line.substr(0, 1) != ";");
   m = cap.size();

   // reading requests
   do
   {
      getline(amplFile, line);
      line = Trim(line);
      cout << line << endl;
   } while (line.substr(0, 1) != "p");

   getline(amplFile, line);
   line = Trim(line);
   elem = split(line, " ");
   n = elem.size() - 1; // there is the := at the end of the line

   i=0;
   do
   {
      getline(amplFile, line);
      line = Trim(line);
      cout << line << endl;
      elem = split(line, " ");
      req.push_back(vector<int>());
      for(j=0;j<n;j++)
         req[i].push_back(atoi(elem[j+1].c_str()));
      i++;
   } while (line.find(";") == string::npos);

   // reading linear costs
   do
   {
      getline(amplFile, line);
      line = Trim(line);
      cout << line << endl;
   } while (line.substr(0, 1) != "p");

   getline(amplFile, line);
   i = 0;
   do
   {
      getline(amplFile, line);
      line = Trim(line);
      cout << line << endl;
      elem = split(line, " ");
      clin.push_back(vector<int>());
      for (j = 0; j<n; j++)
         clin[i].push_back(atoi(elem[j + 1].c_str()));
      i++;
   } while (line.find(";") == string::npos);

   // reading distance quadratic costs
   do
   {
      getline(amplFile, line);
      line = Trim(line);
      cout << line << endl;
   } while (line.substr(0, 1) != "p");

   getline(amplFile, line);
   i = 0;
   do
   {
      getline(amplFile, line);
      line = Trim(line);
      cout << line << endl;
      elem = split(line, " ");
      cqd.push_back(vector<int>());
      for (j = 0; j<m; j++)
         cqd[i].push_back(atoi(elem[j + 1].c_str()));
      i++;
   } while (line.find(";") == string::npos);

   // reading flow quadratic costs
   do
   {
      getline(amplFile, line);
      line = Trim(line);
      cout << line << endl;
   } while (line.substr(0, 1) != "p");

   getline(amplFile, line);
   i = 0;
   do
   {
      getline(amplFile, line);
      line = Trim(line);
      cout << line << endl;
      elem = split(line, " ");
      cqf.push_back(vector<int>());
      for (j = 0; j<n; j++)
         cqf[i].push_back(atoi(elem[j + 1].c_str()));
      i++;
   } while (line.find(";") == string::npos);

   amplFile.close();

   // data writing section


   string str = infile, str2 = "dat", str3 = "json";
   str = str.replace(str.find(str2), str2.length(), str3);
   QGAP->name = str;

   json::Object obj;
   obj["name"]= str.substr(str.find_last_of("/") + 1);
   obj["numcli"]  = n;
   obj["numserv"] = m;
   obj["format"]  = 2;

   json::Array jcostlin;
   for (int i = 0; i<clin.size(); i++)
   {  json::Array vec;
      for (int j = 0; j<clin[i].size(); j++)
         vec.push_back(clin[i][j]);
      jcostlin.push_back(vec);
   }   
   obj["costlin"] = jcostlin;

   json::Array jcostqd;
   for (int i = 0; i<cqd.size(); i++)
   {
      json::Array vec;
      for (int j = 0; j<cqd[i].size(); j++)
         vec.push_back(cqd[i][j]);
      jcostqd.push_back(vec);
   }
   obj["costqd"] = jcostqd;

   json::Array jcostqf;
   for (int i = 0; i<cqf.size(); i++)
   {
      json::Array vec;
      for (int j = 0; j<cqf[i].size(); j++)
         vec.push_back(cqf[i][j]);
      jcostqf.push_back(vec);
   }
   obj["costqf"] = jcostqf;

   json::Array jreq;
   for (int i = 0; i<req.size(); i++)
   {
      json::Array vec;
      for (int j = 0; j<req[i].size(); j++)
         vec.push_back(req[i][j]);
      jreq.push_back(vec);
   }
   obj["req"] = jreq;

   json::Array jcap;
   for (int i = 0; i<cap.size(); i++)
      jcap.push_back(cap[i]);
   obj["cap"] = jcap;

   string jObj = json::Serialize(obj);

   ofstream jsonFile;
   jsonFile.open(str);
   jsonFile << jObj << endl;
   jsonFile.close();
}

