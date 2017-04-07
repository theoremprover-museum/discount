MODE PROOF

NAME COL044_8_rm_eq_rstfp_in

ORDERING XKBO
           apply:1 > strong_fixed_point:1 > n:1 > b:1 > fixed_pt:1

EQUATIONS
           apply(apply(apply(b,X),Y),Z) = apply(X,apply(Y,Z))
           apply(apply(apply(n,X),Y),Z) = apply(apply(apply(X,Z),Y),Z)
           strong_fixed_point = apply(apply(b,apply(apply(b,apply(apply(n,apply(n,apply(apply(b,apply(b,b)),apply(n,apply(apply(b,b),n))))),n)),b)),b)

CONCLUSION apply(strong_fixed_point,fixed_pt) = apply(fixed_pt,apply(strong_fixed_point,fixed_pt))
