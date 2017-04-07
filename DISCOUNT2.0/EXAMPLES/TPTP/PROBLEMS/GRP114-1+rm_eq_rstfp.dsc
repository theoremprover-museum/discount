MODE PROOF

NAME GRP114_1_rm_eq_rstfp_in

ORDERING XKBO
           union:1 > intersection:1 > multiply:1 > positive_part:1 > identity:1 > inverse:1 > negative_part:1 > a:1

EQUATIONS
           multiply(identity,X) = X
           multiply(inverse(X),X) = identity
           multiply(multiply(X,Y),Z) = multiply(X,multiply(Y,Z))
           inverse(identity) = identity
           inverse(inverse(X)) = X
           inverse(multiply(X,Y)) = multiply(inverse(Y),inverse(X))
           intersection(X,X) = X
           union(X,X) = X
           intersection(X,Y) = intersection(Y,X)
           union(X,Y) = union(Y,X)
           intersection(X,intersection(Y,Z)) = intersection(intersection(X,Y),Z)
           union(X,union(Y,Z)) = union(union(X,Y),Z)
           union(intersection(X,Y),Y) = Y
           intersection(union(X,Y),Y) = Y
           multiply(X,union(Y,Z)) = union(multiply(X,Y),multiply(X,Z))
           multiply(X,intersection(Y,Z)) = intersection(multiply(X,Y),multiply(X,Z))
           multiply(union(Y,Z),X) = union(multiply(Y,X),multiply(Z,X))
           multiply(intersection(Y,Z),X) = intersection(multiply(Y,X),multiply(Z,X))
           positive_part(X) = union(X,identity)
           negative_part(X) = intersection(X,identity)

CONCLUSION multiply(positive_part(a),negative_part(a)) = a
