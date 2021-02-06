>rand_prog 85~
emote flicks you the bird.
say Get the hell out of my town.
say Got anyone you need erased?
emote spits.
break
~
>look_prog 100~
if isnpc($n)
      slap $n
         say Who  the hell are you looking at me?
else
    if level ($n) <= 18
           tell $n I warn you.. Do NOT look at me.. 
else
if level($n) > 18
     say Big mistake..
     say Who gave you permission to look at me?
     say Now, you die..
     kill $n
     break
endif
endif
~
|
