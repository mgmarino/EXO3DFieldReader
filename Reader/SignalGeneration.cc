#include "SignalGeneration.h"
#include "DriftTrajectory.h"

ClassImp(SignalGeneration)

SignalGeneration::SignalGeneration(const std::string& filename) : fWFMinLength(0), fWFDelayLength(0) 
{
  fWPot.SetFile(filename);
}

const EXODoubleWaveform& SignalGeneration::GetSignalWithDriftPath(const DriftTrajectory& dt) const
{
  const DriftTrajectory::PathType& path = dt.GetLastDriftedPath();
  size_t newSize = (fWFDelayLength + path.size() < fWFMinLength) ? 
    fWFMinLength : fWFDelayLength + path.size();
  fWFScratch.SetLength(newSize);
  fWFScratch.SetSamplingPeriod(dt.GetSamplingPeriod());
  if (fWFScratch.size() == 0) return fWFScratch;
  fWFScratch.Zero();

  double first_val = 0.0;
  for (size_t i=0;i<path.size();i++) {
    if (!fWPot.ReadValues(path[i].X(), path[i].Y(), path[i].Z())) break;
    if (i==0) first_val = fWPot.fValues[0];
    fWFScratch[fWFDelayLength + i] = fWPot.fValues[0] - first_val;
  }
 
  size_t oldLength = path.size() + fWFDelayLength; 
  if (fWFScratch[oldLength-1] > 0.999) fWFScratch[oldLength-1] = 1.;

  if (oldLength < fWFMinLength) {
    double lastVal = fWFScratch[oldLength - 1];
    for (size_t i=oldLength;i<fWFMinLength;i++) fWFScratch[i] = lastVal; 
  } 
  
  return fWFScratch; 
}
