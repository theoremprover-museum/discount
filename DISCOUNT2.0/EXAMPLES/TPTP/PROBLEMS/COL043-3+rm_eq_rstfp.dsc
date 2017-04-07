MODE PROOF

NAME COL043_3_rm_eq_rstfp_in

ORDERING XKBO
           apply:1 > h:1 > strong_fixed_point:1 > b:1 > fixed_pt:1

EQUATIONS
           apply(apply(apply(b,X),Y),Z) = apply(X,apply(Y,Z))
           apply(apply(apply(h,X),Y),Z) = apply(apply(apply(X,Y),Z),Y)
           strong_fixed_point = apply(apply(b,apply(apply(b,apply(apply(apply(h,apply(apply(b,apply(b,h)),apply(b,b))),apply(apply(h,apply(b,h)),apply(b,b))),h)),b)),b)

CONCLUSION apply(strong_fixed_point,fixed_pt) = apply(fixed_pt,apply(strong_fixed_point,fixed_pt))
