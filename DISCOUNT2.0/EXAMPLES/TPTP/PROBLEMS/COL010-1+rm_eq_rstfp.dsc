MODE PROOF, PARAMOD(2)

NAME COL010_1_rm_eq_rstfp_in

ORDERING XKBO
           apply:1 > b:1 > s2:1 > combinator:1

EQUATIONS
           apply(apply(apply(b,X),Y),Z) = apply(X,apply(Y,Z))
           apply(apply(apply(s2,X),Y),Z) = apply(apply(X,Z),apply(Y,Y))

CONCLUSION Y = apply(combinator,Y)
