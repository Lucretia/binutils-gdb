/* Assembler interface for targets using CGEN. -*- C -*-
   CGEN: Cpu tools GENerator

This file is used to generate m32r-asm.c.

Copyright (C) 1996, 1997 Free Software Foundation, Inc.

This file is part of the GNU Binutils and GDB, the GNU debugger.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#include "sysdep.h"
#include <ctype.h>
#include <stdio.h>
#include "ansidecl.h"
#include "bfd.h"
#include "m32r-opc.h"

/* ??? The layout of this stuff is still work in progress.
   For speed in assembly/disassembly, we use inline functions.  That of course
   will only work for GCC.  When this stuff is finished, we can decide whether
   to keep the inline functions (and only get the performance increase when
   compiled with GCC), or switch to macros, or use something else.
*/

static const char * parse_insn_normal
     PARAMS ((const CGEN_INSN *, const char **, CGEN_FIELDS *));
static void insert_insn_normal
     PARAMS ((const CGEN_INSN *, CGEN_FIELDS *, cgen_insn_t *));

/* Default insertion routine.

   SHIFT is negative for left shifts, positive for right shifts.
   All bits of VALUE to be inserted must be valid as we don't handle
   signed vs unsigned shifts.

   ATTRS is a mask of the boolean attributes.  We don't need any at the
   moment, but for consistency with extract_normal we have them.  */

/* FIXME: This duplicates functionality with bfd's howto table and
   bfd_install_relocation.  */
/* FIXME: For architectures where insns can be representable as ints,
   store insn in `field' struct and add registers, etc. while parsing.  */

static CGEN_INLINE void
insert_normal (value, attrs, start, length, shift, total_length, buffer)
     long value;
     unsigned int attrs;
     int start, length, shift, total_length;
     char *buffer;
{
  bfd_vma x;

#if 0 /*def CGEN_INT_INSN*/
  *buffer |= ((value & ((1 << length) - 1))
	      << (total_length - (start + length)));
#else
  switch (total_length)
    {
    case 8:
      x = *(unsigned char *) buffer;
      break;
    case 16:
      if (CGEN_CURRENT_ENDIAN == CGEN_ENDIAN_BIG)
	x = bfd_getb16 (buffer);
      else
	x = bfd_getl16 (buffer);
      break;
    case 32:
      if (CGEN_CURRENT_ENDIAN == CGEN_ENDIAN_BIG)
	x = bfd_getb32 (buffer);
      else
	x = bfd_getl32 (buffer);
      break;
    default :
      abort ();
    }

  if (shift < 0)
    value <<= -shift;
  else
    value >>= shift;

  x |= ((value & ((1 << length) - 1))
	<< (total_length - (start + length)));

  switch (total_length)
    {
    case 8:
      *buffer = value;
      break;
    case 16:
      if (CGEN_CURRENT_ENDIAN == CGEN_ENDIAN_BIG)
	bfd_putb16 (x, buffer);
      else
	bfd_putl16 (x, buffer);
      break;
    case 32:
      if (CGEN_CURRENT_ENDIAN == CGEN_ENDIAN_BIG)
	bfd_putb32 (x, buffer);
      else
	bfd_putl32 (x, buffer);
      break;
    default :
      abort ();
    }
#endif
}

/* -- assembler routines inserted here */

/* Default insn parser.

   The syntax string is scanned and operands are parsed and stored in FIELDS.
   Relocs are queued as we go via other callbacks.

   ??? Note that this is currently an all-or-nothing parser.  If we fail to
   parse the instruction, we return 0 and the caller will start over from
   the beginning.  Backtracking will be necessary in parsing subexpressions,
   but that can be handled there.  Not handling backtracking here may get
   expensive in the case of the m68k.  Deal with later.

   Returns NULL for success, an error message for failure.
*/

