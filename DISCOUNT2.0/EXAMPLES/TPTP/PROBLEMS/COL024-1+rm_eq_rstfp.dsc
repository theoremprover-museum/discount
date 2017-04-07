MODE PROOF, PARAMOD(2)

NAME COL024_1_rm_eq_rstfp_in

ORDERING XKBO
           apply:1 > m:1 > b:1 > c:1 > combinator:1

EQUATIONS
           apply(apply(apply(b,X),Y),Z) = apply(X,apply(Y,Z))
           apply(m,X) = apply(X,X)
           apply(apply(apply(c,X),Y),Z) = apply(apply(X,Z),Y)

CONCLUSION Y = apply(combinator,Y)
