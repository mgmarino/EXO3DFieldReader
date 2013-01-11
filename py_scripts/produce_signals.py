import ROOT
import numpy
ROOT.gROOT.SetBatch()
ROOT.gSystem.Load("libEXOROOT")
def nearest_channel(pos, isV):
    # Which channel is closest?
    Channel = int(pos/ROOT.CHANNEL_WIDTH) + ROOT.NCHANNEL_PER_WIREPLANE/2;
    if(pos < 0): Channel-=1 #// round down, not towards zero.
    if(Channel < 0): Channel = 0
    if(Channel >= ROOT.NCHANNEL_PER_WIREPLANE): Channel = ROOT.NCHANNEL_PER_WIREPLANE-1;
    Channel = ROOT.NCHANNEL_PER_WIREPLANE - 1 - Channel
    if (isV): Channel += ROOT.NCHANNEL_PER_WIREPLANE 
    return Channel


base_offset = 1600
time_of_events = 1357754448

calib = ROOT.EXOCalibManager.GetCalibManager()
chanmapmgr = ROOT.EXOChannelMapManager.GetChannelMapMgr()
elec = calib.getCalib(ROOT.EXOElectronicsShapersHandler.GetHandlerName(), "measured_times", time_of_events)
#Standare noise for Wires
elec.SetNoiseAmplitudeForWires(800*ROOT.W_VALUE_LXE_EV_PER_ELECTRON)
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

sig_gen_dict = range(ROOT.NCHANNEL_PER_WIREPLANE)

sg =ROOT.SignalGeneration
for i in range(ROOT.NCHANNEL_PER_WIREPLANE):
    sig_gen_dict[i] = [sg(hash_file_dir + "/WP_U%i.hash.bin" % (i+1)),
                        sg(hash_file_dir + "/WP_V%i.hash.bin" % (i+1))]

for val in sig_gen_dict:
    for sg in val: 
        sg.SetTotalWaveformMinLength(standard_wf_length*oversample)
        sg.SetWaveformDelayTerm(standard_wf_length*oversample/2)


rand = ROOT.TRandom3(0)
for k in range(10):
    open_file = ROOT.TFile("temp_%i.root" % k, "recreate")
    tree = ROOT.TTree("tree", "tree")
    ed = ROOT.EXOEventData()
    tree.Branch("EventBranch", ed)
    
    u, v = ROOT.Double(), ROOT.Double()
    for i in range(10000):
        print i
        ed.fRunNumber = k
        ed.fEventNumber = i
        y = rand.Rndm()*200 - 100 
        x = rand.Rndm()*200 - 100 
    
        temp = dt.DriftAndReturnPath(ROOT.TVector3(x, y, z_position))
    
        last_pos = temp[temp.size()-1]
    
        ed.fEventHeader.fTriggerSeconds = time_of_events
        mc_data = ed.fMonteCarloData
        pcd = mc_data.FindOrCreatePixelatedChargeDeposit(
          ROOT.EXOCoordinates(ROOT.EXOMiscUtil.kXYCoordinates, x, y, z_position, 0))
        pcd.fTotalEnergy = energy
        pcd.fTotalIonizationEnergy = energy
    
        chan = 0
        ROOT.EXOMiscUtil.XYToUVCoords(u, v, last_pos.X(), last_pos.Y(), last_pos.Z()) 
    
        uchan = nearest_channel(last_pos.X(), False)
        vchan = nearest_channel(v, True)
    
        channels = []
        for center_ch, around, amin, amax in [
            (uchan, 1, 0, ROOT.NCHANNEL_PER_WIREPLANE), (vchan, 3, ROOT.NCHANNEL_PER_WIREPLANE, 2*ROOT.NCHANNEL_PER_WIREPLANE)]:
            for ch in range(center_ch-around, center_ch+around+1):
                if ch < amin or ch >= amax: continue
                channels.append(ch)
        for ch in channels:
            sig_mkr = None 
            if ch >= ROOT.NCHANNEL_PER_WIREPLANE:
                sig_mkr = sig_gen_dict[ch - ROOT.NCHANNEL_PER_WIREPLANE][1] 
            else:
                sig_mkr = sig_gen_dict[ch][0] 
            awf = sig_mkr.GetSignalWithDriftPath(dt) 
            trans = elec.GetTransferFunctionForChannel(ch) 
            trans.Transform(awf)
            arr = numpy.array(numpy.array(awf)[0:-1:oversample])
            dw_wf = ROOT.EXODoubleWaveform(arr, len(arr))
            dw_wf.SetSamplingPeriod(standard_period)
            dw_wf *= energy
            elec.HasNoiseTransformForChannel(ch) 
            elec.GetNoiseTransformForChannel(ch).Transform(dw_wf)
            dw_wf *= scaling
            dw_wf += base_offset 
            exo_wf = ed.GetWaveformData().GetNewWaveform()
            exo_wf.ConvertFrom(dw_wf)
            dw_wf.IsA().Destructor( dw_wf )
            awf.IsA().Destructor( awf )
            exo_wf.fChannel = ch
    
        for chan in range(226):
            if chan in channels: continue
            dw_wf = ROOT.EXODoubleWaveform()
            dw_wf.SetLength(standard_wf_length)
            elec.HasNoiseTransformForChannel(ch) 
            elec.GetNoiseTransformForChannel(ch).Transform(dw_wf)
            dw_wf *= scaling
            dw_wf +=  base_offset
            exo_wf = ed.GetWaveformData().GetNewWaveform()
            exo_wf.ConvertFrom(dw_wf)
            exo_wf.fChannel = chan
            dw_wf.IsA().Destructor( dw_wf )
    
        ed.GetWaveformData().fNumSamples = standard_wf_length
        ed.GetWaveformData().Compress()
        tree.Fill()
        ed.Clear()
    
    tree.Write()
    open_file.Close() 
    open_file.IsA().Destructor( open_file )
    