static const char *
parse_insn_normal (insn, strp, fields)
     const CGEN_INSN *insn;
     const char **strp;
     CGEN_FIELDS *fields;
{
  const CGEN_SYNTAX *syntax = CGEN_INSN_SYNTAX (insn);
  const char *str = *strp;
  const char *errmsg;
  const char *p;
  const unsigned char *syn;
#ifdef CGEN_MNEMONIC_OPERANDS
  int past_opcode_p;
#endif

  /* For now we assume the mnemonic is first (there are no leading operands).
     We can parse it without needing to set up operand parsing.  */
  p = CGEN_INSN_MNEMONIC (insn);
  while (*p && *p == *str)
    ++p, ++str;
  if (*p || (*str && !isspace (*str)))
    return "unrecognized instruction";

  CGEN_INIT_PARSE ();
  cgen_init_parse_operand ();
#ifdef CGEN_MNEMONIC_OPERANDS
  past_opcode_p = 0;
#endif

  /* We don't check for (*str != '\0') here because we want to parse
     any trailing fake arguments in the syntax string.  */
  syn = CGEN_SYNTAX_STRING (CGEN_INSN_SYNTAX (insn));
  /* Mnemonics come first for now, ensure valid string.  */
  if (! CGEN_SYNTAX_MNEMONIC_P (*syn))
    abort ();
  ++syn;
  while (*syn != 0)
    {
      /* Non operand chars must match exactly.  */
      /* FIXME: Need to better handle whitespace.  */
      if (CGEN_SYNTAX_CHAR_P (*syn))
	{
	  if (*str == CGEN_SYNTAX_CHAR (*syn))
	    {
#ifdef CGEN_MNEMONIC_OPERANDS
	      if (*syn == ' ')
		past_opcode_p = 1;
#endif
	      ++syn;
	      ++str;
	    }
	  else
	    {
	      /* Syntax char didn't match.  Can't be this insn.  */
	      /* FIXME: would like to return "expected char `c'" */
	      return "syntax error";
	    }
	  continue;
	}

      /* We have an operand of some sort.  */
      errmsg = m32r_cgen_parse_operand (CGEN_SYNTAX_FIELD (*syn),
					 &str, fields);
      if (errmsg)
	return errmsg;

      /* Done with this operand, continue with next one.  */
      ++syn;
    }

  /* If we're at the end of the syntax string, we're done.  */
  if (*syn == '\0')
    {
      /* FIXME: For the moment we assume a valid `str' can only contain
	 blanks now.  IE: We needn't try again with a longer version of
	 the insn and it is assumed that longer versions of insns appear
	 before shorter ones (eg: lsr r2,r3,1 vs lsr r2,r3).  */
      while (isspace (*str))
	++str;

      if (*str != '\0')
	return "junk at end of line"; /* FIXME: would like to include `str' */

      return NULL;
    }

  /* We couldn't parse it.  */
  return "unrecognized instruction";
}

/* Default insn builder (insert handler).
   The instruction is recorded in target byte order.  */

static void
insert_insn_normal (insn, fields, buffer)
     const CGEN_INSN *insn;
     CGEN_FIELDS *fields;
     cgen_insn_t *buffer;
{
  const CGEN_SYNTAX *syntax = CGEN_INSN_SYNTAX (insn);
  bfd_vma value;
  const unsigned char *syn;

  CGEN_INIT_INSERT ();
  value = CGEN_INSN_VALUE (insn);

  /* If we're recording insns as numbers (rather than a string of bytes),
     target byte order handling is deferred until later.  */
#undef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#if 0 /*def CGEN_INT_INSN*/
  *buffer = value;
#else
  switch (min (CGEN_BASE_INSN_BITSIZE, CGEN_FIELDS_BITSIZE (fields)))
    {
    case 8:
      *buffer = value;
      break;
    case 16:
      if (CGEN_CURRENT_ENDIAN == CGEN_ENDIAN_BIG)
	bfd_putb16 (value, (char *) buffer);
      else
	bfd_putl16 (value, (char *) buffer);
      break;
    case 32:
      if (CGEN_CURRENT_ENDIAN == CGEN_ENDIAN_BIG)
	bfd_putb32 (value, (char *) buffer);
      else
	bfd_putl32 (value, (char *) buffer);
      break;
    default:
      abort ();
    }
#endif

  /* ??? Rather than scanning the syntax string again, we could store
     in `fields' a null terminated list of the fields that are present.  */

  for (syn = CGEN_SYNTAX_STRING (syntax); *syn != '\0'; ++syn)
    {
      if (CGEN_SYNTAX_CHAR_P (*syn))
	continue;

      m32r_cgen_insert_operand (CGEN_SYNTAX_FIELD (*syn), fields, buffer);
    }
}

