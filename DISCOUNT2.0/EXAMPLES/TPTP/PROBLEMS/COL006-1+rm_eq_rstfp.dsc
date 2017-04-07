MODE PROOF, PARAMOD(2)

NAME COL006_1_rm_eq_rstfp_in

ORDERING XKBO
           apply:1 > k:1 > s:1 > f:1

EQUATIONS
           apply(apply(apply(s,X),Y),Z) = apply(apply(X,Z),apply(Y,Z))
           apply(apply(k,X),Y) = X

CONCLUSION apply(Y,f(Y)) = apply(f(Y),apply(Y,f(Y)))
