mudpi:EmberMUD/src/ember tintin/src/tt++
	echo "Done"

EmberMUD/src/ember:
	make -C EmberMUD/src

tintin/src/tt++:
	(cd tintin/src && ./configure)
	make -C tintin/src
