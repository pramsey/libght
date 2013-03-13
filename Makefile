all:
	$(MAKE) -C src all
	$(MAKE) -C test all

check: all
	$(MAKE) -C test check

clean: 
	$(MAKE) -C src clean
	$(MAKE) -C test clean
