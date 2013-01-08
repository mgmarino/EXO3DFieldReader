

#include "UtilityFunctions.h"
#include "DataStructures.h"
#include "Rtypes.h"
#include <fstream>
#include <cassert>
#include <cstdlib>

#include <iostream>


int main(int argc, char* argv[])
{
  assert(sizeof(float) == 4);
  assert(sizeof(Position) == 12);
  assert(sizeof(Node) == 12 + 4*NUM_VALUES);
  assert(sizeof(Tetrahedron) == 4*sizeof(Node));
  assert(sizeof(InitHashFormat) == sizeof(Tetrahedron) + 8);
  assert(sizeof(FinalHashFormat) == 8);
  assert(sizeof(std::streampos) >= 8); // large file support

  // Must have two arguments, input tetrahedron file and output file.
  assert(argc == 3);

  std::filebuf TetFile;
  std::filebuf* openPtr = TetFile.open(argv[1], std::ios_base::in    |
                                  std::ios_base::binary);
  assert(openPtr);

  SeekOff(TetFile, std::streamoff(-1), std::ios_base::end, std::ios_base::in);
  UChar_t version;
  sgetn(TetFile, version);
  assert(version == 0); // Otherwise, version mismatch.

  UChar_t read_numvalues;
  SeekOff(TetFile, std::streamoff(-2), std::ios_base::end, std::ios_base::in);
  sgetn(TetFile, read_numvalues);
  std::cout<<"read_numvalues = "<<(int)read_numvalues<<std::endl;
  assert(read_numvalues == NUM_VALUES); // Else, you compiled with NUM_VALUES defined incorrectly.

  // Get the size of the hash -- we don't really care about positions, just need ranges for the loops.
  UShort_t NumX, NumY, NumZ;
  SeekOff(TetFile, std::streamoff(-32), std::ios_base::end, std::ios_base::in);
  sgetn(TetFile, NumX);
  SeekOff(TetFile, std::streamoff(-22), std::ios_base::end, std::ios_base::in);
  sgetn(TetFile, NumY);
  SeekOff(TetFile, std::streamoff(-12), std::ios_base::end, std::ios_base::in);
  sgetn(TetFile, NumZ);

  // Output file:  a more neatly-organized hash file of tetrahedra.
  std::filebuf HashMapFile;
  openPtr = HashMapFile.open(argv[2], std::ios_base::out    |
                                      std::ios_base::binary |
                                      std::ios_base::trunc  );
  assert(openPtr);

  InitHashFormat hashNode;
  for(size_t ix = 0; ix < NumX; ix++) {
    std::cout<<"Starting ix = "<<ix<<" of "<<NumX<<std::endl;
    for(size_t iy = 0; iy < NumY; iy++) {
      for(size_t iz = 0; iz < NumZ; iz++) {
        std::vector<Tetrahedron> TetVector;
        std::streampos pos = sizeof(hashNode) * (iz + NumZ*(iy + NumY*ix));
        while(true) {
          SeekPos(TetFile, pos, std::ios_base::in);
          sgetn(TetFile, hashNode);
          if(hashNode.IsFilled) TetVector.push_back(hashNode.tet);
          if(hashNode.NextPos == 0) break;
          pos = hashNode.NextPos;
        }
        if(TetVector.size() > size_t(UShort_t(-1))) {
          std::cout<<"Max vector size exceeded; vector size = "<<TetVector.size()<<std::endl;
          exit(1);
        }
        FinalHashFormat outHash;
        outHash.NumTet = TetVector.size();
        outHash.FilePos = (TetVector.size() == 0 ? std::streampos(0) : GetEOF(HashMapFile));
        /* if(TetVector.size() != 0) {
          sputn(HashMapFile, TetVector[0], sizeof(Tetrahedron)*TetVector.size());
        }*/
        for(size_t iter = 0; iter < TetVector.size(); iter++) {
          sputn(HashMapFile, TetVector[iter]);
        }
        // Go back to fill in the hash map
        pos = sizeof(FinalHashFormat) * (iz + NumZ*(iy + NumY*ix));
        SeekPos(HashMapFile, pos, std::ios_base::out);
        sputn(HashMapFile, outHash);
      } // iz
    } // iy
  } // ix

  // Now, go ahead and fill in the header by copying the last 32 bytes of TetFile to HashMapFile.
  char header[32];
  SeekOff(TetFile, std::streamoff(-32), std::ios_base::end, std::ios_base::in);
  std::streamsize ret = TetFile.sgetn(header, 32);
  assert(ret == 32);

  GetEOF(HashMapFile);
  std::cout<<"About to write footer of the hash file; just FYI (in case it's why the programs are aborting)."<<std::endl;
  ret = HashMapFile.sputn(header, 32);
  assert(ret == 32);
}
