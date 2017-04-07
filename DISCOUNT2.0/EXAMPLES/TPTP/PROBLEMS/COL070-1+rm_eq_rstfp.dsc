MODE PROOF, PARAMOD(2)

NAME COL070_1_rm_eq_rstfp_in

ORDERING XKBO
           apply:1 > n1:1 > b:1 > combinator:1

EQUATIONS
           apply(apply(apply(n1,X),Y),Z) = apply(apply(apply(X,Y),Y),Z)
           apply(apply(apply(b,X),Y),Z) = apply(X,apply(Y,Z))

CONCLUSION Y = apply(combinator,Y)
