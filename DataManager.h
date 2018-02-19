#ifndef DATA_H
#define DATA_H
#include <sstream>   // stringstream
#include "json.h"
#include "QGAP.h"

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