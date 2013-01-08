import ROOT

hash_file_dir = "/exo/scratch1/3DFieldFiles/HashFiles"

ROOT.gStyle.SetOptStat(0)
ROOT.gSystem.Load("../Reader/libReader.so")
c1 = ROOT.TCanvas()
dt = ROOT.DriftTrajectory(hash_file_dir + "/EFieldxyz.hash.bin")

sig_gen_dict = range(38)

sg =ROOT.SignalGeneration
for i in range(38):
    sig_gen_dict[i] = sg(hash_file_dir + "/WP_U%i.hash.bin" % (i+1)),\
                        sg(hash_file_dir + "/WP_V%i.hash.bin" % (i+1))

ahist = ROOT.TH3D("hist", "3D drift", 100, -100, 100, 100, -100, 100, 100, -200, 200)
hist2D = ROOT.TH2D("hist1", "hist1", 100, 0, 80, 100, -3, 3)
for i in range(100):
    print i
    temp = dt.DriftAndReturnPath(ROOT.TVector3(i, 1, 20))
    ahist.Draw()
    dt.Draw("same P")
    c1.Update()
    raw_input("E")

    all_vwfs = []
    all_uwfs = []
    offset = -2
    for u, v in sig_gen_dict: 
        all_uwfs.append(u.GetSignalWithDriftPath(dt)) 
        all_vwfs.append(v.GetSignalWithDriftPath(dt)) 
        for awf in [all_uwfs[-1], all_vwfs[-1]]:
            last_val = awf[len(awf)-1]
            awf.SetLength(len(awf) + 100)
            for k in range(100): awf[len(awf) - 100 + k] = last_val
            awf -= offset
        offset += 4./38.

    for name, all_wfs in [("V Wires", all_vwfs), ("U Wires", all_uwfs)]:
        j = 0
        hist2D.SetTitle(name + ", Initial point (%g, %g, %g) mm" % (i, 1, 20))
        hist2D.Draw()
        same = "same"
        for wf in all_wfs: 
            hist = wf.GimmeHist(str(j))
            hist.SetFillColor(0)
            hist.Draw(same)
            if j == 0:
                hist2D.SetXTitle(hist.GetXaxis().GetTitle())
            j+=1
        
        c1.Update()
        raw_input("E")
