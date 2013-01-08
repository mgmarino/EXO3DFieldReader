/*
This program reads in an ASCII file of field information, provided by Tamer from COMSOL.
It outputs a binary file of Nodes, one for each node in the ASCII file.
The purpose is that, later when we construct the tetrahedron, we know how to find the nth node because these are fixed-length records.

When run, the program requires two inputs:  the path to the ascii file, and the location where the node file should go.

Note that when you compile, NUM_VALUES should be defined to be 1 or 3 (depending on whether it's a weight field or electric field).
*/



#include "DataStructures.h"
#include "UtilityFunctions.h"
#include "EXOUtilities/SystemOfUnits.hh"
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <cassert>


Position ConvertCoordinates(Position Pos)
{
  // Convert so that we're using standard coordinate axes and units (mm rather than meters).
  // He uses North/South mirror symmetry -- assume we're using the North plane (positive z).
  Position RetPos;

  // Our z-coordinate is Tamer's y-coordinate.
  // His y=0 is the u-wire plane.
  // His cathode is centered at 0.19825 meters.
  // Tamer's +Y points North.
  RetPos.z = (0.19825 - Pos.y)*CLHEP::meter;

  // The resistor block (West side of the detector) is at our +X; it is at Tamer's -X.
  // The cylindrical axis is at Tamer's X = 0.18 meters.
  RetPos.x = (0.18 - Pos.x)*CLHEP::meter;

  // Since we all use right-handed coordinate frames.
  // Tamer's (X,Y,Z) is therefore (East, North, Up).
  // Our (X,Y,Z) is (West, Up, North), so our +Y corresponds to Tamer's +Z.
  // The cylindrical axis is at Tamer's Z = 0.272 meters.
  RetPos.y = (Pos.z - 0.272)*CLHEP::meter;

  return RetPos;
}

void ConvertFieldValues(float (&Field)[NUM_VALUES])
{
  // Since we're converting our coordinate system, we also need to rotate our electric field vectors.
  assert(NUM_VALUES == 3);
  Field[0] = -Field[0]; // x --> -x.
  float swap = Field[1];
  Field[1] = Field[2]; // z --> y.
  Field[2] = -swap; // -y -->z.
}

int WriteNodePositions(std::ifstream& OriginalTextFile, std::filebuf& NodeFile)
{
  std::cout<<"Entered WriteNodePositions."<<std::endl;
  int NumNodes = 0;
  std::string readresult;

  Position MinPos, MaxPos;
  MinPos.x = MinPos.y = MinPos.z = 0.0;
  MaxPos.x = MaxPos.y = MaxPos.z = 0.0;

  while(true) {
    std::getline(OriginalTextFile, readresult);
    if(readresult[0] != '%') break; // Beginning of nodes.
  }

  while(true) {
    if(NumNodes % 1000000 == 0) std::cout<<"\t"<<NumNodes<<" nodes written."<<std::endl;
    Position Pos;
    std::stringstream ParseLine(readresult, std::ios_base::in);
    ParseLine.exceptions(std::ios_base::failbit | std::ios_base::badbit);
    ParseLine >> Pos.x >> Pos.y >> Pos.z;
    Pos = ConvertCoordinates(Pos);

    // Track the range of coordinates we see; this will serve as a check for the validity of our world dimensions.
    MinPos.x = std::min(Pos.x, MinPos.x);
    MinPos.y = std::min(Pos.y, MinPos.y);
    MinPos.z = std::min(Pos.z, MinPos.z);
    MaxPos.x = std::max(Pos.x, MaxPos.x);
    MaxPos.y = std::max(Pos.y, MaxPos.y);
    MaxPos.z = std::max(Pos.z, MaxPos.z);

    // sizeof(Node) > sizeof(Position), but this reserves space for later.
    sputn(NodeFile, Pos, sizeof(Node));
    NumNodes++;

    std::getline(OriginalTextFile, readresult);
    assert(readresult.size() != 0);
    if(readresult[0] == '%') break; // Description line for elements
  }
  std::cout<<"Leaving WriteNodePositions; there were "<<NumNodes<<" nodes."<<std::endl;
  std::cout<<"X ranges from "<<MinPos.x<<" to "<<MaxPos.x<<std::endl;
  std::cout<<"Y ranges from "<<MinPos.y<<" to "<<MaxPos.y<<std::endl;
  std::cout<<"Z ranges from "<<MinPos.z<<" to "<<MaxPos.z<<std::endl;
  return NumNodes;
}

