#include "SignalGeneration.h"
#include "DriftTrajectory.h"

ClassImp(SignalGeneration)

SignalGeneration::SignalGeneration(const std::string& filename) 
{
  fWPot.SetFile(filename);
}

EXODoubleWaveform SignalGeneration::GetSignalWithDriftPath(const DriftTrajectory& dt) const
{
  const DriftTrajectory::PathType& path = dt.GetLastDriftedPath();
  EXODoubleWaveform retWF;
  retWF.SetLength(path.size());
  retWF.SetSamplingPeriod(dt.GetSamplingPeriod());

  for (size_t i=0;i<path.size();i++) {
    if (!fWPot.ReadValues(path[i].X(), path[i].Y(), path[i].Z())) break;
    retWF[i] = fWPot.fValues[0];
  }
 
  return retWF; 
}
