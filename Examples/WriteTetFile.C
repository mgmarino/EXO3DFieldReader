/*
This file takes in three arguments:
ASCII file from Tamer
Serialized node file (from WriteNodeFile)
Output file

The output file contains one singly-linked list per hash bin; the elements of the lists are Tetrahedra (along with a bool to indicate whether an element is filled, and a file pointer to indicate where the next element lives).  This structure is chosen because, at this point, we don't know how many tetrahedra will occupy each bin.  It can range from none to hundreds, so this structure is flexible without carrying huge storage-space overhead.

This is the slowest of the three steps; checking whether a tetrahedron and a hash bin overlap is not fast, and enumerating the hash bins overlapping a given tetrahedron is even slower.

When the program is compiled, NUM_VALUES should be defined to 1 or 3, depending on whether this is a weight potential or electric field file we'll be reading.

Note that there is a header at the end of the file, containing:
UShort_t NumX
Float_t MinX
Float_t StepX
UShort_t NumY
Float_t MinY
Float_t StepY
UShort_t NumZ
Float_t MinZ
Float_t StepZ
UChar_t NUM_VALUES (should be 1 or 3 in our usage)
UChar_t version number (currently version 0 -- a 32-byte header)
EOF
*/

#include "DataStructures.h"
#include "UtilityFunctions.h"
#include "BoxTetIntersection.h"
#include "EXOUtilities/SystemOfUnits.hh"
#include <sstream>
#include <fstream>
#include <iostream>
#include <cassert>



