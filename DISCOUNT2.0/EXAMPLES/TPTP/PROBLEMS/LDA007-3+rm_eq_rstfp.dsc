MODE PROOF

NAME LDA007_3_rm_eq_rstfp_in

ORDERING XKBO
           f:1 > tk:1 > k:1 > tsk:1 > tt_ts:1 > ts:1 > s:1 > tt:1 > t:1

EQUATIONS
           f(X,f(Y,Z)) = f(f(X,Y),f(X,Z))
           tt = f(t,t)
           ts = f(t,s)
           tt_ts = f(tt,ts)
           tk = f(t,k)
           tsk = f(ts,k)

CONCLUSION f(t,tsk) = f(tt_ts,tk)
