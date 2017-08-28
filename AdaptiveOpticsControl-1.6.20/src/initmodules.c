#include "CLIcore.h"

#include "cudacomp/cudacomp.h"
#include "AtmosphericTurbulence/AtmosphericTurbulence.h"
#include "AtmosphereModel/AtmosphereModel.h"
#include "psf/psf.h"
#include "AOloopControl/AOloopControl.h"
#include "AOloopControl_IOtools/AOloopControl_IOtools.h"
#include "AOloopControl_acquireCalib/AOloopControl_acquireCalib.h"
#include "AOloopControl_computeCalib/AOloopControl_computeCalib.h"
#include "AOloopControl_PredictiveControl/AOloopControl_PredictiveControl.h"
#include "AOsystSim/AOsystSim.h"
#include "AOloopControl_DM/AOloopControl_DM.h"
#include "OptSystProp/OptSystProp.h"
#include "OpticsMaterials/OpticsMaterials.h"
#include "ZernikePolyn/ZernikePolyn.h"
#include "WFpropagate/WFpropagate.h"
#include "image_basic/image_basic.h"
#include "image_filter/image_filter.h"
#include "kdtree/kdtree.h"
#include "image_gen/image_gen.h"
#include "linopt_imtools/linopt_imtools.h"
#include "statistic/statistic.h"
#include "fft/fft.h"
#include "info/info.h"
#include "COREMOD_arith/COREMOD_arith.h"
#include "COREMOD_iofits/COREMOD_iofits.h"
#include "COREMOD_memory/COREMOD_memory.h"
#include "COREMOD_tools/COREMOD_tools.h"
#include "ImageStreamIO/ImageStreamIO.h"
#include "00CORE/00CORE.h"


extern DATA data;

int init_modules()
{
  init_cudacomp();
  init_AtmosphericTurbulence();
  init_AtmosphereModel();
  init_psf();
  init_AOloopControl();
  init_AOloopControl_IOtools();
  init_AOloopControl_acquireCalib();
  init_AOloopControl_computeCalib();
  init_AOloopControl_PredictiveControl();
  init_AOsystSim();
  init_AOloopControl_DM();
  init_OptSystProp();
  init_OpticsMaterials();
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
  init_ImageStreamIO();
  init_00CORE();

return 0;
}
