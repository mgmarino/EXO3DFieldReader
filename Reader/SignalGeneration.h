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

    void SetTotalWaveformMinLength(size_t i) { fWFMinLength = i; }
    void SetWaveformDelayTerm(size_t i) { fWFDelayLength = i; }

  protected:
    mutable ThreeDWeightPotentialReader fWPot;
    size_t fWFMinLength;
    size_t fWFDelayLength;

  ClassDef(SignalGeneration, 0)
  
};

#endif /* SignalGeneration_hh */
