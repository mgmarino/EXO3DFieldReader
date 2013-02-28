#include "ThreeDFieldReader.h"
#include "TMatrix.h"
#include <cmath>
#include <cassert>

template<int NumValues>
bool ThreeDFieldReader<NumValues>::ReadValues(Float_t x, Float_t y, Float_t z)
{
  // Read the field value(s) at x,y,z.  Fill fValues with the result
  // (V/m, or weight potential with range from 0 to 1).
  // If this position does not lie within the field map geometry, return false.

  // Implement z mirror-symmetry.
  z = std::fabs(z);

  // Check range.
  if(x < fMinX or y < fMinY or z < fMinZ) return false;
  if(x >= fMinX + fStepX*fNumX or
     y >= fMinY + fStepY*fNumY or
     z >= fMinZ + fStepZ*fNumZ) return false;

  // Get indices.
  UShort_t ix = UShort_t((x - fMinX)/fStepX);
  UShort_t iy = UShort_t((y - fMinY)/fStepY);
  UShort_t iz = UShort_t((z - fMinZ)/fStepZ);

  // Be safe -- I hate leaving things to floating-point comparisons.
  if(ix >= fNumX or iy >= fNumY or iz >= fNumZ) return false;

  // Compute the position of the hash bin -- make sure we don't overflow any integer types!
  std::streampos hashpos = sizeof(HashFormat)*(ULong64_t(iz) +
                                               ULong64_t(fNumZ) * ULong64_t(UInt_t(iy) + UInt_t(fNumY)*UInt_t(ix)));
  SeekOff(hashpos, std::ios_base::beg);
  Read(fHashBuffer);

  // Read the appropriate tetrahedra.
  if(fHashBuffer.NumTet == 0) return false;
  SeekOff(fHashBuffer.FilePos, std::ios_base::beg);
  fInternalBuffer.resize(fHashBuffer.NumTet);
  Read(fInternalBuffer[0], sizeof(Tetrahedron)*fHashBuffer.NumTet);

  // Start scanning through for the tetrahedron containing point x,y,z.
  // Check each as we go; this allows for an early exit.
  for(size_t i = 0; i < fInternalBuffer.size(); i++) {
    if(fInternalBuffer[i].Contains(x,y,z)) {
      for(int nval = 0; nval < NumValues; nval++) {
        fValues[nval] = fInternalBuffer[i].Value(nval, x, y, z);
      }
      if(NumValues == 3) {
        // Forgot to rotate electric field vectors.  For now, do it here.
        // If the full serialization gets re-run, this should be eliminated.
        fValues[0] = -fValues[0];
        Float_t temp = fValues[2]; // z
        fValues[2] = -fValues[1];
        fValues[1] = temp;
      }
      return true;
    }
  }

  // Failed to find any tetrahedron; so return false.
  return false; 
}

template<int NumValues>
bool ThreeDFieldReader<NumValues>::Tetrahedron::Contains(Float_t x, Float_t y, Float_t z) const
{
  // Does this tetrahedron contain the given point?

  TVector coord = GetBarycentricCoordinates(x,y,z);
  assert(coord.GetNrows() == 4);
  for(size_t i = 0; i < 4; i++) {
    if(coord[i] < 0. or coord[i] > 1.) return false;
  }
  return true;
}

template<int NumValues>
TVector ThreeDFieldReader<NumValues>::Tetrahedron::GetBarycentricCoordinates(Float_t x, Float_t y, Float_t z) const
{
  // The notation here is from wikipedia.
  TMatrix t(3,3);
  bool allZeroes = node[3].IsZero();
  for(int coord = 0; coord < 3; coord++) {
    t(0, coord) = node[coord].Pos.x - node[3].Pos.x;
    t(1, coord) = node[coord].Pos.y - node[3].Pos.y;
    t(2, coord) = node[coord].Pos.z - node[3].Pos.z;
    if (!node[coord].IsZero()) allZeroes = false;
  }
  if (!allZeroes) t.InvertFast();

  TVector RelPos(3);
  RelPos[0] = x - node[3].Pos.x;
  RelPos[1] = y - node[3].Pos.y;
  RelPos[2] = z - node[3].Pos.z;

  RelPos *= t; // RelPos = t * RelPos

  // The fourth lambda is chosen so they sum to one.
  TVector Lambdas(4);
  for(size_t i = 0; i < 3; i++) Lambdas[i] = RelPos[i];
  Lambdas[3] = 1. - RelPos.Sum();
  return Lambdas;
}

template<int NumValues>
Float_t ThreeDFieldReader<NumValues>::Tetrahedron::Value(int i, Float_t x, Float_t y, Float_t z) const
{
  assert(i >= 0 and i < NumValues);

  TVector coord = GetBarycentricCoordinates(x,y,z);
  assert(coord.GetNrows() == 4);

  Float_t ret = 0.;
  for(size_t inode = 0; inode < 4; inode++) {
    ret += coord[inode]*node[inode].Value[i];
  }
  return ret;
}

template<int NumValues>
void ThreeDFieldReader<NumValues>::SetFile(std::string filename)
{
  // Close any open file, and open filename.
  // Extract all relevant meta-information from the footer of the file.

  fFieldFile.close(); // in case there was another open file.
  fFieldFile.open(filename.c_str(), std::ios_base::in | std::ios_base::binary);

  // Check that this is a file version I know how to interpret.
  SeekOff(-1, std::ios_base::end);
  UChar_t version;
  Read(version);
  if(version != 0) LogEXOMsg("We're using a field map file version that I don't know how to interpret.", EEAlert);

  // Check that this file matches NumValues.
  SeekOff(-2, std::ios_base::end);
  UChar_t readNumValues;
  Read(readNumValues);
  if(readNumValues != NumValues) LogEXOMsg("You've attempted to hand me a file with the wrong NumValues.", EEAlert);

  // Fill the meta-data we'll need later.
  SeekOff(-32, std::ios_base::end);
  Read(fNumX);
  Read(fMinX);
  Read(fStepX);
  Read(fNumY);
  Read(fMinY);
  Read(fStepY);
  Read(fNumZ);
  Read(fMinZ);
  Read(fStepZ);
}

template<int NumValues>
void ThreeDFieldReader<NumValues>::SeekOff(std::streamoff off, std::ios_base::seekdir way)
{
  // Checked pubseekoff.
  std::streampos pos = fFieldFile.pubseekoff(off, way, std::ios_base::in);
  if(pos == std::streampos(std::streamoff(-1))) LogEXOMsg("std::filebuf::pubseekoff failed.", EEAlert);
}


template class ThreeDFieldReader<1>; 
template class ThreeDFieldReader<3>; 
