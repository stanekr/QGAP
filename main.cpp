#include <iostream>
#include "QGAP.h"
#include "DataManager.h"

int main()
{  QuadraticGAP* QGAP = new QuadraticGAP();
   DataManager*  DM   = new DataManager();
   DM->QGAP = QGAP;
   DM->loadConfig();
   if(QGAP->conf->mode == 1)
   {
      DM->readJSONdata(QGAP->conf->datapath + QGAP->conf->datafile);
      QGAP->Qopt();
   }
   else
      DM->transcode(QGAP->conf->datapath + QGAP->conf->datafile);
   cout << "\n<ENTER> to exit ..."; cin.get();

   delete QGAP;
   delete DM;
}
