#---------------------------------------------------------------------------
#
# File:        makefile
#
# Autor:       Stephan Schulz
#
# Inhalt:      makefile fuer alle pcl-Programme
#
# Aenderungen: <1> 12.2.1991 neu
#              <2>  2.4.1991 Einfuegen pcl_analyse
#              <3>  ??  1992 Einfuegen vieles :-)
#              <4>  2.3.1994 Einfuegen pcl_mininorm
#              <5> 20.4.1994 Einfuegen pcl_prologout, "tags"
#              <6> 25.4.1994 Einfuegen pcl2tree
#
#---------------------------------------------------------------------------



CC = gcc 
CFLAGS = -Wall -c -O2 -funroll-loops
LD = gcc


all : revert trans extract mextract rextract lemma proof pcl2tree 

depend : *.c *.h
	makedepend *.c

tags: *.c *.h
	etags *.c *.h

BASICS = pcl_strings.o pcl_mem.o pcl_scan.o pcl_types.o pcl_printer.o

NORMAL = pcl_parse.o pcl_terms.o

MINI = pcl_miniparse.o pcl_miniterms.o pcl_mininorm.o

COMPLEX = pcl_doio.o pcl_analyse.o pcl_buildtree.o pcl_lemma.o 

# ************ extract ***********

EXTRACT = extract.o pcl_doio.o pcl_analyse.o $(BASICS) $(NORMAL)

extract: $(EXTRACT) 
	$(LD) -o extract $(EXTRACT) 



# ************ lemma ***********


LEMMA = lemma.o $(COMPLEX) $(BASICS) $(NORMAL)

lemma: $(LEMMA)
	$(LD) -o lemma $(LEMMA) 

# ************ pcl2tree ***********


PCL2TREE = pcl2tree.o $(COMPLEX) $(BASICS) $(NORMAL)

pcl2tree: $(PCL2TREE)
	$(LD) -o pcl2tree $(PCL2TREE) 

# ************ mextract ***********

MEXTRACT = mextract.o pcl_minidoio.o pcl_minianalyse.o $(BASICS) $(MINI) 

mextract: $(MEXTRACT) 
	$(LD) -o mextract $(MEXTRACT) 

# ************ revert *************

revert: revert.o
	$(LD) -o revert revert.o

# ************ rextract ***********

REXTRACT = rextract.o pcl_minianalyse.o $(BASICS) $(MINI)

rextract: $(REXTRACT) 
	$(LD) -o rextract $(REXTRACT)



# ************ trans ***********

TRANS = trans.o pcl_mem.o parse.o nameadmin.o scan.o pcl_strings.o

trans : $(TRANS) 
	$(LD) -o trans $(TRANS) 

#************* proof ************

PROOF = proof.o pcl_subst.o pcl_eqchains.o pcl_fproof.o \
pcl_printproof.o pcl_latexout.o pcl_prologout.o $(COMPLEX) $(BASICS) $(NORMAL)  

proof : $(PROOF)
	$(LD) -o proof $(PROOF)


# DO NOT DELETE THIS LINE -- make depend depends on it.

