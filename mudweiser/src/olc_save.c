/**************************************************************************
 *  File: olc_save.c                                                       *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
 ***************************************************************************/
/* OLC_SAVE.C
 * This takes care of saving all the .are information.
 * Notes:
 * -If a good syntax checker is used for setting vnum ranges of areas
 *  then it would become possible to just cycle through vnums instead
 *  of using the iHash stuff and checking that the room or reset or
 *  mob etc is part of that area.
 */

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "olc.h"

/* Local's */
void write_mobprog( MOB_INDEX_DATA * pMobIndex );
void write_roomprog( ROOM_INDEX_DATA * pRoom );
void write_objprog( OBJ_INDEX_DATA * pObj );

/*
 *  Verbose writes reset data in plain english into the comments
 *  section of the resets.  It makes areas considerably larger but
 *  may aid in debugging.
 */

/*
#define VERBOSE 
*/

/*
 * Local functions.
 */

char *                  mprog_type_to_name      args( ( int type ) );

/*****************************************************************************
 Name:		fix_string
 Purpose:	Returns a string without \r and ~.
 ****************************************************************************/
char *fix_string( const char *str )
{
    static char strfix[MAX_STRING_LENGTH];
    int i;
    int o;

    if ( str == NULL )
        return '\0';

    for ( o = i = 0; str[i+o] != '\0'; i++ )
    {
        if (str[i+o] == '\r' || str[i+o] == '~')
            o++;
        strfix[i] = str[i+o];
    }
    strfix[i] = '\0';
    return strfix;
}



/*****************************************************************************
 Name:		save_area_list
 Purpose:	Saves the listing of files to be loaded at startup.
 Called by:	do_asave(olc_save.c).
 ****************************************************************************/
void save_area_list()
{
    FILE *fp;
    AREA_DATA *pArea;

    if ( ( fp = fopen( "area.lst", "w" ) ) == NULL )
    {
	bug( "Save_area_list: fopen", 0 );
	perror( "area.lst" );
    }
    else
    {
	/*
	 * Add any help files that need to be loaded at
	 * startup to this section.
	 */
/***/
       fprintf( fp, "clans.are\n"  ); 
       fprintf( fp, "olc.hlp\n"    );    /* WHY? */
       fprintf( fp, "help.are\n"   );
       fprintf( fp, "social.are\n" );    /* ROM OLC */
       fprintf( fp, "ember.are\n"    );    /* ROM OLC */
       
	for( pArea = area_first; pArea; pArea = pArea->next )
	{
	    fprintf( fp, "%s\n", pArea->filename );
	}

	fprintf( fp, "$\n" );
	fclose( fp );
    }

    return;
}


/*
 * ROM OLC
 * Used in save_mobile and save_object below.  Writes
 * flags on the form fread_flag reads.
 * 
 * buf[] must hold at least 32+1 characters.
 *
 * -- Hugin
 */
char *fwrite_flag( long flags, char buf[] )
{
    char offset;
    char *cp;

    buf[0] = '\0';

    if ( flags == 0 )
    {
	strcpy( buf, "0" );
	return buf;
    }

    /* 32 -- number of bits in a long */

    for ( offset = 0, cp = buf; offset < 32; offset++ )
	if ( flags & ( (long)1 << offset ) )
	{
	    if ( offset <= 'Z' - 'A' )
		*(cp++) = 'A' + offset;
	    else
		*(cp++) = 'a' + offset - ( 'Z' - 'A' + 1 );
	}

    *cp = '\0';

    return buf;
}




/*****************************************************************************
 Name:		save_mobile
 Purpose:	Save one mobile to file, new format -- Hugin
 Called by:	save_mobiles (below).
 ****************************************************************************/
