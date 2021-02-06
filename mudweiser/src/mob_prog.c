/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
 *  The MOBprograms have been contributed by N'Atas-ha.  Any support for   *
 *  these routines should not be expected from Merc Industries.  However,  *
 *  under no circumstances should the blame for bugs, etc be placed on     *
 *  Merc Industries.  They are not guaranteed to work on all systems due   *
 *  to their frequent use of strxxx functions.  They are also not the most *
 *  efficient way to perform their tasks, but hopefully should be in the   *
 *  easiest possible way to install and begin using. Documentation for     *
 *  such installation can be found in INSTALL.  Enjoy...         N'Atas-Ha *
 ***************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "merc.h"

/*
 * Local function prototypes
 */

char *	mprog_next_command	args( ( char* clist ) );
bool	mprog_seval		args( ( char* lhs, char* opr, char* rhs ) );
bool	mprog_veval		args( ( int lhs, char* opr, int rhs ) );
long	mprog_do_ifchck		args( ( char* ifchck, CHAR_DATA* mob,
				       CHAR_DATA* actor, OBJ_DATA* obj,
				       void* vo, CHAR_DATA* rndm ) );
char *	mprog_process_if	args( ( char* ifchck, char* com_list, 
				       CHAR_DATA* mob, CHAR_DATA* actor,
				       OBJ_DATA* obj, void* vo,
				       CHAR_DATA* rndm ) );
void	mprog_translate		args( ( char ch, char* t, CHAR_DATA* mob,
				       CHAR_DATA* actor, OBJ_DATA* obj,
				       void* vo, CHAR_DATA* rndm ) );
void	mprog_process_cmnd	args( ( char* cmnd, CHAR_DATA* mob, 
				       CHAR_DATA* actor, OBJ_DATA* obj,
				       void* vo, CHAR_DATA* rndm ) );
void	mprog_driver		args( ( char* com_list, CHAR_DATA* mob,
				       CHAR_DATA* actor, OBJ_DATA* obj,
				       void* vo ) );

/***************************************************************************
 * Local function code and brief comments.
 */

/* if you dont have these functions, you damn well should... */

#ifdef DUNNO_STRSTR
char * strstr(s1,s2) const char *s1; const char *s2;
{
  char *cp;
  int i,j=strlen(s1)-strlen(s2),k=strlen(s2);
  if(j<0)
    return NULL;
  for(i=0; i<=j && strncmp(s1++,s2, k)!=0; i++);
  return (i>j) ? NULL : (s1-1);
}
#endif

/* Used to get sequential lines of a multi line string (separated by "\n\r")
 * Thus its like one_argument(), but a trifle different. It is destructive
 * to the multi line string argument, and thus clist must not be shared.
 */
char *mprog_next_command( char *clist )
{

  char *pointer = clist;

  if ( !pointer )
	return NULL;

  while ( *pointer != '\n' && *pointer != '\0' )
    pointer++;
  if ( *pointer == '\n' )
    *pointer++ = '\0';
  if ( *pointer == '\r' )
    *pointer++ = '\0';

  return ( pointer );

}

/* These two functions do the basic evaluation of ifcheck operators.
 *  It is important to note that the string operations are not what
 *  you probably expect.  Equality is exact and division is substring.
 *  remember that lhs has been stripped of leading space, but can
 *  still have trailing spaces so be careful when editing since:
 *  "guard" and "guard " are not equal.
 */
bool mprog_seval( char *lhs, char *opr, char *rhs )
{

  if ( !str_cmp( opr, "==" ) )
    return ( bool )( !str_cmp( lhs, rhs ) );
  if ( !str_cmp( opr, "!=" ) )
    return ( bool )( str_cmp( lhs, rhs ) );
  if ( !str_cmp( opr, "/" ) )
    return ( bool )( !str_infix( rhs, lhs ) );
  if ( !str_cmp( opr, "!/" ) )
    return ( bool )( str_infix( rhs, lhs ) );

  bug ( "Improper MOBprog operator\n\r", 0 );
  return 0;

}

bool mprog_veval( int lhs, char *opr, int rhs )
{

  if ( !str_cmp( opr, "==" ) )
    return ( lhs == rhs );
  if ( !str_cmp( opr, "!=" ) )
    return ( lhs != rhs );
  if ( !str_cmp( opr, ">" ) )
    return ( lhs > rhs );
  if ( !str_cmp( opr, "<" ) )
    return ( lhs < rhs );
  if ( !str_cmp( opr, ">=" ) )
    return ( lhs <= rhs );
  if ( !str_cmp( opr, ">=" ) )
    return ( lhs >= rhs );
  if ( !str_cmp( opr, "&" ) )
    return ( lhs & rhs );
  if ( !str_cmp( opr, "|" ) )
    return ( lhs | rhs );

  bug ( "Improper MOBprog operator\n\r", 0 );
  return 0;

}

/* This function performs the evaluation of the if checks.  It is
 * here that you can add any ifchecks which you so desire. Hopefully
 * it is clear from what follows how one would go about adding your
 * own. The syntax for an if check is: ifchck ( arg ) [opr val]
 * where the parenthesis are required and the opr and val fields are
 * optional but if one is there then both must be. The spaces are all
 * optional. The evaluation of the opr expressions is farmed out
 * to reduce the redundancy of the mammoth if statement list.
 * If there are errors, then return -1 otherwise return boolean 1,0
 */
