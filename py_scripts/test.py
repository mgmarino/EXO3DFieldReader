import ROOT

ROOT.gSystem.Load("../Reader/libReader.so")
c1 = ROOT.TCanvas()
dt = ROOT.DriftTrajectory("/exo/scratch1/3DFieldFiles/HashFiles/EFieldxyz.hash.bin")

ahist = ROOT.TH3D("hist", "hist", 100, -100, 100, 100, -100, 100, 100, -200, 200)
ahist.Draw()
for i in range(100):
    print i
    temp = dt.DriftAndReturnPath(ROOT.TVector3(i, 1, 20))
    dt.Draw("same P")
    c1.Update()
    raw_input("E")