void save_mobile( FILE *fp, MOB_INDEX_DATA *pMobIndex )
{
char buf[MAX_STRING_LENGTH];
   char   letter;
    sh_int race = pMobIndex->race;
  /*  MPROG_DATA *mprg; */

    fprintf( fp, "#%d\n",         pMobIndex->vnum );
    fprintf( fp, "%s~\n",         pMobIndex->player_name );
    fprintf( fp, "%s~\n",         pMobIndex->short_descr );
    fprintf( fp, "%s~\n",         fix_string( pMobIndex->long_descr ) );
    fprintf( fp, "%s~\n",         fix_string( pMobIndex->description) );
    fprintf( fp, "%s~\n",         race_table[race].name );
    fprintf( fp, "%s ",	          fwrite_flag( pMobIndex->act,         buf ) );
    fprintf( fp, "%s ",	          fwrite_flag( pMobIndex->affected_by, buf ) );
    fprintf( fp, "%d S\n",        pMobIndex->alignment );
    fprintf( fp, "%d ",	          pMobIndex->level );
    fprintf( fp, "%d ",	          pMobIndex->hitroll );
    fprintf( fp, "%dd%d+%d ",     pMobIndex->hit[DICE_NUMBER], 
	     	     	          pMobIndex->hit[DICE_TYPE], 
	     	     	          pMobIndex->hit[DICE_BONUS] );
    fprintf( fp, "%dd%d+%d ",     pMobIndex->mana[DICE_NUMBER], 
	     	     	          pMobIndex->mana[DICE_TYPE], 
	     	     	          pMobIndex->mana[DICE_BONUS] );
    fprintf( fp, "%dd%d+%d ",     pMobIndex->damage[DICE_NUMBER], 
	     	     	          pMobIndex->damage[DICE_TYPE], 
	     	     	          pMobIndex->damage[DICE_BONUS] );
    fprintf( fp, "%d\n",          pMobIndex->dam_type );
    fprintf( fp, "%d %d %d %d\n", pMobIndex->ac[AC_PIERCE] / 10, 
	     	     	          pMobIndex->ac[AC_BASH]   / 10, 
	     	     	          pMobIndex->ac[AC_SLASH]  / 10, 
	     	     	          pMobIndex->ac[AC_EXOTIC] / 10 );
    fprintf( fp, "%s ",           fwrite_flag( pMobIndex->off_flags,  buf ) );
    fprintf( fp, "%s ",	          fwrite_flag( pMobIndex->imm_flags,  buf ) );
    fprintf( fp, "%s ",           fwrite_flag( pMobIndex->res_flags,  buf ) );
    fprintf( fp, "%s\n",          fwrite_flag( pMobIndex->vuln_flags, buf ) );
    fprintf( fp, "%d %d %d %ld\n",
	                          pMobIndex->start_pos,
	         	     	  pMobIndex->default_pos,
	         	     	  pMobIndex->sex,
	         	     	  pMobIndex->gold );
    fprintf( fp, "%s ",           fwrite_flag( pMobIndex->form,  buf ) );
    fprintf( fp, "%s ",      	  fwrite_flag( pMobIndex->parts, buf ) );

    switch ( pMobIndex->size )
    {
        default:          letter = 'M'; break;
        case SIZE_TINY:   letter = 'T'; break;
        case SIZE_SMALL:  letter = 'S'; break;
    	case SIZE_MEDIUM: letter = 'M'; break;
        case SIZE_LARGE:  letter = 'L'; break;
        case SIZE_HUGE:   letter = 'H'; break;
        case SIZE_GIANT:  letter = 'G'; break;
    }

    fprintf( fp, "%c ",           letter );
    fprintf( fp, "%s\n",          material_name( pMobIndex->material ) );
    if(pMobIndex->rnd_obj_percent >= 1 && pMobIndex->rnd_obj_num >= 1) 
    {
       fprintf( fp, "R %d %d ",		pMobIndex->rnd_obj_percent, pMobIndex->rnd_obj_num );
       fprintf( fp, "%s \n",           fwrite_flag( pMobIndex->rnd_obj_types,  buf ) );
    }
/*
    for ( mprg = pMobIndex->mobprogs; mprg != NULL; mprg = mprg->next )
    {
      fprintf( fp, ">%s %s~\n%s~\n",
              mprog_type_to_name( mprg->type ),
              mprg->arglist,
              mprg->comlist );
    }

    if (pMobIndex->mobprogs != NULL)
	fprintf( fp, "|\n");
*/
    return;
}


/*****************************************************************************
 Name:		save_mobiles
 Purpose:	Save #MOBILES secion of an area file.
 Called by:	save_area(olc_save.c).
 Notes:         Changed for ROM OLC.
 ****************************************************************************/
void save_mobiles( FILE *fp, AREA_DATA *pArea )
{
    int i;
    MOB_INDEX_DATA *pMob;

    fprintf( fp, "#MOBILES\n" );

    for( i = pArea->lvnum; i <= pArea->uvnum; i++ )
    {
	if ( (pMob = get_mob_index( i )) )
	    save_mobile( fp, pMob );
    }

    fprintf( fp, "#0\n\n\n\n" );
    return;
}

/*****************************************************************************
 Name:		save_object
 Purpose:	Save one object to file.
                new ROM format saving -- Hugin
 Called by:	save_objects (below).
 ****************************************************************************/
