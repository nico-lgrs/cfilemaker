CC = gcc
CFLAGS = -O2

cwd = $(shell pwd)
executable = cfilemaker
bindir = bin
srcdir = src

add_path: $(bindir)/$(executable)
	@echo Creating a symbolic link:
	sudo ln -s $(cwd)/$(bindir)/$(executable) /usr/local/bin/$(executable)

$(bindir)/$(executable): dirs $(srcdir)/cfilemaker.c
	$(CC) $(CFLAGS) $(srcdir)/cfilemaker.c -o $(bindir)/$(executable)

dirs:
	@mkdir -p $(bindir)

rm_path:
	@echo Removing the symbolic link:
	sudo rm /usr/local/bin/$(executable)

uninstall: rm_path
	@rm $(bindir)/$(executable)
	@rmdir $(bindir)
