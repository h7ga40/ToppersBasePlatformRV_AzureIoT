
MAKE = make

.PHONY: all
all:
	$(MAKE) -j -C Debug all

kernel_cfg.h:
	$(MAKE) -j 1 -C Debug kernel_cfg.h

.PHONY: clean
clean:
	$(MAKE) -j -C Debug clean

.PHONY: realclean
realclean:
	$(MAKE) -j -C Debug realclean