void WriteNodeValues(std::ifstream& OriginalTextFile, std::filebuf& NodeFile, int NumNodes, int offset)
{
  std::cout<<"Entered WriteNodeValues with " << NumNodes << " nodes."<<std::endl;

  std::streampos pos = sizeof(Position) + offset*sizeof(Float_t);
  for(int i = 0; i < NumNodes; i++) {
    if(i % 1000000 == 0) std::cout<<"\t"<<i<<" node values written."<<std::endl;

    float Field;
    std::string readLine;
    std::getline(OriginalTextFile, readLine);
    std::istringstream str(readLine);
    str.exceptions(std::ios_base::failbit | std::ios_base::badbit);
    try {
      str >> Field;
    } catch (std::ios_base::failure except) {
      std::cout<<"Exception caught from \"OriginlTextFile >> Field\" with i = "<<i<<std::endl;
      std::cout<<"We are in WriteNodeValues with offset = "<<offset<<std::endl;
      std::cout<<"The line was \""<<readLine<<"\""<<std::endl;
      throw;
    }

    SeekPos(NodeFile, pos, std::ios_base::out);
    sputn(NodeFile, Field);
    pos += sizeof(Node);
  }

  std::cout<<"Done with WriteNodeValues."<<std::endl;
}

int main(int argc, char* argv[])
{
  assert(sizeof(float) == 4);
  assert(sizeof(Position) == 12);
  assert(sizeof(Node) == 12 + 4*NUM_VALUES);
  assert(sizeof(Tetrahedron) == 4*sizeof(Node));
  assert(sizeof(InitHashFormat) == sizeof(Tetrahedron) + 8);
  assert(sizeof(FinalHashFormat) == 8);
  assert(sizeof(std::streampos) >= 8); // large file support

  // Must have two arguments, input file and output file.
  assert(argc == 3);

  std::ifstream OriginalEFile(argv[1]);
  OriginalEFile.exceptions(std::ios_base::failbit | std::ios_base::badbit); // Also catches if the constructor failed.

  std::filebuf NodeFile;
  std::filebuf* openPtr = NodeFile.open(argv[2], std::ios_base::in     |
                                                 std::ios_base::out    |
                                                 std::ios_base::binary |
                                                 std::ios_base::trunc);
  assert(openPtr);

  int NumNodes = WriteNodePositions(OriginalEFile, NodeFile);

  // Skip the elements -- we'll handle them in WriteTetFile.
  while(true) {
    std::string skipLine;
    std::getline(OriginalEFile, skipLine);
    assert(skipLine.size() != 0);
    if(skipLine[0] == '%') break; // Description line for data
  }

  // Write out the values -- they're stored in the file in segments by value.
  for(int i = 0; i < NUM_VALUES; i++) {
    WriteNodeValues(OriginalEFile, NodeFile, NumNodes, i);
    OriginalEFile.peek(); // We have to peek before eof gets set properly.
    if(not OriginalEFile.eof()) {
      std::string skipLine;
      std::getline(OriginalEFile, skipLine);
      assert(skipLine.size() and skipLine[0] == '%');
    }
  }
  std::cout<<"Done writing values.  Checking that we really reached the end of the ASCII file."<<std::endl;
  assert(OriginalEFile.eof());
  std::cout<<"Yep, OriginalEFile.eof() is true."<<std::endl;

  // If we're dealing with an electric field file, we need to go back and rotate the field vector.
  if(NUM_VALUES == 3) {
    std::cout<<"Beginning to rotate the electric field vectors appropriately."<<std::endl;
    std::streampos pos = sizeof(Position);
    float FieldValues[NUM_VALUES];
    for(int i = 0; i < NumNodes; i++) {
      if(i % 1000000 == 0) std::cout<<"\t"<<i<<" E vectors successfully rotated."<<std::endl;
      SeekPos(NodeFile, pos, std::ios_base::in | std::ios_base::out);
      sgetn(NodeFile, FieldValues);
      ConvertFieldValues(FieldValues);
      SeekPos(NodeFile, pos, std::ios_base::in | std::ios_base::out);
      sputn(NodeFile, FieldValues);
      pos += sizeof(Node);
    }
  }
  std::cout<<"Done."<<std::endl;
}
