#include "CLIcore.h"

extern DATA data;

int init_modules()
{
  init_cudacomp();
  init_AOloopControl();
  init_AOloopControl_DM();
  init_ZernikePolyn();
  init_WFpropagate();
  init_image_basic();
  init_image_filter();
  init_kdtree();
  init_image_gen();
  init_linopt_imtools();
  init_statistic();
  init_fft();
  init_info();
  init_COREMOD_arith();
  init_COREMOD_iofits();
  init_COREMOD_memory();
  init_COREMOD_tools();
  init_00CORE();

return 0;
}
