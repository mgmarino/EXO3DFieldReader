#ifndef DataStructures_hh
#define DataStructures_hh

#include "Rtypes.h"

// is properly aligned (4-byte aligment)
struct Position
{
  Float_t x;
  Float_t y;
  Float_t z;
};

// is properly aligned (4-byte alignment)
struct Node
{
  Position Pos;
  Float_t Value[NUM_VALUES];
};

// is properly aligned (4-byte alignment)
struct Tetrahedron
{
  Node node[4];
};

// Node format for the singly-linked list I use as the first format for writing out tetrahedra.
// I use the packed attribute because otherwise padding will be automatically added here.
struct __attribute__((__packed__)) InitHashFormat
{
  Tetrahedron tet;
  bool IsFilled;
  ULong64_t NextPos : 56;
};

// Hash table format for the final binary file.
// I make FilePos a bitfield because this lets the (packed) object guaranteed to be 8-byte.
struct __attribute__((__packed__)) FinalHashFormat
{
  UShort_t NumTet; // Number of tetrahedra (each of which is 96 bytes = 24 floats)
  ULong64_t FilePos : 48; // File position (bytes) where the tetrahedron array starts
};

#endif /* DataStructures_hh */
