MODE PROOF, PARAMOD(2)

NAME COL009_1_rm_eq_rstfp_in

ORDERING XKBO
           apply:1 > l2:1 > b:1 > combinator:1

EQUATIONS
           apply(apply(apply(b,X),Y),Z) = apply(X,apply(Y,Z))
           apply(apply(l2,X),Y) = apply(Y,apply(X,X))

CONCLUSION Y = apply(combinator,Y)