long mprog_do_ifchck( char *ifchck, CHAR_DATA *mob, CHAR_DATA *actor,
		     OBJ_DATA *obj, void *vo, CHAR_DATA *rndm)
{

  char buf[ MAX_INPUT_LENGTH ];
  char arg[ MAX_INPUT_LENGTH ];
  char opr[ MAX_INPUT_LENGTH ];
  char val[ MAX_INPUT_LENGTH ];
  CHAR_DATA *vict = (CHAR_DATA *) vo;
  OBJ_DATA *v_obj = (OBJ_DATA  *) vo;
  char     *bufpt = buf;
  char     *argpt = arg;
  char     *oprpt = opr;
  char     *valpt = val;
  char     *point = ifchck;
  int       lhsvl;
  int       rhsvl;

  if ( *point == '\0' ) 
    {
      bug ( "Mob: %d null ifchck", mob->pIndexData->vnum ); 
      return -1;
    }   
  /* skip leading spaces */
  while ( *point == ' ' )
    point++;

  /* get whatever comes before the left paren.. ignore spaces */
  while ( *point != '(' ) 
    if ( *point == '\0' ) 
      {
	bug ( "Mob: %d ifchck syntax error", mob->pIndexData->vnum ); 
	return -1;
      }   
    else
      if ( *point == ' ' )
	point++;
      else 
	*bufpt++ = *point++; 

  *bufpt = '\0';
  point++;

  /* get whatever is in between the parens.. ignore spaces */
  while ( *point != ')' ) 
    if ( *point == '\0' ) 
      {
	bug ( "Mob: %d ifchck syntax error", mob->pIndexData->vnum ); 
	return -1;
      }   
    else
      if ( *point == ' ' )
	point++;
      else 
	*argpt++ = *point++; 

  *argpt = '\0';
  point++;

  /* check to see if there is an operator */
  while ( *point == ' ' )
    point++;
  if ( *point == '\0' ) 
    {
      *opr = '\0';
      *val = '\0';
    }   
  else /* there should be an operator and value, so get them */
    {
      while ( ( *point != ' ' ) && ( !isalnum( *point ) ) ) 
	if ( *point == '\0' ) 
	  {
	    bug ( "Mob: %d ifchck operator without value",
		 mob->pIndexData->vnum ); 
	    return -1;
	  }   
	else
	  *oprpt++ = *point++; 

      *oprpt = '\0';
 
      /* finished with operator, skip spaces and then get the value */
      while ( *point == ' ' )
	point++;
      for( ; ; )
	{
	  if ( ( *point != ' ' ) && ( *point == '\0' ) )
	    break;
	  else
	    *valpt++ = *point++; 
	}

      *valpt = '\0';
    }
  bufpt = buf;
  argpt = arg;
  oprpt = opr;
  valpt = val;

  /* Ok... now buf contains the ifchck, arg contains the inside of the
   *  parentheses, opr contains an operator if one is present, and val
   *  has the value if an operator was present.
   *  So.. basically use if statements and run over all known ifchecks
   *  Once inside, use the argument and expand the lhs. Then if need be
   *  send the lhs,opr,rhs off to be evaluated.
   */


  if ( !str_cmp( buf, "memory" ) )
     {  /* if memory() returns TRUE if anything is in memory      */
	/* if memory() == $* returns TRUE if memory ==,!=,/,!/ $* */
	if ( mob->memory == NULL ) return FALSE;
	else if (get_char_world(mob,mob->memory->name) == NULL) return FALSE;
	if (opr[0]=='\0') return TRUE;
	if (val[0]=='$') switch (val[1])
	  {
	   case 'i': return FALSE; /* can't remember myself */
	   case 'n': if (actor)
	       return mprog_seval(actor->name,opr,mob->memory->name);
	       else return -1;
	   case 't': if (vict)
	       return mprog_seval(vict->name,opr,mob->memory->name);
	       else return -1;
	   case 'r': if (rndm)
	       return mprog_seval(rndm->name,opr,mob->memory->name);
	       else return -1;
	   default : bug("Mob: %d bad val for 'if memory'",mob->pIndexData->vnum);
	             return -1;
	  }
	return mprog_seval(val,opr,mob->memory->name);
     }

   if ( !str_cmp( buf, "rand" ) )
    {
      return ( number_percent() <= atoi(arg) );
    }

  if ( !str_cmp( buf, "ispc" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': return 0;
	case 'n': if ( actor )
 	             return ( !IS_NPC( actor ) );
	          else return -1;
	case 't': if ( vict )
                     return ( !IS_NPC( vict ) );
	          else return -1;
	case 'r': if ( rndm )
                     return ( !IS_NPC( rndm ) );
	          else return -1;
	default:
	  bug ( "Mob: %d bad argument to 'ispc'", mob->pIndexData->vnum ); 
	  return -1;
	}
    }

  if ( !str_cmp( buf, "isnpc" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': return 1;
	case 'n': if ( actor )
	             return IS_NPC( actor );
	          else return -1;
	case 't': if ( vict )
                     return IS_NPC( vict );
	          else return -1;
	case 'r': if ( rndm )
	             return IS_NPC( rndm );
	          else return -1;
	default:
	  bug ("Mob: %d bad argument to 'isnpc'", mob->pIndexData->vnum ); 
	  return -1;
	}
    }

  if ( !str_cmp( buf, "isgood" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': return IS_GOOD( mob );
	case 'n': if ( actor )
	             return IS_GOOD( actor );
	          else return -1;
	case 't': if ( vict )
	             return IS_GOOD( vict );
	          else return -1;
	case 'r': if ( rndm )
	             return IS_GOOD( rndm );
	          else return -1;
	default:
	  bug ( "Mob: %d bad argument to 'isgood'", mob->pIndexData->vnum ); 
	  return -1;
	}
    }

  if ( !str_cmp( buf, "isfight" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': return ( mob->fighting ) ? 1 : 0;
	case 'n': if ( actor )
	             return ( actor->fighting ) ? 1 : 0;
	          else return -1;
	case 't': if ( vict )
	             return ( vict->fighting ) ? 1 : 0;
	          else return -1;
	case 'r': if ( rndm )
	             return ( rndm->fighting ) ? 1 : 0;
	          else return -1;
	default:
	  bug ( "Mob: %d bad argument to 'isfight'", mob->pIndexData->vnum ); 
	  return -1;
	}
    }

  if ( !str_cmp( buf, "isimmort" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': return ( get_trust( mob ) > LEVEL_IMMORTAL );
	case 'n': if ( actor )
	             return ( get_trust( actor ) > LEVEL_IMMORTAL );
  	          else return -1;
	case 't': if ( vict )
	             return ( get_trust( vict ) > LEVEL_IMMORTAL );
                  else return -1;
	case 'r': if ( rndm )
	             return ( get_trust( rndm ) > LEVEL_IMMORTAL );
                  else return -1;
	default:
	  bug ( "Mob: %d bad argument to 'isimmort'", mob->pIndexData->vnum ); 
	  return -1;
	}
    }

  if ( !str_cmp( buf, "ischarmed" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': return IS_SET( mob->affected_by , AFF_CHARM );
	case 'n': if ( actor )
	             return IS_AFFECTED( actor, AFF_CHARM );
	          else return -1;
	case 't': if ( vict )
	             return IS_AFFECTED( vict, AFF_CHARM );
	          else return -1;
	case 'r': if ( rndm )
	             return IS_AFFECTED( rndm, AFF_CHARM );
	          else return -1;
	default:
	  bug ( "Mob: %d bad argument to 'ischarmed'",
	       mob->pIndexData->vnum ); 
	  return -1;
	}
    }

  if ( !str_cmp( buf, "isfollow" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': return ( mob->master != NULL
			  && mob->master->in_room == mob->in_room );
	case 'n': if ( actor )
	             return ( actor->master != NULL
			     && actor->master->in_room == actor->in_room );
	          else return -1;
	case 't': if ( vict )
	             return ( vict->master != NULL
			     && vict->master->in_room == vict->in_room );
	          else return -1;
	case 'r': if ( rndm )
	             return ( rndm->master != NULL
			     && rndm->master->in_room == rndm->in_room );
	          else return -1;
	default:
	  bug ( "Mob: %d bad argument to 'isfollow'", mob->pIndexData->vnum ); 
	  return -1;
	}
    }

  if ( !str_cmp( buf, "isaffected" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': return ( mob->affected_by & atoi( arg ) );
	case 'n': if ( actor )
	             return ( actor->affected_by & atoi( arg ) );
	          else return -1;
	case 't': if ( vict )
	             return ( vict->affected_by & atoi( arg ) );
	          else return -1;
	case 'r': if ( rndm )
	             return ( rndm->affected_by & atoi( arg ) );
	          else return -1;
	default:
	  bug ( "Mob: %d bad argument to 'isaffected'",
	       mob->pIndexData->vnum ); 
	  return -1;
	}
    }

  if ( !str_cmp( buf, "hitprcnt" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': lhsvl = mob->hit / mob->max_hit;
	          rhsvl = atoi( val );
         	  return mprog_veval( lhsvl, opr, rhsvl );
	case 'n': if ( actor )
	          {
		    lhsvl = actor->hit / actor->max_hit;
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 't': if ( vict )
	          {
		    lhsvl = vict->hit / vict->max_hit;
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 'r': if ( rndm )
	          {
		    lhsvl = rndm->hit / rndm->max_hit;
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	default:
	  bug ( "Mob: %d bad argument to 'hitprcnt'", mob->pIndexData->vnum ); 
	  return -1;
	}
    }

  if ( !str_cmp( buf, "inroom" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': lhsvl = mob->in_room->vnum;
	          rhsvl = atoi(val);
	          return mprog_veval( lhsvl, opr, rhsvl );
	case 'n': if ( actor )
	          {
		    lhsvl = actor->in_room->vnum;
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 't': if ( vict )
	          {
		    lhsvl = vict->in_room->vnum;
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 'r': if ( rndm )
	          {
		    lhsvl = rndm->in_room->vnum;
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	default:
	  bug ( "Mob: %d bad argument to 'inroom'", mob->pIndexData->vnum ); 
	  return -1;
	}
    }

  if ( !str_cmp( buf, "sex" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': lhsvl = mob->sex;
	          rhsvl = atoi( val );
	          return mprog_veval( lhsvl, opr, rhsvl );
	case 'n': if ( actor )
	          {
		    lhsvl = actor->sex;
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 't': if ( vict )
	          {
		    lhsvl = vict->sex;
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 'r': if ( rndm )
	          {
		    lhsvl = rndm->sex;
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	default:
	  bug ( "Mob: %d bad argument to 'sex'", mob->pIndexData->vnum ); 
	  return -1;
	}
    }

  if ( !str_cmp( buf, "position" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': lhsvl = mob->position;
	          rhsvl = atoi( val );
	          return mprog_veval( lhsvl, opr, rhsvl );
	case 'n': if ( actor )
	          {
		    lhsvl = actor->position;
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 't': if ( vict )
	          {
		    lhsvl = vict->position;
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 'r': if ( rndm )
	          {
		    lhsvl = rndm->position;
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	default:
	  bug ( "Mob: %d bad argument to 'position'", mob->pIndexData->vnum ); 
	  return -1;
	}
    }

  if ( !str_cmp( buf, "level" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': lhsvl = get_trust( mob );
	          rhsvl = atoi( val );
	          return mprog_veval( lhsvl, opr, rhsvl );
	case 'n': if ( actor )
	          {
		    lhsvl = get_trust( actor );
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else 
		    return -1;
	case 't': if ( vict )
	          {
		    lhsvl = get_trust( vict );
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 'r': if ( rndm )
	          {
		    lhsvl = get_trust( rndm );
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	default:
	  bug ( "Mob: %d bad argument to 'level'", mob->pIndexData->vnum ); 
	  return -1;
	}
    }

  if ( !str_cmp( buf, "class" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': lhsvl = mob->class;
	          rhsvl = atoi( val );
                  return mprog_veval( lhsvl, opr, rhsvl );
	case 'n': if ( actor )
	          {
		    lhsvl = actor->class;
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else 
		    return -1;
	case 't': if ( vict )
	          {
		    lhsvl = vict->class;
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 'r': if ( rndm )
	          {
		    lhsvl = rndm->class;
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	default:
	  bug ( "Mob: %d bad argument to 'class'", mob->pIndexData->vnum ); 
	  return -1;
	}
    }

  if ( !str_cmp( buf, "goldamt" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': lhsvl = mob->gold;
                  rhsvl = atoi( val );
                  return mprog_veval( lhsvl, opr, rhsvl );
	case 'n': if ( actor )
	          {
		    lhsvl = actor->gold;
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 't': if ( vict )
	          {
		    lhsvl = vict->gold;
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 'r': if ( rndm )
	          {
		    lhsvl = rndm->gold;
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	default:
	  bug ( "Mob: %d bad argument to 'goldamt'", mob->pIndexData->vnum ); 
	  return -1;
	}
    }

  if ( !str_cmp( buf, "objtype" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'o': if ( obj )
	          {
		    lhsvl = obj->item_type;
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	         else
		   return -1;
	case 'p': if ( v_obj )
	          {
		    lhsvl = v_obj->item_type;
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	default:
	  bug ( "Mob: %d bad argument to 'objtype'", mob->pIndexData->vnum ); 
	  return -1;
	}
    }

  if ( !str_cmp( buf, "objval0" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'o': if ( obj )
	          {
		    lhsvl = obj->value[0];
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 'p': if ( v_obj )
	          {
		    lhsvl = v_obj->value[0];
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else 
		    return -1;
	default:
	  bug ( "Mob: %d bad argument to 'objval0'", mob->pIndexData->vnum ); 
	  return -1;
	}
    }

  if ( !str_cmp( buf, "objval1" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'o': if ( obj )
	          {
		    lhsvl = obj->value[1];
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 'p': if ( v_obj )
	          {
		    lhsvl = v_obj->value[1];
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	default:
	  bug ( "Mob: %d bad argument to 'objval1'", mob->pIndexData->vnum ); 
	  return -1;
	}
    }

  if ( !str_cmp( buf, "objval2" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'o': if ( obj )
	          {
		    lhsvl = obj->value[2];
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 'p': if ( v_obj )
	          {
		    lhsvl = v_obj->value[2];
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	default:
	  bug ( "Mob: %d bad argument to 'objval2'", mob->pIndexData->vnum ); 
	  return -1;
	}
    }

  if ( !str_cmp( buf, "objval3" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'o': if ( obj )
	          {
		    lhsvl = obj->value[3];
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 'p': if ( v_obj ) 
	          {
		    lhsvl = v_obj->value[3];
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	default:
	  bug ( "Mob: %d bad argument to 'objval3'", mob->pIndexData->vnum ); 
	  return -1;
	}
    }

  if ( !str_cmp( buf, "number" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': lhsvl = mob->gold;
	          rhsvl = atoi( val );
	          return mprog_veval( lhsvl, opr, rhsvl );
	case 'n': if ( actor )
	          {
		    if IS_NPC( actor )
		    {
		      lhsvl = actor->pIndexData->vnum;
		      rhsvl = atoi( val );
		      return mprog_veval( lhsvl, opr, rhsvl );
		    }
		  }
	          else
		    return -1;
	case 't': if ( vict )
	          {
		    if IS_NPC( actor )
		    {
		      lhsvl = vict->pIndexData->vnum;
		      rhsvl = atoi( val );
		      return mprog_veval( lhsvl, opr, rhsvl );
		    }
		  }
                  else
		    return -1;
	case 'r': if ( rndm )
	          {
		    if IS_NPC( actor )
		    {
		      lhsvl = rndm->pIndexData->vnum;
		      rhsvl = atoi( val );
		      return mprog_veval( lhsvl, opr, rhsvl );
		    }
		  }
	         else return -1;
	case 'o': if ( obj )
	          {
		    lhsvl = obj->pIndexData->vnum;
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 'p': if ( v_obj )
	          {
		    lhsvl = v_obj->pIndexData->vnum;
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	default:
	  bug ( "Mob: %d bad argument to 'number'", mob->pIndexData->vnum ); 
	  return -1;
	}
    }

  if ( !str_cmp( buf, "name" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': return mprog_seval( mob->name, opr, val );
	case 'n': if ( actor )
	            return mprog_seval( actor->name, opr, val );
	          else
		    return -1;
	case 't': if ( vict )
	            return mprog_seval( vict->name, opr, val );
	          else
		    return -1;
	case 'r': if ( rndm )
	            return mprog_seval( rndm->name, opr, val );
	          else
		    return -1;
	case 'o': if ( obj )
	            return mprog_seval( obj->name, opr, val );
	          else
		    return -1;
	case 'p': if ( v_obj )
	            return mprog_seval( v_obj->name, opr, val );
	          else
		    return -1;
	default:
	  bug ( "Mob: %d bad argument to 'name'", mob->pIndexData->vnum ); 
	  return -1;
	}
    }

  /* Ok... all the ifchcks are done, so if we didnt find ours then something
   * odd happened.  So report the bug and abort the MOBprogram (return error)
   */
  bug ( "Mob: %d unknown ifchck", mob->pIndexData->vnum ); 
  return -1;

}
/* Quite a long and arduous function, this guy handles the control
 * flow part of MOBprograms.  Basicially once the driver sees an
 * 'if' attention shifts to here.  While many syntax errors are
 * caught, some will still get through due to the handling of break
 * and errors in the same fashion.  The desire to break out of the
 * recursion without catastrophe in the event of a mis-parse was
 * believed to be high. Thus, if an error is found, it is bugged and
 * the parser acts as though a break were issued and just bails out
 * at that point. I havent tested all the possibilites, so I'm speaking
 * in theory, but it is 'guaranteed' to work on syntactically correct
 * MOBprograms, so if the mud crashes here, check the mob carefully!
 */
char *mprog_process_if( char *ifchck, char *com_list, CHAR_DATA *mob,
		       CHAR_DATA *actor, OBJ_DATA *obj, void *vo,
		       CHAR_DATA *rndm )
{

 char null[ 1 ];
 char buf[ MAX_INPUT_LENGTH ];
 char *morebuf = '\0';
 char    *cmnd = '\0';
 bool loopdone = FALSE;
 bool     flag = FALSE;
 long  legal;

 *null = '\0';

 /* check for trueness of the ifcheck */
 if ( ( legal = mprog_do_ifchck( ifchck, mob, actor, obj, vo, rndm ) ) )
   if ( legal == 1 )
     flag = TRUE;
   else
     return '\0';

 while( loopdone == FALSE ) /*scan over any existing or statements */
 {
     cmnd     = com_list;
     com_list = mprog_next_command( com_list );
     while ( *cmnd == ' ' )
       cmnd++;
     if ( *cmnd == '\0' )
     {
	 bug ( "Mob: %d no commands after IF/OR", mob->pIndexData->vnum ); 
	 return '\0';
     }
     morebuf = one_argument( cmnd, buf );
     if ( !str_cmp( buf, "or" ) )
     {
	 if ( ( legal = mprog_do_ifchck( morebuf,mob,actor,obj,vo,rndm ) ) )
	   if ( legal == 1 )
	     flag = TRUE;
	   else
	     return '\0';
     }
     else
       loopdone = TRUE;
 }
 
 if ( flag )
   for ( ; ; ) /*ifcheck was true, do commands but ignore else to endif*/ 
   {
       if ( !str_cmp( buf, "if" ) )
       { 
	   com_list = mprog_process_if(morebuf,com_list,mob,actor,obj,vo,rndm);
	   while ( *cmnd==' ' )
	     cmnd++;
	   if ( *com_list == '\0' )
	     return '\0';
	   cmnd     = com_list;
	   com_list = mprog_next_command( com_list );
	   morebuf  = one_argument( cmnd,buf );
	   continue;
       }
       if ( !str_cmp( buf, "break" ) )
	 return '\0';
       if ( !str_cmp( buf, "endif" ) )
	 return com_list; 
       if ( !str_cmp( buf, "else" ) ) 
       {
	   while ( str_cmp( buf, "endif" ) ) 
	   {
	       cmnd     = com_list;
	       com_list = mprog_next_command( com_list );
	       while ( *cmnd == ' ' )
		 cmnd++;
	       if ( *cmnd == '\0' )
	       {
		   bug ( "Mob: %d missing endif after else",
			mob->pIndexData->vnum );
		   return '\0';
	       }
	       morebuf = one_argument( cmnd,buf );
	   }
	   return com_list; 
       }
       mprog_process_cmnd( cmnd, mob, actor, obj, vo, rndm );
       cmnd     = com_list;
       com_list = mprog_next_command( com_list );
       while ( *cmnd == ' ' )
	 cmnd++;
       if ( *cmnd == '\0' )
       {
           bug ( "Mob: %d missing else or endif", mob->pIndexData->vnum ); 
           return '\0';
       }
       morebuf = one_argument( cmnd, buf );
   }
 else /*false ifcheck, find else and do existing commands or quit at endif*/
   {
     while ( ( str_cmp( buf, "else" ) ) && ( str_cmp( buf, "endif" ) ) )
       {
	 cmnd     = com_list;
	 com_list = mprog_next_command( com_list );
	 while ( *cmnd == ' ' )
	   cmnd++;
	 if ( *cmnd == '\0' )
	   {
	     bug ( "Mob: %d missing an else or endif",
		  mob->pIndexData->vnum ); 
	     return '\0';
	   }
	 morebuf = one_argument( cmnd, buf );
       }

     /* found either an else or an endif.. act accordingly */
     if ( !str_cmp( buf, "endif" ) )
       return com_list;
     cmnd     = com_list;
     com_list = mprog_next_command( com_list );
     while ( *cmnd == ' ' )
       cmnd++;
     if ( *cmnd == '\0' )
       { 
	 bug ( "Mob: %d missing endif", mob->pIndexData->vnum ); 
	 return '\0';
       }
     morebuf = one_argument( cmnd, buf );
     
     for ( ; ; ) /*process the post-else commands until an endif is found.*/
       {
	 if ( !str_cmp( buf, "if" ) )
	   { 
	     com_list = mprog_process_if( morebuf, com_list, mob, actor,
					 obj, vo, rndm );
	     while ( *cmnd == ' ' )
	       cmnd++;
	     if ( *com_list == '\0' )
	       return '\0';
	     cmnd     = com_list;
	     com_list = mprog_next_command( com_list );
	     morebuf  = one_argument( cmnd,buf );
	     continue;
	   }
	 if ( !str_cmp( buf, "else" ) ) 
	   {
	     bug ( "Mob: %d found else in an else section",
		  mob->pIndexData->vnum ); 
	     return '\0';
	   }
	 if ( !str_cmp( buf, "break" ) )
	   return '\0';
	 if ( !str_cmp( buf, "endif" ) )
	   return com_list; 
	 mprog_process_cmnd( cmnd, mob, actor, obj, vo, rndm );
	 cmnd     = com_list;
	 com_list = mprog_next_command( com_list );
	 while ( *cmnd == ' ' )
	   cmnd++;
	 if ( *cmnd == '\0' )
	   {
	     bug ( "Mob:%d missing endif in else section",
		  mob->pIndexData->vnum ); 
	     return '\0';
	   }
	 morebuf = one_argument( cmnd, buf );
       }
   }
}

/* This routine handles the variables for command expansion.
 * If you want to add any go right ahead, it should be fairly
 * clear how it is done and they are quite easy to do, so you
 * can be as creative as you want. The only catch is to check
 * that your variables exist before you use them. At the moment,
 * using $t when the secondary target refers to an object 
 * i.e. >prog_act drops~<nl>if ispc($t)<nl>sigh<nl>endif<nl>~<nl>
 * probably makes the mud crash (vice versa as well) The cure
 * would be to change act() so that vo becomes vict & v_obj.
 * but this would require a lot of small changes all over the code.
 */
void mprog_translate( char ch, char *t, CHAR_DATA *mob, CHAR_DATA *actor,
                    OBJ_DATA *obj, void *vo, CHAR_DATA *rndm )
{
 static char *he_she        [] = { "it",  "he",  "she" };
 static char *him_her       [] = { "it",  "him", "her" };
 static char *his_her       [] = { "its", "his", "her" };
 CHAR_DATA   *vict             = (CHAR_DATA *) vo;
 OBJ_DATA    *v_obj            = (OBJ_DATA  *) vo;

 *t = '\0';
 switch ( ch ) {
     case 'i':
         one_argument( mob->name, t );
      break;

     case 'I':
         strcpy( t, mob->short_descr );
      break;

     case 'n':
         if ( actor )  
	{
	   if ( can_see( mob,actor ) )
	     one_argument( actor->name, t );
         if ( !IS_NPC( actor ) )
	   *t = UPPER( *t );
	}
   	 else
   	 {
   	     bug( "Mprog_Translate: case 'n', NULL actor", 0 );
   	     strcpy( t, "someone" );
   	 }
      break;

     case 'N':
         if ( actor ) 
            if ( can_see( mob, actor ) )
	       if ( IS_NPC( actor ) )
		 strcpy( t, actor->short_descr );
	       else
	       {
		   strcpy( t, actor->name );
		   strcat( t, " " );
		   strcat( t, actor->pcdata->title );
	       }
	    else
	      strcpy( t, "someone" );
	 break;

     case 't':
         if ( vict )
	   if ( can_see( mob, vict ) )
	     one_argument( vict->name, t );
         if ( !IS_NPC( vict ) )
	   *t = UPPER( *t );
	 break;

     case 'T':
         if ( vict ) 
            if ( can_see( mob, vict ) )
	       if ( IS_NPC( vict ) )
		 strcpy( t, vict->short_descr );
	       else
	       {
		 strcpy( t, vict->name );
		 strcat( t, " " );
		 strcat( t, vict->pcdata->title );
	       }
	    else
	      strcpy( t, "someone" );
	 break;
     
     case 'r':
         if ( rndm )
	   if ( can_see( mob, rndm ) )
	     one_argument( rndm->name, t );
         if ( !IS_NPC( rndm ) )
	   *t = UPPER( *t );
      break;

     case 'R':
         if ( rndm ) 
            if ( can_see( mob, rndm ) )
	       if ( IS_NPC( rndm ) )
		 strcpy(t,rndm->short_descr);
	       else
	       {
		 strcpy( t, rndm->name );
		 strcat( t, " " );
		 strcat( t, rndm->pcdata->title );
	       }
	    else
	      strcpy( t, "someone" );
	 break;

     case 'e':
         if ( actor )
	   can_see( mob, actor ) ? strcpy( t, he_she[ actor->sex ] )
	                         : strcpy( t, "someone" );
	 break;
  
     case 'm':
         if ( actor )
	   can_see( mob, actor ) ? strcpy( t, him_her[ actor->sex ] )
                                 : strcpy( t, "someone" );
	 break;
  
     case 's':
         if ( actor )
	   can_see( mob, actor ) ? strcpy( t, his_her[ actor->sex ] )
	                         : strcpy( t, "someone's" );
	 break;
     
     case 'E':
         if ( vict )
	   can_see( mob, vict ) ? strcpy( t, he_she[ vict->sex ] )
                                : strcpy( t, "someone" );
	 break;
  
     case 'M':
         if ( vict )
	   can_see( mob, vict ) ? strcpy( t, him_her[ vict->sex ] )
                                : strcpy( t, "someone" );
	 break;
  
     case 'S':
         if ( vict )
	   can_see( mob, vict ) ? strcpy( t, his_her[ vict->sex ] )
                                : strcpy( t, "someone's" ); 
	 break;

     case 'j':
	 strcpy( t, he_she[ mob->sex ] );
	 break;
  
     case 'k':
	 strcpy( t, him_her[ mob->sex ] );
	 break;
  
     case 'l':
	 strcpy( t, his_her[ mob->sex ] );
	 break;

     case 'J':
         if ( rndm )
	   can_see( mob, rndm ) ? strcpy( t, he_she[ rndm->sex ] )
	                        : strcpy( t, "someone" );
	 break;
  
     case 'K':
         if ( rndm )
	   can_see( mob, rndm ) ? strcpy( t, him_her[ rndm->sex ] )
                                : strcpy( t, "someone" );
	 break;
  
     case 'L':
         if ( rndm )
	   can_see( mob, rndm ) ? strcpy( t, his_her[ rndm->sex ] )
	                        : strcpy( t, "someone's" );
	 break;

     case 'o':
         if ( obj )
	   can_see_obj( mob, obj ) ? one_argument( obj->name, t )
                                   : strcpy( t, "something" );
	 break;

     case 'O':
         if ( obj )
	   can_see_obj( mob, obj ) ? strcpy( t, obj->short_descr )
                                   : strcpy( t, "something" );
	 break;

     case 'p':
         if ( v_obj )
	   can_see_obj( mob, v_obj ) ? one_argument( v_obj->name, t )
                                     : strcpy( t, "something" );
	 break;

     case 'P':
         if ( v_obj )
	   can_see_obj( mob, v_obj ) ? strcpy( t, v_obj->short_descr )
                                     : strcpy( t, "something" );
      break;

     case 'a':
         if ( obj ) 
          switch ( *( obj->name ) )
	  {
	    case 'a': case 'e': case 'i':
            case 'o': case 'u': strcpy( t, "an" );
	      break;
            default: strcpy( t, "a" );
          }
	 break;

     case 'A':
         if ( v_obj ) 
          switch ( *( v_obj->name ) )
	  {
            case 'a': case 'e': case 'i':
	    case 'o': case 'u': strcpy( t, "an" );
	      break;
            default: strcpy( t, "a" );
          }
	 break;

     case '$':
         strcpy( t, "$" );
	 break;

     default:
         bug( "Mob: %d bad $var", mob->pIndexData->vnum );
	 break;
       }

 return;

}

/* This procedure simply copies the cmnd to a buffer while expanding
 * any variables by calling the translate procedure.  The observant
 * code scrutinizer will notice that this is taken from act()
 */
void mprog_process_cmnd( char *cmnd, CHAR_DATA *mob, CHAR_DATA *actor,
			OBJ_DATA *obj, void *vo, CHAR_DATA *rndm )
{
  char buf[ MAX_INPUT_LENGTH ];
  char tmp[ MAX_INPUT_LENGTH ];
  char *str;
  char *i;
  char *point;

  point   = buf;
  str     = cmnd;

  while ( *str != '\0' )
  {
    if ( *str != '$' )
    {
      *point++ = *str++;
      continue;
    }
    str++;
    mprog_translate( *str, tmp, mob, actor, obj, vo, rndm );
    i = tmp;
    ++str;
    while ( ( *point = *i ) != '\0' )
      ++point, ++i;
  }
  *point = '\0';
  interpret( mob, buf );

  return;

}

/* The main focus of the MOBprograms.  This routine is called 
 *  whenever a trigger is successful.  It is responsible for parsing
 *  the command list and figuring out what to do. However, like all
 *  complex procedures, everything is farmed out to the other guys.
 */
void mprog_driver ( char *com_list, CHAR_DATA *mob, CHAR_DATA *actor,
		   OBJ_DATA *obj, void *vo)
{

 char tmpcmndlst[ MAX_STRING_LENGTH ];
 char buf       [ MAX_INPUT_LENGTH ];
 char *morebuf;
 char *command_list;
 char *cmnd;
 CHAR_DATA *rndm  = NULL;
 CHAR_DATA *vch   = NULL;
 int        count = 0;

 if ( !mob )
 {
     bug( "MPDriver:  NULL mob!", 0 );
     return;
 }

 if IS_AFFECTED( mob, AFF_CHARM )
   return;

 /* get a random visable mortal player who is in the room with the mob */
 for ( vch = mob->in_room->people; vch; vch = vch->next_in_room )
   if ( !IS_NPC( vch )
       &&  vch->level < MAX_LEVEL-3
       &&  can_see( mob, vch ) )
     {
       if ( number_range( 0, count ) == 0 )
	 rndm = vch;
       count++;
     }
  
 strcpy( tmpcmndlst, com_list );
 command_list = tmpcmndlst;
 cmnd         = command_list;
 command_list = mprog_next_command( command_list );
 while ( cmnd && *cmnd )
   {
     morebuf = one_argument( cmnd, buf );
     if ( !str_cmp( buf, "if" ) )
       command_list = mprog_process_if( morebuf, command_list, mob,
				       actor, obj, vo, rndm );
     else
       mprog_process_cmnd( cmnd, mob, actor, obj, vo, rndm );
     cmnd         = command_list;
     command_list = mprog_next_command( command_list );
   }

 return;

}

/***************************************************************************
 * Global function code and brief comments.
 */

/* The next two routines are the basic trigger types. Either trigger
 *  on a certain percent, or trigger on a keyword or word phrase.
 *  To see how this works, look at the various trigger routines..
 */
void mprog_wordlist_check( char *arg, CHAR_DATA *mob, CHAR_DATA *actor,
			  OBJ_DATA *obj, void *vo, int type )
{

  char        temp1[ MAX_STRING_LENGTH ];
  char        temp2[ MAX_INPUT_LENGTH ];
  char        word[ MAX_INPUT_LENGTH ];
  MPROG_DATA *mprg;
  char       *list;
  char       *start;
  char       *dupl;
  char       *end;
  int         i;

  for ( mprg = mob->pIndexData->mudprogs; mprg != NULL; mprg = mprg->next )
    if ( mprg->type & type )
      {
	strcpy( temp1, mprg->arglist );
	list = temp1;
	for ( i = 0; i < strlen( list ); i++ )
	  list[i] = LOWER( list[i] );
	strcpy( temp2, arg );
	dupl = temp2;
	for ( i = 0; i < strlen( dupl ); i++ )
	  dupl[i] = LOWER( dupl[i] );
	if ( ( list[0] == 'p' ) && ( list[1] == ' ' ) )
	  {
	    list += 2;
	    while ( ( start = strstr( dupl, list ) ) )
	      if ( (start == dupl || *(start-1) == ' ' )
		  && ( *(end = start + strlen( list ) ) == ' '
		      || *end == '\n'
		      || *end == '\r'
		      || *end == '\0' ) )
		{
		  mprog_driver( mprg->comlist, mob, actor, obj, vo );
		  break;
		}
	      else
		dupl = start+1;
	  }
	else
	  {
	    list = one_argument( list, word );
	    for( ; word[0] != '\0'; list = one_argument( list, word ) )
	      while ( ( start = strstr( dupl, word ) ) )
		if ( ( start == dupl || *(start-1) == ' ' )
		    && ( *(end = start + strlen( word ) ) == ' '
			|| *end == '\n'
			|| *end == '\r'
			|| *end == '\0' ) )
		  {
		    mprog_driver( mprg->comlist, mob, actor, obj, vo );
		    break;
		  }
		else
		  dupl = start+1;
	  }
      }

  return;

}

void mprog_percent_check( CHAR_DATA *mob, CHAR_DATA *actor, OBJ_DATA *obj,
			 void *vo, int type)
{
 MPROG_DATA * mprg;

 for ( mprg = mob->pIndexData->mudprogs; mprg != NULL; mprg = mprg->next )
   if ( ( mprg->type & type )
       && ( number_percent( ) < atoi( mprg->arglist ) ) )
     {
       mprog_driver( mprg->comlist, mob, actor, obj, vo );
       if ( type != GREET_PROG && type != ALL_GREET_PROG )
	 break;
     }

 return;

}

/* The triggers.. These are really basic, and since most appear only
 * once in the code (hmm. i think they all do) it would be more efficient
 * to substitute the code in and make the mprog_xxx_check routines global.
 * However, they are all here in one nice place at the moment to make it
 * easier to see what they look like. If you do substitute them back in,
 * make sure you remember to modify the variable names to the ones in the
 * trigger calls.
 */
void mprog_act_trigger( char *buf, CHAR_DATA *mob, CHAR_DATA *ch,
		       OBJ_DATA *obj, void *vo)
{

  MPROG_ACT_LIST * tmp_act;
  MPROG_DATA *mprg;
  bool found = FALSE;
  if ( IS_NPC( mob )
       && IS_SET( mob->pIndexData->progtypes, ACT_PROG ) )
  {
        if ( IS_NPC( ch ) && ch->pIndexData == mob->pIndexData )
           return;
        
        for ( mprg = mob->pIndexData->mudprogs; mprg; mprg = mprg->next )
            if ( mprg->type & ACT_PROG )
            {
                found = TRUE;
                break;
            }
            
        if ( !found )
           return;
           
        tmp_act = alloc_mem( sizeof( MPROG_ACT_LIST ) );
        if ( mob->mpactnum > 0 )
	  tmp_act->next = mob->mpact;
        else
	  tmp_act->next = NULL;

        mob->mpact      = tmp_act;
        mob->mpact->buf = str_dup( buf );
        mob->mpact->ch  = ch; 
        mob->mpact->obj = obj; 
        mob->mpact->vo  = vo; 
        mob->mpactnum++;
  }
  return;

}
void mprog_bribe_trigger( CHAR_DATA *mob, CHAR_DATA *ch, int amount )
{

  char        buf[ MAX_STRING_LENGTH ];
  MPROG_DATA *mprg;
  OBJ_DATA   *obj;

  if ( IS_NPC( mob )
      && ( mob->pIndexData->progtypes & BRIBE_PROG ) )
    {
      if ( IS_NPC( ch ) && ch->pIndexData == mob->pIndexData )
         return;
      obj = create_object( get_obj_index( OBJ_VNUM_MONEY_SOME ), 0 );
      sprintf( buf, obj->short_descr, amount );
      free_string( obj->short_descr );
      obj->short_descr = str_dup( buf );
      obj->value[0]    = amount;
      obj_to_char( obj, mob );
      mob->gold -= amount;

      for ( mprg = mob->pIndexData->mudprogs; mprg != NULL; mprg = mprg->next )
	if ( ( mprg->type & BRIBE_PROG )
	    && ( amount >= atoi( mprg->arglist ) ) )
	  {
	    mprog_driver( mprg->comlist, mob, ch, obj, NULL );
	    break;
	  }
    }
  
  return;

}

void mprog_death_trigger( CHAR_DATA *mob )
{

 if ( IS_NPC( mob )
     && ( mob->pIndexData->progtypes & DEATH_PROG ) )
   {
     mob->position = POS_RESTING;
     mprog_percent_check( mob, NULL, NULL, NULL, DEATH_PROG );
     mob->position = POS_DEAD;
   }
   else
   {
   death_cry( mob );
   }
 return;

}

void mprog_entry_trigger( CHAR_DATA *mob )
{

 if ( IS_NPC( mob )
     && ( mob->pIndexData->progtypes & ENTRY_PROG ) )
   mprog_percent_check( mob, NULL, NULL, NULL, ENTRY_PROG );

 return;

}

void mprog_fight_trigger( CHAR_DATA *mob, CHAR_DATA *ch )
{

 if ( IS_NPC( mob )
     && ( mob->pIndexData->progtypes & FIGHT_PROG ) )
	{
   if ( IS_NPC( ch ) && ch->pIndexData == mob->pIndexData )
            return;
   mprog_percent_check( mob, ch, NULL, NULL, FIGHT_PROG );
	}

 return;

}

void mprog_give_trigger( CHAR_DATA *mob, CHAR_DATA *ch, OBJ_DATA *obj )
{

 char        buf[MAX_INPUT_LENGTH];
 MPROG_DATA *mprg;

 if ( IS_NPC( mob )
     && ( mob->pIndexData->progtypes & GIVE_PROG ) )
   for ( mprg = mob->pIndexData->mudprogs; mprg != NULL; mprg = mprg->next )
     {
       if ( IS_NPC( ch ) && ch->pIndexData == mob->pIndexData )
            return;
       one_argument( mprg->arglist, buf );
       if ( ( mprg->type & GIVE_PROG )
	   && ( ( !str_cmp( obj->name, mprg->arglist ) )
	       || ( !str_cmp( "all", buf ) ) ) )
	 {
	   mprog_driver( mprg->comlist, mob, ch, obj, NULL );
	   break;
	 }
     }

 return;

}

void mprog_greet_trigger( CHAR_DATA *ch )
{

 CHAR_DATA *vmob;

 for ( vmob = ch->in_room->people; vmob != NULL; vmob = vmob->next_in_room )
   if ( IS_NPC( vmob )
       && ch != vmob
       && can_see( vmob, ch )
       && ( vmob->fighting == NULL )
       && IS_AWAKE( vmob )
       && ( vmob->pIndexData->progtypes & GREET_PROG) )
     mprog_percent_check( vmob, ch, NULL, NULL, GREET_PROG );
   else
     if ( IS_NPC( vmob )
	 && ( vmob->fighting == NULL )
	 && IS_AWAKE( vmob )
	 && ( vmob->pIndexData->progtypes & ALL_GREET_PROG ) )
       mprog_percent_check(vmob,ch,NULL,NULL,ALL_GREET_PROG);

 return;

}

void mprog_hitprcnt_trigger( CHAR_DATA *mob, CHAR_DATA *ch)
{

 MPROG_DATA *mprg;

 if ( IS_NPC( mob )
     && ( mob->pIndexData->progtypes & HITPRCNT_PROG ) )
{
	if ( IS_NPC( ch ) && ch->pIndexData == mob->pIndexData )
       return;
   for ( mprg = mob->pIndexData->mudprogs; mprg != NULL; mprg = mprg->next )
     if ( ( mprg->type & HITPRCNT_PROG )
	 && ( ( 100*mob->hit / mob->max_hit ) < atoi( mprg->arglist ) ) )
       {
	 mprog_driver( mprg->comlist, mob, ch, NULL, NULL );
	 break;
       }
 	 }
 return;

}

void mprog_random_trigger( CHAR_DATA *mob )
{

  if ( mob->pIndexData->progtypes & RAND_PROG)
    mprog_percent_check(mob,NULL,NULL,NULL,RAND_PROG);

  return;

}

void mprog_speech_trigger( char *txt, CHAR_DATA *mob )
{

  CHAR_DATA *vmob;

  for ( vmob = mob->in_room->people; vmob != NULL; vmob = vmob->next_in_room )
    if ( IS_NPC( vmob ) && ( vmob->pIndexData->progtypes & SPEECH_PROG ) )
{
	if ( IS_NPC( mob ) && ( vmob->pIndexData == mob->pIndexData ) )
            continue;
      mprog_wordlist_check( txt, vmob, mob, NULL, NULL, SPEECH_PROG );
}  
  return;

}

 /*****************************************************************
  *   ROOM PROG SUPPORT STARTS HERE:                              *
  *          most of this code was taken from the SMAUG code base *
  *          with modifications made to fit our mud...all mods    *
  *          were done by Thanatos                                *
  *****************************************************************/
  /* 
   * Structure needed to efficiently handle room and object act
   * triggers. 
   */
 struct  act_prog_data
 {
     struct act_prog_data *next;
     void *vo;
 };
 
 CHAR_DATA *supermob;
 struct act_prog_data *room_act_list;
 struct act_prog_data *obj_act_list;
 
 void init_supermob( )
 {
    supermob = create_mobile( get_mob_index(MOB_VNUM_SUPERMOB) );
    char_to_room( supermob, get_room_index(ROOM_VNUM_SUPERMOB) );
    
    return;
 }
 
 void rset_supermob( ROOM_INDEX_DATA *room)
 {
   char buf[200];
 
   if (room)
   {
     if ( !supermob )
     {
 	supermob = create_mobile( get_mob_index(MOB_VNUM_SUPERMOB) );
 	char_to_room( supermob, get_room_index(ROOM_VNUM_SUPERMOB) );
     }
     if ( supermob->short_descr )
     	free_string(supermob->short_descr);
     supermob->short_descr = str_dup(room->name);
     if ( supermob->name )
     	free_string(supermob->name);
     supermob->name        = str_dup(room->name);
 
     /* Added by Jenny to allow bug messages to show the vnum
        of the room, and not just supermob's vnum */
     sprintf( buf, "Room #%d", room->vnum );
     if ( supermob->description )
     	free_string( supermob->description );
     supermob->description = str_dup( buf );
 
     char_from_room ( supermob );
     char_to_room( supermob, room); 
   }
 }
 
 void set_supermob( OBJ_DATA *obj)
 {
   ROOM_INDEX_DATA *room;
   OBJ_DATA *in_obj;
   char buf[200];
 
   if ( !supermob )
     supermob = create_mobile(get_mob_index( MOB_VNUM_SUPERMOB ));
 
   if(!obj)
      return;
 
   for ( in_obj = obj; in_obj->in_obj; in_obj = in_obj->in_obj )
     ;
 
   if ( in_obj->carried_by )
       room = in_obj->carried_by->in_room;
   else
       room = obj->in_room;
 
   if(!room)
      return;
 
   if (supermob->short_descr)
      free_string(supermob->short_descr);
 
   supermob->short_descr = str_dup(obj->short_descr);
 
   /* Added by Jenny to allow bug messages to show the vnum
      of the object, and not just supermob's vnum */
   sprintf( buf, "Object #%d", obj->pIndexData->vnum );
   free_string( supermob->description );
   supermob->description = str_dup( buf );
 
   if( room )
   {
     char_from_room (supermob );
     char_to_room( supermob, room); 
   }
   
   return;
 }
 
 
 void release_supermob( )
 {
   char_from_room( supermob );
   if ( supermob->name )
      free_string( supermob->name );
   supermob->name = str_dup("SuperMob");
   char_to_room( supermob, get_room_index( ROOM_VNUM_SUPERMOB ) );
 }
 
 void rprog_wordlist_check( char *arg, CHAR_DATA *mob, CHAR_DATA *actor,
 			  OBJ_DATA *obj, void *vo, int type, ROOM_INDEX_DATA *room )
 {
 
   char        temp1[ MAX_STRING_LENGTH ];
   char        temp2[ MAX_INPUT_LENGTH ];
   char        word[ MAX_INPUT_LENGTH ];
   MPROG_DATA *mprg;
   char       *list;
   char       *start;
   char       *dupl;
   char       *end;
   int         i;
 
   if ( actor && actor->in_room )
     room = actor->in_room;
 
   for ( mprg = room->mudprogs; mprg; mprg = mprg->next )
     if ( mprg->type & type )
       {
 	strcpy( temp1, mprg->arglist );
 	list = temp1;
 	for ( i = 0; i < strlen( list ); i++ )
 	  list[i] = LOWER( list[i] );
 	strcpy( temp2, arg );
 	dupl = remove_color(temp2);
 	for ( i = 0; i < strlen( dupl ); i++ )
 	  dupl[i] = LOWER( dupl[i] );
 	if ( ( list[0] == 'p' ) && ( list[1] == ' ' ) )
 	  {
 	    list += 2;
 	    while ( ( start = strstr( dupl, list ) ) )
 	      if ( (start == dupl || *(start-1) == ' ' )
 		  && ( *(end = start + strlen( list ) ) == ' '
 		      || *end == '\n'
 		      || *end == '\r'
 		      || *end == '\0' ) )
 		{
 		  rset_supermob( room );
 		  mprog_driver( mprg->comlist, mob, actor, obj, vo );
 		  release_supermob() ;
 		  break;
 		}
 	      else
 		dupl = start+1;
 	  }
 	else
 	  {
 	    list = one_argument( list, word );
 	    for( ; word[0] != '\0'; list = one_argument( list, word ) )
 	      while ( ( start = strstr( dupl, word ) ) )
 		if ( ( start == dupl || *(start-1) == ' ' )
 		    && ( *(end = start + strlen( word ) ) == ' '
 			|| *end == '\n'
 			|| *end == '\r'
 			|| *end == '\0' ) )
 		  {
 		    rset_supermob( room );
 		    mprog_driver( mprg->comlist, mob, actor, obj, vo );
 		    release_supermob();
 		    break;
 		  }
 		else
 		  dupl = start+1;
 	  }
       }
       return;
 }
 
 
 void rprog_percent_check( CHAR_DATA *mob, CHAR_DATA *actor, OBJ_DATA *obj,
 			 void *vo, int type)
 {
  MPROG_DATA * mprg;
 
  if(!mob->in_room)
    return;
 
  for ( mprg = mob->in_room->mudprogs; mprg; mprg = mprg->next )
    if ( ( mprg->type & type )
        && ( number_percent( ) <= atoi( mprg->arglist ) ) )
      {
        mprog_driver( mprg->comlist, mob, actor, obj, vo );
        if(type!=ENTER_PROG)
           break;
      }
 
      return;
 }
 
 /*
  * Triggers follow
  */
 
 
 /*
  *  Hold on this
  * Unhold. -- Alty
  */
 void room_act_add( ROOM_INDEX_DATA *room );
 void rprog_act_trigger( char *buf, ROOM_INDEX_DATA *room, CHAR_DATA *ch,
 			OBJ_DATA *obj, void *vo )
 {
    if ( room->progtypes & ACT_PROG ) 
    {
       MPROG_ACT_LIST *tmp_act;
       
       /* supermob can't trigger it's own mprog */
       if ( IS_NPC(ch) && ch->pIndexData == supermob->pIndexData )
          return;
          
       tmp_act = alloc_mem( sizeof( MPROG_ACT_LIST ) );
       if ( room->mpactnum > 0 )
         tmp_act->next = room->mpact;
       else
         tmp_act->next = NULL;
       
       room->mpact = tmp_act;
       room->mpact->buf = str_dup(buf);
       room->mpact->ch = ch;
       room->mpact->obj = obj;
       room->mpact->vo = vo;
       room->mpactnum++;
       room_act_add(room);
    }
    return;
 }
 /*
  *
  */
 void rprog_leave_trigger( CHAR_DATA *ch )
 {
   if( ch->in_room->progtypes & LEAVE_PROG ) 
   {
     rset_supermob( ch->in_room );
     rprog_percent_check( supermob, ch, NULL, NULL, LEAVE_PROG );
     release_supermob();
   }
   return;
 }
 
 void rprog_enter_trigger( CHAR_DATA *ch )
 {
   if( ch->in_room->progtypes & ENTER_PROG ) 
   {
     rset_supermob( ch->in_room );
     rprog_percent_check( supermob, ch, NULL, NULL, ENTER_PROG );
     release_supermob();
   }
   return;
 }
 
 void rprog_sleep_trigger( CHAR_DATA *ch )
 {
   if( ch->in_room->progtypes & SLEEP_PROG ) 
   {
     rset_supermob( ch->in_room );
     rprog_percent_check( supermob, ch, NULL, NULL, SLEEP_PROG );
     release_supermob();
   }
   return;
 }
 
 void rprog_rest_trigger( CHAR_DATA *ch )
 {
   if( ch->in_room->progtypes & REST_PROG ) 
   {
     rset_supermob( ch->in_room );
     rprog_percent_check( supermob, ch, NULL, NULL, REST_PROG );
     release_supermob();
   }
   return;
 }
 
 void rprog_rfight_trigger( CHAR_DATA *ch )
 {
   if( ch->in_room->progtypes & RFIGHT_PROG ) 
   {
     rset_supermob( ch->in_room );
     rprog_percent_check( supermob, ch, NULL, NULL, RFIGHT_PROG );
     release_supermob();
   }
   return;
 }
 
 void rprog_death_trigger( CHAR_DATA *ch )
 {
   if( ch->in_room->progtypes & RDEATH_PROG ) 
   {
     rset_supermob( ch->in_room );
     rprog_percent_check( supermob, ch, NULL, NULL, RDEATH_PROG );
     release_supermob();
   }
   return;
 }
 
 void rprog_speech_trigger( char *txt, CHAR_DATA *ch )
 {
   /* prevent circular triggers by not allowing mob to trigger itself */
   if ( IS_NPC(ch) && ch->pIndexData == supermob->pIndexData )
      return;
      
   if( ch->in_room->progtypes & SPEECH_PROG ) 
   {
     /* supermob is set and released in rprog_wordlist_check */
     rprog_wordlist_check( txt, supermob, ch, NULL, NULL, SPEECH_PROG, ch->in_room );
   }
  return;
 }
 
void rprog_random_trigger( CHAR_DATA *ch )
 {
if ( !ch || !ch->in_room )
{
bug("NULL in_room for rprog_random_trigger",0);
return;
} 

   if ( ch->in_room->progtypes & RAND_PROG)
   {
     rset_supermob( ch->in_room );
     rprog_percent_check(supermob,ch,NULL,NULL,RAND_PROG);
     release_supermob();
   }
   return;
 }
  
 /* Written by Jenny, Nov 29/95 */
 void progbug( char *str, CHAR_DATA *mob )
 {
 
   /* Check if we're dealing with supermob, which means the bug occurred
      in a room or obj prog. */
   if ( mob->pIndexData->vnum == MOB_VNUM_SUPERMOB )
   {
     /* It's supermob.  In set_supermob and rset_supermob, the description
        was set to indicate the object or room, so we just need to show
        the description in the bug message. */
     bug(  str, 0 );
     bug( !mob->description ? "(unknown)" : mob->description, 0 );
   }
   else
   {
     bug( str, 0);
     bug( "Mob %d", mob->pIndexData->vnum );
   }
   return;
 }
 
 
 /* Room act prog updates.  Use a separate list cuz we dont really wanna go
    thru 5-10000 rooms every pulse.. can we say lag? -- Alty */
 
 void room_act_add( ROOM_INDEX_DATA *room )
 {
     struct act_prog_data *runner;
    
     for ( runner = room_act_list; runner; runner = runner->next )
 	if ( runner->vo == room )
 	   return;
 
     runner = alloc_mem( sizeof(*runner) );
     runner->vo = room;
     runner->next = room_act_list;
     room_act_list = runner;
 }
 
 
 void room_act_update( void )
 {
   struct act_prog_data *runner;
   MPROG_ACT_LIST *mpact;
   
   while ( (runner = room_act_list) )
   {
     ROOM_INDEX_DATA *room = runner->vo;
     
     while ( (mpact = room->mpact) )
     {
       if ( mpact->ch->in_room == room )
         rprog_wordlist_check(mpact->buf, supermob, mpact->ch, mpact->obj,
                              mpact->vo, ACT_PROG, room);
       room->mpact = mpact->next;
       free_string( mpact->buf );
       free_mem( mpact, sizeof(*mpact) );
     }
     room->mpact = NULL;
     room->mpactnum = 0;
     room_act_list = runner->next;
     free_mem( runner, sizeof(*runner) );
   }
   return;
 }
 
 /*****************************************************************
  *   OBJECT PROG SUPPORT STARTS HERE:				 *
  *          most of this code was taken from the SMAUG code base *
  *          with modifications made to fit our mud...all mods    *
  *          were done by Thanatos				 *
  *****************************************************************/
 
 void oprog_wordlist_check( char *arg, CHAR_DATA *mob, CHAR_DATA *actor,
 			  OBJ_DATA *obj, void *vo, int type, OBJ_DATA *iobj )
 {
 
   char        temp1[ MAX_STRING_LENGTH ];
   char        temp2[ MAX_INPUT_LENGTH ];
   char        word[ MAX_INPUT_LENGTH ];
   MPROG_DATA *mprg;
   char       *list;
   char       *start;
   char       *dupl;
   char       *end;
   int         i;
 
   for ( mprg = iobj->pIndexData->mudprogs; mprg; mprg = mprg->next )
     if ( mprg->type & type )
       {
 	strcpy( temp1, mprg->arglist );
 	list = temp1;
 	for ( i = 0; i < strlen( list ); i++ )
 	  list[i] = LOWER( list[i] );
 	strcpy( temp2, arg );
 	dupl = remove_color(temp2);
 	for ( i = 0; i < strlen( dupl ); i++ )
 	  dupl[i] = LOWER( dupl[i] );
 	if ( ( list[0] == 'p' ) && ( list[1] == ' ' ) )
 	  {
 	    list += 2;
 	    while ( ( start = strstr( dupl, list ) ) )
 	      if ( (start == dupl || *(start-1) == ' ' )
 		  && ( *(end = start + strlen( list ) ) == ' '
 		      || *end == '\n'
 		      || *end == '\r'
 		      || *end == '\0' ) )
 		{
 		  set_supermob( iobj );
 		  mprog_driver( mprg->comlist, mob, actor, obj, vo );
 		  release_supermob() ;
 		  break;
 		}
 	      else
 		dupl = start+1;
 	  }
 	else
 	  {
 	    list = one_argument( list, word );
 	    for( ; word[0] != '\0'; list = one_argument( list, word ) )
 	      while ( ( start = strstr( dupl, word ) ) )
 		if ( ( start == dupl || *(start-1) == ' ' )
 		    && ( *(end = start + strlen( word ) ) == ' '
 			|| *end == '\n'
 			|| *end == '\r'
 			|| *end == '\0' ) )
 		  {
 		    set_supermob( iobj );
 		    mprog_driver( mprg->comlist, mob, actor, obj, vo );
 		    release_supermob();
 		    break;
 		  }
 		else
 		  dupl = start+1;
 	  }
       }
 
   return;
 }
 
 
 bool oprog_percent_check( CHAR_DATA *mob, CHAR_DATA *actor, OBJ_DATA *obj,
 			 void *vo, int type)
 {
  MPROG_DATA * mprg;
  bool executed = FALSE;
 
  for ( mprg = obj->pIndexData->mudprogs; mprg; mprg = mprg->next )
    if ( ( mprg->type & type )
        && ( number_percent( ) <= atoi( mprg->arglist ) ) )
      {
        executed = TRUE;
        mprog_driver( mprg->comlist, mob, actor, obj, vo );
        if ( type != GREET_PROG )
 	 break;
      }
 
  return executed;
 
 }
 
 /*
  * Triggers follow
  */
 void oprog_greet_trigger( CHAR_DATA *ch )
 {
   OBJ_DATA *vobj;
 
   for ( vobj=ch->in_room->contents; vobj; vobj = vobj->next_content )
     if  ( vobj->pIndexData->progtypes & GREET_PROG ) 
     {
      set_supermob( vobj );  /* not very efficient to do here */
      oprog_percent_check( supermob, ch, vobj, NULL, GREET_PROG );
      release_supermob();
     }
 
   return;
 }
 
 void oprog_speech_trigger( char *txt, CHAR_DATA *ch )
 {
    OBJ_DATA *vobj;
 
   /* supermob is set and released in oprog_wordlist_check */
   for ( vobj=ch->in_room->contents; vobj; vobj = vobj->next_content )
     if  ( vobj->pIndexData->progtypes & SPEECH_PROG ) 
     {
       oprog_wordlist_check( txt, supermob, ch, vobj, NULL, SPEECH_PROG, vobj );
     }
 
  return;
 }
 
 /*
  * Called at top of obj_update
  * make sure to put an if(!obj) continue
  * after it
  */
 void oprog_random_trigger( OBJ_DATA *obj )
 {
 
   if ( obj->pIndexData->progtypes & RAND_PROG)
   {
      set_supermob( obj );
      oprog_percent_check(supermob,NULL,obj,NULL,RAND_PROG);
      release_supermob();
   }
   return;
 }
 
 /*
  * in wear_obj, between each successful equip_char 
  * the subsequent return
  */
 void oprog_wear_trigger( CHAR_DATA *ch, OBJ_DATA *obj )
 {
    if ( obj->pIndexData->progtypes & WEAR_PROG ) 
    {
       set_supermob( obj );
       oprog_percent_check( supermob, ch, obj, NULL, WEAR_PROG );
       release_supermob();
    }
    return;
 }
 
 bool oprog_use_trigger( CHAR_DATA *ch, OBJ_DATA *obj, CHAR_DATA *vict,
                         OBJ_DATA *targ, void *vo )
 {
    bool executed = FALSE;
 
    if ( obj->pIndexData->progtypes & USE_PROG ) 
    {
       set_supermob( obj );
       if ( obj->item_type == ITEM_STAFF )
       {
         if ( vict )
           executed = oprog_percent_check( supermob, ch, obj, vict, USE_PROG );
         else
           executed = oprog_percent_check( supermob, ch, obj, targ, USE_PROG );
       }
       else
       {
         executed = oprog_percent_check( supermob, ch, obj, NULL, USE_PROG );
       }
       release_supermob();
    }
    return executed;
 }
 
 /*
  * call in remove_obj, right after unequip_char   
  * do a if(!ch) return right after, and return TRUE (?)
  * if !ch
  */
 void oprog_remove_trigger( CHAR_DATA *ch, OBJ_DATA *obj )
 {
    if ( obj->pIndexData->progtypes & REMOVE_PROG ) 
    {
      set_supermob( obj );
      oprog_percent_check( supermob, ch, obj, NULL, REMOVE_PROG );
      release_supermob();
    }
    return;
 }
 
 
 /*
  * call in do_sac, right before extract_obj
  */
 void oprog_sac_trigger( CHAR_DATA *ch, OBJ_DATA *obj )
 {
    if ( obj->pIndexData->progtypes & SAC_PROG ) 
    {
       set_supermob( obj );
       oprog_percent_check( supermob, ch, obj, NULL, SAC_PROG );
       release_supermob();
    }
    return;
 }
 
 /*
  * call in do_get, right before check_for_trap
  * do a if(!ch) return right after
  */
 void oprog_get_trigger( CHAR_DATA *ch, OBJ_DATA *obj )
 {
    if ( obj->pIndexData->progtypes & GET_PROG ) 
    {
       set_supermob( obj );
       oprog_percent_check( supermob, ch, obj, NULL, GET_PROG );
       release_supermob();
    }
    return;
 }
  
 /*
  * called in damage_obj in act_obj.c
  */
 void oprog_damage_trigger( CHAR_DATA *ch, OBJ_DATA *obj )
 {
    if ( obj->pIndexData->progtypes & DAMAGE_PROG ) 
    {
       set_supermob( obj );
       oprog_percent_check( supermob, ch, obj, NULL, DAMAGE_PROG );
       release_supermob();
    }
    return;
  }
  
 /*
  * called in do_repair in shops.c
  */
 void oprog_repair_trigger( CHAR_DATA *ch, OBJ_DATA *obj )
 {
 
    if ( obj->pIndexData->progtypes & REPAIR_PROG ) 
    {
       set_supermob( obj );
       oprog_percent_check( supermob, ch, obj, NULL, REPAIR_PROG );
       release_supermob();
    }
    return;
 }

 /*
  * call twice in do_drop, right after the act( AT_ACTION,...)
  * do a if(!ch) return right after
  */
 void oprog_drop_trigger( CHAR_DATA *ch, OBJ_DATA *obj )
 {
    if ( obj->pIndexData->progtypes & DROP_PROG ) 
    {
       set_supermob( obj );
       oprog_percent_check( supermob, ch, obj, NULL, DROP_PROG );
       release_supermob();
    }
    return;
 }
 
 /*
  * call towards end of do_examine, right before check_for_trap
  */
 void oprog_examine_trigger( CHAR_DATA *ch, OBJ_DATA *obj )
 {
    if ( obj->pIndexData->progtypes & EXA_PROG ) 
    {
       set_supermob( obj );
       oprog_percent_check( supermob, ch, obj, NULL, EXA_PROG );
       release_supermob();
    }
    return;
 }
 
 
 /*
  * call in fight.c, group_gain, after (?) the obj_to_room
  */
 void oprog_zap_trigger( CHAR_DATA *ch, OBJ_DATA *obj )
 {
    if ( obj->pIndexData->progtypes & ZAP_PROG ) 
    {
       set_supermob( obj );
       oprog_percent_check( supermob, ch, obj, NULL, ZAP_PROG );
       release_supermob();
    }
    return;
 }
 
 void obj_act_add( OBJ_DATA *obj );
 void oprog_act_trigger( char *buf, OBJ_DATA *mobj, CHAR_DATA *ch,
 			OBJ_DATA *obj, void *vo )
 {
    if ( IS_NPC(ch) && ch->pIndexData == supermob->pIndexData )
       return;   /* prevent supermob from triggering itself */
       
    if ( mobj->pIndexData->progtypes & ACT_PROG ) 
    {
       MPROG_ACT_LIST *tmp_act;
       
       tmp_act = alloc_mem( sizeof(MPROG_ACT_LIST) );
       if ( mobj->mpactnum > 0 )
         tmp_act->next = mobj->mpact;
       else
         tmp_act->next = NULL;
       
       mobj->mpact = tmp_act;
       mobj->mpact->buf = str_dup(buf);
       mobj->mpact->ch = ch;
       mobj->mpact->obj = obj;
       mobj->mpact->vo = vo;
       mobj->mpactnum++;
       obj_act_add(mobj);
    }
    return;
 }
 
 void obj_act_add( OBJ_DATA *obj )
 {
   struct act_prog_data *runner;
 
   for ( runner = obj_act_list; runner; runner = runner->next )
     if ( runner->vo == obj )
       return;
 
   runner = alloc_mem( sizeof(*runner) );
   runner->vo = obj;
   runner->next = obj_act_list;
   obj_act_list = runner;
 }
 
 void obj_act_update( void )
 {
   struct act_prog_data *runner;
   MPROG_ACT_LIST *mpact;
 
   while ( (runner = obj_act_list) != NULL )
   {
     OBJ_DATA *obj = runner->vo;
     
     while ( (mpact = obj->mpact) != NULL )
     {
       oprog_wordlist_check(mpact->buf, supermob, mpact->ch, mpact->obj,
                            mpact->vo, ACT_PROG, obj);
       obj->mpact = mpact->next;
       free_string(mpact->buf);
       free_mem(mpact, sizeof(*mpact) );
     }
     obj->mpact = NULL;
     obj->mpactnum = 0;
     obj_act_list = runner->next;
     free_mem(runner, sizeof(*runner));
   }
   return;
}

char *remove_color( const char *str )
{
    char *nocolor;
    static char nocolor_buf[ MAX_STRING_LENGTH ];
    
    bzero( nocolor_buf, MAX_STRING_LENGTH );
    nocolor = nocolor_buf;
    
    while ( *str )
    {
 	if (*str != '`')   /* If there's no `, add to the count  */
 	{
              *nocolor++ = *str++;
 	     continue;
 	}
 	str++;             /* If there _IS_ a `, check next char */
 	switch (*str)
 	{
 /* If it's \0, count the first ` and get outta here */
 	   case '\0': *nocolor++ = '`'; 
 	   	      *nocolor = '\0';
 	   	      return nocolor;
 	   
 /* If it's a color code, skip over it */
 	   case 'k': case 'K': case 'r': case 'R': case 'b': case 'B':
 	   case 'c': case 'C': case 'y': case 'Y': case 'm': case 'M':
 	   case 'w': case 'W': case 'g': case 'G': str++; break;
 /* If it's not a color code, count the ` and the following char, and advance */
 	   default: *nocolor++ = '`';
 	            break;
 	}
 	continue;
    }
    return nocolor_buf;
}	
