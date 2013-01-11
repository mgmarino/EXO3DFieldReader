import ROOT
import math

def nearest_channel(pos, isV):
    # Which channel is closest?
    Channel = int(pos/ROOT.CHANNEL_WIDTH) + ROOT.NCHANNEL_PER_WIREPLANE/2;
    if(pos < 0): Channel-=1 #// round down, not towards zero.
    if(Channel < 0): Channel = 0
    if(Channel >= ROOT.NCHANNEL_PER_WIREPLANE): Channel = ROOT.NCHANNEL_PER_WIREPLANE-1;
    if (isV): Channel += ROOT.NCHANNEL_PER_WIREPLANE 
    return Channel

#def convert(u, v, x, y, z):
#    u = -x
#    v = y*math.sqrt(3)/2

hash_file_dir = "/exo/scratch1/3DFieldFiles/HashFiles"
standard_wf_length = 2048
standard_period = 1*ROOT.CLHEP.microsecond
oversample = 20
energy = 2000 # keV
scaling = ROOT.ADC_BITS/(ROOT.ADC_FULL_SCALE_ELECTRONS_WIRE*ROOT.W_VALUE_LXE_EV_PER_ELECTRON) 
z_position = 50


ROOT.gStyle.SetOptStat(0)
ROOT.gSystem.Load("../Reader/libReader.so")
c1 = ROOT.TCanvas()
dt = ROOT.DriftTrajectory(hash_file_dir + "/EFieldxyz.hash.bin")
dt.SetSamplingPeriod(standard_period/oversample)
rand = ROOT.TRandom3(0)

sig_gen_dict = range(38)

sg =ROOT.SignalGeneration
for i in range(38):
    sig_gen_dict[i] = sg(hash_file_dir + "/WP_U%i.hash.bin" % (i+1)),\
                        sg(hash_file_dir + "/WP_V%i.hash.bin" % (i+1))

for val in sig_gen_dict:
    for sg in val: 
        sg.SetTotalWaveformMinLength(standard_wf_length*oversample)
        sg.SetWaveformDelayTerm(standard_wf_length*oversample/2)


ahist = ROOT.TH3D("hist", "3D drift", 100, -100, 100, 100, -100, 100, 100, -200, 200)
hist2D = ROOT.TH2D("hist1", "hist1", 2048, 0, 2048, 100, -3, 3)
x, y = ROOT.Double(), ROOT.Double()
convert = ROOT.EXOMiscUtil.XYToUVCoords
upos, vpos = ROOT.Double(), ROOT.Double()
for i in range(30):
    print i
    #convert(i*10, i*10, x, y, 20)
    y = 0 
    x = -170 + 340*(i/30.) 
    y = 120#rand.Rndm()*200 - 100 
    x = 100#rand.Rndm()*200 - 100 
    temp = dt.DriftAndReturnPath(ROOT.TVector3(x, y, 20))
    ahist.Draw()
    dt.Draw("same P")
    c1.Update()
    #raw_input("E")
    for pos in temp: 
        pos.Print()
        convert(upos, vpos, pos.X(), pos.Y(), pos.Z())
        ROOT.TVector3(-pos.X(), vpos, pos.Z()).Print()
    last_pos = temp[temp.size()-1]
    last_pos.Print()
    convert(upos, vpos, last_pos.X(), last_pos.Y(), last_pos.Z())
    print "U: %i " % (nearest_channel(last_pos.X(), False))
    uind = (nearest_channel(last_pos.X(), False))
    print "V: %i " % (nearest_channel(vpos, True)-38)
    vind = (nearest_channel(vpos, True)-38)

    all_vwfs = []
    all_uwfs = []
    offset = -2
    achan = 0
    for u, v in [(sig_gen_dict[37-uind][0], sig_gen_dict[37-vind][1])]: 
        all_uwfs.append(u.GetSignalWithDriftPath(dt)) 
        all_vwfs.append(v.GetSignalWithDriftPath(dt)) 
        for awf in [all_uwfs[-1], all_vwfs[-1]]:
            awf -= offset
        offset += 4./38.
        achan += 1

    for name, all_wfs in [("V Wires", all_vwfs), ("U Wires", all_uwfs)]:
        j = 0
        hist2D.SetTitle(name + ", Initial point (%g, %g, %g) mm" % (i*10,i*10, 20))
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
