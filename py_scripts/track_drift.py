import ROOT
import numpy
ROOT.gROOT.SetBatch()
ROOT.gSystem.Load("libEXOROOT")

#Standare noise for Wires
standard_wf_length = 2048
standard_period = 1*ROOT.CLHEP.microsecond
oversample = 20
energy = 4 # keV
scaling = ROOT.ADC_BITS/(ROOT.ADC_FULL_SCALE_ELECTRONS_WIRE*ROOT.W_VALUE_LXE_EV_PER_ELECTRON) 
z_position = 50

hash_file_dir = "/exo/scratch1/3DFieldFiles/HashFiles"

ROOT.gSystem.Load("../Reader/libReader.so")
dt = ROOT.DriftTrajectory(hash_file_dir + "/EFieldxyz.hash.bin")
dt.SetSamplingPeriod(standard_period/oversample)


begin_pos = ROOT.TVector3()
end_pos = ROOT.TVector3()
open_file = ROOT.TFile("output.root", "recreate")
tree = ROOT.TTree("pos_tree", "Position Tree")
tree.Branch("begin", begin_pos)
tree.Branch("end", end_pos)

rand = ROOT.TRandom3(0)
for k in range(100000):
    if k % 100 == 0: print k
    y = rand.Rndm()*400 - 200 
    x = rand.Rndm()*400 - 200
    begin_pos.__assign__(ROOT.TVector3(x, y, z_position))
    temp = dt.DriftAndReturnPath(begin_pos)
    if temp.size() == 0:
        end_pos.__assign__(begin_pos)
    else:
        end_pos.__assign__(temp[temp.size()-1])

    tree.Fill()
   
tree.Write()
    
