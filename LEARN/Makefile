#---------------------------------------------------------------------------
#
# File:        makefile
#
# Autor:       Stephan Schulz
#
# Inhalt:      makefile fuer Programme zum Lernen
#
# Anmerkung:   Die Lernprogramme bauen auf PCL-Programmen auf. Dieses
#              Makefile entstand aus dem PCL-Makefile vom 25.4.1994.
#
# Aenderungen: <1>  10.6.1994 neu
#
#---------------------------------------------------------------------------



CC = gcc 
CFLAGS = -Wall -O2 -c -funroll-loops -ansi 
LD = gcc


all : makedom domparse kb_create kb_insert

depend : *.c *.h
	makedepend *.c

tags: *.c *.h
	etags *.c *.h

BASICS = pcl_strings.o pcl_mem.o pcl_scan.o pcl_types.o pcl_printer.o

NORMAL = pcl_parse.o pcl_terms.o pcl_doio.o

COMPLEX =  pcl_analyse.o pcl_buildtree.o pcl_lemma.o

NORMEQNS = lrn_norm.o lrn_eqnoccur.o lrn_eqn.o lrn_eqntrees.o 

PCLTRANS = lrn_pcltrans.o lrn_domains.o lrn_sigs.o lrn_fnames.o

INSERT = lrn_insert.o lrn_parse.o lrn_filevars.o


# makedom - Hole Signaturinformationen und norm-gerichtete Gleichungen
# aus einem PCL-listing

MAKEDOM = makedom.o $(BASICS) $(NORMAL) $(COMPLEX) $(NORMEQNS)\
	$(PCLTRANS)

makedom: $(MAKEDOM) 
	$(LD) -o makedom $(MAKEDOM)


# domparse - Parse Domaenen (zu Testzwecken)

DOMPARSE = domparse.o $(BASICS) $(NORMAL) $(COMPLEX) $(NORMEQNS)\
	$(PCLTRANS) lrn_parse.o

domparse: $(DOMPARSE)
	$(LD) -o domparse $(DOMPARSE)	

# kb_create - Erzeuge (leere) Wissensbasis

KB_CREATE = kb_create.o $(BASICS) $(NORMAL) $(NORMEQNS) $(PCLTRANS)

kb_create: $(KB_CREATE)
	$(LD) -o kb_create $(KB_CREATE)

# kb_insert - Einfuegen eines Beispieles in eine Wissensbasis

KB_INSERT = kb_insert.o $(BASICS) $(NORMAL) $(COMPLEX) $(NORMEQNS)\
	$(PCLTRANS) $(INSERT) 

kb_insert: $(KB_INSERT)
	$(LD) -o kb_insert $(KB_INSERT) -lm



# DO NOT DELETE THIS LINE -- make depend depends on it.

