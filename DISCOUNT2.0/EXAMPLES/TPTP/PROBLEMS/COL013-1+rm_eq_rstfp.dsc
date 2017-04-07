MODE PROOF, PARAMOD(2)

NAME COL013_1_rm_eq_rstfp_in

ORDERING XKBO
           apply:1 > l:1 > s:1 > combinator:1

EQUATIONS
           apply(apply(apply(s,X),Y),Z) = apply(apply(X,Z),apply(Y,Z))
           apply(apply(l,X),Y) = apply(X,apply(Y,Y))

CONCLUSION Y = apply(combinator,Y)
