#include <iostream>
#include "QGAP.h"
#include "DataManager.h"

int main()
{  QuadraticGAP* QGAP = new QuadraticGAP();
   DataManager*  DM   = new DataManager();
   DM->QGAP = QGAP;
   DM->loadConfig();
   DM->readJSONdata("../../../instances/"+QGAP->conf->datafile);

   QGAP->Qopt();
   cout << "\n<ENTER> to exit ..."; cin.get();

   delete QGAP;
   delete DM;
}
