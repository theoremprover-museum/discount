MODE PROOF

NAME COL003_19_rm_eq_rstfp_in

ORDERING XKBO
           apply:1 > w:1 > strong_fixed_point:1 > b:1 > fixed_pt:1

EQUATIONS
           apply(apply(apply(b,X),Y),Z) = apply(X,apply(Y,Z))
           apply(apply(w,X),Y) = apply(apply(X,Y),Y)
           strong_fixed_point = apply(apply(b,apply(apply(b,apply(w,w)),apply(apply(b,apply(b,w)),b))),b)

CONCLUSION apply(strong_fixed_point,fixed_pt) = apply(fixed_pt,apply(strong_fixed_point,fixed_pt))
