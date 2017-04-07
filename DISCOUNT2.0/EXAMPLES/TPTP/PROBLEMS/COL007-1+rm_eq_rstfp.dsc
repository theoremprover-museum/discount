MODE PROOF, PARAMOD(2)

NAME COL007_1_rm_eq_rstfp_in

ORDERING XKBO
           apply:1 > l:1 > combinator:1

EQUATIONS
           apply(apply(l,X),Y) = apply(X,apply(Y,Y))

CONCLUSION Y = apply(combinator,Y)
