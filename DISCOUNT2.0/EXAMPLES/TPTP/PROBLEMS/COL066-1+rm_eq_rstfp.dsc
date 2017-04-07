MODE PROOF, PARAMOD(2)

NAME COL066_1_rm_eq_rstfp_in

ORDERING XKBO
           apply:1 > w:1 > b:1 > q:1 > h:1 > f:1 > g:1

EQUATIONS
           apply(apply(apply(b,X),Y),Z) = apply(X,apply(Y,Z))
           apply(apply(apply(q,X),Y),Z) = apply(Y,apply(X,Z))
           apply(apply(w,X),Y) = apply(apply(X,Y),Y)

CONCLUSION apply(apply(apply(apply(X,f(X)),g(X)),g(X)),h(X)) = apply(apply(f(X),g(X)),apply(apply(f(X),g(X)),h(X)))