/* Main entry point.
   This routine is called for each instruction to be assembled.
   STR points to the insn to be assembled.
   We assume all necessary tables have been initialized.
   The result is a pointer to the insn's entry in the opcode table,
   or NULL if an error occured (an error message will have already been
   printed).  */

const CGEN_INSN *
m32r_cgen_assemble_insn (str, fields, buf, errmsg)
     const char *str;
     CGEN_FIELDS *fields;
     cgen_insn_t *buf;
     char **errmsg;
{
  const char *start;
  CGEN_INSN_LIST *ilist;

  /* Skip leading white space.  */
  while (isspace (*str))
    ++str;

  /* The instructions are stored in hashed lists.
     Get the first in the list.  */
  ilist = CGEN_ASM_LOOKUP_INSN (str);

  /* Keep looking until we find a match.  */

  start = str;
  for ( ; ilist != NULL ; ilist = CGEN_ASM_NEXT_INSN (ilist))
    {
      const CGEN_INSN *insn = ilist->insn;

#if 0 /* not needed as unsupported opcodes shouldn't be in the hash lists */
      /* Is this insn supported by the selected cpu?  */
      if (! m32r_cgen_insn_supported (insn))
	continue;
#endif

#if 1 /* FIXME: wip */
      /* If the RELAX attribute is set, this is an insn that shouldn't be
	 chosen immediately.  Instead, it is used during assembler/linker
	 relaxation if possible.  */
      if (CGEN_INSN_ATTR (insn, CGEN_INSN_RELAX) != 0)
	continue;
#endif

      str = start;

      /* Record a default length for the insn.  This will get set to the
	 correct value while parsing.  */
      /* FIXME: wip */
      CGEN_FIELDS_BITSIZE (fields) = CGEN_INSN_BITSIZE (insn);

      /* ??? The extent to which moving the parse and insert handlers into
         this function (thus removing the function call) will speed things up
	 is unclear.  The simplicity and flexibility of the current scheme is
	 appropriate for now.  One could have the best of both worlds with
	 inline functions but of course that would only work for gcc.  Since
	 we're machine generating some code we could do that here too.  Maybe
	 later.  */
      if (! (*CGEN_PARSE_FN (insn)) (insn, &str, fields))
	{
	  (*CGEN_INSERT_FN (insn)) (insn, fields, buf);
	  /* It is up to the caller to actually output the insn and any
	     queued relocs.  */
	  return insn;
	}

      /* Try the next entry.  */
    }

  /* FIXME: We can return a better error message than this.
     Need to track why it failed and pick the right one.  */
  {
    static char errbuf[100];
    sprintf (errbuf, "bad instruction `%.50s%s'",
	     start, strlen (start) > 50 ? "..." : "");
    *errmsg = errbuf;
    return NULL;
  }
}

#if 0 /* This calls back to GAS which we can't do without care.  */

/* Record each member of OPVALS in the assembler's symbol table.
   This lets GAS parse registers for us.
   ??? Interesting idea but not currently used.  */

/* Record each member of OPVALS in the assembler's symbol table.
   FIXME: Not currently used.  */

void
m32r_cgen_asm_hash_keywords (opvals)
     CGEN_KEYWORD *opvals;
{
  CGEN_KEYWORD_SEARCH search = cgen_keyword_search_init (opvals, NULL);
  const CGEN_KEYWORD_ENTRY *ke;

  while ((ke = cgen_keyword_search_next (&search)) != NULL)
    {
#if 0 /* Unnecessary, should be done in the search routine.  */
      if (! m32r_cgen_opval_supported (ke))
	continue;
#endif
      cgen_asm_record_register (ke->name, ke->value);
    }
}

#endif /* 0 */
