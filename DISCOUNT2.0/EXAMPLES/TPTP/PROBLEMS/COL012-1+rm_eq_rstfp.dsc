MODE PROOF, PARAMOD(2)

NAME COL012_1_rm_eq_rstfp_in

ORDERING XKBO
           apply:1 > u:1 > combinator:1

EQUATIONS
           apply(apply(u,X),Y) = apply(Y,apply(apply(X,X),Y))

CONCLUSION Y = apply(combinator,Y)
