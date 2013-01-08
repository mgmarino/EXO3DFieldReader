
.PHONY: recursive clean all doall 

DIRS := Reader Examples

all: check doall

doall: 
	@$(MAKE) --no-print-directory TARGET=all recursive

check::

clean:
	@$(MAKE) --no-print-directory TARGET=clean recursive

recursive: 
	@for dir in $(DIRS); do $(MAKE) --no-print-directory -C $$dir $(TARGET) || exit $$?; done