void save_object( FILE *fp, OBJ_INDEX_DATA *pObjIndex )
{
   char buf[MAX_STRING_LENGTH];
   char letter;
    AFFECT_DATA *pAf;
    EXTRA_DESCR_DATA *pEd;

    fprintf( fp, "#%d\n",    pObjIndex->vnum );
    fprintf( fp, "%s~\n",    pObjIndex->name );
    fprintf( fp, "%s~\n",    pObjIndex->short_descr );
    fprintf( fp, "%s~\n",    fix_string( pObjIndex->description ) );
    fprintf( fp, "%s~\n",    material_name( pObjIndex->material ) );
    fprintf( fp, "%d ",      pObjIndex->item_type );
    fprintf( fp, "%s ",      fwrite_flag( pObjIndex->extra_flags, buf ) );
    fprintf( fp, "%s\n",     fwrite_flag( pObjIndex->wear_flags,  buf ) );

/*
 *  Using fwrite_flag to write most values gives a strange
 *  looking area file, consider making a case for each
 *  item type later.
 */

    switch ( pObjIndex->item_type )
    {
        default:
	    fprintf( fp, "%s ",  fwrite_flag( pObjIndex->value[0], buf ) );
	    fprintf( fp, "%s ",  fwrite_flag( pObjIndex->value[1], buf ) );
	    fprintf( fp, "%s ",  fwrite_flag( pObjIndex->value[2], buf ) );
	    fprintf( fp, "%s ",  fwrite_flag( pObjIndex->value[3], buf ) );
	    fprintf( fp, "%s\n", fwrite_flag( pObjIndex->value[4], buf ) );
	    break;

        case ITEM_LIGHT:
	    fprintf( fp, "0 0 %d 0 0\n",
		     pObjIndex->value[2] < 1 ? 999  /* infinite */
		     : pObjIndex->value[2] );
	    break;

        case ITEM_PILL:
        case ITEM_POTION:
        case ITEM_SCROLL:
	    fprintf( fp, "%d %d %d %d %d\n",
		     pObjIndex->value[0] > 0 ? /* no negative numbers */
		     pObjIndex->value[0]
		     : 0,
		     pObjIndex->value[1] != -1 ?
		     skill_table[pObjIndex->value[1]].slot
		     : 0,
		     pObjIndex->value[2] != -1 ?
		     skill_table[pObjIndex->value[2]].slot
		     : 0,
		     pObjIndex->value[3] != -1 ?
		     skill_table[pObjIndex->value[3]].slot
		     : 0,
		     0 /* unused */ );
	    break;

        case ITEM_STAFF:
        case ITEM_WAND:
	    fprintf( fp, "%s ", fwrite_flag( pObjIndex->value[0], buf ) );
	    fprintf( fp, "%s ", fwrite_flag( pObjIndex->value[1], buf ) );
	    fprintf( fp, "%s %d 0\n",
		     fwrite_flag( pObjIndex->value[2], buf ),
		     pObjIndex->value[3] != -1 ?
		       skill_table[pObjIndex->value[3]].slot
		       : 0 );
	    break;
    }

    fprintf( fp, "%d ", pObjIndex->level );
    fprintf( fp, "%d ", pObjIndex->weight );
    fprintf( fp, "%d ", pObjIndex->cost );

         if ( pObjIndex->condition >  90 ) letter = 'P';
    else if ( pObjIndex->condition >  75 ) letter = 'G';
    else if ( pObjIndex->condition >  50 ) letter = 'A';
    else if ( pObjIndex->condition >  25 ) letter = 'W';
    else if ( pObjIndex->condition >  10 ) letter = 'D';
    else if ( pObjIndex->condition >   0 ) letter = 'B';
    else                                   letter = 'R';

    fprintf( fp, "%c\n", letter );

    if( pObjIndex->clan > 0 )
	fprintf( fp, "C %d\n", pObjIndex->clan );

    for( pAf = pObjIndex->affected; pAf; pAf = pAf->next )
    {
        fprintf( fp, "A\n%d %d\n",  pAf->location, pAf->modifier );
    }

    for( pEd = pObjIndex->extra_descr; pEd; pEd = pEd->next )
    {
        fprintf( fp, "E\n%s~\n%s~\n", pEd->keyword,
		 fix_string( pEd->description ) );
    }

    return;
}
 



/*****************************************************************************
 Name:		save_objects
 Purpose:	Save #OBJECTS section of an area file.
 Called by:	save_area(olc_save.c).
 Notes:         Changed for ROM OLC.
 ****************************************************************************/
void save_objects( FILE *fp, AREA_DATA *pArea )
{
    int i;
    OBJ_INDEX_DATA *pObj;

    fprintf( fp, "#OBJECTS\n" );

    for( i = pArea->lvnum; i <= pArea->uvnum; i++ )
    {
	if ( (pObj = get_obj_index( i )) )
	    save_object( fp, pObj );
    }

    fprintf( fp, "#0\n\n\n\n" );
    return;
}
 




/*****************************************************************************
 Name:		save_rooms
 Purpose:	Save #ROOMS section of an area file.
 Called by:	save_area(olc_save.c).
 ****************************************************************************/
