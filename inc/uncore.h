#ifndef UNCORE_H
#define UNCORE_H

#include "champsim.h"
#include "cache.h"
#include "dram_controller.h"
//#include "drc_controller.h"

//#define DRC_MSHR_SIZE 48

// uncore
class UNCORE {
  public:

    // LLC
    CACHE LLC{"LLC", LLC_SET, LLC_WAY, LLC_SET*LLC_WAY, LLC_WQ_SIZE, LLC_RQ_SIZE, LLC_PQ_SIZE, LLC_MSHR_SIZE};

    // DRAM
    //MEMORY_CONTROLLER DRAM{"DRAM"}; 
    //MEMORY_CONTROLLER DRAM{"DRAM",tRP,tRCD,tCAS};
    //MEMORY_CONTROLLER NVM{"NVM",100*tRP,100*tRCD,100*tCAS};
    //MEMORY_CONTROLLER NVM{"NVM",tRP,tRCD,tCAS};
    MEMORY_CONTROLLER DRAM{"DRAM",TRP_DRAM,TRCD_DRAM,TCAS_DRAM}; 
//    MEMORY_CONTROLLER NVM{"NVM",TRP_NVM,TRCD_NVM,TCAS_NVM};
    MEMORY_CONTROLLER NVM{"NVM",TRP_DRAM,TRCD_DRAM,TCAS_DRAM};

    UNCORE(); 
};

extern UNCORE uncore;

#endif
