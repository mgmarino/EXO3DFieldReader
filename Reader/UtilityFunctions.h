#ifndef UtilityFunctions_hh 
#define UtilityFunctions_hh 

/*
Just some utility functions, to abbreviate the safety checking I'm doing.
In principle I could derive from filebuf, but it's clearer what I'm doing this way.
*/

#include <fstream>
#include <cassert>
#include <iostream>

template<typename T>
void sputn(std::filebuf& file, T data, size_t size = sizeof(T))
{
  char* addr = (char*)&data;
/*It seemed, at one point, like there were issues with writing too-big chunks.  Didn't pan out.  
  while(size > 1000) {
    std::streamsize check = file.sputn(addr, 1000);
    if(check != 1000) {
      std::cout<<"check != size in sputn."<<std::endl;
      std::cout<<"size = 1000 (partial write) (sizeof(T) = "<<sizeof(T)<<")"<<std::endl;
      std::cout<<"check = "<<check<<std::endl;
    }
    assert(check == 1000);
    size -= 1000;
    addr += 1000;
  }*/

  std::streamsize check = file.sputn(addr, size);
  if(check != (std::streamsize)size) {
    std::cout<<"check != size in sputn."<<std::endl;
    std::cout<<"size = "<<size<<" (sizeof(T) = "<<sizeof(T)<<")"<<std::endl;
    std::cout<<"check = "<<check<<std::endl;
  }
  assert(check == (std::streamsize) size);
}

template<typename T>
void sputn(std::filebuf& file, T* data)
{
  assert("A template argument was a pointer type -- invalid.");
}

template<typename T>
void sgetn(std::filebuf& file, T& data)
{
  std::streamsize check = file.sgetn((char*)&data, sizeof(T));
  assert(check == sizeof(T));
}

template<typename T>
void sgetn(std::filebuf& file, T* data)
{
  assert("A template argument was a pointer type -- invalid.");
}

std::streampos GetEOF(std::filebuf& file)
{
  // Moves the output file pointer to the end of the file; returns the size of the file.
  std::streampos pos = file.pubseekoff(0, std::ios_base::end, std::ios_base::out);
  assert(pos != std::streampos(std::streamoff(-1)));
  return pos;
}

void SeekPos(std::filebuf& file, std::streampos pos, std::ios_base::openmode which)
{
  std::streampos ret = file.pubseekpos(pos, which);
  assert(ret == pos);
}

void SeekOff(std::filebuf& file, std::streamoff off, std::ios_base::seekdir way, std::ios_base::openmode which)
{
  std::streampos pos = file.pubseekoff(off, way, which);
  assert(pos != std::streampos(std::streamoff(-1)));
}

#endif /* UtilityFunctions_hh */ 