void save_rooms( FILE *fp, AREA_DATA *pArea )
{
    ROOM_INDEX_DATA *pRoomIndex;
    EXTRA_DESCR_DATA *pEd;
    EXIT_DATA *pExit;
    int iHash;
    int door;

    fprintf( fp, "#ROOMS\n" );
    for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
        for( pRoomIndex = room_index_hash[iHash]; pRoomIndex; pRoomIndex = pRoomIndex->next )
        {
            if ( pRoomIndex->area == pArea )
            {
                fprintf( fp, "#%d\n",		pRoomIndex->vnum );
                fprintf( fp, "%s~\n",		pRoomIndex->name );
                fprintf( fp, "%s~\n",		fix_string( pRoomIndex->description ) );
		fprintf( fp, "0 " );
                fprintf( fp, "%d ",		pRoomIndex->room_flags );
                fprintf( fp, "%d\n",		pRoomIndex->sector_type );

                for ( pEd = pRoomIndex->extra_descr; pEd;
                      pEd = pEd->next )
                {
                    fprintf( fp, "E\n%s~\n%s~\n", pEd->keyword,
                                                  fix_string( pEd->description ) );
                }
                for( door = 0; door < MAX_DIR; door++ )	/* I hate this! */
                {
                    if ( ( pExit = pRoomIndex->exit[door] )
                          && pExit->u1.to_room )
                    {
			int locks = 0;
			if ( IS_SET( pExit->rs_flags, EX_ISDOOR ) 
			&& ( !IS_SET( pExit->rs_flags, EX_PICKPROOF ) ) 
/* ROM OLC */  		&& ( !IS_SET( pExit->rs_flags, EX_HIDDEN ) ) 
		    	&& ( !IS_SET( pExit->rs_flags, EX_PASSPROOF ) )  )
			    locks = 1;
			if ( IS_SET( pExit->rs_flags, EX_ISDOOR )
			&& ( IS_SET( pExit->rs_flags, EX_PICKPROOF ) )
		        && ( !IS_SET( pExit->rs_flags, EX_HIDDEN ) )
		        && ( !IS_SET( pExit->rs_flags, EX_PASSPROOF ) )  )
			    locks = 2;
/* Removed for ROM OLC */ /*added back by Thexder */
			if ( IS_SET( pExit->rs_flags, EX_ISDOOR )
			&& ( !IS_SET( pExit->rs_flags, EX_PICKPROOF ) )
		        && ( IS_SET( pExit->rs_flags, EX_HIDDEN ) )
		        && ( IS_SET( pExit->rs_flags, EX_PASSPROOF ) ) )
			    locks = 3;
			if ( IS_SET( pExit->rs_flags, EX_ISDOOR )
			&& ( IS_SET( pExit->rs_flags, EX_PICKPROOF ) )
			&& ( IS_SET( pExit->rs_flags, EX_HIDDEN ) ) 
			&& ( IS_SET( pExit->rs_flags, EX_PASSPROOF ) ) )
			    locks = 4;
			if ( IS_SET( pExit->rs_flags, EX_ISDOOR )
			&& ( !IS_SET( pExit->rs_flags, EX_PICKPROOF ) )
			&& ( !IS_SET( pExit->rs_flags, EX_HIDDEN ) )
			&& ( IS_SET( pExit->rs_flags, EX_PASSPROOF ) ) )
			    locks = 5;
			if ( IS_SET( pExit->rs_flags, EX_ISDOOR )
			&& ( IS_SET( pExit->rs_flags, EX_PICKPROOF ) )
			&& ( !IS_SET( pExit->rs_flags, EX_HIDDEN ) )
			&& ( IS_SET( pExit->rs_flags, EX_PASSPROOF ) ) )
			    locks = 6;
			if ( IS_SET( pExit->rs_flags, EX_ISDOOR )
			&& ( !IS_SET( pExit->rs_flags, EX_PICKPROOF ) )
			&& ( IS_SET( pExit->rs_flags, EX_HIDDEN ) )
			&& ( !IS_SET( pExit->rs_flags, EX_PASSPROOF ) ) )
			    locks = 7;
			if ( IS_SET( pExit->rs_flags, EX_ISDOOR )
			&& ( IS_SET( pExit->rs_flags, EX_PICKPROOF ) )
			&& ( IS_SET( pExit->rs_flags, EX_HIDDEN ) )
			&& ( !IS_SET( pExit->rs_flags, EX_PASSPROOF ) ) )
			    locks = 8; 
/* ROM OLC */
			
                        fprintf( fp, "D%d\n",      pExit->orig_door );
                        fprintf( fp, "%s~\n",      fix_string( pExit->description ) );
                        fprintf( fp, "%s~\n",      pExit->keyword );
                        fprintf( fp, "%d %d %d\n", locks,
                                                   pExit->key,
                                                   pExit->u1.to_room->vnum );
                    }
                }
		fprintf( fp, "S\n" );
            }
        }
    }
    fprintf( fp, "#0\n\n\n\n" );
    return;
}

/*
 * Name: save_raffects
 * Purpose: Save #RAFFECTS section of area file.
 * Called by: save_area(olc_save.c)
 */
void save_raffects( FILE *fp, AREA_DATA *pArea )
{
   int vnum;
   ROOM_INDEX_DATA *pRoomIndex;
   RAFFECT_DATA *raf;
   
   fprintf( fp, "#RAFFECTS\n" );
   
   for( vnum = pArea->lvnum; vnum <= pArea->uvnum; vnum++)
     {
	if ( ( pRoomIndex = get_room_index( vnum ) ) )
	  {
	     if ( pRoomIndex->raffect != NULL)
	       {
		  for( raf = pRoomIndex->raffect; raf != NULL; raf = raf->next)
		    {
		       fprintf(fp, "#%d\n", vnum);
		       fprintf(fp, "%d\n", raf->type);
		       fprintf(fp, "%s~\n%s~\n%s~\n", raf->dam_name,
			       raf->room_message, raf->vict_message);
		       fprintf(fp, "%d %d %d %d %d %d\n", raf->timer,
			       raf->val0, raf->val1, raf->val2, raf->val3, raf->val4);
		    }
	       }
	  }
     }
   fprintf(fp, "#0\n\n\n");
   return;
}


/*****************************************************************************
 Name:          save_mobprogs
 Purpose:       Save #MOBPROGS section of area file.    -- By Maniac
 Called by:     save_area(olc_save.c).
 ****************************************************************************/
void save_mobprogs( FILE *fp, AREA_DATA *pArea )
{
     int vnum;
     MOB_INDEX_DATA *pMobIndex;
 
     fprintf( fp, "#MOBPROGS\n" );
     for( vnum = pArea->lvnum; vnum <= pArea->uvnum; vnum++ )
     {
         if( ( pMobIndex = get_mob_index(vnum) ) )
         {
             if ( pMobIndex->area == pArea && pMobIndex->progtypes ) /* prog */
             {
                 /* Step 1: Write the prog call to the area file */
                 fprintf( fp, "M %d %d.prg\n",
                 pMobIndex->vnum,
                 pMobIndex->vnum );
                 /* Step 2: Create the .prg file... this is the hard part */
                 write_mobprog(pMobIndex);
             }
         }
     }
 
     fprintf( fp, "S\n\n\n\n" );
     return;
 }
 
