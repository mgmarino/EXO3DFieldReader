import sys
import ROOT

class Circle:
    def __init__(self, r):
        self.func = [0,1]
        self.func[0] = ROOT.TF1("1", "sqrt([0]**2 - x**2)")
        self.func[1] = ROOT.TF1("2", "-sqrt([0]**2 - x**2)")
        self.set_r(r)

        self.SetNpx(100000)

    def set_r(self, r):
        self.SetParameter(0, r)
        self.SetRange(-r, r)

    def __getattr__(self, name): 
        return lambda *x: map(lambda y: getattr(y, name)(*x), self.func)  


ROOT.gStyle.SetOptStat(0)
inner_circ = Circle(1)
outer_circ = Circle(2)


chain = ROOT.TChain("pos_tree")
for afile in sys.argv[1:]: chain.Add(afile)

begin_hist = ROOT.TH2D("begin", "Begin Position", 400, -200, 200, 400, -200, 200)
end_hist = ROOT.TH2D("end", "End Position", 400, -200, 200, 400, -200, 200)
for ahist in [begin_hist, end_hist]:
    ahist.SetXTitle("X (mm)")
    ahist.SetYTitle("Y (mm)")

for circ in [inner_circ, outer_circ]:
    circ.SetLineColor(ROOT.kRed)
    circ.SetLineWidth(2)
c1 = ROOT.TCanvas()

for i in range(100, 200, 10):
    inner_rad = i - 10
    inner_circ.set_r(i)
    outer_circ.set_r(inner_rad)

    title = "(R > %g (mm) && R < %g (mm))" % (inner_rad, i)
    
    chain.Draw("begin.fX:begin.fY >> %s" % begin_hist.GetName(), "end.fZ > 190 && begin.Perp() < %f && begin.Perp() > %f" % (float(i), inner_rad))
    begin_hist.SetTitle("Begin Position, " + title)
    inner_circ.Draw("same")
    outer_circ.Draw("same")
    c1.Update()
    raw_input("E")
    
    chain.Draw("end.fX:end.fY >> %s" % end_hist.GetName(), "end.fZ > 190 && begin.Perp() < %f && begin.Perp() > %f" % (float(i), inner_rad))
    end_hist.SetTitle("End Position, " + title)
    inner_circ.Draw("same")
    outer_circ.Draw("same")
    c1.Update()
    raw_input("E")
