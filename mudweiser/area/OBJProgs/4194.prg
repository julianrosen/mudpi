>wear_prog 100~
mpecho The accursed pendant seems to radiate darkness and evil.
cast 'curse' $n
mpechoaround $n $n seems to be cursed.
mpechoat $n You have been cursed by the pendant.
break
~
>speech_prog p pendant to blade~
mpecho An accursed pendant shudders violently and explodes.
mpoload 4198 21
drop accbl
mpforce $n sac accpnd
mpforce $n get accbl
break
~
>remove_prog 98~
mpecho The accursed pendant's curse loses its effectiveness.
cast 'remove curse' $n
cast 'cause serious' $n
break
~
|
