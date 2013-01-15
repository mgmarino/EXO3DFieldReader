#include "DriftTrajectory.h"
#include "EXOUtilities/EXODimensions.hh"
#include <iostream>
#include "TAxis.h"
using namespace std;

ClassImp(DriftTrajectory)

DriftTrajectory::DriftTrajectory(const std::string& filename)
 : fDriftVelocity(DRIFT_VELOCITY), fSamplingPeriod(SAMPLE_TIME_HIGH_BANDWIDTH),
   fMaxDriftPoints((size_t)-1)
{
  fEfield.SetFile(filename);
}

const DriftTrajectory::PathType& 
  DriftTrajectory::DriftAndReturnPath(const TVector3& startPoint) const
{
  fLastDriftPath.clear();
  TVector3 currentPoint = startPoint;
  while ( fEfield.ReadValues(currentPoint.X(), currentPoint.Y(), currentPoint.Z() ) ) {
    fLastDriftPath.push_back(currentPoint); 

    TVector3 evec(fEfield.fValues[0], fEfield.fValues[1], fEfield.fValues[2]);
    //cout << evec.Mag() << " ";
    //currentPoint.Print();
    if (evec.Mag() > 1e10) break;
    evec *= (1./evec.Mag());

    currentPoint += -fDriftVelocity*fSamplingPeriod*evec; 
     
    if (fLastDriftPath.size() >= fMaxDriftPoints) {
      LogEXOMsg("Drifted beyond max?!", EEWarning);
      break;
    }
  }

  return fLastDriftPath;
}

void DriftTrajectory::Draw(Option_t* opt) 
{
  fCurrentGraph.Clear();
  fCurrentGraph.Set(fLastDriftPath.size());
  for (size_t i=0;i<fLastDriftPath.size();i++) {
    fCurrentGraph.SetPoint(i, fLastDriftPath[i].X(), 
                              fLastDriftPath[i].Y(),
                              fLastDriftPath[i].Z());
  }

  fCurrentGraph.Draw(opt); 
}
