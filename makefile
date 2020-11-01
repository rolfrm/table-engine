all:
	make -f makefile.engine
	make -f makefile.graphics
	make -f makefile.test
clean:
	make -f makefile.engine clean
	make -f makefile.graphics clean
	make -f makefile.test clean

