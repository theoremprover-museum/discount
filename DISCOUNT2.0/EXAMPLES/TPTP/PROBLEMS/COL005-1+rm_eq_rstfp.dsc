MODE PROOF, PARAMOD(2)

NAME COL005_1_rm_eq_rstfp_in

ORDERING XKBO
           apply:1 > w:1 > s:1 > combinator:1

EQUATIONS
           apply(apply(apply(s,X),Y),Z) = apply(apply(X,Z),apply(Y,Z))
           apply(apply(w,X),Y) = apply(apply(X,Y),Y)

CONCLUSION Y = apply(combinator,Y)
