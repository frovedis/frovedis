SOURCES := $(shell find . -name '*.md')

all: man pdf

man:
	@for t in $(SOURCES); do \
	  name=`basename $$t .md`; \
	  pandoc -s -t man $$t -o $$name.3; \
	done

pdf:
	@for t in $(SOURCES); do \
	  name=`basename $$t .md`; \
	  pandoc $$t -o $$name.pdf -V geometry:margin=1in; \
	done

clean:
	rm -f *.3 *.pdf *.~
