#include "SignalGeneration.h"
#include "DriftTrajectory.h"

ClassImp(SignalGeneration)

SignalGeneration::SignalGeneration(const std::string& filename) : fWFMinLength(0), fWFDelayLength(0) 
{
  fWPot.SetFile(filename);
}

EXODoubleWaveform SignalGeneration::GetSignalWithDriftPath(const DriftTrajectory& dt) const
{
  const DriftTrajectory::PathType& path = dt.GetLastDriftedPath();
  EXODoubleWaveform retWF;
  retWF.SetLength(fWFDelayLength+path.size());
  retWF.SetSamplingPeriod(dt.GetSamplingPeriod());
  if (retWF.size() == 0) return retWF;

  double first_val = 0.0;
  for (size_t i=0;i<path.size();i++) {
    if (!fWPot.ReadValues(path[i].X(), path[i].Y(), path[i].Z())) break;
    if (i==0) first_val = fWPot.fValues[0];
    retWF[fWFDelayLength + i] = fWPot.fValues[0] - first_val;
  }
 
  size_t oldLength = retWF.size();
  if (retWF[oldLength-1] > 0.999) retWF[oldLength-1] = 1.;

  if (retWF.size() < fWFMinLength) {
    double lastVal = retWF[oldLength - 1];
    retWF.SetLength(fWFMinLength);
    for (size_t i=oldLength;i<fWFMinLength;i++) retWF[i] = lastVal; 
  } 
  
  return retWF; 
}
