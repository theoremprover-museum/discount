MODE PROOF, PARAMOD(2)

NAME COL064_1_rm_eq_rstfp_in

ORDERING XKBO
           apply:1 > b:1 > t:1 > h:1 > f:1 > g:1

EQUATIONS
           apply(apply(apply(b,X),Y),Z) = apply(X,apply(Y,Z))
           apply(apply(t,X),Y) = apply(Y,X)

CONCLUSION apply(apply(apply(X,f(X)),g(X)),h(X)) = apply(apply(h(X),f(X)),g(X))
