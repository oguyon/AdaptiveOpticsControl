#ifndef _LINARFILTERPRED_H
#define _LINARFILTERPRED_H


int_fast8_t init_linARfilterPred();


long LINARFILTERPRED_LoadASCIIfiles(double tstart, double dt, long NBpt, long NBfr, const char *IDoutname);

long LINARFILTERPRED_SelectBlock(const char *IDin_name, const char *IDblknb_name, long blkNB, const char *IDout_name);

long LINARFILTERPRED_Build_LinPredictor(const char *IDin_name, long PForder, float PFlag, double SVDeps, double RegLambda, const char *IDoutPF_name, int outMode, int LOOPmode, float LOOPgain);

long LINARFILTERPRED_Apply_LinPredictor_RT(const char *IDfilt_name, const char *IDin_name, const char *IDout_name);

long LINARFILTERPRED_Apply_LinPredictor(const char *IDfilt_name, const char *IDin_name, float PFlag, const char *IDout_name);

float LINARFILTERPRED_ScanGain(char* IDin_name, float multfact, float framelag);

long LINARFILTERPRED_PF_updatePFmatrix(const char *IDPF_name, const char *IDPFM_name, float alpha);

long LINARFILTERPRED_PF_RealTimeApply(const char *IDmodevalOL_name, long IndexOffset, int semtrig, const char *IDPFM_name, long NBPFstep, const char *IDPFout_name, int nbGPU, long loop, long NBiter, int SAVEMODE, float tlag, long PFindex);

#endif
