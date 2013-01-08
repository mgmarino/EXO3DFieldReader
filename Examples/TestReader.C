#include "ThreeDFieldReader.h"
#include <iostream>
#include <cstring>
#include <cstdlib>
using std::cout;
using std::endl;
using std::cin;

int main(int argc, char** argv)
{
  if(argc != 2) {
    cout<<"USAGE: TestReader <hashfilename>"<<endl;
    exit(1);
  }

  cout<<"Using argument "<<argv[1]<<endl;
  float x, y, z;

  if(std::strstr(argv[1], "EFieldxyz")) {
    cout<<"Using E-field reader."<<endl;
    ThreeDElectricFieldReader reader;
    reader.SetFile(argv[1]);
    while(true) {
      cout<<"X Y Z (mm):  ";
      cin>>x>>y>>z;
      bool ret = reader.ReadValues(x,y,z);
      if(not ret) {
        cout<<"Point not contained in mesh."<<endl;
        continue;
      }
      cout<<"Field (V/m) = "<<reader.fValues[0]<<" "<<reader.fValues[1]<<" "<<reader.fValues[2]<<endl;
    }
  }

  else if(std::strstr(argv[1], "WP_")) {
    cout<<"Using the weight potential reader."<<endl;
    ThreeDWeightPotentialReader reader;
    reader.SetFile(argv[1]);
    while(true) {
      cout<<"X Y Z (mm):  ";
      cin>>x>>y>>z;
      bool ret = reader.ReadValues(x,y,z);
      if(not ret) {
        cout<<"Point not contained in mesh."<<endl;
        continue;
      }
      cout<<"Weight potential = "<<reader.fValues[0]<<endl;
    }
  }

  else {
    cout<<"Did not recognize file name as EField or Weight Potential file."<<endl;
  }
}
