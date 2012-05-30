include ../Makefile.mk

dirs = gl vc cli

all: $(dirs)

$(dirs): force_check
	$(MAKE) -C $@ all

clean:
	@for dir in $(dirs); do \
		$(MAKE) -C $$dir clean; \
	done

