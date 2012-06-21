include ../Makefile.mk

dirs = gl vc cli

all: $(dirs)

$(dirs): FORCE
	$(MAKE) -C $@ all

clean:
	@for dir in $(dirs); do \
		$(MAKE) -C $$dir clean; \
	done

FORCE:

.PHONY: FORCE all clean
