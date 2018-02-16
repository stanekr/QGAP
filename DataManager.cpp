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

   QGAP->conf->datafile  = JSV["datafile"];
   QGAP->conf->isverbose = JSV["isverbose"];
   return QGAP->conf;
}

// reads instance data from json formatted files
void DataManager::readJSONdata(string infile)
{
   string line;
   size_t i, j, cont;

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

   QGAP->cqd = (double**)malloc(QGAP->m * sizeof(double *));
   for (i = 0; i<JSV["costqd"].size(); i++)
   {
      QGAP->cqd[i] = (double*)malloc(QGAP->n * sizeof(double));
      for (j = 0; j<JSV["costqd"][i].size(); j++)
         QGAP->cqd[i][j] = JSV["costqd"][i][j];
   }

   QGAP->cqf = (double**)malloc(QGAP->m * sizeof(double *));
   for (i = 0; i<JSV["costqf"].size(); i++)
   {
      QGAP->cqf[i] = (double*)malloc(QGAP->n * sizeof(double));
      for (j = 0; j<JSV["costqf"][i].size(); j++)
         QGAP->cqf[i][j] = JSV["costqf"][i][j];
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