/*****************************************************************************
 Name:          save_roomprogs
 Purpose:       Save #ROOMPROGS section of area file.    -- By Maniac
 Called by:     save_area(olc_save.c).
 ****************************************************************************/
void save_roomprogs( FILE *fp, AREA_DATA *pArea )
{
     int vnum;
     ROOM_INDEX_DATA *pRoom;
 
     fprintf( fp, "#ROOMPROGS\n" );
     for( vnum = pArea->lvnum; vnum <= pArea->uvnum; vnum++ )
     {
         if( ( pRoom = get_room_index(vnum) ) )
         {
             if ( pRoom->area == pArea && pRoom->progtypes ) /* prog */
             {
                 /* Step 1: Write the prog call to the area file */
                 fprintf( fp, "R %d %d.prg\n",
                     pRoom->vnum,
                     pRoom->vnum );
                 /* Step 2: Create the .prg file... this is the hard part */
                 write_roomprog(pRoom);
             }
         }
     }
 
     fprintf( fp, "S\n\n\n\n" );
     return;
}
  
/*****************************************************************************
 Name:          save_objprogs
 Purpose:       Save #OBJPROGS section of area file.    -- By Maniac
 Called by:     save_area(olc_save.c).
 ****************************************************************************/
void save_objprogs( FILE *fp, AREA_DATA *pArea )
{
     int vnum;
     OBJ_INDEX_DATA *pObj;
  
     fprintf( fp, "#OBJPROGS\n" );
     for( vnum = pArea->lvnum; vnum <= pArea->uvnum; vnum++ )
     {
         if( ( pObj = get_obj_index(vnum) ) )
         {
             if ( pObj->area == pArea && pObj->progtypes ) /* prog */
             {
                 /* Step 1: Write the prog call to the area file */
                 fprintf( fp, "M %d %d.prg\n",
                     pObj->vnum,
                     pObj->vnum );
                 /* Step 2: Create the .prg file... this is the hard part */
                 write_objprog(pObj);
             }
         }
     }
 
     fprintf( fp, "S\n\n\n\n" );
     return;
}			       


/*****************************************************************************
 Name:		save_specials
 Purpose:	Save #SPECIALS section of area file.
 Called by:	save_area(olc_save.c).
 ****************************************************************************/
void save_specials( FILE *fp, AREA_DATA *pArea )
{
    int iHash;
    MOB_INDEX_DATA *pMobIndex;
    
    fprintf( fp, "#SPECIALS\n" );

    for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
        for( pMobIndex = mob_index_hash[iHash]; pMobIndex; pMobIndex = pMobIndex->next )
        {
            if ( pMobIndex && pMobIndex->area == pArea && pMobIndex->spec_fun )
            {
                fprintf( fp, "M %d %s Load to: %s\n", pMobIndex->vnum,
                                                      spec_string( pMobIndex->spec_fun ),
                                                      pMobIndex->short_descr );
            }
        }
    }

    fprintf( fp, "S\n\n\n\n" );
    return;
}



/*
 * This function is obsolete.  It it not needed but has been left here
 * for historical reasons.  It is used currently for the same reason.
 *
 * I don't think it's obsolete in ROM -- Hugin.
 */
void save_door_resets( FILE *fp, AREA_DATA *pArea )
{
    int iHash;
    ROOM_INDEX_DATA *pRoomIndex;
    EXIT_DATA *pExit;
    int door;
    int flags=0;

    for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
        for( pRoomIndex = room_index_hash[iHash]; pRoomIndex; pRoomIndex = pRoomIndex->next )
        {
            if ( pRoomIndex->area == pArea )
            {
                for( door = 0; door < MAX_DIR; door++ )
                {
                    if ( ( pExit = pRoomIndex->exit[door] )
                          && pExit->u1.to_room 
                          && ( IS_SET( pExit->rs_flags, EX_CLOSED )
                          || IS_SET( pExit->rs_flags, EX_LOCKED ) ) ) {
                          if IS_SET( pExit->rs_flags, EX_CLOSED ) flags=0;
                          if IS_SET( pExit->rs_flags, EX_LOCKED ) flags=1;
                          if IS_SET( pExit->rs_flags, EX_PICKPROOF ) flags=2;
	     		  fprintf( fp, "D 0 %d %d %d\n", 
	 			pRoomIndex->vnum,
				pExit->orig_door,
				flags);
		    }
		}
	    }
	}
    }
    return;
}




/*****************************************************************************
 Name:		save_resets
 Purpose:	Saves the #RESETS section of an area file.
 Called by:	save_area(olc_save.c)
 ****************************************************************************/
