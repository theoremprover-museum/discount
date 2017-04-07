MODE PROOF

NAME COL042_8_rm_eq_rstfp_in

ORDERING XKBO
           apply:1 > strong_fixed_point:1 > b:1 > w1:1 > fixed_pt:1

EQUATIONS
           apply(apply(apply(b,X),Y),Z) = apply(X,apply(Y,Z))
           apply(apply(w1,X),Y) = apply(apply(Y,X),X)
           strong_fixed_point = apply(apply(b,apply(apply(b,apply(w1,w1)),apply(apply(b,apply(b,w1)),b))),b)

CONCLUSION apply(strong_fixed_point,fixed_pt) = apply(fixed_pt,apply(strong_fixed_point,fixed_pt))