domparse.o: pcl_doio.h pcl_parse.h pcl_types.h pcl_scan.h pcl_strings.h
domparse.o: pcl_mem.h pcl_defs.h /usr/include/malloc.h
domparse.o: /usr/include/features.h /usr/include/sys/cdefs.h
domparse.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
domparse.o: /usr/include/stdio.h /usr/include/libio.h
domparse.o: /usr/include/_G_config.h /usr/include/stdlib.h
domparse.o: /usr/include/errno.h /usr/include/linux/errno.h
domparse.o: /usr/include/asm/errno.h /usr/include/alloca.h
domparse.o: /usr/include/ctype.h /usr/include/endian.h /usr/include/bytesex.h
domparse.o: /usr/include/string.h /usr/include/sys/param.h
domparse.o: /usr/include/limits.h /usr/include/posix1_lim.h
domparse.o: /usr/include/linux/limits.h /usr/include/posix2_lim.h
domparse.o: /usr/include/linux/param.h /usr/include/asm/param.h
domparse.o: /usr/include/sys/types.h /usr/include/linux/types.h
domparse.o: /usr/include/linux/posix_types.h /usr/include/asm/posix_types.h
domparse.o: /usr/include/asm/types.h /usr/include/sys/bitypes.h
domparse.o: systeminclude.h pcl_printer.h pcl_terms.h lrn_parse.h
domparse.o: lrn_domains.h lrn_eqntrees.h lrn_eqn.h lrn_norm.h lrn_eqnoccur.h
domparse.o: lrn_sigs.h
kb_create.o: pcl_doio.h pcl_parse.h pcl_types.h pcl_scan.h pcl_strings.h
kb_create.o: pcl_mem.h pcl_defs.h /usr/include/malloc.h
kb_create.o: /usr/include/features.h /usr/include/sys/cdefs.h
kb_create.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
kb_create.o: /usr/include/stdio.h /usr/include/libio.h
kb_create.o: /usr/include/_G_config.h /usr/include/stdlib.h
kb_create.o: /usr/include/errno.h /usr/include/linux/errno.h
kb_create.o: /usr/include/asm/errno.h /usr/include/alloca.h
kb_create.o: /usr/include/ctype.h /usr/include/endian.h
kb_create.o: /usr/include/bytesex.h /usr/include/string.h
kb_create.o: /usr/include/sys/param.h /usr/include/limits.h
kb_create.o: /usr/include/posix1_lim.h /usr/include/linux/limits.h
kb_create.o: /usr/include/posix2_lim.h /usr/include/linux/param.h
kb_create.o: /usr/include/asm/param.h /usr/include/sys/types.h
kb_create.o: /usr/include/linux/types.h /usr/include/linux/posix_types.h
kb_create.o: /usr/include/asm/posix_types.h /usr/include/asm/types.h
kb_create.o: /usr/include/sys/bitypes.h systeminclude.h pcl_printer.h
kb_create.o: pcl_terms.h lrn_fnames.h lrn_domains.h lrn_eqntrees.h lrn_eqn.h
kb_create.o: lrn_norm.h lrn_eqnoccur.h lrn_sigs.h /usr/include/sys/stat.h
kb_create.o: /usr/include/linux/stat.h
kb_insert.o: pcl_lemma.h pcl_buildtree.h pcl_parse.h pcl_types.h pcl_scan.h
kb_insert.o: pcl_strings.h pcl_mem.h pcl_defs.h /usr/include/malloc.h
kb_insert.o: /usr/include/features.h /usr/include/sys/cdefs.h
kb_insert.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
kb_insert.o: /usr/include/stdio.h /usr/include/libio.h
kb_insert.o: /usr/include/_G_config.h /usr/include/stdlib.h
kb_insert.o: /usr/include/errno.h /usr/include/linux/errno.h
kb_insert.o: /usr/include/asm/errno.h /usr/include/alloca.h
kb_insert.o: /usr/include/ctype.h /usr/include/endian.h
kb_insert.o: /usr/include/bytesex.h /usr/include/string.h
kb_insert.o: /usr/include/sys/param.h /usr/include/limits.h
kb_insert.o: /usr/include/posix1_lim.h /usr/include/linux/limits.h
kb_insert.o: /usr/include/posix2_lim.h /usr/include/linux/param.h
kb_insert.o: /usr/include/asm/param.h /usr/include/sys/types.h
kb_insert.o: /usr/include/linux/types.h /usr/include/linux/posix_types.h
kb_insert.o: /usr/include/asm/posix_types.h /usr/include/asm/types.h
kb_insert.o: /usr/include/sys/bitypes.h systeminclude.h pcl_printer.h
kb_insert.o: pcl_terms.h pcl_analyse.h pcl_doio.h lrn_fnames.h lrn_pcltrans.h
kb_insert.o: lrn_domains.h lrn_eqntrees.h lrn_eqn.h lrn_norm.h lrn_eqnoccur.h
kb_insert.o: lrn_sigs.h lrn_insert.h /usr/include/math.h
kb_insert.o: /usr/include/huge_val.h /usr/include/nan.h
kb_insert.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/float.h
kb_insert.o: /usr/include/values.h /usr/include/ieee754.h
kb_insert.o: /usr/include/ieee854.h lrn_parse.h lrn_filevars.h
lrn_domains.o: lrn_domains.h lrn_eqntrees.h lrn_eqn.h lrn_norm.h pcl_terms.h
lrn_domains.o: pcl_types.h pcl_scan.h pcl_strings.h pcl_mem.h pcl_defs.h
lrn_domains.o: /usr/include/malloc.h /usr/include/features.h
lrn_domains.o: /usr/include/sys/cdefs.h
lrn_domains.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
lrn_domains.o: /usr/include/stdio.h /usr/include/libio.h
lrn_domains.o: /usr/include/_G_config.h /usr/include/stdlib.h
lrn_domains.o: /usr/include/errno.h /usr/include/linux/errno.h
lrn_domains.o: /usr/include/asm/errno.h /usr/include/alloca.h
lrn_domains.o: /usr/include/ctype.h /usr/include/endian.h
lrn_domains.o: /usr/include/bytesex.h /usr/include/string.h
lrn_domains.o: /usr/include/sys/param.h /usr/include/limits.h
lrn_domains.o: /usr/include/posix1_lim.h /usr/include/linux/limits.h
lrn_domains.o: /usr/include/posix2_lim.h /usr/include/linux/param.h
lrn_domains.o: /usr/include/asm/param.h /usr/include/sys/types.h
lrn_domains.o: /usr/include/linux/types.h /usr/include/linux/posix_types.h
lrn_domains.o: /usr/include/asm/posix_types.h /usr/include/asm/types.h
lrn_domains.o: /usr/include/sys/bitypes.h systeminclude.h pcl_printer.h
lrn_domains.o: pcl_doio.h pcl_parse.h lrn_eqnoccur.h lrn_sigs.h
lrn_eqn.o: lrn_eqn.h lrn_norm.h pcl_terms.h pcl_types.h pcl_scan.h
lrn_eqn.o: pcl_strings.h pcl_mem.h pcl_defs.h /usr/include/malloc.h
lrn_eqn.o: /usr/include/features.h /usr/include/sys/cdefs.h
lrn_eqn.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
lrn_eqn.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
lrn_eqn.o: /usr/include/stdlib.h /usr/include/errno.h
lrn_eqn.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
lrn_eqn.o: /usr/include/alloca.h /usr/include/ctype.h /usr/include/endian.h
lrn_eqn.o: /usr/include/bytesex.h /usr/include/string.h
lrn_eqn.o: /usr/include/sys/param.h /usr/include/limits.h
lrn_eqn.o: /usr/include/posix1_lim.h /usr/include/linux/limits.h
lrn_eqn.o: /usr/include/posix2_lim.h /usr/include/linux/param.h
lrn_eqn.o: /usr/include/asm/param.h /usr/include/sys/types.h
lrn_eqn.o: /usr/include/linux/types.h /usr/include/linux/posix_types.h
lrn_eqn.o: /usr/include/asm/posix_types.h /usr/include/asm/types.h
lrn_eqn.o: /usr/include/sys/bitypes.h systeminclude.h pcl_printer.h
lrn_eqn.o: pcl_doio.h pcl_parse.h lrn_eqnoccur.h
lrn_eqnoccur.o: lrn_eqnoccur.h pcl_doio.h pcl_parse.h pcl_types.h pcl_scan.h
lrn_eqnoccur.o: pcl_strings.h pcl_mem.h pcl_defs.h /usr/include/malloc.h
lrn_eqnoccur.o: /usr/include/features.h /usr/include/sys/cdefs.h
lrn_eqnoccur.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
lrn_eqnoccur.o: /usr/include/stdio.h /usr/include/libio.h
lrn_eqnoccur.o: /usr/include/_G_config.h /usr/include/stdlib.h
lrn_eqnoccur.o: /usr/include/errno.h /usr/include/linux/errno.h
lrn_eqnoccur.o: /usr/include/asm/errno.h /usr/include/alloca.h
lrn_eqnoccur.o: /usr/include/ctype.h /usr/include/endian.h
lrn_eqnoccur.o: /usr/include/bytesex.h /usr/include/string.h
lrn_eqnoccur.o: /usr/include/sys/param.h /usr/include/limits.h
lrn_eqnoccur.o: /usr/include/posix1_lim.h /usr/include/linux/limits.h
lrn_eqnoccur.o: /usr/include/posix2_lim.h /usr/include/linux/param.h
lrn_eqnoccur.o: /usr/include/asm/param.h /usr/include/sys/types.h
lrn_eqnoccur.o: /usr/include/linux/types.h /usr/include/linux/posix_types.h
lrn_eqnoccur.o: /usr/include/asm/posix_types.h /usr/include/asm/types.h
lrn_eqnoccur.o: /usr/include/sys/bitypes.h systeminclude.h pcl_printer.h
lrn_eqnoccur.o: pcl_terms.h
lrn_eqntrees.o: lrn_eqntrees.h lrn_eqn.h lrn_norm.h pcl_terms.h pcl_types.h
lrn_eqntrees.o: pcl_scan.h pcl_strings.h pcl_mem.h pcl_defs.h
lrn_eqntrees.o: /usr/include/malloc.h /usr/include/features.h
lrn_eqntrees.o: /usr/include/sys/cdefs.h
lrn_eqntrees.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
lrn_eqntrees.o: /usr/include/stdio.h /usr/include/libio.h
lrn_eqntrees.o: /usr/include/_G_config.h /usr/include/stdlib.h
lrn_eqntrees.o: /usr/include/errno.h /usr/include/linux/errno.h
lrn_eqntrees.o: /usr/include/asm/errno.h /usr/include/alloca.h
lrn_eqntrees.o: /usr/include/ctype.h /usr/include/endian.h
lrn_eqntrees.o: /usr/include/bytesex.h /usr/include/string.h
lrn_eqntrees.o: /usr/include/sys/param.h /usr/include/limits.h
lrn_eqntrees.o: /usr/include/posix1_lim.h /usr/include/linux/limits.h
lrn_eqntrees.o: /usr/include/posix2_lim.h /usr/include/linux/param.h
lrn_eqntrees.o: /usr/include/asm/param.h /usr/include/sys/types.h
lrn_eqntrees.o: /usr/include/linux/types.h /usr/include/linux/posix_types.h
lrn_eqntrees.o: /usr/include/asm/posix_types.h /usr/include/asm/types.h
lrn_eqntrees.o: /usr/include/sys/bitypes.h systeminclude.h pcl_printer.h
lrn_eqntrees.o: pcl_doio.h pcl_parse.h lrn_eqnoccur.h
lrn_filevars.o: lrn_filevars.h pcl_doio.h pcl_parse.h pcl_types.h pcl_scan.h
lrn_filevars.o: pcl_strings.h pcl_mem.h pcl_defs.h /usr/include/malloc.h
lrn_filevars.o: /usr/include/features.h /usr/include/sys/cdefs.h
lrn_filevars.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
lrn_filevars.o: /usr/include/stdio.h /usr/include/libio.h
lrn_filevars.o: /usr/include/_G_config.h /usr/include/stdlib.h
lrn_filevars.o: /usr/include/errno.h /usr/include/linux/errno.h
lrn_filevars.o: /usr/include/asm/errno.h /usr/include/alloca.h
lrn_filevars.o: /usr/include/ctype.h /usr/include/endian.h
lrn_filevars.o: /usr/include/bytesex.h /usr/include/string.h
lrn_filevars.o: /usr/include/sys/param.h /usr/include/limits.h
lrn_filevars.o: /usr/include/posix1_lim.h /usr/include/linux/limits.h
lrn_filevars.o: /usr/include/posix2_lim.h /usr/include/linux/param.h
lrn_filevars.o: /usr/include/asm/param.h /usr/include/sys/types.h
lrn_filevars.o: /usr/include/linux/types.h /usr/include/linux/posix_types.h
lrn_filevars.o: /usr/include/asm/posix_types.h /usr/include/asm/types.h
lrn_filevars.o: /usr/include/sys/bitypes.h systeminclude.h pcl_printer.h
lrn_filevars.o: pcl_terms.h
lrn_fnames.o: lrn_fnames.h /usr/include/sys/param.h /usr/include/features.h
lrn_fnames.o: /usr/include/sys/cdefs.h /usr/include/limits.h
lrn_fnames.o: /usr/include/posix1_lim.h /usr/include/linux/limits.h
lrn_fnames.o: /usr/include/posix2_lim.h /usr/include/linux/param.h
lrn_fnames.o: /usr/include/asm/param.h /usr/include/sys/types.h
lrn_fnames.o: /usr/include/linux/types.h /usr/include/linux/posix_types.h
lrn_fnames.o: /usr/include/asm/posix_types.h /usr/include/asm/types.h
lrn_fnames.o: /usr/include/sys/bitypes.h pcl_strings.h pcl_mem.h pcl_defs.h
lrn_fnames.o: /usr/include/malloc.h
lrn_fnames.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
lrn_fnames.o: /usr/include/stdio.h /usr/include/libio.h
lrn_fnames.o: /usr/include/_G_config.h /usr/include/stdlib.h
lrn_fnames.o: /usr/include/errno.h /usr/include/linux/errno.h
lrn_fnames.o: /usr/include/asm/errno.h /usr/include/alloca.h
lrn_fnames.o: /usr/include/ctype.h /usr/include/endian.h
lrn_fnames.o: /usr/include/bytesex.h /usr/include/string.h systeminclude.h
lrn_insert.o: lrn_insert.h /usr/include/math.h /usr/include/features.h
lrn_insert.o: /usr/include/sys/cdefs.h /usr/include/huge_val.h
lrn_insert.o: /usr/include/endian.h /usr/include/bytesex.h /usr/include/nan.h
lrn_insert.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/float.h
lrn_insert.o: /usr/include/values.h /usr/include/ieee754.h
lrn_insert.o: /usr/include/ieee854.h lrn_domains.h lrn_eqntrees.h lrn_eqn.h
lrn_insert.o: lrn_norm.h pcl_terms.h pcl_types.h pcl_scan.h pcl_strings.h
lrn_insert.o: pcl_mem.h pcl_defs.h /usr/include/malloc.h
lrn_insert.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
lrn_insert.o: /usr/include/stdio.h /usr/include/libio.h
lrn_insert.o: /usr/include/_G_config.h /usr/include/stdlib.h
lrn_insert.o: /usr/include/errno.h /usr/include/linux/errno.h
lrn_insert.o: /usr/include/asm/errno.h /usr/include/alloca.h
lrn_insert.o: /usr/include/ctype.h /usr/include/string.h
lrn_insert.o: /usr/include/sys/param.h /usr/include/limits.h
lrn_insert.o: /usr/include/posix1_lim.h /usr/include/linux/limits.h
lrn_insert.o: /usr/include/posix2_lim.h /usr/include/linux/param.h
lrn_insert.o: /usr/include/asm/param.h /usr/include/sys/types.h
lrn_insert.o: /usr/include/linux/types.h /usr/include/linux/posix_types.h
lrn_insert.o: /usr/include/asm/posix_types.h /usr/include/asm/types.h
lrn_insert.o: /usr/include/sys/bitypes.h systeminclude.h pcl_printer.h
lrn_insert.o: pcl_doio.h pcl_parse.h lrn_eqnoccur.h lrn_sigs.h lrn_parse.h
lrn_insert.o: lrn_pcltrans.h
lrn_norm.o: lrn_norm.h pcl_terms.h pcl_types.h pcl_scan.h pcl_strings.h
lrn_norm.o: pcl_mem.h pcl_defs.h /usr/include/malloc.h
lrn_norm.o: /usr/include/features.h /usr/include/sys/cdefs.h
lrn_norm.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
lrn_norm.o: /usr/include/stdio.h /usr/include/libio.h
lrn_norm.o: /usr/include/_G_config.h /usr/include/stdlib.h
lrn_norm.o: /usr/include/errno.h /usr/include/linux/errno.h
lrn_norm.o: /usr/include/asm/errno.h /usr/include/alloca.h
lrn_norm.o: /usr/include/ctype.h /usr/include/endian.h /usr/include/bytesex.h
lrn_norm.o: /usr/include/string.h /usr/include/sys/param.h
lrn_norm.o: /usr/include/limits.h /usr/include/posix1_lim.h
lrn_norm.o: /usr/include/linux/limits.h /usr/include/posix2_lim.h
lrn_norm.o: /usr/include/linux/param.h /usr/include/asm/param.h
lrn_norm.o: /usr/include/sys/types.h /usr/include/linux/types.h
lrn_norm.o: /usr/include/linux/posix_types.h /usr/include/asm/posix_types.h
lrn_norm.o: /usr/include/asm/types.h /usr/include/sys/bitypes.h
lrn_norm.o: systeminclude.h pcl_printer.h pcl_doio.h pcl_parse.h
lrn_parse.o: lrn_parse.h lrn_domains.h lrn_eqntrees.h lrn_eqn.h lrn_norm.h
lrn_parse.o: pcl_terms.h pcl_types.h pcl_scan.h pcl_strings.h pcl_mem.h
lrn_parse.o: pcl_defs.h /usr/include/malloc.h /usr/include/features.h
lrn_parse.o: /usr/include/sys/cdefs.h
lrn_parse.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
lrn_parse.o: /usr/include/stdio.h /usr/include/libio.h
lrn_parse.o: /usr/include/_G_config.h /usr/include/stdlib.h
lrn_parse.o: /usr/include/errno.h /usr/include/linux/errno.h
lrn_parse.o: /usr/include/asm/errno.h /usr/include/alloca.h
lrn_parse.o: /usr/include/ctype.h /usr/include/endian.h
lrn_parse.o: /usr/include/bytesex.h /usr/include/string.h
lrn_parse.o: /usr/include/sys/param.h /usr/include/limits.h
lrn_parse.o: /usr/include/posix1_lim.h /usr/include/linux/limits.h
lrn_parse.o: /usr/include/posix2_lim.h /usr/include/linux/param.h
lrn_parse.o: /usr/include/asm/param.h /usr/include/sys/types.h
lrn_parse.o: /usr/include/linux/types.h /usr/include/linux/posix_types.h
lrn_parse.o: /usr/include/asm/posix_types.h /usr/include/asm/types.h
lrn_parse.o: /usr/include/sys/bitypes.h systeminclude.h pcl_printer.h
lrn_parse.o: pcl_doio.h pcl_parse.h lrn_eqnoccur.h lrn_sigs.h
lrn_pcltrans.o: lrn_pcltrans.h lrn_domains.h lrn_eqntrees.h lrn_eqn.h
lrn_pcltrans.o: lrn_norm.h pcl_terms.h pcl_types.h pcl_scan.h pcl_strings.h
lrn_pcltrans.o: pcl_mem.h pcl_defs.h /usr/include/malloc.h
lrn_pcltrans.o: /usr/include/features.h /usr/include/sys/cdefs.h
lrn_pcltrans.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
lrn_pcltrans.o: /usr/include/stdio.h /usr/include/libio.h
lrn_pcltrans.o: /usr/include/_G_config.h /usr/include/stdlib.h
lrn_pcltrans.o: /usr/include/errno.h /usr/include/linux/errno.h
lrn_pcltrans.o: /usr/include/asm/errno.h /usr/include/alloca.h
lrn_pcltrans.o: /usr/include/ctype.h /usr/include/endian.h
lrn_pcltrans.o: /usr/include/bytesex.h /usr/include/string.h
lrn_pcltrans.o: /usr/include/sys/param.h /usr/include/limits.h
lrn_pcltrans.o: /usr/include/posix1_lim.h /usr/include/linux/limits.h
lrn_pcltrans.o: /usr/include/posix2_lim.h /usr/include/linux/param.h
lrn_pcltrans.o: /usr/include/asm/param.h /usr/include/sys/types.h
lrn_pcltrans.o: /usr/include/linux/types.h /usr/include/linux/posix_types.h
lrn_pcltrans.o: /usr/include/asm/posix_types.h /usr/include/asm/types.h
lrn_pcltrans.o: /usr/include/sys/bitypes.h systeminclude.h pcl_printer.h
lrn_pcltrans.o: pcl_doio.h pcl_parse.h lrn_eqnoccur.h lrn_sigs.h
lrn_sigs.o: lrn_sigs.h lrn_eqn.h lrn_norm.h pcl_terms.h pcl_types.h
lrn_sigs.o: pcl_scan.h pcl_strings.h pcl_mem.h pcl_defs.h
lrn_sigs.o: /usr/include/malloc.h /usr/include/features.h
lrn_sigs.o: /usr/include/sys/cdefs.h
lrn_sigs.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
lrn_sigs.o: /usr/include/stdio.h /usr/include/libio.h
lrn_sigs.o: /usr/include/_G_config.h /usr/include/stdlib.h
lrn_sigs.o: /usr/include/errno.h /usr/include/linux/errno.h
lrn_sigs.o: /usr/include/asm/errno.h /usr/include/alloca.h
lrn_sigs.o: /usr/include/ctype.h /usr/include/endian.h /usr/include/bytesex.h
lrn_sigs.o: /usr/include/string.h /usr/include/sys/param.h
lrn_sigs.o: /usr/include/limits.h /usr/include/posix1_lim.h
lrn_sigs.o: /usr/include/linux/limits.h /usr/include/posix2_lim.h
lrn_sigs.o: /usr/include/linux/param.h /usr/include/asm/param.h
lrn_sigs.o: /usr/include/sys/types.h /usr/include/linux/types.h
lrn_sigs.o: /usr/include/linux/posix_types.h /usr/include/asm/posix_types.h
lrn_sigs.o: /usr/include/asm/types.h /usr/include/sys/bitypes.h
lrn_sigs.o: systeminclude.h pcl_printer.h pcl_doio.h pcl_parse.h
lrn_sigs.o: lrn_eqnoccur.h
makedom.o: pcl_doio.h pcl_parse.h pcl_types.h pcl_scan.h pcl_strings.h
makedom.o: pcl_mem.h pcl_defs.h /usr/include/malloc.h /usr/include/features.h
makedom.o: /usr/include/sys/cdefs.h
makedom.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
makedom.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
makedom.o: /usr/include/stdlib.h /usr/include/errno.h
makedom.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
makedom.o: /usr/include/alloca.h /usr/include/ctype.h /usr/include/endian.h
makedom.o: /usr/include/bytesex.h /usr/include/string.h
makedom.o: /usr/include/sys/param.h /usr/include/limits.h
makedom.o: /usr/include/posix1_lim.h /usr/include/linux/limits.h
makedom.o: /usr/include/posix2_lim.h /usr/include/linux/param.h
makedom.o: /usr/include/asm/param.h /usr/include/sys/types.h
makedom.o: /usr/include/linux/types.h /usr/include/linux/posix_types.h
makedom.o: /usr/include/asm/posix_types.h /usr/include/asm/types.h
makedom.o: /usr/include/sys/bitypes.h systeminclude.h pcl_printer.h
makedom.o: pcl_terms.h pcl_lemma.h pcl_buildtree.h pcl_analyse.h
makedom.o: lrn_pcltrans.h lrn_domains.h lrn_eqntrees.h lrn_eqn.h lrn_norm.h
makedom.o: lrn_eqnoccur.h lrn_sigs.h lrn_fnames.h
pcl_analyse.o: pcl_analyse.h pcl_parse.h pcl_types.h pcl_scan.h pcl_strings.h
pcl_analyse.o: pcl_mem.h pcl_defs.h /usr/include/malloc.h
pcl_analyse.o: /usr/include/features.h /usr/include/sys/cdefs.h
pcl_analyse.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
pcl_analyse.o: /usr/include/stdio.h /usr/include/libio.h
pcl_analyse.o: /usr/include/_G_config.h /usr/include/stdlib.h
pcl_analyse.o: /usr/include/errno.h /usr/include/linux/errno.h
pcl_analyse.o: /usr/include/asm/errno.h /usr/include/alloca.h
pcl_analyse.o: /usr/include/ctype.h /usr/include/endian.h
pcl_analyse.o: /usr/include/bytesex.h /usr/include/string.h
pcl_analyse.o: /usr/include/sys/param.h /usr/include/limits.h
pcl_analyse.o: /usr/include/posix1_lim.h /usr/include/linux/limits.h
pcl_analyse.o: /usr/include/posix2_lim.h /usr/include/linux/param.h
pcl_analyse.o: /usr/include/asm/param.h /usr/include/sys/types.h
pcl_analyse.o: /usr/include/linux/types.h /usr/include/linux/posix_types.h
pcl_analyse.o: /usr/include/asm/posix_types.h /usr/include/asm/types.h
pcl_analyse.o: /usr/include/sys/bitypes.h systeminclude.h pcl_printer.h
pcl_analyse.o: pcl_terms.h
pcl_buildtree.o: pcl_buildtree.h pcl_parse.h pcl_types.h pcl_scan.h
pcl_buildtree.o: pcl_strings.h pcl_mem.h pcl_defs.h /usr/include/malloc.h
pcl_buildtree.o: /usr/include/features.h /usr/include/sys/cdefs.h
pcl_buildtree.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
pcl_buildtree.o: /usr/include/stdio.h /usr/include/libio.h
pcl_buildtree.o: /usr/include/_G_config.h /usr/include/stdlib.h
pcl_buildtree.o: /usr/include/errno.h /usr/include/linux/errno.h
pcl_buildtree.o: /usr/include/asm/errno.h /usr/include/alloca.h
pcl_buildtree.o: /usr/include/ctype.h /usr/include/endian.h
pcl_buildtree.o: /usr/include/bytesex.h /usr/include/string.h
pcl_buildtree.o: /usr/include/sys/param.h /usr/include/limits.h
pcl_buildtree.o: /usr/include/posix1_lim.h /usr/include/linux/limits.h
pcl_buildtree.o: /usr/include/posix2_lim.h /usr/include/linux/param.h
pcl_buildtree.o: /usr/include/asm/param.h /usr/include/sys/types.h
pcl_buildtree.o: /usr/include/linux/types.h /usr/include/linux/posix_types.h
pcl_buildtree.o: /usr/include/asm/posix_types.h /usr/include/asm/types.h
pcl_buildtree.o: /usr/include/sys/bitypes.h systeminclude.h pcl_printer.h
pcl_buildtree.o: pcl_terms.h pcl_analyse.h
pcl_doio.o: pcl_doio.h pcl_parse.h pcl_types.h pcl_scan.h pcl_strings.h
pcl_doio.o: pcl_mem.h pcl_defs.h /usr/include/malloc.h
pcl_doio.o: /usr/include/features.h /usr/include/sys/cdefs.h
pcl_doio.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
pcl_doio.o: /usr/include/stdio.h /usr/include/libio.h
pcl_doio.o: /usr/include/_G_config.h /usr/include/stdlib.h
pcl_doio.o: /usr/include/errno.h /usr/include/linux/errno.h
pcl_doio.o: /usr/include/asm/errno.h /usr/include/alloca.h
pcl_doio.o: /usr/include/ctype.h /usr/include/endian.h /usr/include/bytesex.h
pcl_doio.o: /usr/include/string.h /usr/include/sys/param.h
pcl_doio.o: /usr/include/limits.h /usr/include/posix1_lim.h
pcl_doio.o: /usr/include/linux/limits.h /usr/include/posix2_lim.h
pcl_doio.o: /usr/include/linux/param.h /usr/include/asm/param.h
pcl_doio.o: /usr/include/sys/types.h /usr/include/linux/types.h
pcl_doio.o: /usr/include/linux/posix_types.h /usr/include/asm/posix_types.h
pcl_doio.o: /usr/include/asm/types.h /usr/include/sys/bitypes.h
pcl_doio.o: systeminclude.h pcl_printer.h pcl_terms.h
pcl_lemma.o: pcl_lemma.h pcl_buildtree.h pcl_parse.h pcl_types.h pcl_scan.h
pcl_lemma.o: pcl_strings.h pcl_mem.h pcl_defs.h /usr/include/malloc.h
pcl_lemma.o: /usr/include/features.h /usr/include/sys/cdefs.h
pcl_lemma.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
pcl_lemma.o: /usr/include/stdio.h /usr/include/libio.h
pcl_lemma.o: /usr/include/_G_config.h /usr/include/stdlib.h
pcl_lemma.o: /usr/include/errno.h /usr/include/linux/errno.h
pcl_lemma.o: /usr/include/asm/errno.h /usr/include/alloca.h
pcl_lemma.o: /usr/include/ctype.h /usr/include/endian.h
pcl_lemma.o: /usr/include/bytesex.h /usr/include/string.h
pcl_lemma.o: /usr/include/sys/param.h /usr/include/limits.h
pcl_lemma.o: /usr/include/posix1_lim.h /usr/include/linux/limits.h
pcl_lemma.o: /usr/include/posix2_lim.h /usr/include/linux/param.h
pcl_lemma.o: /usr/include/asm/param.h /usr/include/sys/types.h
pcl_lemma.o: /usr/include/linux/types.h /usr/include/linux/posix_types.h
pcl_lemma.o: /usr/include/asm/posix_types.h /usr/include/asm/types.h
pcl_lemma.o: /usr/include/sys/bitypes.h systeminclude.h pcl_printer.h
pcl_lemma.o: pcl_terms.h pcl_analyse.h pcl_doio.h
pcl_mem.o: pcl_mem.h pcl_defs.h /usr/include/malloc.h /usr/include/features.h
pcl_mem.o: /usr/include/sys/cdefs.h
pcl_mem.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
pcl_mem.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
pcl_mem.o: /usr/include/stdlib.h /usr/include/errno.h
pcl_mem.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
pcl_mem.o: /usr/include/alloca.h /usr/include/ctype.h /usr/include/endian.h
pcl_mem.o: /usr/include/bytesex.h /usr/include/string.h
pcl_mem.o: /usr/include/sys/param.h /usr/include/limits.h
pcl_mem.o: /usr/include/posix1_lim.h /usr/include/linux/limits.h
pcl_mem.o: /usr/include/posix2_lim.h /usr/include/linux/param.h
pcl_mem.o: /usr/include/asm/param.h /usr/include/sys/types.h
pcl_mem.o: /usr/include/linux/types.h /usr/include/linux/posix_types.h
pcl_mem.o: /usr/include/asm/posix_types.h /usr/include/asm/types.h
pcl_mem.o: /usr/include/sys/bitypes.h systeminclude.h
pcl_parse.o: pcl_parse.h pcl_types.h pcl_scan.h pcl_strings.h pcl_mem.h
pcl_parse.o: pcl_defs.h /usr/include/malloc.h /usr/include/features.h
pcl_parse.o: /usr/include/sys/cdefs.h
pcl_parse.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
pcl_parse.o: /usr/include/stdio.h /usr/include/libio.h
pcl_parse.o: /usr/include/_G_config.h /usr/include/stdlib.h
pcl_parse.o: /usr/include/errno.h /usr/include/linux/errno.h
pcl_parse.o: /usr/include/asm/errno.h /usr/include/alloca.h
pcl_parse.o: /usr/include/ctype.h /usr/include/endian.h
pcl_parse.o: /usr/include/bytesex.h /usr/include/string.h
pcl_parse.o: /usr/include/sys/param.h /usr/include/limits.h
pcl_parse.o: /usr/include/posix1_lim.h /usr/include/linux/limits.h
pcl_parse.o: /usr/include/posix2_lim.h /usr/include/linux/param.h
pcl_parse.o: /usr/include/asm/param.h /usr/include/sys/types.h
pcl_parse.o: /usr/include/linux/types.h /usr/include/linux/posix_types.h
pcl_parse.o: /usr/include/asm/posix_types.h /usr/include/asm/types.h
pcl_parse.o: /usr/include/sys/bitypes.h systeminclude.h pcl_printer.h
pcl_parse.o: pcl_terms.h
pcl_printer.o: pcl_printer.h pcl_defs.h /usr/include/malloc.h
pcl_printer.o: /usr/include/features.h /usr/include/sys/cdefs.h
pcl_printer.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
pcl_printer.o: /usr/include/stdio.h /usr/include/libio.h
pcl_printer.o: /usr/include/_G_config.h /usr/include/stdlib.h
pcl_printer.o: /usr/include/errno.h /usr/include/linux/errno.h
pcl_printer.o: /usr/include/asm/errno.h /usr/include/alloca.h
pcl_printer.o: /usr/include/ctype.h /usr/include/endian.h
pcl_printer.o: /usr/include/bytesex.h /usr/include/string.h
pcl_printer.o: /usr/include/sys/param.h /usr/include/limits.h
pcl_printer.o: /usr/include/posix1_lim.h /usr/include/linux/limits.h
pcl_printer.o: /usr/include/posix2_lim.h /usr/include/linux/param.h
pcl_printer.o: /usr/include/asm/param.h /usr/include/sys/types.h
pcl_printer.o: /usr/include/linux/types.h /usr/include/linux/posix_types.h
pcl_printer.o: /usr/include/asm/posix_types.h /usr/include/asm/types.h
pcl_printer.o: /usr/include/sys/bitypes.h systeminclude.h
pcl_scan.o: pcl_scan.h pcl_strings.h pcl_mem.h pcl_defs.h
pcl_scan.o: /usr/include/malloc.h /usr/include/features.h
pcl_scan.o: /usr/include/sys/cdefs.h
pcl_scan.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
pcl_scan.o: /usr/include/stdio.h /usr/include/libio.h
pcl_scan.o: /usr/include/_G_config.h /usr/include/stdlib.h
pcl_scan.o: /usr/include/errno.h /usr/include/linux/errno.h
pcl_scan.o: /usr/include/asm/errno.h /usr/include/alloca.h
pcl_scan.o: /usr/include/ctype.h /usr/include/endian.h /usr/include/bytesex.h
pcl_scan.o: /usr/include/string.h /usr/include/sys/param.h
pcl_scan.o: /usr/include/limits.h /usr/include/posix1_lim.h
pcl_scan.o: /usr/include/linux/limits.h /usr/include/posix2_lim.h
pcl_scan.o: /usr/include/linux/param.h /usr/include/asm/param.h
pcl_scan.o: /usr/include/sys/types.h /usr/include/linux/types.h
pcl_scan.o: /usr/include/linux/posix_types.h /usr/include/asm/posix_types.h
pcl_scan.o: /usr/include/asm/types.h /usr/include/sys/bitypes.h
pcl_scan.o: systeminclude.h
pcl_strings.o: pcl_strings.h pcl_mem.h pcl_defs.h /usr/include/malloc.h
pcl_strings.o: /usr/include/features.h /usr/include/sys/cdefs.h
pcl_strings.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
pcl_strings.o: /usr/include/stdio.h /usr/include/libio.h
pcl_strings.o: /usr/include/_G_config.h /usr/include/stdlib.h
pcl_strings.o: /usr/include/errno.h /usr/include/linux/errno.h
pcl_strings.o: /usr/include/asm/errno.h /usr/include/alloca.h
pcl_strings.o: /usr/include/ctype.h /usr/include/endian.h
pcl_strings.o: /usr/include/bytesex.h /usr/include/string.h
pcl_strings.o: /usr/include/sys/param.h /usr/include/limits.h
pcl_strings.o: /usr/include/posix1_lim.h /usr/include/linux/limits.h
pcl_strings.o: /usr/include/posix2_lim.h /usr/include/linux/param.h
pcl_strings.o: /usr/include/asm/param.h /usr/include/sys/types.h
pcl_strings.o: /usr/include/linux/types.h /usr/include/linux/posix_types.h
pcl_strings.o: /usr/include/asm/posix_types.h /usr/include/asm/types.h
pcl_strings.o: /usr/include/sys/bitypes.h systeminclude.h
pcl_subst.o: pcl_subst.h pcl_parse.h pcl_types.h pcl_scan.h pcl_strings.h
pcl_subst.o: pcl_mem.h pcl_defs.h /usr/include/malloc.h
pcl_subst.o: /usr/include/features.h /usr/include/sys/cdefs.h
pcl_subst.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
pcl_subst.o: /usr/include/stdio.h /usr/include/libio.h
pcl_subst.o: /usr/include/_G_config.h /usr/include/stdlib.h
pcl_subst.o: /usr/include/errno.h /usr/include/linux/errno.h
pcl_subst.o: /usr/include/asm/errno.h /usr/include/alloca.h
pcl_subst.o: /usr/include/ctype.h /usr/include/endian.h
pcl_subst.o: /usr/include/bytesex.h /usr/include/string.h
pcl_subst.o: /usr/include/sys/param.h /usr/include/limits.h
pcl_subst.o: /usr/include/posix1_lim.h /usr/include/linux/limits.h
pcl_subst.o: /usr/include/posix2_lim.h /usr/include/linux/param.h
pcl_subst.o: /usr/include/asm/param.h /usr/include/sys/types.h
pcl_subst.o: /usr/include/linux/types.h /usr/include/linux/posix_types.h
pcl_subst.o: /usr/include/asm/posix_types.h /usr/include/asm/types.h
pcl_subst.o: /usr/include/sys/bitypes.h systeminclude.h pcl_printer.h
pcl_subst.o: pcl_terms.h
pcl_terms.o: pcl_terms.h pcl_types.h pcl_scan.h pcl_strings.h pcl_mem.h
pcl_terms.o: pcl_defs.h /usr/include/malloc.h /usr/include/features.h
pcl_terms.o: /usr/include/sys/cdefs.h
pcl_terms.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
pcl_terms.o: /usr/include/stdio.h /usr/include/libio.h
pcl_terms.o: /usr/include/_G_config.h /usr/include/stdlib.h
pcl_terms.o: /usr/include/errno.h /usr/include/linux/errno.h
pcl_terms.o: /usr/include/asm/errno.h /usr/include/alloca.h
pcl_terms.o: /usr/include/ctype.h /usr/include/endian.h
pcl_terms.o: /usr/include/bytesex.h /usr/include/string.h
pcl_terms.o: /usr/include/sys/param.h /usr/include/limits.h
pcl_terms.o: /usr/include/posix1_lim.h /usr/include/linux/limits.h
pcl_terms.o: /usr/include/posix2_lim.h /usr/include/linux/param.h
pcl_terms.o: /usr/include/asm/param.h /usr/include/sys/types.h
pcl_terms.o: /usr/include/linux/types.h /usr/include/linux/posix_types.h
pcl_terms.o: /usr/include/asm/posix_types.h /usr/include/asm/types.h
pcl_terms.o: /usr/include/sys/bitypes.h systeminclude.h pcl_printer.h
pcl_types.o: pcl_types.h pcl_scan.h pcl_strings.h pcl_mem.h pcl_defs.h
pcl_types.o: /usr/include/malloc.h /usr/include/features.h
pcl_types.o: /usr/include/sys/cdefs.h
pcl_types.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
pcl_types.o: /usr/include/stdio.h /usr/include/libio.h
pcl_types.o: /usr/include/_G_config.h /usr/include/stdlib.h
pcl_types.o: /usr/include/errno.h /usr/include/linux/errno.h
pcl_types.o: /usr/include/asm/errno.h /usr/include/alloca.h
pcl_types.o: /usr/include/ctype.h /usr/include/endian.h
pcl_types.o: /usr/include/bytesex.h /usr/include/string.h
pcl_types.o: /usr/include/sys/param.h /usr/include/limits.h
pcl_types.o: /usr/include/posix1_lim.h /usr/include/linux/limits.h
pcl_types.o: /usr/include/posix2_lim.h /usr/include/linux/param.h
pcl_types.o: /usr/include/asm/param.h /usr/include/sys/types.h
pcl_types.o: /usr/include/linux/types.h /usr/include/linux/posix_types.h
pcl_types.o: /usr/include/asm/posix_types.h /usr/include/asm/types.h
pcl_types.o: /usr/include/sys/bitypes.h systeminclude.h pcl_printer.h
