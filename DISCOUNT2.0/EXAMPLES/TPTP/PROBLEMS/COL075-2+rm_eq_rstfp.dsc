MODE PROOF, PARAMOD(2)

NAME COL075_2_rm_eq_rstfp_in

ORDERING XKBO
           apply:1 > k:1 > abstraction:1 > b:1 > c:1

EQUATIONS
           apply(apply(k,X),Y) = X
           apply(apply(apply(abstraction,X),Y),Z) = apply(apply(X,apply(k,Z)),apply(Y,Z))

CONCLUSION apply(apply(Y,b(Y)),c(Y)) = apply(b(Y),b(Y))