extract.o: pcl_doio.h pcl_parse.h pcl_types.h pcl_scan.h pcl_strings.h
extract.o: pcl_mem.h pcl_defs.h /usr/include/malloc.h /usr/include/features.h
extract.o: /usr/include/sys/cdefs.h
extract.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
extract.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
extract.o: /usr/include/stdlib.h /usr/include/errno.h
extract.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
extract.o: /usr/include/alloca.h /usr/include/ctype.h /usr/include/endian.h
extract.o: /usr/include/bytesex.h /usr/include/string.h systeminclude.h
extract.o: pcl_printer.h pcl_terms.h pcl_analyse.h
lemma.o: pcl_doio.h pcl_parse.h pcl_types.h pcl_scan.h pcl_strings.h
lemma.o: pcl_mem.h pcl_defs.h /usr/include/malloc.h /usr/include/features.h
lemma.o: /usr/include/sys/cdefs.h
lemma.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
lemma.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
lemma.o: /usr/include/stdlib.h /usr/include/errno.h
lemma.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
lemma.o: /usr/include/alloca.h /usr/include/ctype.h /usr/include/endian.h
lemma.o: /usr/include/bytesex.h /usr/include/string.h systeminclude.h
lemma.o: pcl_printer.h pcl_terms.h pcl_lemma.h pcl_buildtree.h pcl_analyse.h
mextract.o: pcl_minidoio.h pcl_miniparse.h pcl_types.h pcl_scan.h
mextract.o: pcl_strings.h pcl_mem.h pcl_defs.h /usr/include/malloc.h
mextract.o: /usr/include/features.h /usr/include/sys/cdefs.h
mextract.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
mextract.o: /usr/include/stdio.h /usr/include/libio.h
mextract.o: /usr/include/_G_config.h /usr/include/stdlib.h
mextract.o: /usr/include/errno.h /usr/include/linux/errno.h
mextract.o: /usr/include/asm/errno.h /usr/include/alloca.h
mextract.o: /usr/include/ctype.h /usr/include/endian.h /usr/include/bytesex.h
mextract.o: /usr/include/string.h systeminclude.h pcl_printer.h
mextract.o: pcl_miniterms.h pcl_minianalyse.h pcl_mininorm.h
nameadmin.o: nameadmin.h scan.h pcl_strings.h pcl_mem.h pcl_defs.h
nameadmin.o: /usr/include/malloc.h /usr/include/features.h
nameadmin.o: /usr/include/sys/cdefs.h
nameadmin.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
nameadmin.o: /usr/include/stdio.h /usr/include/libio.h
nameadmin.o: /usr/include/_G_config.h /usr/include/stdlib.h
nameadmin.o: /usr/include/errno.h /usr/include/linux/errno.h
nameadmin.o: /usr/include/asm/errno.h /usr/include/alloca.h
nameadmin.o: /usr/include/ctype.h /usr/include/endian.h
nameadmin.o: /usr/include/bytesex.h /usr/include/string.h systeminclude.h
parse.o: parse.h nameadmin.h scan.h pcl_strings.h pcl_mem.h pcl_defs.h
parse.o: /usr/include/malloc.h /usr/include/features.h
parse.o: /usr/include/sys/cdefs.h
parse.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
parse.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
parse.o: /usr/include/stdlib.h /usr/include/errno.h
parse.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
parse.o: /usr/include/alloca.h /usr/include/ctype.h /usr/include/endian.h
parse.o: /usr/include/bytesex.h /usr/include/string.h systeminclude.h
pcl2tree.o: pcl_doio.h pcl_parse.h pcl_types.h pcl_scan.h pcl_strings.h
pcl2tree.o: pcl_mem.h pcl_defs.h /usr/include/malloc.h
pcl2tree.o: /usr/include/features.h /usr/include/sys/cdefs.h
pcl2tree.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
pcl2tree.o: /usr/include/stdio.h /usr/include/libio.h
pcl2tree.o: /usr/include/_G_config.h /usr/include/stdlib.h
pcl2tree.o: /usr/include/errno.h /usr/include/linux/errno.h
pcl2tree.o: /usr/include/asm/errno.h /usr/include/alloca.h
pcl2tree.o: /usr/include/ctype.h /usr/include/endian.h /usr/include/bytesex.h
pcl2tree.o: /usr/include/string.h systeminclude.h pcl_printer.h pcl_terms.h
pcl2tree.o: pcl_lemma.h pcl_buildtree.h pcl_analyse.h
pcl_analyse.o: pcl_analyse.h pcl_parse.h pcl_types.h pcl_scan.h pcl_strings.h
pcl_analyse.o: pcl_mem.h pcl_defs.h /usr/include/malloc.h
pcl_analyse.o: /usr/include/features.h /usr/include/sys/cdefs.h
pcl_analyse.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
pcl_analyse.o: /usr/include/stdio.h /usr/include/libio.h
pcl_analyse.o: /usr/include/_G_config.h /usr/include/stdlib.h
pcl_analyse.o: /usr/include/errno.h /usr/include/linux/errno.h
pcl_analyse.o: /usr/include/asm/errno.h /usr/include/alloca.h
pcl_analyse.o: /usr/include/ctype.h /usr/include/endian.h
pcl_analyse.o: /usr/include/bytesex.h /usr/include/string.h systeminclude.h
pcl_analyse.o: pcl_printer.h pcl_terms.h
pcl_buildtree.o: pcl_buildtree.h pcl_parse.h pcl_types.h pcl_scan.h
pcl_buildtree.o: pcl_strings.h pcl_mem.h pcl_defs.h /usr/include/malloc.h
pcl_buildtree.o: /usr/include/features.h /usr/include/sys/cdefs.h
pcl_buildtree.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
pcl_buildtree.o: /usr/include/stdio.h /usr/include/libio.h
pcl_buildtree.o: /usr/include/_G_config.h /usr/include/stdlib.h
pcl_buildtree.o: /usr/include/errno.h /usr/include/linux/errno.h
pcl_buildtree.o: /usr/include/asm/errno.h /usr/include/alloca.h
pcl_buildtree.o: /usr/include/ctype.h /usr/include/endian.h
pcl_buildtree.o: /usr/include/bytesex.h /usr/include/string.h systeminclude.h
pcl_buildtree.o: pcl_printer.h pcl_terms.h pcl_analyse.h
pcl_doio.o: pcl_doio.h pcl_parse.h pcl_types.h pcl_scan.h pcl_strings.h
pcl_doio.o: pcl_mem.h pcl_defs.h /usr/include/malloc.h
pcl_doio.o: /usr/include/features.h /usr/include/sys/cdefs.h
pcl_doio.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
pcl_doio.o: /usr/include/stdio.h /usr/include/libio.h
pcl_doio.o: /usr/include/_G_config.h /usr/include/stdlib.h
pcl_doio.o: /usr/include/errno.h /usr/include/linux/errno.h
pcl_doio.o: /usr/include/asm/errno.h /usr/include/alloca.h
pcl_doio.o: /usr/include/ctype.h /usr/include/endian.h /usr/include/bytesex.h
pcl_doio.o: /usr/include/string.h systeminclude.h pcl_printer.h pcl_terms.h
pcl_eqchains.o: pcl_eqchains.h pcl_subst.h pcl_parse.h pcl_types.h pcl_scan.h
pcl_eqchains.o: pcl_strings.h pcl_mem.h pcl_defs.h /usr/include/malloc.h
pcl_eqchains.o: /usr/include/features.h /usr/include/sys/cdefs.h
pcl_eqchains.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
pcl_eqchains.o: /usr/include/stdio.h /usr/include/libio.h
pcl_eqchains.o: /usr/include/_G_config.h /usr/include/stdlib.h
pcl_eqchains.o: /usr/include/errno.h /usr/include/linux/errno.h
pcl_eqchains.o: /usr/include/asm/errno.h /usr/include/alloca.h
pcl_eqchains.o: /usr/include/ctype.h /usr/include/endian.h
pcl_eqchains.o: /usr/include/bytesex.h /usr/include/string.h systeminclude.h
pcl_eqchains.o: pcl_printer.h pcl_terms.h
pcl_fproof.o: pcl_fproof.h pcl_eqchains.h pcl_subst.h pcl_parse.h pcl_types.h
pcl_fproof.o: pcl_scan.h pcl_strings.h pcl_mem.h pcl_defs.h
pcl_fproof.o: /usr/include/malloc.h /usr/include/features.h
pcl_fproof.o: /usr/include/sys/cdefs.h
pcl_fproof.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
pcl_fproof.o: /usr/include/stdio.h /usr/include/libio.h
pcl_fproof.o: /usr/include/_G_config.h /usr/include/stdlib.h
pcl_fproof.o: /usr/include/errno.h /usr/include/linux/errno.h
pcl_fproof.o: /usr/include/asm/errno.h /usr/include/alloca.h
pcl_fproof.o: /usr/include/ctype.h /usr/include/endian.h
pcl_fproof.o: /usr/include/bytesex.h /usr/include/string.h systeminclude.h
pcl_fproof.o: pcl_printer.h pcl_terms.h
pcl_latexout.o: pcl_latexout.h pcl_eqchains.h pcl_subst.h pcl_parse.h
pcl_latexout.o: pcl_types.h pcl_scan.h pcl_strings.h pcl_mem.h pcl_defs.h
pcl_latexout.o: /usr/include/malloc.h /usr/include/features.h
pcl_latexout.o: /usr/include/sys/cdefs.h
pcl_latexout.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
pcl_latexout.o: /usr/include/stdio.h /usr/include/libio.h
pcl_latexout.o: /usr/include/_G_config.h /usr/include/stdlib.h
pcl_latexout.o: /usr/include/errno.h /usr/include/linux/errno.h
pcl_latexout.o: /usr/include/asm/errno.h /usr/include/alloca.h
pcl_latexout.o: /usr/include/ctype.h /usr/include/endian.h
pcl_latexout.o: /usr/include/bytesex.h /usr/include/string.h systeminclude.h
pcl_latexout.o: pcl_printer.h pcl_terms.h
pcl_lemma.o: pcl_lemma.h pcl_buildtree.h pcl_parse.h pcl_types.h pcl_scan.h
pcl_lemma.o: pcl_strings.h pcl_mem.h pcl_defs.h /usr/include/malloc.h
pcl_lemma.o: /usr/include/features.h /usr/include/sys/cdefs.h
pcl_lemma.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
pcl_lemma.o: /usr/include/stdio.h /usr/include/libio.h
pcl_lemma.o: /usr/include/_G_config.h /usr/include/stdlib.h
pcl_lemma.o: /usr/include/errno.h /usr/include/linux/errno.h
pcl_lemma.o: /usr/include/asm/errno.h /usr/include/alloca.h
pcl_lemma.o: /usr/include/ctype.h /usr/include/endian.h
pcl_lemma.o: /usr/include/bytesex.h /usr/include/string.h systeminclude.h
pcl_lemma.o: pcl_printer.h pcl_terms.h pcl_analyse.h pcl_doio.h
pcl_mem.o: pcl_mem.h pcl_defs.h /usr/include/malloc.h /usr/include/features.h
pcl_mem.o: /usr/include/sys/cdefs.h
pcl_mem.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
pcl_mem.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
pcl_mem.o: /usr/include/stdlib.h /usr/include/errno.h
pcl_mem.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
pcl_mem.o: /usr/include/alloca.h /usr/include/ctype.h /usr/include/endian.h
pcl_mem.o: /usr/include/bytesex.h /usr/include/string.h systeminclude.h
pcl_minianalyse.o: pcl_minianalyse.h pcl_mininorm.h pcl_miniparse.h
pcl_minianalyse.o: pcl_types.h pcl_scan.h pcl_strings.h pcl_mem.h pcl_defs.h
pcl_minianalyse.o: /usr/include/malloc.h /usr/include/features.h
pcl_minianalyse.o: /usr/include/sys/cdefs.h
pcl_minianalyse.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
pcl_minianalyse.o: /usr/include/stdio.h /usr/include/libio.h
pcl_minianalyse.o: /usr/include/_G_config.h /usr/include/stdlib.h
pcl_minianalyse.o: /usr/include/errno.h /usr/include/linux/errno.h
pcl_minianalyse.o: /usr/include/asm/errno.h /usr/include/alloca.h
pcl_minianalyse.o: /usr/include/ctype.h /usr/include/endian.h
pcl_minianalyse.o: /usr/include/bytesex.h /usr/include/string.h
pcl_minianalyse.o: systeminclude.h pcl_printer.h pcl_miniterms.h
pcl_minianalyse.o: pcl_minidoio.h
pcl_minidoio.o: pcl_minidoio.h pcl_miniparse.h pcl_types.h pcl_scan.h
pcl_minidoio.o: pcl_strings.h pcl_mem.h pcl_defs.h /usr/include/malloc.h
pcl_minidoio.o: /usr/include/features.h /usr/include/sys/cdefs.h
pcl_minidoio.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
pcl_minidoio.o: /usr/include/stdio.h /usr/include/libio.h
pcl_minidoio.o: /usr/include/_G_config.h /usr/include/stdlib.h
pcl_minidoio.o: /usr/include/errno.h /usr/include/linux/errno.h
pcl_minidoio.o: /usr/include/asm/errno.h /usr/include/alloca.h
pcl_minidoio.o: /usr/include/ctype.h /usr/include/endian.h
pcl_minidoio.o: /usr/include/bytesex.h /usr/include/string.h systeminclude.h
pcl_minidoio.o: pcl_printer.h pcl_miniterms.h
pcl_mininorm.o: pcl_mininorm.h pcl_miniparse.h pcl_types.h pcl_scan.h
pcl_mininorm.o: pcl_strings.h pcl_mem.h pcl_defs.h /usr/include/malloc.h
pcl_mininorm.o: /usr/include/features.h /usr/include/sys/cdefs.h
pcl_mininorm.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
pcl_mininorm.o: /usr/include/stdio.h /usr/include/libio.h
pcl_mininorm.o: /usr/include/_G_config.h /usr/include/stdlib.h
pcl_mininorm.o: /usr/include/errno.h /usr/include/linux/errno.h
pcl_mininorm.o: /usr/include/asm/errno.h /usr/include/alloca.h
pcl_mininorm.o: /usr/include/ctype.h /usr/include/endian.h
pcl_mininorm.o: /usr/include/bytesex.h /usr/include/string.h systeminclude.h
pcl_mininorm.o: pcl_printer.h pcl_miniterms.h
pcl_miniparse.o: pcl_miniparse.h pcl_types.h pcl_scan.h pcl_strings.h
pcl_miniparse.o: pcl_mem.h pcl_defs.h /usr/include/malloc.h
pcl_miniparse.o: /usr/include/features.h /usr/include/sys/cdefs.h
pcl_miniparse.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
pcl_miniparse.o: /usr/include/stdio.h /usr/include/libio.h
pcl_miniparse.o: /usr/include/_G_config.h /usr/include/stdlib.h
pcl_miniparse.o: /usr/include/errno.h /usr/include/linux/errno.h
pcl_miniparse.o: /usr/include/asm/errno.h /usr/include/alloca.h
pcl_miniparse.o: /usr/include/ctype.h /usr/include/endian.h
pcl_miniparse.o: /usr/include/bytesex.h /usr/include/string.h systeminclude.h
pcl_miniparse.o: pcl_printer.h pcl_miniterms.h
pcl_miniprinter.o: pcl_miniprinter.h pcl_defs.h /usr/include/malloc.h
pcl_miniprinter.o: /usr/include/features.h /usr/include/sys/cdefs.h
pcl_miniprinter.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
pcl_miniprinter.o: /usr/include/stdio.h /usr/include/libio.h
pcl_miniprinter.o: /usr/include/_G_config.h /usr/include/stdlib.h
pcl_miniprinter.o: /usr/include/errno.h /usr/include/linux/errno.h
pcl_miniprinter.o: /usr/include/asm/errno.h /usr/include/alloca.h
pcl_miniprinter.o: /usr/include/ctype.h /usr/include/endian.h
pcl_miniprinter.o: /usr/include/bytesex.h /usr/include/string.h
pcl_miniprinter.o: systeminclude.h
pcl_miniterms.o: pcl_miniterms.h pcl_types.h pcl_scan.h pcl_strings.h
pcl_miniterms.o: pcl_mem.h pcl_defs.h /usr/include/malloc.h
pcl_miniterms.o: /usr/include/features.h /usr/include/sys/cdefs.h
pcl_miniterms.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
pcl_miniterms.o: /usr/include/stdio.h /usr/include/libio.h
pcl_miniterms.o: /usr/include/_G_config.h /usr/include/stdlib.h
pcl_miniterms.o: /usr/include/errno.h /usr/include/linux/errno.h
pcl_miniterms.o: /usr/include/asm/errno.h /usr/include/alloca.h
pcl_miniterms.o: /usr/include/ctype.h /usr/include/endian.h
pcl_miniterms.o: /usr/include/bytesex.h /usr/include/string.h systeminclude.h
pcl_miniterms.o: pcl_printer.h
pcl_parse.o: pcl_parse.h pcl_types.h pcl_scan.h pcl_strings.h pcl_mem.h
pcl_parse.o: pcl_defs.h /usr/include/malloc.h /usr/include/features.h
pcl_parse.o: /usr/include/sys/cdefs.h
pcl_parse.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
pcl_parse.o: /usr/include/stdio.h /usr/include/libio.h
pcl_parse.o: /usr/include/_G_config.h /usr/include/stdlib.h
pcl_parse.o: /usr/include/errno.h /usr/include/linux/errno.h
pcl_parse.o: /usr/include/asm/errno.h /usr/include/alloca.h
pcl_parse.o: /usr/include/ctype.h /usr/include/endian.h
pcl_parse.o: /usr/include/bytesex.h /usr/include/string.h systeminclude.h
pcl_parse.o: pcl_printer.h pcl_terms.h
pcl_printer.o: pcl_printer.h pcl_defs.h /usr/include/malloc.h
pcl_printer.o: /usr/include/features.h /usr/include/sys/cdefs.h
pcl_printer.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
pcl_printer.o: /usr/include/stdio.h /usr/include/libio.h
pcl_printer.o: /usr/include/_G_config.h /usr/include/stdlib.h
pcl_printer.o: /usr/include/errno.h /usr/include/linux/errno.h
pcl_printer.o: /usr/include/asm/errno.h /usr/include/alloca.h
pcl_printer.o: /usr/include/ctype.h /usr/include/endian.h
pcl_printer.o: /usr/include/bytesex.h /usr/include/string.h systeminclude.h
pcl_printproof.o: pcl_printproof.h pcl_fproof.h pcl_eqchains.h pcl_subst.h
pcl_printproof.o: pcl_parse.h pcl_types.h pcl_scan.h pcl_strings.h pcl_mem.h
pcl_printproof.o: pcl_defs.h /usr/include/malloc.h /usr/include/features.h
pcl_printproof.o: /usr/include/sys/cdefs.h
pcl_printproof.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
pcl_printproof.o: /usr/include/stdio.h /usr/include/libio.h
pcl_printproof.o: /usr/include/_G_config.h /usr/include/stdlib.h
pcl_printproof.o: /usr/include/errno.h /usr/include/linux/errno.h
pcl_printproof.o: /usr/include/asm/errno.h /usr/include/alloca.h
pcl_printproof.o: /usr/include/ctype.h /usr/include/endian.h
pcl_printproof.o: /usr/include/bytesex.h /usr/include/string.h
pcl_printproof.o: systeminclude.h pcl_printer.h pcl_terms.h pcl_latexout.h
pcl_printproof.o: pcl_prologout.h
pcl_prologout.o: pcl_prologout.h pcl_eqchains.h pcl_subst.h pcl_parse.h
pcl_prologout.o: pcl_types.h pcl_scan.h pcl_strings.h pcl_mem.h pcl_defs.h
pcl_prologout.o: /usr/include/malloc.h /usr/include/features.h
pcl_prologout.o: /usr/include/sys/cdefs.h
pcl_prologout.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
pcl_prologout.o: /usr/include/stdio.h /usr/include/libio.h
pcl_prologout.o: /usr/include/_G_config.h /usr/include/stdlib.h
pcl_prologout.o: /usr/include/errno.h /usr/include/linux/errno.h
pcl_prologout.o: /usr/include/asm/errno.h /usr/include/alloca.h
pcl_prologout.o: /usr/include/ctype.h /usr/include/endian.h
pcl_prologout.o: /usr/include/bytesex.h /usr/include/string.h systeminclude.h
pcl_prologout.o: pcl_printer.h pcl_terms.h
pcl_scan.o: pcl_scan.h pcl_strings.h pcl_mem.h pcl_defs.h
pcl_scan.o: /usr/include/malloc.h /usr/include/features.h
pcl_scan.o: /usr/include/sys/cdefs.h
pcl_scan.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
pcl_scan.o: /usr/include/stdio.h /usr/include/libio.h
pcl_scan.o: /usr/include/_G_config.h /usr/include/stdlib.h
pcl_scan.o: /usr/include/errno.h /usr/include/linux/errno.h
pcl_scan.o: /usr/include/asm/errno.h /usr/include/alloca.h
pcl_scan.o: /usr/include/ctype.h /usr/include/endian.h /usr/include/bytesex.h
pcl_scan.o: /usr/include/string.h systeminclude.h
pcl_strings.o: pcl_strings.h pcl_mem.h pcl_defs.h /usr/include/malloc.h
pcl_strings.o: /usr/include/features.h /usr/include/sys/cdefs.h
pcl_strings.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
pcl_strings.o: /usr/include/stdio.h /usr/include/libio.h
pcl_strings.o: /usr/include/_G_config.h /usr/include/stdlib.h
pcl_strings.o: /usr/include/errno.h /usr/include/linux/errno.h
pcl_strings.o: /usr/include/asm/errno.h /usr/include/alloca.h
pcl_strings.o: /usr/include/ctype.h /usr/include/endian.h
pcl_strings.o: /usr/include/bytesex.h /usr/include/string.h systeminclude.h
pcl_subst.o: pcl_subst.h pcl_parse.h pcl_types.h pcl_scan.h pcl_strings.h
pcl_subst.o: pcl_mem.h pcl_defs.h /usr/include/malloc.h
pcl_subst.o: /usr/include/features.h /usr/include/sys/cdefs.h
pcl_subst.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
pcl_subst.o: /usr/include/stdio.h /usr/include/libio.h
pcl_subst.o: /usr/include/_G_config.h /usr/include/stdlib.h
pcl_subst.o: /usr/include/errno.h /usr/include/linux/errno.h
pcl_subst.o: /usr/include/asm/errno.h /usr/include/alloca.h
pcl_subst.o: /usr/include/ctype.h /usr/include/endian.h
pcl_subst.o: /usr/include/bytesex.h /usr/include/string.h systeminclude.h
pcl_subst.o: pcl_printer.h pcl_terms.h
pcl_terms.o: pcl_terms.h pcl_types.h pcl_scan.h pcl_strings.h pcl_mem.h
pcl_terms.o: pcl_defs.h /usr/include/malloc.h /usr/include/features.h
pcl_terms.o: /usr/include/sys/cdefs.h
pcl_terms.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
pcl_terms.o: /usr/include/stdio.h /usr/include/libio.h
pcl_terms.o: /usr/include/_G_config.h /usr/include/stdlib.h
pcl_terms.o: /usr/include/errno.h /usr/include/linux/errno.h
pcl_terms.o: /usr/include/asm/errno.h /usr/include/alloca.h
pcl_terms.o: /usr/include/ctype.h /usr/include/endian.h
pcl_terms.o: /usr/include/bytesex.h /usr/include/string.h systeminclude.h
pcl_terms.o: pcl_printer.h
pcl_types.o: pcl_types.h pcl_scan.h pcl_strings.h pcl_mem.h pcl_defs.h
pcl_types.o: /usr/include/malloc.h /usr/include/features.h
pcl_types.o: /usr/include/sys/cdefs.h
pcl_types.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
pcl_types.o: /usr/include/stdio.h /usr/include/libio.h
pcl_types.o: /usr/include/_G_config.h /usr/include/stdlib.h
pcl_types.o: /usr/include/errno.h /usr/include/linux/errno.h
pcl_types.o: /usr/include/asm/errno.h /usr/include/alloca.h
pcl_types.o: /usr/include/ctype.h /usr/include/endian.h
pcl_types.o: /usr/include/bytesex.h /usr/include/string.h systeminclude.h
pcl_types.o: pcl_printer.h
proof.o: pcl_doio.h pcl_parse.h pcl_types.h pcl_scan.h pcl_strings.h
proof.o: pcl_mem.h pcl_defs.h /usr/include/malloc.h /usr/include/features.h
proof.o: /usr/include/sys/cdefs.h
proof.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
proof.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
proof.o: /usr/include/stdlib.h /usr/include/errno.h
proof.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
proof.o: /usr/include/alloca.h /usr/include/ctype.h /usr/include/endian.h
proof.o: /usr/include/bytesex.h /usr/include/string.h systeminclude.h
proof.o: pcl_printer.h pcl_terms.h pcl_lemma.h pcl_buildtree.h pcl_analyse.h
proof.o: pcl_fproof.h pcl_eqchains.h pcl_subst.h pcl_printproof.h
proof.o: pcl_latexout.h pcl_prologout.h
revert.o: pcl_defs.h /usr/include/malloc.h /usr/include/features.h
revert.o: /usr/include/sys/cdefs.h
revert.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
revert.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
revert.o: /usr/include/stdlib.h /usr/include/errno.h
revert.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
revert.o: /usr/include/alloca.h /usr/include/ctype.h /usr/include/endian.h
revert.o: /usr/include/bytesex.h /usr/include/string.h systeminclude.h
rextract.o: pcl_minianalyse.h pcl_mininorm.h pcl_miniparse.h pcl_types.h
rextract.o: pcl_scan.h pcl_strings.h pcl_mem.h pcl_defs.h
rextract.o: /usr/include/malloc.h /usr/include/features.h
rextract.o: /usr/include/sys/cdefs.h
rextract.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
rextract.o: /usr/include/stdio.h /usr/include/libio.h
rextract.o: /usr/include/_G_config.h /usr/include/stdlib.h
rextract.o: /usr/include/errno.h /usr/include/linux/errno.h
rextract.o: /usr/include/asm/errno.h /usr/include/alloca.h
rextract.o: /usr/include/ctype.h /usr/include/endian.h /usr/include/bytesex.h
rextract.o: /usr/include/string.h systeminclude.h pcl_printer.h
rextract.o: pcl_miniterms.h pcl_minidoio.h pcl_miniprinter.h
scan.o: scan.h pcl_strings.h pcl_mem.h pcl_defs.h /usr/include/malloc.h
scan.o: /usr/include/features.h /usr/include/sys/cdefs.h
scan.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
scan.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
scan.o: /usr/include/stdlib.h /usr/include/errno.h /usr/include/linux/errno.h
scan.o: /usr/include/asm/errno.h /usr/include/alloca.h /usr/include/ctype.h
scan.o: /usr/include/endian.h /usr/include/bytesex.h /usr/include/string.h
scan.o: systeminclude.h
trans.o: pcl_defs.h /usr/include/malloc.h /usr/include/features.h
trans.o: /usr/include/sys/cdefs.h
trans.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
trans.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
trans.o: /usr/include/stdlib.h /usr/include/errno.h
trans.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
trans.o: /usr/include/alloca.h /usr/include/ctype.h /usr/include/endian.h
trans.o: /usr/include/bytesex.h /usr/include/string.h systeminclude.h parse.h
trans.o: nameadmin.h scan.h pcl_strings.h pcl_mem.h
