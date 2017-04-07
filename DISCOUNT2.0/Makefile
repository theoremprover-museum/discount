#===============================================================================#
# Globales Makefile f"ur das DISCOUNT-System. Enthaelt
# Installationsprozeduren und triggert normale Makefiles
#
# Editiert werden sollen NUR Dateien in ".". Wenn neue C- oder
# Header-Files angelegt werden, so erzeugt ein neues "make install"
# die notwendigen Links. 
#
# Zum Umschalten zwischen BSD-isch (SunOs 4.*.*) und SVR4 in 
# Makefile.common die entsprechenden Aenderungen vornehmen!
#
# Autor: Stephan Schulz
#
#===============================================================================

# If your make does not support the following hacks, just set the
# values by hand. 

SHELL = /bin/sh

# Linux solution with GNU make:
ACTPATH = $(shell pwd)
DISCOUNT_DIR  = $(shell "basename `pwd`")

# Solaris/SunOs with SUN make
# ACTPATH:sh = pwd
# DISCOUNT_DIR:sh = basename `pwd`

# Read central variable definitions and stuff 

include Makefile.common

# Erzeugt werden die 4 Versionen von DISCOUNT

all: test_install discount rdiscount pcl rpcl

# Sicherstellen, dass bei Aufruf von "make" alles installiert ist

test_install:
	@if(test -f INSTALL_DATE) then \
        (echo "Installed: " `cat INSTALL_DATE`)\
        else (echo "Installing..."; make install) fi

# Erzeuge alle Object-Directories, lege initiale Links

install:
	@echo "Anlegen von V_DISCOUNT"
	@if (test -d V_DISCOUNT) then (rm -r V_DISCOUNT) fi
	@mkdir V_DISCOUNT
	@if(test -h discount) then (rm discount) fi
	@if(test -h rdiscount) then (rm rdiscount) fi	
	@if(test -f discount) then (rm discount) fi
	@if(test -f rdiscount) then (rm rdiscount) fi	
	@ln -s $(ACTPATH)/V_DISCOUNT/discount ./discount
	@ln -s $(ACTPATH)/V_DISCOUNT/discount ./rdiscount
	#
	@echo "Anlegen von V_PCL"
	@if (test -d V_PCL) then (rm -r V_PCL) fi
	@mkdir V_PCL
	@if(test -h pcl) then (rm pcl) fi
	@if(test -h rpcl) then (rm rpcl) fi
	@if(test -f pcl) then (rm pcl) fi
	@if(test -f rpcl) then (rm rpcl) fi
	@ln -s $(ACTPATH)/V_PCL/pcl ./pcl
	@ln -s $(ACTPATH)/V_PCL/pcl ./rpcl
	#
	@make source_files
	#
	@echo Pfade im global.cfg anpassen
	@./path_install
	@make depend
	@make tags
	#
	@date > INSTALL_DATE

# Installiere alle notwendigen Sourcefiles in den Subdirectories, ohne
# dabei eventuelle Object-files zu beruehren

source_files:
	@echo Create links to all real source files
	@echo "Removing emacs backup files"
	@touch avoid_rm_error~
	@rm *~
	@echo "Removing old source file links"
	@touch V_DISCOUNT/avoid_rm_error.c V_PCL/avoid_rm_error.c 
	@touch V_DISCOUNT/Makefile.avoid_rm_error V_PCL/Makefile.avoid_rm_error
	@rm V_DISCOUNT/*.[ch] V_DISCOUNT/Makefile* V_PCL/*.[ch] V_PCL/Makefile*
	@echo "Create new links to source files"
	@ln -s $(ACTPATH)/*.c $(ACTPATH)/*.h $(ACTPATH)/Makefile.* V_DISCOUNT
	@ln -s $(ACTPATH)/Makefile.Subdirs V_DISCOUNT/Makefile
	@ln -s $(ACTPATH)/*.c $(ACTPATH)/*.h $(ACTPATH)/Makefile.* V_PCL
	@ln -s $(ACTPATH)/Makefile.Subdirs V_PCL/Makefile

# Loesche Objekt-Direktories und eventuelle Links auf die Executables

deinstall:
	@echo "Loesche Verzeichnis V_DISCOUNT"
	@if(test -h discount) then (rm discount) fi
	@if (test -d V_DISCOUNT) then (rm -r V_DISCOUNT) fi
	#
	@echo "Loesche Verzeichnis V_PCL"
	@if(test -h pcl) then (rm pcl) fi
	@if (test -d V_PCL) then (rm -r V_PCL) fi
	#
	@echo "Loesche Datei INSTALL_DATE"
	@if(test -f INSTALL_DATE) then (rm INSTALL_DATE) fi

# Automatisches Bauen einer Distribution - benoetigt Utility basename! 

distrib: deinstall
	@echo "Baue komprimiertes tar-file"
	@cd ..; tar -cf DISCOUNT2.0.tar $(DISCOUNT_DIR); gzip DISCOUNT2.0.tar
	@echo "Distribution bereit"


# Baue eine Distribution, ohne das System zu deinstallieren...TEUER!

tmpdistrib:
	@echo "Kopie anlegen"
	@if (test ! -d $(HOME)/tmp) then (mkdir $(HOME)/tmp) fi	
	@cd ..;cp -r DISCOUNT2.0 $(HOME)/tmp/DISCOUNT2.0
	@echo Distribution aus der Kopie bauen
	@cd $(HOME)/tmp/DISCOUNT2.0; make distrib
	@echo Aufraeumen
	@rm -r $(HOME)/tmp/DISCOUNT2.0
	@mv $(HOME)/tmp/DISCOUNT2.0.tar.gz ..

# Erzeuge alles und das garantiert korrekt:

clean:
	@make install
	@make

# Abhaengigkeiten global erzeugen

depend: *.c *.h
	@echo "makedepend fuer discount"
	@makedepend *.c $(DEFS)  -f Makefile.discount
	@echo "makedepend fuer pcl"
	@makedepend *.c $(DEFS) -f Makefile.pcl -DPCL

# TAGS-File bauen

tags: *.c *.h
	@echo Aktuelles TAGS-File erzeugen
	@etags *.[ch]


# Uebersetzen der einzelnen Versionen

discount: *.c *.h
	@echo "# ------    discount wird generiert     ------"
	@cd V_DISCOUNT; make -f Makefile.discount
	@if( [ ! -h discount ] ) then (ln -s\
         $(ACTPATH)/V_DISCOUNT/discount) fi	 

rdiscount: discount
	@echo "# ------ discount wird per link erzeugt ------"
	@if(! [ -h rdiscount ] ) then (ln -s\
         $(ACTPATH)/V_DISCOUNT/discount rdiscount) fi

pcl: *.c *.h
	@echo "# ------         pcl wird generiert     ------"
	@cd V_PCL; make -f Makefile.pcl
	@if( [ ! -h pcl ] ) then (ln -s\
         $(ACTPATH)/V_PCL/pcl) fi	 

rpcl: pcl
	@echo "# ------   rpcl wird per link erzeugt   ------"
	@if( [ !  -h rpcl ] ) then (ln -s\
         $(ACTPATH)/V_DISCOUNT/discount rpcl) fi

# DO NOT DELETE
