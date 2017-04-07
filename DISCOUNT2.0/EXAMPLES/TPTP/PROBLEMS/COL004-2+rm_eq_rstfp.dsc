MODE PROOF

NAME COL004_2_rm_eq_rstfp_in

ORDERING XKBO
           apply:1 > k:1 > s:1 > x:1 > y:1

EQUATIONS
           apply(apply(apply(s,X),Y),Z) = apply(apply(X,Z),apply(Y,Z))
           apply(apply(k,X),Y) = X

CONCLUSION apply(apply(apply(apply(s,apply(apply(s,apply(k,s)),k)),apply(apply(s,apply(k,apply(s,apply(apply(s,apply(apply(s,k),k)),apply(apply(s,k),k))))),k)),x),y) = apply(y,apply(apply(x,x),y))
