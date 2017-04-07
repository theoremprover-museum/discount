MODE PROOF

NAME COL059_1_rm_eq_rstfp_in

ORDERING XKBO
           response:1 > kestrel:1 > x2:1 > lark:1 > l2:1 > l3:1

EQUATIONS
           response(response(kestrel,X1),X2) = X1
           response(response(lark,X1),X2) = response(X1,response(X2,X2))
           response(response(response(lark,lark),X1),X2) = response(response(X1,X1),response(X2,X2))
           response(response(response(response(lark,lark),lark),X1),X2) = response(response(response(X1,X1),response(X1,X1)),response(x2,x2))
           response(lark,lark) = l2
           response(l2,lark) = l3
           response(l3,l3) = l3

CONCLUSION response(l2,l2) = l2