void save_resets( FILE *fp, AREA_DATA *pArea )
{
char buf[MAX_STRING_LENGTH];

   RESET_DATA *pReset;
    MOB_INDEX_DATA *pLastMob = NULL;
    OBJ_INDEX_DATA *pLastObj;
    ROOM_INDEX_DATA *pRoom;
    int iHash;

    fprintf( fp, "#RESETS\n" );

    save_door_resets( fp, pArea );

    for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
        for( pRoom = room_index_hash[iHash]; pRoom; pRoom = pRoom->next )
        {
            if ( pRoom->area == pArea )
	    {
    for ( pReset = pRoom->reset_first; pReset; pReset = pReset->next )
    {
#ifndef IS_PLAY_SITE
	if( pReset->not_save==TRUE ) continue;
#endif
	switch ( pReset->command )
	{
	default:
	    bug( "Save_resets: bad command %c.", pReset->command );
	    break;

#if defined( VERBOSE )
	case 'M':
            pLastMob = get_mob_index( pReset->arg1 );
	    fprintf( fp, "M 0 %d %d %d Load %s\n", 
	        pReset->arg1,
                pReset->arg2,
                pReset->arg3,
                pLastMob->short_descr );
            break;

	case 'O':
            pLastObj = get_obj_index( pReset->arg1 );
            pRoom = get_room_index( pReset->arg3 );
	    fprintf( fp, "O 0 %d 0 %d %s loaded to %s\n", 
	        pReset->arg1,
                pReset->arg3,
                capitalize(pLastObj->short_descr),
                pRoom->name );
            break;

	case 'P':
            pLastObj = get_obj_index( pReset->arg1 );
	    fprintf( fp, "P 0 %d 0 %d %s put inside %s\n", 
	        pReset->arg1,
                pReset->arg3,
                capitalize(get_obj_index( pReset->arg1 )->short_descr),
                pLastObj->short_descr );
            break;

	case 'G':
	    fprintf( fp, "G 0 %d 0 %s is given to %s\n",
	        pReset->arg1,
	        capitalize(get_obj_index( pReset->arg1 )->short_descr),
                pLastMob ? pLastMob->short_descr : "!NO_MOB!" );
            if ( !pLastMob )
            {
                sprintf( buf, "Save_resets: !NO_MOB! in [%s]", pArea->filename );
                bug( buf, 0 );
            }
            break;

	case 'E':
	    fprintf( fp, "E 0 %d 0 %d %s is loaded %s of %s\n",
	        pReset->arg1,
                pReset->arg3,
                capitalize(get_obj_index( pReset->arg1 )->short_descr),
                flag_string( wear_loc_strings, pReset->arg3 ),
                pLastMob ? pLastMob->short_descr : "!NO_MOB!" );
            if ( !pLastMob )
            {
                sprintf( buf, "Save_resets: !NO_MOB! in [%s]", pArea->filename );
                bug( buf, 0 );
            }
            break;

	case 'D':
            break;

	case 'R':
            pRoom = get_room_index( pReset->arg1 );
	    fprintf( fp, "R 0 %d %d Randomize %s\n", 
	        pReset->arg1,
                pReset->arg2,
                pRoom->name );
            break;
            }
#endif
#if !defined( VERBOSE )
	case 'M':
            pLastMob = get_mob_index( pReset->arg1 );
	    fprintf( fp, "M 0 %d %d %d\n", 
	        pReset->arg1,
                pReset->arg2,
                pReset->arg3 );
            break;

	case 'O':
            pLastObj = get_obj_index( pReset->arg1 );
            pRoom = get_room_index( pReset->arg3 );
	    fprintf( fp, "O 0 %d 0 %d\n", 
	        pReset->arg1,
                pReset->arg3 );
            break;

	case 'P':
	if ( pRoom->vnum == 0 )
		bug("Possible object %d to room 0", pReset->arg1 );
            pLastObj = get_obj_index( pReset->arg1 );
	    fprintf( fp, "P 0 %d 0 %d\n", 
	        pReset->arg1,
                pReset->arg3  );
            break;

	case 'G':
	    fprintf( fp, "G 0 %d 0\n", pReset->arg1 );
            if ( !pLastMob )
            {
                sprintf( buf,
                    "Save_resets: !NO_MOB! in [%s]", pArea->filename );
                bug( buf, 0 );
            }
            break;

	case 'E':
	    fprintf( fp, "E 0 %d 0 %d\n",
	        pReset->arg1,
                pReset->arg3 );
            if ( !pLastMob )
            {
                sprintf( buf,
                    "Save_resets: !NO_MOB! in [%s]", pArea->filename );
                bug( buf, 0 );
            }
            break;

	case 'D':
            break;

	case 'R':
            pRoom = get_room_index( pReset->arg1 );
	    fprintf( fp, "R 0 %d %d\n", 
	        pReset->arg1,
                pReset->arg2 );
            break;
            }
#endif
        }
	    }	/* End if correct area */
	}	/* End for pRoom */
    }	/* End for iHash */
    fprintf( fp, "S\n\n\n\n" );
    return;
}



/*****************************************************************************
 Name:		save_shops
 Purpose:	Saves the #SHOPS section of an area file.
 Called by:	save_area(olc_save.c)
 ****************************************************************************/
void save_shops( FILE *fp, AREA_DATA *pArea )
{
    SHOP_DATA *pShopIndex;
    MOB_INDEX_DATA *pMobIndex;
    int iTrade;
    int iHash;
    
    fprintf( fp, "#SHOPS\n" );

    for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
        for( pMobIndex = mob_index_hash[iHash]; pMobIndex; pMobIndex = pMobIndex->next )
        {
            if ( pMobIndex && pMobIndex->area == pArea && pMobIndex->pShop )
            {
                pShopIndex = pMobIndex->pShop;

                fprintf( fp, "%d ", pShopIndex->keeper );
                for ( iTrade = 0; iTrade < MAX_TRADE; iTrade++ )
                {
                    if ( pShopIndex->buy_type[iTrade] != 0 )
                    {
                       fprintf( fp, "%d ", pShopIndex->buy_type[iTrade] );
                    }
                    else
                       fprintf( fp, "0 ");
                }
                fprintf( fp, "%d %d ", pShopIndex->profit_buy, pShopIndex->profit_sell );
                fprintf( fp, "%d %d\n", pShopIndex->open_hour, pShopIndex->close_hour );
            }
        }
    }

    fprintf( fp, "0\n\n\n\n" );
    return;
}



