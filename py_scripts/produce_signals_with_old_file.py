import ROOT
import numpy
import sys
ROOT.gROOT.SetBatch()
ROOT.gSystem.Load("libEXOROOT")
ROOT.gSystem.Load("../Reader/libReader.so")

standard_wf_length = 2048
standard_period = 1*ROOT.CLHEP.microsecond
oversample = 20
scaling = ROOT.ADC_BITS/(ROOT.ADC_FULL_SCALE_ELECTRONS_WIRE*ROOT.W_VALUE_LXE_EV_PER_ELECTRON) 
z_position = 50
base_offset = 1600
time_of_events = 1357754448

hash_file_dir = "/exo/scratch1/3DFieldFiles/HashFiles"

noise_amplitude = 0


class ProduceSignals:
    sig_gen_dict = range(ROOT.NCHANNEL_PER_WIREPLANE)
    
    sg =ROOT.SignalGeneration
    for i in range(ROOT.NCHANNEL_PER_WIREPLANE):
        sig_gen_dict[i] = [sg(hash_file_dir + "/WP_U%i.hash.bin" % (i+1)),
                            sg(hash_file_dir + "/WP_V%i.hash.bin" % (i+1))]
    
    for val in sig_gen_dict:
        for sg in val: 
            sg.SetTotalWaveformMinLength(standard_wf_length*oversample)
            sg.SetWaveformDelayTerm(standard_wf_length*oversample/2)

    calib = ROOT.EXOCalibManager.GetCalibManager()
    chanmapmgr = ROOT.EXOChannelMapManager.GetChannelMapMgr()
    elec = calib.getCalib(ROOT.EXOElectronicsShapersHandler.GetHandlerName(), "measured_times", time_of_events)
    #Standare noise for Wires
    elec.SetNoiseAmplitudeForWires(noise_amplitude*ROOT.W_VALUE_LXE_EV_PER_ELECTRON)
    
    dt = ROOT.DriftTrajectory(hash_file_dir + "/EFieldxyz.hash.bin")
    dt.SetSamplingPeriod(standard_period/oversample)
    dt.SetMaxDriftPoints(standard_wf_length*oversample/2)
    gain_dict = {}

    def nearest_channel(self, pos, isV):
        # Which channel is closest?
        Channel = int(pos/ROOT.CHANNEL_WIDTH) + ROOT.NCHANNEL_PER_WIREPLANE/2;
        if(pos < 0): Channel-=1 #// round down, not towards zero.
        if(Channel < 0): Channel = 0
        if(Channel >= ROOT.NCHANNEL_PER_WIREPLANE): Channel = ROOT.NCHANNEL_PER_WIREPLANE-1;
        Channel = ROOT.NCHANNEL_PER_WIREPLANE - 1 - Channel
        if (isV): Channel += ROOT.NCHANNEL_PER_WIREPLANE 
        return Channel
    
    def produce_signals(self, ed, wf_data):
        ch_dict = {}
    
        dt = self.dt
        elec = self.elec
        sig_gen_dict = self.sig_gen_dict
        gain_dict = self.gain_dict

        u, v = ROOT.Double(), ROOT.Double()
        for pcd in ed.fMonteCarloData.GetPixelatedChargeDepositsArray():
            cent = pcd.GetPixelCenter()
            z = cent.GetZ()
            if z < 0: z = -z
            temp = dt.DriftAndReturnPath(ROOT.TVector3(cent.GetX(), cent.GetY(), z))
        
            if temp.size() == 0: 
                print "Not drifted."
                continue
            last_pos = temp[temp.size()-1]
        
            ROOT.EXOMiscUtil.XYToUVCoords(u, v, last_pos.X(), last_pos.Y(), last_pos.Z()) 
        
            uchan = self.nearest_channel(last_pos.X(), False)
            vchan = self.nearest_channel(v, True)
        
            energy = pcd.fTotalIonizationEnergy
            channels = []
            for center_ch, around, amin, amax in [
                (uchan, 1, 0, ROOT.NCHANNEL_PER_WIREPLANE), 
                (vchan, 3, ROOT.NCHANNEL_PER_WIREPLANE, 2*ROOT.NCHANNEL_PER_WIREPLANE)]:
                for ch in range(center_ch-around, center_ch+around+1):
                    if ch < amin or ch >= amax: continue
                    channels.append(ch)
            for ch in channels:
                sig_mkr = None 
                if ch >= ROOT.NCHANNEL_PER_WIREPLANE:
                    sig_mkr = sig_gen_dict[ch - ROOT.NCHANNEL_PER_WIREPLANE][1] 
                else:
                    sig_mkr = sig_gen_dict[ch][0] 
                if ch not in ch_dict:  
                    ch_dict[ch] = ROOT.EXODoubleWaveform(sig_mkr.GetSignalWithDriftPath(dt)) 
                    ch_dict[ch] *= energy
                else:
                    dw_temp = ROOT.EXODoubleWaveform(sig_mkr.GetSignalWithDriftPath(dt)) 
                    dw_temp *= energy
                    awf = ch_dict[ch] 
                    awf += dw_temp
                    dw_temp.IsA().Destructor( dw_temp )
                
                
        # Deal with the wires which where affected
        for ch, awf in ch_dict.items():
            trans = elec.GetTransferFunctionForChannel(ch) 
            trans.Transform(awf)
            if ch not in gain_dict:
                gain_dict[ch] = trans.GetGain()
            awf /= gain_dict[ch]
            arr = numpy.array(numpy.array(awf)[0:-1:oversample])
            dw_wf = ROOT.EXODoubleWaveform(arr, len(arr))
            dw_wf.SetSamplingPeriod(standard_period)
            if noise_amplitude != 0.0:
                elec.HasNoiseTransformForChannel(ch) 
                elec.GetNoiseTransformForChannel(ch).Transform(dw_wf)
            dw_wf *= scaling
            exo_wf = wf_data.GetNewWaveform()
            exo_wf.ConvertFrom(dw_wf)
            exo_wf += base_offset 
            dw_wf.IsA().Destructor( dw_wf )
            awf.IsA().Destructor( awf )
            exo_wf.fChannel = ch
        
        # Deal with the rest
        for chan in range(226):
            if chan in ch_dict: continue
            dw_wf = ROOT.EXODoubleWaveform()
            dw_wf.SetLength(standard_wf_length)
            if noise_amplitude != 0.0:
                elec.HasNoiseTransformForChannel(chan) 
                elec.GetNoiseTransformForChannel(chan).Transform(dw_wf)
                dw_wf *= scaling
            dw_wf +=  base_offset
            exo_wf = wf_data.GetNewWaveform()
            exo_wf.ConvertFrom(dw_wf)
            exo_wf.fChannel = chan
            dw_wf.IsA().Destructor( dw_wf )
        
        wf_data.fNumSamples = standard_wf_length


input_file = ROOT.TFile(sys.argv[1])
input_tree = input_file.Get("tree")

num_to_digi = int(sys.argv[2])
begin_event = int(sys.argv[3])

open_file = ROOT.TFile(sys.argv[4], "recreate")
tree = ROOT.TTree("tree", "tree")
ed = ROOT.EXOEventData()
tree.Branch("EventBranch", ed)
    

processWFs = ProduceSignals()

digi = 0
for digi in range(num_to_digi):

    input_tree.GetEntry(digi+begin_event)
    if num_to_digi == digi: break 
    ed.Clear()
    ed_two = input_tree.EventBranch
    print ed_two.fEventNumber
    ed_two.ResetForReconstruction()
    #ed.fMonteCarloData.__assign__(ed_two.fMonteCarloData)
    ed.fEventHeader.fTriggerSeconds = time_of_events
    
    processWFs.produce_signals(ed_two, ed.GetWaveformData()) 

    ed.GetWaveformData().Compress()
    tree.Fill()
    

print "Writing"
tree.Write()
open_file.Close() 
open_file.IsA().Destructor( open_file )
    
