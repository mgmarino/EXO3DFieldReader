#ifndef DriftTrajectory_hh
#define DriftTrajectory_hh

#include "TObject.h"
#include "TVector3.h"
#include "TGraph2D.h"
#include "ThreeDFieldReader.h"
#include <vector>
#include <string>

class DriftTrajectory : public TObject
{

  public:
    typedef std::vector<TVector3> PathType;

    DriftTrajectory(const std::string& filename);

    const PathType& DriftAndReturnPath(const TVector3& startPoint) const;

    void Draw(Option_t* opt = "");

    void SetDriftVelocity(double dv) { fDriftVelocity = dv; }
    void SetSamplingPeriod(double sp) { fSamplingPeriod = sp; }

    double GetSamplingPeriod() const { return fSamplingPeriod; }

    const PathType& GetLastDriftedPath() const { return fLastDriftPath; }
  protected:

    DriftTrajectory(); 

    mutable ThreeDElectricFieldReader fEfield;
    mutable PathType                  fLastDriftPath;
    Double_t                  fDriftVelocity;
    Double_t                  fSamplingPeriod;
    TGraph2D                  fCurrentGraph;

    ClassDef(DriftTrajectory, 0)

};

#endif /* DriftTrajectory_hh */
