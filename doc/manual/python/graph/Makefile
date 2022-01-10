SOURCES := $(shell find . -name '*.md')

all: man

man:
	@for t in $(SOURCES); do \
	  name=`basename $$t .md`; \
	  pandoc -s -t man $$t -o $$name.3p; \
	done

clean:
	rm -f *.3p *.~
