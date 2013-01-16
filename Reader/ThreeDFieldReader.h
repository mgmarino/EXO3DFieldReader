#ifndef ThreeDFieldReader_hh
#define ThreeDFieldReader_hh

#include "EXOUtilities/EXOErrorLogger.hh"
#include "Rtypes.h"
#include "TVector.h"
#include <string>
#include <fstream>
#include <iostream>

template<int NumValues>
class ThreeDFieldReader
{
 public:
  void SetFile(std::string filename);
  bool ReadValues(Float_t x, Float_t y, Float_t z);
  Float_t fValues[NumValues];

 private:
  std::filebuf fFieldFile;

  UShort_t fNumX;
  Float_t fMinX;
  Float_t fStepX;
  UShort_t fNumY;
  Float_t fMinY;
  Float_t fStepY;
  UShort_t fNumZ;
  Float_t fMinZ;
  Float_t fStepZ;

  // is properly aligned (4-byte aligment)
  struct Position
  {
    Float_t x;
    Float_t y;
    Float_t z;
    void Print() const {
      std::cout<<"("<<x<<", "<<y<<", "<<z<<")";
    }
    bool IsZero() const {
      return (x == 0 && y == 0 && z == 0); 
    }
  };

  // is properly aligned (4-byte alignment)
  struct Node
  {
    Position Pos;
    Float_t Value[NumValues];
    void Print() const {
      std::cout<<"F";
      Pos.Print();
      std::cout<<" = ";
      for(size_t i = 0; i < NumValues; i++) {
        std::cout<<Value[i];
        if(i+1 != NumValues) std::cout<<"\t";
      }
      std::cout<<std::endl;
    }
    bool IsZero() const {
      return Pos.IsZero(); 
    }

  };

  // is properly aligned (4-byte alignment)
  struct Tetrahedron
  {
    Node node[4];
    TVector GetBarycentricCoordinates(Float_t x, Float_t y, Float_t z) const;
    bool Contains(Float_t x, Float_t y, Float_t z) const;
    Float_t Value(int i, Float_t x, Float_t y, Float_t z) const;
    void Print() const {
      std::cout<<"Tetrahedron with:"<<std::endl;
      for(size_t i = 0; i < 4; i++) {
        std::cout<<"\t";
        node[i].Print();
      }
    }
  };

  std::vector<Tetrahedron> fInternalBuffer;

  // Hash table format for the final binary file.
  // I make FilePos a bitfield because this lets the (packed) object guaranteed to be 10-byte.
  struct __attribute__((__packed__)) HashFormat
  {
    UShort_t NumTet; // Number of tetrahedra (each of which is 96 bytes = 24 floats)
    ULong64_t FilePos : 48; // File position (bytes) where the tetrahedron array starts
  } fHashBuffer;

  // Functions
  void SeekOff(std::streamoff off, std::ios_base::seekdir way);

  template<typename T> void Read(T& data, size_t size = sizeof(T)) 
  {
    // Read in from the file, filling data.
    // Note that if data is a pointer, you must specify size.
    if(not fFieldFile.is_open()) LogEXOMsg("Attempting to read without opening a file.", EEAlert);
    std::streamsize check = fFieldFile.sgetn((char*)&data, size);
    if(check != (std::streamsize) size) LogEXOMsg("Read operation failed.", EEAlert);
  }

  template<typename T> void Read(T*)
  {
    // Prevent this!
    LogEXOMsg("Attempted to pass a pointer type to the Read function -- how can we infer the read size?", EEAlert);
  }

  ClassDefT(ThreeDFieldReader,1) // ThreeDField reader

};

typedef ThreeDFieldReader<1> ThreeDWeightPotentialReader;
typedef ThreeDFieldReader<3> ThreeDElectricFieldReader;

#endif /* ThreeDFieldReader_hh */
