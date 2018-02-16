#ifndef DATA_H
#define DATA_H
#include "json.h"
#include "QGAP.h"
#include <sstream>   // stringstream

class DataManager
{
public:
   DataManager();
   ~DataManager();

   Config* loadConfig();
   void readJSONdata(string);

   QuadraticGAP* QGAP;

protected:
private:
};

#endif // DATA_H