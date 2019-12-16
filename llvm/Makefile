LLVM_VERSION=7.0.0

all: install

$(LLVM_VERSION): patch.tar
	./scripts/compile.sh $(LLVM_VERSION) $^

patch.tar: patch
	cd $^ ; tar cf $@ * ; mv $@ ../ ;

install: $(LLVM_VERSION)
	ln -s $(LLVM_VERSION)/release $@

clean:
	./scripts/clean.sh $(LLVM_VERSION)
	rm -f patch.tar 

uninstall:
	rm -rf $(LLVM_VERSION) install

.PHONY: clean install uninstall
