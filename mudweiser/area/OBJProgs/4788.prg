>wear_prog 100~
if class($n) == 0
or class($n) == 7
if inroom($n) == 4155
break
endif
if isfight($n)
mpechoat $n You may not invoke this item while fighting.
break
endif
mpechoat $n You are transported to the mage's guild.
mpechoaround $n $n invokes the symbol of $s guild and vanishes in a puff of smoke.
mptran $n 4155
mpgoto 4155
mpechoaround $n $n arrives in a beam of bright light.
break
endif
~
|
