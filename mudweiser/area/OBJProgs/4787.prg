>wear_prog 100~
if class($n) != 3
break
endif
if inroom($n) == 4766
break
endif
if isfight($n)
mpechoat $n You may not invoke this item while fighting.
break
endif
mpechoat $n You are transported to the warrior's guild.
mpechoaround $n $n invokes the symbol of $s guild and vanishes in a puff of smoke.
mptran $n 4766
mpgoto 4766
mpechoaround $n $n arrives in a beam of bright light.
~
|
