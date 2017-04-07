MODE PROOF

NAME COL064_7_rm_eq_rstfp_in

ORDERING XKBO
           apply:1 > b:1 > t:1 > x:1 > y:1 > z:1

EQUATIONS
           apply(apply(apply(b,X),Y),Z) = apply(X,apply(Y,Z))
           apply(apply(t,X),Y) = apply(Y,X)

CONCLUSION apply(apply(apply(apply(apply(b,apply(t,apply(apply(b,b),t))),apply(apply(b,b),apply(apply(b,t),t))),x),y),z) = apply(apply(z,x),y)
