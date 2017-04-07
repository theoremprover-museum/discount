MODE PROOF

NAME COL006_7_rm_eq_rstfp_in

ORDERING XKBO
           apply:1 > k:1 > strong_fixed_point:1 > s:1 > fixed_pt:1

EQUATIONS
           apply(apply(apply(s,X),Y),Z) = apply(apply(X,Z),apply(Y,Z))
           apply(apply(k,X),Y) = X
           strong_fixed_point = apply(apply(s,apply(k,apply(apply(apply(s,s),apply(apply(s,k),k)),apply(apply(s,s),apply(s,k))))),apply(apply(s,apply(k,s)),k))

CONCLUSION apply(strong_fixed_point,fixed_pt) = apply(fixed_pt,apply(strong_fixed_point,fixed_pt))
