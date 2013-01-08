#ifndef SignalGeneration_hh
#define SignalGeneration_hh

#include "EXOUtilities/EXOWaveform.hh"
#include "ThreeDFieldReader.h"
#include <string>

class DriftTrajectory;

class SignalGeneration {

  public:
    SignalGeneration(const std::string& filename);

    
    EXODoubleWaveform GetSignalWithDriftPath(const DriftTrajectory& dt) const;

  protected:
    mutable ThreeDWeightPotentialReader fWPot;

  ClassDef(SignalGeneration, 0)
  
};

#endif /* SignalGeneration_hh */
