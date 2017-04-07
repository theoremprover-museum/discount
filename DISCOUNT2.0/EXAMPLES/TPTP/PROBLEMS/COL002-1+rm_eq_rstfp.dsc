MODE PROOF, PARAMOD(2)

NAME COL002_1_rm_eq_rstfp_in

ORDERING XKBO
           apply:1 > i:1 > b:1 > c:1 > s:1 > fixed_pt:1

EQUATIONS
           apply(apply(apply(s,X),Y),Z) = apply(apply(X,Z),apply(Y,Z))
           apply(apply(apply(b,X),Y),Z) = apply(X,apply(Y,Z))
           apply(apply(apply(c,X),Y),Z) = apply(apply(X,Z),Y)
           apply(i,X) = X

CONCLUSION Y = apply(fixed_pt,Y)