/*****************************************************************************
 Name:		save_area
 Purpose:	Save an area, note that this format is new.
 Called by:	do_asave(olc_save.c).
 ****************************************************************************/
void save_area( AREA_DATA *pArea )
{
    FILE *fp;

    fclose( fpReserve );
    if ( !( fp = fopen( pArea->filename, "w" ) ) )
    {
	bug( "Open_area: fopen", 0 );
       perror( pArea->filename );
    }
   
   fprintf( fp, "#AREADATA\n" );
   fprintf( fp, "Name        %s~\n",        pArea->name );
   fprintf( fp, "Builders    %s~\n",        fix_string( pArea->builders ) );
   fprintf( fp, "VNUMs       %d %d\n",      pArea->lvnum, pArea->uvnum );
   fprintf( fp, "Security    %d\n",         pArea->security );
/*    fprintf( fp, "Recall      %d\n",         pArea->recall );  ROM OLC */
   fprintf( fp, "End\n\n\n\n" );
   
   save_mobiles( fp, pArea );
   save_objects( fp, pArea );
   save_rooms( fp, pArea );
   save_raffects( fp, pArea );
   save_specials( fp, pArea );
   save_resets( fp, pArea );
   save_mobprogs ( fp, pArea 	);
   save_roomprogs( fp, pArea	);
   save_objprogs ( fp, pArea	);
   save_shops( fp, pArea );
   
   fprintf( fp, "#$\n" );
   
   fclose( fp );
   fpReserve = fopen( NULL_FILE, "r" );
   return;
}



/*****************************************************************************
 Name:		do_asave
 Purpose:	Entry point for saving area data.
 Called by:	interpreter(interp.c)
 ****************************************************************************/
void do_asave( CHAR_DATA *ch, char *argument )
{
char buf[MAX_STRING_LENGTH];
char arg[MAX_INPUT_LENGTH];
   AREA_DATA *pArea;
    FILE *fp;
    int value;

    fp = NULL;

    if ( !ch )       /* Do an autosave */
    {
	save_area_list();
	for( pArea = area_first; pArea; pArea = pArea->next )
	{
	    save_area( pArea );
	    REMOVE_BIT( pArea->area_flags, AREA_CHANGED );
	}
	return;
    }

    smash_tilde( argument );
    strcpy( arg, argument );

    if ( arg[0] == '\0' )
    {
    send_to_char( "Syntax:\n\r", ch );
    send_to_char( "  asave <vnum>   - saves a particular area\n\r",	ch );
    send_to_char( "  asave list     - saves the area.lst file\n\r",	ch );
    send_to_char( "  asave area     - saves the area being edited\n\r",	ch );
    send_to_char( "  asave changed  - saves all changed zones\n\r",	ch );
    send_to_char( "  asave world    - saves the world! (db dump)\n\r",	ch );
/*    send_to_char( "  asave clans    - saves the clans.are file\n\r",    ch ); */
    send_to_char( "\n\r", ch );
        return;
    }

    /* Snarf the value (which need not be numeric). */
    value = atoi( arg );

    if ( !( pArea = get_area_data( value ) ) && is_number( arg ) )
    {
	send_to_char( "That area does not exist.\n\r", ch );
	return;
    }

    /* Save area of given vnum. */
    /* ------------------------ */

    if ( is_number( arg ) )
    {
	if ( !IS_BUILDER( ch, pArea ) )
	{
	    send_to_char( "You are not a builder for this area.\n\r", ch );
	    return;
	}
	save_area_list();
	save_area( pArea );
	return;
    }


    /* Save the clan file */
    /* ------------------ */
/*  
 *   if ( !str_cmp( "clans", arg ))
 *   {
 *	 save_clans();
 *	 send_to_char( "Clan file saved!\n\r", ch);
 *	 return;
 *   }
 */

    /* Save the world, only authorized areas. */
    /* -------------------------------------- */

    if ( !str_cmp( "world", arg ) )
    {
	save_area_list();
/*	save_clans(); */
	for( pArea = area_first; pArea; pArea = pArea->next )
	{
	    /* Builder must be assigned this area. */
	    if ( !IS_BUILDER( ch, pArea ) )
		continue;	  

	    save_area( pArea );
	    REMOVE_BIT( pArea->area_flags, AREA_CHANGED );
	}
	send_to_char( "You saved the world.\n\r", ch );
/*	send_to_all_char( "Database saved.\n\r" );                 ROM OLC */
	return;
    }

    /* Save changed areas, only authorized areas. */
    /* ------------------------------------------ */

    if ( !str_cmp( "changed", arg ) )
    {
	save_area_list();
/*	save_clans(); */

	send_to_char( "Saved zones:\n\r", ch );
	sprintf( buf, "None.\n\r" );

	for( pArea = area_first; pArea; pArea = pArea->next )
	{
	    /* Builder must be assigned this area. */
	    if ( !IS_BUILDER( ch, pArea ) )
		continue;

	    /* Save changed areas. */
	    if ( IS_SET(pArea->area_flags, AREA_CHANGED) )
	    {
		save_area( pArea );
		sprintf( buf, "%24s - '%s'\n\r", pArea->name, pArea->filename );
		send_to_char( buf, ch );
		REMOVE_BIT( pArea->area_flags, AREA_CHANGED );
	    }
        }
	if ( !str_cmp( buf, "None.\n\r" ) )
	    send_to_char( buf, ch );
        return;
    }

    /* Save the area.lst file. */
    /* ----------------------- */
    if ( !str_cmp( arg, "list" ) )
    {
	save_area_list();
	return;
    }

    /* Save area being edited, if authorized. */
    /* -------------------------------------- */
    if ( !str_cmp( arg, "area" ) )
    {
	/* Is character currently editing. */
	if ( ch->desc->editor == 0 )
	{
	    send_to_char( "You are not editing an area, "
		"therefore an area vnum is required.\n\r", ch );
	    return;
	}
	
	/* Find the area to save. */
	switch (ch->desc->editor)
	{
	    case ED_AREA:
		pArea = (AREA_DATA *)ch->desc->pEdit;
		break;
	    case ED_ROOM:
		pArea = ch->in_room->area;
		break;
	    case ED_OBJECT:
		pArea = ( (OBJ_INDEX_DATA *)ch->desc->pEdit )->area;
		break;
	    case ED_MOBILE:
		pArea = ( (MOB_INDEX_DATA *)ch->desc->pEdit )->area;
		break;
	    default:
		pArea = ch->in_room->area;
		break;
	}

	if ( !IS_BUILDER( ch, pArea ) )
	{
	    send_to_char( "You are not a builder for this area.\n\r", ch );
	    return;
	}

	save_area_list();
	save_area( pArea );
	REMOVE_BIT( pArea->area_flags, AREA_CHANGED );
	send_to_char( "Area saved.\n\r", ch );
	return;
    }

    /* Show correct syntax. */
    /* -------------------- */
    do_asave( ch, "" );
    return;
}

