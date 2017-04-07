MODE PROOF

NAME COL002_4_rm_eq_rstfp_in

ORDERING XKBO
           apply:1 > i:1 > weak_fixed_point:1 > b:1 > c:1 > s:1 > fixed_pt:1

EQUATIONS
           apply(apply(apply(s,X),Y),Z) = apply(apply(X,Z),apply(Y,Z))
           apply(apply(apply(b,X),Y),Z) = apply(X,apply(Y,Z))
           apply(apply(apply(c,X),Y),Z) = apply(apply(X,Z),Y)
           apply(i,X) = X
           weak_fixed_point = apply(apply(apply(s,apply(b,X)),i),apply(apply(s,apply(b,X)),i))

CONCLUSION weak_fixed_point = apply(fixed_pt,weak_fixed_point)
