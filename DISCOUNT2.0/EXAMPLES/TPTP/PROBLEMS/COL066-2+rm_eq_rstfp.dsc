MODE PROOF

NAME COL066_2_rm_eq_rstfp_in

ORDERING XKBO
           apply:1 > w:1 > b:1 > q:1 > x:1 > y:1 > z:1

EQUATIONS
           apply(apply(apply(b,X),Y),Z) = apply(X,apply(Y,Z))
           apply(apply(apply(q,X),Y),Z) = apply(Y,apply(X,Z))
           apply(apply(w,X),Y) = apply(apply(X,Y),Y)

CONCLUSION apply(apply(apply(apply(apply(apply(q,q),apply(w,apply(q,apply(q,q)))),x),y),y),z) = apply(apply(x,y),apply(apply(x,y),z))