/* Not using this yet.... But it looks fun to do later. */
/*
void save_helps()
{
     HELP_DATA *pHelp;
     FILE *fp;
     
         fclose( fpReserve );
 
 	if ( !( fp = fopen(HELP_FILE, "w") ) )
 	{
                 bug( "help.are:  fopen");
                 perror( HELP_FILE );
 		return;
 	}
 	
 	fprintf( fp, "#HELPS\n\n"		);
 	
 	for ( pHelp = help_first; pHelp; pHelp = pHelp->next )
 	{
 	
 		fprintf( fp, "%d", pHelp->level 	);
 		fprintf( fp, " %s", pHelp->keyword	);
 		fprintf( fp, "~\n"			);
 		fprintf( fp, "%s~", fix_string( pHelp->text ) );
 		fprintf( fp, "\n"			);
 		
 	}
 	
 	fprintf( fp, "\n\n0 $~"		);
 	fprintf( fp, "\n\n#$\n"		);
 	fclose( fp );
         fpReserve = fopen( NULL_FILE, "r" );	
 	return;
}
*/

void write_mobprog( MOB_INDEX_DATA * pMobIndex )
{
 	FILE *		fp;
 	MPROG_DATA *	mprg;
 	char		filename[50];
 
 	filename[0] = '\0';
 
	sprintf (filename, "%s%d.prg", MOB_DIR, pMobIndex->vnum );
 
 	if ( !(fp = fopen( filename, "w" ) ) )
 	{
 		bug ("write_mobprog: open", 0 );
 		perror ( filename );
 	}
 
 	for ( mprg = pMobIndex->mudprogs; mprg; mprg = mprg->next )
 	{
 		fprintf (fp, ">%s %s~\n%s~\n",
 			mprog_type_to_name( mprg->type ),
 			mprg->arglist,
 			mprg->comlist );
 	}
 	fprintf (fp, "|\n");
 
 	fclose (fp);
 	return;
}
 
void write_roomprog( ROOM_INDEX_DATA * pRoom )
{
 	FILE *		fp;
 	MPROG_DATA *	mprg;
 	char		filename[50];
 
 	filename[0] = '\0';
 
 	sprintf (filename, "%s%d.prg", ROOM_DIR, pRoom->vnum );
 
 	if ( !(fp = fopen( filename, "w" ) ) )
 	{
 		bug ("write_roomprog: open", 0 );
 		perror ( filename );
 	}
 
 	for ( mprg = pRoom->mudprogs; mprg; mprg = mprg->next )
 	{
 		fprintf (fp, ">%s %s~\n%s~\n",
 			mprog_type_to_name( mprg->type ),
 			mprg->arglist,
 			mprg->comlist );
 	}
 	fprintf (fp, "|\n");
 
 	fclose (fp);
 	return;
}
 
void write_objprog( OBJ_INDEX_DATA * pObj )
{
 	FILE *		fp;
 	MPROG_DATA *	mprg;
 	char		filename[50];
 
 	filename[0] = '\0';
 
 	sprintf (filename, "%s%d.prg", OBJ_DIR, pObj->vnum );
 
 	if ( !(fp = fopen( filename, "w" ) ) )
 	{
 		bug ("write_objprog: open",0 );
 		perror ( filename );
 	}
 
 	for ( mprg = pObj->mudprogs; mprg; mprg = mprg->next )
 	{
 		fprintf (fp, ">%s %s~\n%s~\n",
 			mprog_type_to_name( mprg->type ),
 			mprg->arglist,
 			mprg->comlist );
 	}
 	fprintf (fp, "|\n");
 
 	fclose (fp);
 	return;
}
