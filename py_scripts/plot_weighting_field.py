import ROOT
hash_file_dir = "/exo/scratch1/3DFieldFiles/HashFiles"
ROOT.gSystem.Load("../Reader/libReader.so")
c1 = ROOT.TCanvas()
sg = ROOT.ThreeDFieldReader('1')
sig_gen_dict = range(38)
for i in range(38):
    sig_gen_dict[i] = sg(), sg() 
    sig_gen_dict[i][0].SetFile(hash_file_dir + "/WP_U%i.hash.bin" % (i+1))
    sig_gen_dict[i][1].SetFile(hash_file_dir + "/WP_V%i.hash.bin" % (i+1))


z_position = 180
hist = ROOT.TH2D("hist", "hist", 400, -200, 200, 400, -200, 200)
hist.SetXTitle("X (mm)")
hist.SetYTitle("Y (mm)")

sg = sig_gen_dict[19][0]
for i in range(len(sig_gen_dict)):
    print "Channel, ", i
    file_name = "VWire"
    for sg, name in [(sig_gen_dict[i][1], "V Wire, Ch %i" % i), (sig_gen_dict[i][0], "U Wire, Ch %i" % i)]:
        hist.Clear()
        hist.SetTitle(name)
        for x in range(-200, 200):
            print x
            for y in range(-200, 200):
                if not sg.ReadValues(x, y, z_position): continue 
                bin_number = hist.FindBin(x, y)
                hist.SetBinContent(bin_number,sg.fValues[0])
        
        hist.Draw("colz")
        c1.Update()
        c1.Print(file_name + str(i) + ".png") 
        file_name = "UWire"
        