int main(int argc, char* argv[])
{
  assert(sizeof(float) == 4);
  assert(sizeof(Position) == 12);
  assert(sizeof(Node) == 12 + 4*NUM_VALUES);
  assert(sizeof(Tetrahedron) == 4*sizeof(Node));
  assert(sizeof(InitHashFormat) == sizeof(Tetrahedron) + 8);
  assert(sizeof(FinalHashFormat) == 8);
  assert(sizeof(std::streampos) >= 8); // large file support

  // Must have three arguments, input ASCII and node files and output file.
  assert(argc == 4);

  std::ifstream OriginalEFile(argv[1]);
  OriginalEFile.exceptions(std::ios_base::failbit | std::ios_base::badbit); // Also catches if the constructor failed.

  std::filebuf NodeFile;
  std::filebuf* openPtr = NodeFile.open(argv[2], std::ios_base::in    |
                                                 std::ios_base::binary);
  assert(openPtr);

  // OK, we've written the nodes.  Now, let's get ready to create and hash tetrahedra as phase 2.
  // Phase 3 will be compression (ie using a hash table and the actual number of tetrahedra in each grid).
  Float_t StepX = CLHEP::mm;
  Float_t MinX = -227.5*CLHEP::mm;
  UShort_t NumX = 455;

  Float_t StepY = CLHEP::mm;
  Float_t MinY = -227.5*CLHEP::mm;
  UShort_t NumY = 455;

  Float_t StepZ = CLHEP::mm;
  Float_t MinZ = -14.5*CLHEP::mm;
  UShort_t NumZ = 233;

  // For this first write-out, I won't know how many tetrahedra there are per hash bin.
  // So, treat each hash bin as a singly-linked list of InitHashFormat.
  const UInt_t NumGrid = UInt_t(NumX) * UInt_t(NumY) * UInt_t(NumZ);
  InitHashFormat hashNode;
  hashNode.IsFilled = false;
  hashNode.NextPos = 0;
  std::filebuf TetOutFile;
  openPtr = TetOutFile.open(argv[3], std::ios_base::in     |
                                     std::ios_base::out    |
                                     std::ios_base::binary |
                                     std::ios_base::trunc  );
  assert(openPtr);
  std::cout<<"Initializing "<<argv[3]<<"."<<std::endl;
  for(UInt_t i = 0; i < NumGrid; i++) sputn(TetOutFile, hashNode);

  assert(GetEOF(TetOutFile) == (std::streampos) NumGrid*sizeof(InitHashFormat));
  std::cout<<"Done initializing "<<argv[3]<<"; let's fill it."<<std::endl;

  // Advance through the ASCII file until we start reading elements.
  while(true) {
    // First, get to the nodes.
    std::string skipLine;
    std::getline(OriginalEFile, skipLine);
    assert(skipLine.size() != 0);
    if(skipLine[0] != '%') break;
  }
  while(true) {
    // Now skip through the nodes; on the last iteration, we'll read the header for the elements.
    std::string skipLine;
    std::getline(OriginalEFile, skipLine);
    assert(skipLine.size() != 0);
    if(skipLine[0] == '%') break;
  }

  // Now, iterate through the tetrahedra
  std::string readLine;
  std::getline(OriginalEFile, readLine);
  assert(readLine.size() != 0);
  int NumElementsWritten = 0;
  while(readLine[0] != '%') {
    if(NumElementsWritten % 1000000 == 0) std::cout<<"\t"<<NumElementsWritten<<" tetrahedral elements written."<<std::endl;
    NumElementsWritten++;
    int nodeIndex[4]; // Where we'll save node indices.
    std::istringstream stream(readLine);
    stream >> nodeIndex[0] >> nodeIndex[1] >> nodeIndex[2] >> nodeIndex[3];
    Tetrahedron outTet;
    for(int j = 0; j < 4; j++) {
      nodeIndex[j] -= 1; // 1 --> the first node, so subtract 1 from the node index.
      SeekPos(NodeFile, nodeIndex[j]*sizeof(Node), std::ios_base::in);
      sgetn(NodeFile, outTet.node[j]);
    }
    // OK, outTet is filled with this tetrahedron -- where does it belong?
    for(int j = 0; j < 4; j++) {
      // First, let's just assert that the positions are all legit, for safety.
      assert(outTet.node[j].Pos.x >= MinX and outTet.node[j].Pos.x < MinX + NumX*StepX);
      assert(outTet.node[j].Pos.y >= MinY and outTet.node[j].Pos.y < MinY + NumY*StepY);
      assert(outTet.node[j].Pos.z >= MinZ and outTet.node[j].Pos.z < MinZ + NumZ*StepZ);
    }
    size_t MinIndexX = std::min(std::min(size_t((outTet.node[0].Pos.x - MinX)/StepX),
                                         size_t((outTet.node[1].Pos.x - MinX)/StepX)),
                                std::min(size_t((outTet.node[2].Pos.x - MinX)/StepX),
                                         size_t((outTet.node[3].Pos.x - MinX)/StepX)));
    size_t MaxIndexX = std::max(std::max(size_t((outTet.node[0].Pos.x - MinX)/StepX),
                                         size_t((outTet.node[1].Pos.x - MinX)/StepX)),
                                std::max(size_t((outTet.node[2].Pos.x - MinX)/StepX),
                                         size_t((outTet.node[3].Pos.x - MinX)/StepX)));

    size_t MinIndexY = std::min(std::min(size_t((outTet.node[0].Pos.y - MinY)/StepY),
                                         size_t((outTet.node[1].Pos.y - MinY)/StepY)),
                                std::min(size_t((outTet.node[2].Pos.y - MinY)/StepY),
                                         size_t((outTet.node[3].Pos.y - MinY)/StepY)));
    size_t MaxIndexY = std::max(std::max(size_t((outTet.node[0].Pos.y - MinY)/StepY),
                                         size_t((outTet.node[1].Pos.y - MinY)/StepY)),
                                std::max(size_t((outTet.node[2].Pos.y - MinY)/StepY),
                                         size_t((outTet.node[3].Pos.y - MinY)/StepY)));

    size_t MinIndexZ = std::min(std::min(size_t((outTet.node[0].Pos.z - MinZ)/StepZ),
                                         size_t((outTet.node[1].Pos.z - MinZ)/StepZ)),
                                std::min(size_t((outTet.node[2].Pos.z - MinZ)/StepZ),
                                         size_t((outTet.node[3].Pos.z - MinZ)/StepZ)));
    size_t MaxIndexZ = std::max(std::max(size_t((outTet.node[0].Pos.z - MinZ)/StepZ),
                                         size_t((outTet.node[1].Pos.z - MinZ)/StepZ)),
                                std::max(size_t((outTet.node[2].Pos.z - MinZ)/StepZ),
                                         size_t((outTet.node[3].Pos.z - MinZ)/StepZ)));

    // Now we have to iterate through all of these bins where the tetrahedron might overlap.
    BoxTetIntersection::Tet BoxTetIntersection_t;
    for(int j = 0; j < 4; j++) {
      BoxTetIntersection_t.vertex[j].X = outTet.node[j].Pos.x;
      BoxTetIntersection_t.vertex[j].Y = outTet.node[j].Pos.y;
      BoxTetIntersection_t.vertex[j].Z = outTet.node[j].Pos.z;
    }
    for(size_t ix = MinIndexX; ix <= MaxIndexX; ix++) {
      for(size_t iy = MinIndexY; iy <= MaxIndexY; iy++) {
        for(size_t iz = MinIndexZ; iz <= MaxIndexZ; iz++) {
          BoxTetIntersection::Box3D box;
          box.X.min = MinX + ix*StepX;
          box.X.max = box.X.min + StepX;
          box.Y.min = MinY + iy*StepY;
          box.Y.max = box.Y.min + StepY;
          box.Z.min = MinZ + iz*StepZ;
          box.Z.max = box.Z.min + StepZ;
          if(BoxTetIntersection::TestIntersection(BoxTetIntersection_t, box)) {
            // The tetrahedron does actually intersect with this hash bin.  Make it so.
            std::streampos pos = sizeof(hashNode) * (iz + NumZ*(iy + NumY*ix));


            // Follow the singly-linked list until we come to an empty node.
            // It is OK if pos points to bytes that haven't yet been created -- we're just going to write to them.
            while(true) {
              SeekPos(TetOutFile, pos, std::ios_base::in);
              sgetn(TetOutFile, hashNode);
              if(not hashNode.IsFilled) break; // We're pointing to a valid empty slot.
              if(hashNode.NextPos == 0) {
                hashNode.NextPos = GetEOF(TetOutFile);
                SeekPos(TetOutFile, pos, std::ios_base::out); // Need to go back and update node pointer.
                sputn(TetOutFile, hashNode);
                pos = hashNode.NextPos;
                break;
              }
              else {
                pos = hashNode.NextPos;
              }
            } // Done finding an empty slot -- now pos is the position of an empty slot, and hashNode can still be filled.

            // Fill hashNode
            hashNode.IsFilled = true;
            hashNode.tet = outTet;
            hashNode.NextPos = 0;

            // Move to the next available slot, and write hashNode.
            SeekPos(TetOutFile, pos, std::ios_base::out);
            sputn(TetOutFile, hashNode);
          } // end writing outTet
        } // end loop over iz
      } // end loop over iy
    } // end loop over ix
    std::getline(OriginalEFile, readLine);
    assert(readLine.size() != 0);
  } // end loop over tetrahedra
  std::cout<<"Finished filling "<<argv[3]<<"; there were "<<(int)NumElementsWritten<<" elements."<<std::endl;
  std::cout<<"Now, appending header information so the binning information is saved with the file."<<std::endl;
  // Now I suppose I should write out "header" information so it's saved directly into the file.
  std::streampos SizeBeforeHeader = GetEOF(TetOutFile);

  sputn(TetOutFile, NumX);
  sputn(TetOutFile, MinX);
  sputn(TetOutFile, StepX);

  sputn(TetOutFile, NumY);
  sputn(TetOutFile, MinY);
  sputn(TetOutFile, StepY);

  sputn(TetOutFile, NumZ);
  sputn(TetOutFile, MinZ);
  sputn(TetOutFile, StepZ);

  sputn(TetOutFile, (UChar_t)NUM_VALUES);
  sputn(TetOutFile, (UChar_t)0); // A version identifier -- to give some rudimentary check on this.

  assert(GetEOF(TetOutFile) == SizeBeforeHeader + std::streamoff(32));
}
