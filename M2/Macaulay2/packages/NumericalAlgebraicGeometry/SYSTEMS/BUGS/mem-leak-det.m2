{*
trackHomotopy GateHomotopySystem    
produces a small leak
*}

restart
needsPackage "SLPexpressions"
debug SLPexpressions
X = inputGate symbol X
C = inputGate symbol C
XpC = X+C
XXC = productGate{X,X,C}
detXCCX = detGate{{X,C},{C,X}}
XoC = X/C
s = makeSLProgram({C,X},{XXC,detXCCX,XoC,XpC+XoC}) -- does not leak for a finite field
-- s = makeSLProgram({C,X},{XXC,XoC,XpC+XoC}) -- fine

debug Core
K = CC
(consts,indets):=(positions({C},s), positions({X},s))
eQQ = rawSLEvaluator(s,consts,indets,raw matrix{{3_K}})

m7 = raw matrix{{7_K}};
for i to 100000000 do (
    rawSLEvaluatorEvaluate(eQQ, m7); 
    if i%10000 == 0 then print i;
    )   

end
restart
load "mem-leak-det.m2"
