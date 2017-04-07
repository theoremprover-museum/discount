MODE PROOF, PARAMOD(2)

NAME COL001_2_rm_eq_rstfp_in

ORDERING XKBO
           apply:1 > i:1 > x:1 > k:1 > b:1 > s:1 > combinator:1

EQUATIONS
           apply(apply(apply(s,X),Y),Z) = apply(apply(X,Z),apply(Y,Z))
           apply(apply(k,X),Y) = X
           apply(apply(apply(b,X),Y),Z) = apply(X,apply(Y,Z))
           apply(i,X) = X
           apply(apply(apply(s,apply(b,X)),i),apply(apply(s,apply(b,X)),i)) = apply(x,apply(apply(apply(s,apply(b,X)),i),apply(apply(s,apply(b,X)),i)))

CONCLUSION Y = apply(combinator,Y)
