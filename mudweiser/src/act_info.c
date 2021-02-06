/* act_info.c */

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include "merc.h"

/* command procedures needed */
DECLARE_DO_FUN(	do_exits	);
DECLARE_DO_FUN( do_look		);
DECLARE_DO_FUN( do_help		);

char buf[MAX_STRING_LENGTH];

char *	const	where_name	[] =
{
    "<used as light>     ",
    "<worn on finger>    ",
    "<worn on finger>    ",
    "<worn around neck>  ",
    "<worn around neck>  ",
    "<worn on body>      ",
    "<worn on head>      ",
    "<worn on legs>      ",
    "<worn on feet>      ",
    "<worn on hands>     ",
    "<worn on arms>      ",
    "<worn as shield>    ",
    "<worn about body>   ",
    "<worn about waist>  ",
    "<worn around wrist> ",
    "<worn around wrist> ",
    "<wielded>           ",
    "<held>              ",
    "<secondary wield>   "
};

/*
 * Local functions.
 */
char *	format_obj_to_char	args( ( OBJ_DATA *obj, CHAR_DATA *ch,
				    bool fShort ) );
void	show_list_to_char	args( ( OBJ_DATA *list, CHAR_DATA *ch,
				    bool fShort, bool fShowNothing ) );
void	show_char_to_char_0	args( ( CHAR_DATA *victim, CHAR_DATA *ch ) );
void	show_char_to_char_1	args( ( CHAR_DATA *victim, CHAR_DATA *ch ) );
void	show_char_to_char	args( ( CHAR_DATA *list, CHAR_DATA *ch ) );
bool	check_blind		args( ( CHAR_DATA *ch ) );
/***/
CLAN_DATA * get_clan    args( ( int clannum ) );

char *format_obj_to_char( OBJ_DATA *obj, CHAR_DATA *ch, bool fShort )
{
   buf[0] = '\0';
    if ( IS_OBJ_STAT(obj, ITEM_INVIS)     )   strcat( buf, "(Invis) "     );
    if ( IS_AFFECTED(ch, AFF_DETECT_EVIL)
         && IS_OBJ_STAT(obj, ITEM_EVIL)   )   strcat( buf, "(Red Aura) "  );
    if ( IS_AFFECTED(ch, AFF_DETECT_MAGIC)
         && IS_OBJ_STAT(obj, ITEM_MAGIC)  )   strcat( buf, "(Magical) "   );
    if ( IS_OBJ_STAT(obj, ITEM_GLOW)      )   strcat( buf, "(Glowing) "   );
    if ( IS_OBJ_STAT(obj, ITEM_HUM)       )   strcat( buf, "(Humming) "   );

    if ( fShort )
    {
	if ( obj->short_descr != NULL )
	    strcat( buf, obj->short_descr );
    }
    else
    {
	if ( obj->description != NULL )
	    strcat( buf, obj->description );
    }

    return buf;
}

/*
 * Show a list to a character.
 * Can coalesce duplicated items.
 */
void show_list_to_char( OBJ_DATA *list, CHAR_DATA *ch, bool fShort, bool fShowNothing )
{
   char buf[MAX_STRING_LENGTH]; 
   char **prgpstrShow;
    int *prgnShow;
    char *pstrShow;
    OBJ_DATA *obj;
    int nShow;
    int iShow;
    int count;
    bool fCombine;

    if ( ch->desc == NULL )
	return;

    /*
     * Alloc space for output lines.
     */
    count = 0;
    for ( obj = list; obj != NULL; obj = obj->next_content )
	count++;
    prgpstrShow	= alloc_mem( count * sizeof(char *) );
    prgnShow    = alloc_mem( count * sizeof(int)    );
    nShow	= 0;

    /*
     * Format the list of objects.
     */
    for ( obj = list; obj != NULL; obj = obj->next_content )
    { 
	if ( obj->wear_loc == WEAR_NONE && can_see_obj( ch, obj ) )
	{
	    pstrShow = format_obj_to_char( obj, ch, fShort );
	    fCombine = FALSE;

	    if ( IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE) )
	    {
		/*
		 * Look for duplicates, case sensitive.
		 * Matches tend to be near end so run loop backwords.
		 */
		for ( iShow = nShow - 1; iShow >= 0; iShow-- )
		{
		    if ( !strcmp( prgpstrShow[iShow], pstrShow ) )
		    {
			prgnShow[iShow]++;
			fCombine = TRUE;
			break;
		    }
		}
	    }

	    /*
	     * Couldn't combine, or didn't want to.
	     */
	    if ( !fCombine )
	    {
		prgpstrShow [nShow] = str_dup( pstrShow );
		prgnShow    [nShow] = 1;
		nShow++;
	    }
	}
    }

    /*
     * Output the formatted list.
     */
    for ( iShow = 0; iShow < nShow; iShow++ )
    {
	if ( IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE) )
	{
	    if ( prgnShow[iShow] != 1 )
	    {
		sprintf( buf, "`g(%2d) ", prgnShow[iShow] );
		send_to_char( buf, ch );
	    }
	    else
	    {
		send_to_char( "`g     ", ch );
	    }
	}
	send_to_char( prgpstrShow[iShow], ch );
	send_to_char( "\n\r`w", ch );
	free_string( prgpstrShow[iShow] );
    }

    if ( fShowNothing && nShow == 0 )
    {
	if ( IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE) )
	    send_to_char( "`g     ", ch );
	send_to_char( "Nothing.\n\r`w", ch );
    }

    /*
     * Clean up.
     */
    free_mem( prgpstrShow, count * sizeof(char *) );
    free_mem( prgnShow,    count * sizeof(int)    );

    return;
}

void show_char_to_char_0( CHAR_DATA *victim, CHAR_DATA *ch )
{
      char buf[MAX_STRING_LENGTH]; 

   buf[0] = '\0';
   
   if ( IS_AFFECTED(victim, AFF_INVISIBLE)   ) strcat( buf, "`W(Invis) `C"      );
   if ( IS_SET(victim->act, PLR_WIZINVIS)
       && !IS_NPC(victim)   ) strcat( buf, "`B(Wizi) `C"	     );
   if ( IS_AFFECTED(victim, AFF_HIDE)        ) strcat( buf, "`K(Hide) `C"       );
   if ( IS_AFFECTED(victim, AFF_CHARM)       ) strcat( buf, "`Y(Charmed) `C"    );
   if ( IS_AFFECTED(victim, AFF_PASS_DOOR)   ) strcat( buf, "`w(Translucent) `C");
   if ( IS_AFFECTED(victim, AFF_FAERIE_FIRE) ) strcat( buf, "`R(Pink Aura) `C"  );
   if ( IS_EVIL(victim)
       &&   IS_AFFECTED(ch, AFF_DETECT_EVIL)     ) strcat( buf, "`R(Red Aura) `C"   );
   if ( IS_AFFECTED(victim, AFF_SANCTUARY)   ) strcat( buf, "`W(White Aura) `C" );
   if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_AFK ) )
     strcat( buf, "`W(AFK) `C"     );
   if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_KILLER ) )
     strcat( buf, "`R(PK) `C"     );
   if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_THIEF  ) )
     strcat( buf, "`K(THIEF) `C"      );
   
   if ( victim->position == victim->start_pos && victim->long_descr[0] != '\0' )
     {
    	strcat( buf, "`c" );
	strcat( buf, victim->long_descr );
    }
   if ( IS_AFFECTED(victim, AFF_WEB) && IS_NPC(victim) 
       && victim->position == victim->start_pos) 
     {
        strcat( buf, "`g" );
	strcat( buf, PERS( victim, ch ) );
	strcat( buf, " is covered in sticky webs.\n`w");
     }
   if ( victim->position == victim->start_pos && victim->long_descr[0] != '\0' )
     {
	send_to_char( buf, ch );
	send_to_char( "`w", ch);
	return;
     }
   
   
    strcat( buf, PERS( victim, ch ) );
   if ( !IS_NPC(victim) && !IS_SET(ch->comm, COMM_BRIEF) )
     strcat( buf, victim->pcdata->title );
   
   switch ( victim->position )
     {
      case POS_DEAD:     strcat( buf, " `Cis `RDEAD`C!!" );              break;
      case POS_MORTAL:   strcat( buf, " `Cis mortally wounded." );   break;
      case POS_INCAP:    strcat( buf, " `Cis incapacitated." );      break;
      case POS_STUNNED:  strcat( buf, " `Cis lying here stunned." ); break;
      case POS_SLEEPING: strcat( buf, " `Cis sleeping here." );      break;
      case POS_RESTING:  strcat( buf, " `Cis resting here." );       break;
      case POS_SITTING:  strcat( buf, " `Cis sitting here." );	     break;
      case POS_STANDING: strcat( buf, " `Cis here." );               break;
      case POS_FIGHTING:
	strcat( buf, " is here, fighting " );
	if ( victim->fighting == NULL )
	  strcat( buf, "thin air??" );
	else if ( victim->fighting == ch )
	  strcat( buf, "YOU!  Doh..." );
	else if ( victim->in_room == victim->fighting->in_room )
	  {
	     strcat( buf, PERS( victim->fighting, ch ) );
	     strcat( buf, "." );
	  }
	else
	  strcat( buf, "somone who left??" );
	break;
     }
   if ( IS_AFFECTED(victim, AFF_WEB)   ) 
     {
        strcat( buf, "\n\r`g" );
	strcat( buf, PERS( victim, ch ) );
	strcat( buf, " is covered in sticky webs.`w");
     }
   strcat( buf, "\n\r`w" );
   buf[0] = UPPER(buf[0]);
   send_to_char( "`C", ch );
   send_to_char( buf, ch );
   return;
}

void show_char_to_char_1( CHAR_DATA *victim, CHAR_DATA *ch )
{
    
      char buf[MAX_STRING_LENGTH]; 

   OBJ_DATA *obj;
    int iWear;
    int percent;
    bool found;

    if ( can_see( victim, ch ) )
    {
	if (ch == victim)
	    act( "$n looks at $mself.",ch,NULL,NULL,TO_ROOM);
	else
	{
	    act( "$n looks at you.", ch, NULL, victim, TO_VICT    );
	    act( "$n looks at $N.",  ch, NULL, victim, TO_NOTVICT );
	}
    }

   if ( victim->description[0] != '\0' )
     {
	send_to_char( victim->description, ch );
	send_to_char( "`w", ch );
     }
    else
    {
	act( "You see nothing special about $M.", ch, NULL, victim, TO_CHAR );
    }

    if ( victim->max_hit > 0 )
	percent = ( 100 * victim->hit ) / victim->max_hit;
    else
	percent = -1;

    strcpy( buf, PERS(victim, ch) );

    if (percent >= 100) 
	strcat( buf, " is in excellent condition.\n\r");
    else if (percent >= 90) 
	strcat( buf, " has a few scratches.\n\r");
    else if (percent >= 75) 
	strcat( buf," has some small wounds and bruises.\n\r");
    else if (percent >=  50) 
	strcat( buf, " has quite a few wounds.\n\r");
    else if (percent >= 30)
	strcat( buf, " has some big nasty wounds and scratches.\n\r");
    else if (percent >= 15)
	strcat ( buf, " looks pretty hurt.\n\r");
    else if (percent >= 0 )
	strcat (buf, " is in awful condition.\n\r");
    else
	strcat(buf, " is `Rbleeding to death.`w\n\r");

    buf[0] = UPPER(buf[0]);
    send_to_char( buf, ch );

    found = FALSE;
    for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
    {
	if ( ( obj = get_eq_char( victim, iWear ) ) != NULL
	&&   can_see_obj( ch, obj ) )
	{
	    if ( !found )
	    {
		send_to_char( "\n\r", ch );
		act( "$N is using:", ch, NULL, victim, TO_CHAR );
		found = TRUE;
	    }
	    send_to_char( where_name[iWear], ch );
	    send_to_char( format_obj_to_char( obj, ch, TRUE ), ch );
	    send_to_char( "\n\r", ch );
	}
    }

    if ( victim != ch
    &&   !IS_NPC(ch)
    &&   number_percent( ) < ch->pcdata->learned[gsn_peek] )
    {
	send_to_char( "\n\rYou peek at the inventory:\n\r", ch );
	check_improve(ch,gsn_peek,TRUE,4);
	show_list_to_char( victim->carrying, ch, TRUE, TRUE );
    }

    return;
}

void show_char_to_char( CHAR_DATA *list, CHAR_DATA *ch )
{
   CHAR_DATA *rch;

    for ( rch = list; rch != NULL; rch = rch->next_in_room )
    {
	if ( rch == ch )
	    continue;

	if ( !IS_NPC(rch)
	&&   IS_SET(rch->act, PLR_WIZINVIS)
	&&   get_trust( ch ) < rch->invis_level )
	    continue;

	if ( can_see( ch, rch ) )
	{
	    show_char_to_char_0( rch, ch );
	}
	else if ( room_is_dark( ch->in_room )
	&&        IS_AFFECTED(rch, AFF_INFRARED ) )
	{
	    send_to_char( "`RYou see glowing red eyes watching you!\n\r`w", ch );
	}
    }

    return;
} 

bool check_blind( CHAR_DATA *ch )
{

    if (!IS_NPC(ch) && IS_SET(ch->act,PLR_HOLYLIGHT))
	return TRUE;

    if ( IS_AFFECTED(ch, AFF_BLIND) )
    { 
	send_to_char( "You can't see a thing!\n\r", ch ); 
	return FALSE; 
    }

    return TRUE;
}

/* changes your scroll */
void do_scroll(CHAR_DATA *ch, char *argument)
{
   char arg[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   
   int lines;

    one_argument(argument,arg);
    
    if (arg[0] == '\0')
    {
	if (ch->lines == 0)
	    send_to_char("You do not page long messages.\n\r",ch);
	else
	{
	    sprintf(buf,"You currently display %d lines per page.\n\r",
		    ch->lines + 2);
	    send_to_char(buf,ch);
	}
	return;
    }

    if (!is_number(arg))
    {
	send_to_char("You must provide a number.\n\r",ch);
	return;
    }

    lines = atoi(arg);

    if (lines == 0)
    {
        send_to_char("Paging disabled.\n\r",ch);
        ch->lines = 0;
        return;
    }

    if (lines < 10 || lines > 100)
    {
	send_to_char("You must provide a reasonable number.\n\r",ch);
	return;
    }

    sprintf(buf,"Scroll set to %d lines.\n\r",lines);
    send_to_char(buf,ch);
    ch->lines = lines - 2;
}

/* RT does socials */
void do_socials(CHAR_DATA *ch, char *argument)
{
   char buf[MAX_STRING_LENGTH]; 
   
   int iSocial;
    int col;
     
    col = 0;
   
    for (iSocial = 0; social_table[iSocial].name[0] != '\0'; iSocial++)
    {
	sprintf(buf,"%-12s",social_table[iSocial].name);
	send_to_char(buf,ch);
	if (++col % 6 == 0)
	    send_to_char("\n\r",ch);
    }

    if ( col % 6 != 0)
	send_to_char("\n\r",ch);
    return;
}
 
/* RT Commands to replace news, motd, imotd, etc from ROM */
void do_news(CHAR_DATA *ch, char *argument)
{
    do_help(ch,"news");
}

void do_motd(CHAR_DATA *ch, char *argument)
{
    do_help(ch,"motd");
}

void do_imotd(CHAR_DATA *ch, char *argument)
{  
    do_help(ch,"imotd");
}

void do_rules(CHAR_DATA *ch, char *argument)
{
    do_help(ch,"rules");
}

void do_story(CHAR_DATA *ch, char *argument)
{
    do_help(ch,"story");
}

void do_changes(CHAR_DATA *ch, char *argument)
{
    do_help(ch,"changes");
}

void do_wizlist(CHAR_DATA *ch, char *argument)
{
    do_help(ch,"wizlist");
}

void do_autolist(CHAR_DATA *ch, char *argument)
{
    /* lists most player flags */
    if (IS_NPC(ch))
      return;

    send_to_char("   action     status\n\r",ch);
    send_to_char("---------------------\n\r",ch);
 
    send_to_char("autoassist     ",ch);
    if (IS_SET(ch->act,PLR_AUTOASSIST))
        send_to_char("`GON`w\n\r",ch);
    else
        send_to_char("`ROFF`w\n\r",ch); 

    send_to_char("autoexit       ",ch);
    if (IS_SET(ch->act,PLR_AUTOEXIT))
        send_to_char("`GON`w\n\r",ch);
    else
        send_to_char("`ROFF`w\n\r",ch);

    send_to_char("autogold       ",ch);
    if (IS_SET(ch->act,PLR_AUTOGOLD))
        send_to_char("`GON`w\n\r",ch);
    else
        send_to_char("`ROFF`w\n\r",ch);

    send_to_char("autoloot       ",ch);
    if (IS_SET(ch->act,PLR_AUTOLOOT))
        send_to_char("`GON`w\n\r",ch);
    else
        send_to_char("`ROFF`w\n\r",ch);

    send_to_char("autosac        ",ch);
    if (IS_SET(ch->act,PLR_AUTOSAC))
        send_to_char("`GON`w\n\r",ch);
    else
        send_to_char("`ROFF`w\n\r",ch);

    send_to_char("autosplit      ",ch);
    if (IS_SET(ch->act,PLR_AUTOSPLIT))
        send_to_char("`GON`w\n\r",ch);
    else
        send_to_char("`ROFF`w\n\r",ch);

    send_to_char("prompt         ",ch);
    if (IS_SET(ch->comm,COMM_PROMPT))
	send_to_char("`GON`w\n\r",ch);
    else
	send_to_char("`ROFF`w\n\r",ch);

    send_to_char("combine items  ",ch);
    if (IS_SET(ch->comm,COMM_COMBINE))
	send_to_char("`GON`w\n\r",ch);
    else
	send_to_char("`ROFF`w\n\r",ch);

    if (!IS_SET(ch->act,PLR_CANLOOT))
	send_to_char("Your corpse is safe from thieves.\n\r",ch);
    else 
        send_to_char("Your corpse may be looted.\n\r",ch);

    if (IS_SET(ch->act,PLR_NOSUMMON))
	send_to_char("You cannot be summoned.\n\r",ch);
    else
	send_to_char("You can be summoned.\n\r",ch);
   
    if (IS_SET(ch->act,PLR_NOFOLLOW))
	send_to_char("You do not welcome followers.\n\r",ch);
    else
	send_to_char("You accept followers.\n\r",ch);
	
    if (IS_SET(ch->act,PLR_COLOR))
    	send_to_char("You have ansi `Yc`Ro`Bl`Co`Gr`w turned `Yon`w.\n\r",ch);
    else
        send_to_char("You have ansi color turned off.\n\r",ch);
}

void do_autoassist(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
    
    if (IS_SET(ch->act,PLR_AUTOASSIST))
    {
      send_to_char("Autoassist removed.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_AUTOASSIST);
    }
    else
    {
      send_to_char("You will now assist when needed.\n\r",ch);
      SET_BIT(ch->act,PLR_AUTOASSIST);
    }
}

void do_autoexit(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
 
    if (IS_SET(ch->act,PLR_AUTOEXIT))
    {
      send_to_char("Exits will no longer be displayed.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_AUTOEXIT);
    }
    else
    {
      send_to_char("Exits will now be displayed.\n\r",ch);
      SET_BIT(ch->act,PLR_AUTOEXIT);
    }
}

void do_autogold(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
 
    if (IS_SET(ch->act,PLR_AUTOGOLD))
    {
      send_to_char("Autogold removed.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_AUTOGOLD);
    }
    else
    {
      send_to_char("Automatic gold looting set.\n\r",ch);
      SET_BIT(ch->act,PLR_AUTOGOLD);
    }
}

void do_autoloot(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
 
    if (IS_SET(ch->act,PLR_AUTOLOOT))
    {
      send_to_char("Autolooting removed.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_AUTOLOOT);
    }
    else
    {
      send_to_char("Automatic corpse looting set.\n\r",ch);
      SET_BIT(ch->act,PLR_AUTOLOOT);
    }
}

void do_autosac(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
 
    if (IS_SET(ch->act,PLR_AUTOSAC))
    {
      send_to_char("Autosacrificing removed.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_AUTOSAC);
    }
    else
    {
      send_to_char("Automatic corpse sacrificing set.\n\r",ch);
      SET_BIT(ch->act,PLR_AUTOSAC);
    }
}

void do_autosplit(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
 
    if (IS_SET(ch->act,PLR_AUTOSPLIT))
    {
      send_to_char("Autosplitting removed.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_AUTOSPLIT);
    }
    else
    {
      send_to_char("Automatic gold splitting set.\n\r",ch);
      SET_BIT(ch->act,PLR_AUTOSPLIT);
    }
}

void do_brief(CHAR_DATA *ch, char *argument)
{
    if (IS_SET(ch->comm,COMM_BRIEF))
    {
      send_to_char("Full descriptions activated.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_BRIEF);
    }
    else
    {
      send_to_char("Short descriptions activated.\n\r",ch);
      SET_BIT(ch->comm,COMM_BRIEF);
    }
}

void do_compact(CHAR_DATA *ch, char *argument)
{
    if (IS_SET(ch->comm,COMM_COMPACT))
    {
      send_to_char("Compact mode removed.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_COMPACT);
    }
    else
    {
      send_to_char("Compact mode set.\n\r",ch);
      SET_BIT(ch->comm,COMM_COMPACT);
    }
}

void do_prompt(CHAR_DATA *ch, char *argument)
{
   if (!IS_NPC(ch)) 
   {
        if( !strcmp( argument, "default" ) ) {
            free_string( ch->pcdata->prompt );
            ch->pcdata->prompt = str_dup( "%i`K/`W%H`w HP %n`K/`W%M`w MP %w`K/`W%V`w MV `K> ");
        }

   	else if( !strcmp( argument, "combat" ) ) {
   	     free_string( ch->pcdata->prompt );
	     ch->pcdata->prompt = str_dup( "`gTank: %l  `rEnemy: %e%r%i`K/`W%H `wHP %n`K/`W%M `wMP %w`K/`W%V `wMV `K>" );
	}

        else
   	 {
	    free_string( ch->pcdata->prompt );
	    smash_tilde(argument);
	    ch->pcdata->prompt = strdup( argument );
   	 }
     send_to_char("Prompt set.\n\r",ch);
     return;
   }
   else send_to_char("Mobiles may not change thier prompts.\n\r",ch);
}

void do_combine(CHAR_DATA *ch, char *argument)
{
    if (IS_SET(ch->comm,COMM_COMBINE))
    {
      send_to_char("Long inventory selected.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_COMBINE);
    }
    else
    {
      send_to_char("Combined inventory selected.\n\r",ch);
      SET_BIT(ch->comm,COMM_COMBINE);
    }
}

void do_noloot(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
 
    if (IS_SET(ch->act,PLR_CANLOOT))
    {
      send_to_char("Your corpse is now safe from thieves.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_CANLOOT);
    }
    else
    {
      send_to_char("Your corpse may now be looted.\n\r",ch);
      SET_BIT(ch->act,PLR_CANLOOT);
    }
}

void do_nofollow(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
 
    if (IS_SET(ch->act,PLR_NOFOLLOW))
    {
      send_to_char("You now accept followers.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_NOFOLLOW);
    }
    else
    {
      send_to_char("You no longer accept followers.\n\r",ch);
      SET_BIT(ch->act,PLR_NOFOLLOW);
	if( !IS_AFFECTED( ch, AFF_CHARM ) )
      die_follower( ch );
    }
}

void do_nosummon(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
    {
      if (IS_SET(ch->imm_flags,IMM_SUMMON))
      {
	send_to_char("You are no longer immune to summon.\n\r",ch);
	REMOVE_BIT(ch->imm_flags,IMM_SUMMON);
      }
      else
      {
	send_to_char("You are now immune to summoning.\n\r",ch);
	SET_BIT(ch->imm_flags,IMM_SUMMON);
      }
    }
    else
    {
      if (IS_SET(ch->act,PLR_NOSUMMON))
      {
        send_to_char("You are no longer immune to summon.\n\r",ch);
        REMOVE_BIT(ch->act,PLR_NOSUMMON);
      }
      else
      {
        send_to_char("You are now immune to summoning.\n\r",ch);
        SET_BIT(ch->act,PLR_NOSUMMON);
      }
    }
}

void do_nocolor(CHAR_DATA *ch, char *argument)
{
  if (IS_SET(ch->act,PLR_COLOR))
  {
    send_to_char("You no longer see in color.\n\r",ch);
    REMOVE_BIT(ch->act,PLR_COLOR);
  }
  else
  {
    send_to_char("You can see in `Yc`Ro`Bl`Co`Gr`w.\n\r",ch);
    SET_BIT(ch->act,PLR_COLOR);
  }
}

void do_afk(CHAR_DATA *ch, char *argument)
{
   char buf2[MAX_STRING_LENGTH];
   
   if (IS_NPC( ch )) return;
   
   if (IS_SET(ch->act,PLR_AFK))
     {
	send_to_char("You are no longer set AFK.\n\r",ch);
	REMOVE_BIT(ch->act,PLR_AFK);
	if (ch->pcdata->message != NULL) {
		sprintf(buf2, "You have `W%d message%s`w waiting, type `Wmessages`w to read them.\n\r",
				ch->pcdata->messages, (ch->pcdata->messages > 1) ? "s" : "" );
		send_to_char(buf2, ch);
	}
	act( "`R$n returns to $s keyboard.`w",ch,NULL,NULL,TO_ROOM);
	sprintf(buf2, "%s is no-longer AFK.", ch->name);
	if (!IS_SET(ch->act,PLR_WIZINVIS)) do_sendinfo(ch, buf2);
     }
   else
     {
	send_to_char("You are now set AFK.  Messages are being recorded.\n\r",ch);
	SET_BIT(ch->act,PLR_AFK);
	act( "`W$n is away from $s keyboard for awhile.`w",ch,NULL,NULL,TO_ROOM);
	sprintf(buf2, "%s has gone AFK.", ch->name);
	if (!IS_SET(ch->act,PLR_WIZINVIS)) do_sendinfo(ch, buf2);
     }
}

void do_pk(CHAR_DATA *ch, char *argument)
{
   char buf2[MAX_STRING_LENGTH];
   /***/
   CLAN_DATA *clan;
   
   if (IS_NPC(ch))
	return;
  
   if (ch->pcdata->confirm_pk)
   {
	if (argument[0] != '\0')
	{
	    send_to_char("PK readiness status removed.\n\r",ch);
	    ch->pcdata->confirm_pk = FALSE;
	    return;
	}
	else
	{
	    if (IS_SET(ch->act,PLR_KILLER)) return;
	    SET_BIT(ch->act,PLR_KILLER);
	    act("`R$n glows briefly with a red aura, you get the feeling you should keep your distance.`w",
	    ch,NULL,NULL,TO_ROOM);
	    send_to_char("`RYou are now a Player Killer, good luck, you'll need it.\n\r`w", ch);
	    sprintf(buf2, "%s has become a player killer!", ch->name);
	    do_sendinfo(ch, buf2);
	    return;
 	}
    }

    if (argument[0] != '\0')
    {
	send_to_char("Just type pk. No argument.\n\r",ch);
	return;
    }
    if( ch->pcdata->clan != 0 )
    {
	clan = get_clan( ch->pcdata->clan );
	if( IS_SET( clan->join_flags, CLAN_NO_PK ) )
	{
		send_to_char("Your clan does not allow PK!\n\r", ch);
		return;
	}
    }

    send_to_char("Type pk again to confirm this command.\n\r",ch);
    send_to_char("WARNING: this command is virtually irreversible.\n\r",ch);
    send_to_char("If you don't know what pk is for read help pk, DON'T type this command again.\n\r",ch);
    send_to_char("Typing pk with an argument will undo pk readiness status.\n\r",
	ch);
    ch->pcdata->confirm_pk = TRUE;
}

void eval_dir(char *dir, int mov_dir, int num,CHAR_DATA *ch, int *see, ROOM_INDEX_DATA *first_room)
{
      char buf[MAX_STRING_LENGTH]; 
  
   
   CHAR_DATA *mob_in_room;

        for ( mob_in_room=first_room->people ; mob_in_room != NULL ;mob_in_room=mob_in_room->next_in_room) {

	if (can_see(ch, mob_in_room)) {

                    if (*see == 0) {
                        sprintf(buf, "  `W%s `wfrom you, you see :\n\r",dir);
                        send_to_char(buf, ch);
                        *see += 1;
                    }
                    if(!IS_NPC(mob_in_room)) {
                        sprintf(buf, "     %s%s - %d %s\n\r",mob_in_room->name,
                                        mob_in_room->pcdata->title,num,dir);
                        send_to_char(buf, ch);
                    }
                     else {
                        sprintf(buf, "     %s - %d %s\n\r",
                           mob_in_room->short_descr,num,dir);
                        send_to_char( buf, ch);
                    }
            }
        }
}

void show_dir_mobs(char *dir,int move_dir,CHAR_DATA *ch,int depth) {
	ROOM_INDEX_DATA *cur_room=ch->in_room;
	EXIT_DATA *pexit;
	int see=0;
	int i;
	
for (i=1;( i<=depth && (pexit = cur_room->exit[move_dir])
             &&   pexit->u1.to_room
             &&   pexit->u1.to_room != cur_room )
             && !IS_SET(pexit->exit_info, EX_CLOSED);i++) {
             cur_room = pexit->u1.to_room;
             eval_dir(dir,move_dir,i,ch,&see,cur_room);
	}     
}

char *dir_text[]={"North","East","South","West","Up","Down"};

void do_scan(CHAR_DATA *ch, char *argument)
{
      char buf[MAX_STRING_LENGTH]; 
  
   int door;
        CHAR_DATA *mob_in_room;

        for ( mob_in_room=ch->in_room->people ; mob_in_room != NULL ;
         mob_in_room=mob_in_room->next_in_room) {
            if (can_see(ch, mob_in_room)) {

                    if(!IS_NPC(mob_in_room)) {
                        sprintf(buf, "     %s%s - right here.\n\r",
                        mob_in_room->name,mob_in_room->pcdata->title);
                        send_to_char(buf, ch);
                    }
                    else {
                        sprintf(buf, "     %s - right here.\n\r",
                           mob_in_room->short_descr);
                        send_to_char( buf, ch);
                    }

             }

        }
        for ( door = 0; door <= 5; door++ )
                show_dir_mobs(dir_text[door],door,ch,3);
         act( "$n scans $s surroundings.", ch, NULL, NULL, TO_ROOM );

}

void do_effects( CHAR_DATA *ch, char *argument)
{
      char buf[MAX_STRING_LENGTH]; 
 
   
   AFFECT_DATA *paf;
    
    switch ( ch->position )
    {
    case POS_DEAD:     
	send_to_char( "You are DEAD!!\n\r",		ch );
	break;
    case POS_MORTAL:
	send_to_char( "You are mortally wounded.\n\r",	ch );
	break;
    case POS_INCAP:
	send_to_char( "You are incapacitated.\n\r",	ch );
	break;
    case POS_STUNNED:
	send_to_char( "You are stunned.\n\r",		ch );
	break;
    case POS_SLEEPING:
	send_to_char( "You are sleeping.\n\r",		ch );
	break;
    case POS_RESTING:
	send_to_char( "You are resting.\n\r",		ch );
	break;
    case POS_STANDING:
	send_to_char( "You are standing.\n\r",		ch );
	break;
    case POS_FIGHTING:
	send_to_char( "You are fighting.\n\r",		ch );
	break;
    }
    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK]   > 10 )
	send_to_char( "You are drunk.\n\r",   ch );
    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_THIRST] ==  0 )
	send_to_char( "You are thirsty.\n\r", ch );
    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_FULL]   ==  0 )
	send_to_char( "You are hungry.\n\r",  ch );
    if ( ch->affected != NULL )
    {
	send_to_char( "You are affected by:\n\r", ch );
	for ( paf = ch->affected; paf != NULL; paf = paf->next )
	{
	    sprintf( buf, "Spell: '%s'", skill_table[paf->type].name );
	    send_to_char( buf, ch );

	    if ( ch->level >= 20 )
	    {
		sprintf( buf,
		    " modifies %s by %d for %d hours",
		    affect_loc_name( paf->location ),
		    paf->modifier,
		    paf->duration );
		send_to_char( buf, ch );
	    }

	    send_to_char( ".\n\r", ch );
	}
    }
}

void do_levels( CHAR_DATA *ch, char *argument)
{
      char buf[MAX_STRING_LENGTH]; 
 
   
   int x;
    int orig_level;

    if(!IS_NPC(ch)) {
    	orig_level=ch->level;
        if (ch->level < 4) switch(ch->level) {
            case 1:	break;
            case 2:
	    case 3:	ch->level=1;
	    		break;
        } else ch->level-=3;
        for (x = ch->level; x < orig_level+5 ; x++) {
            if (x == orig_level)
                sprintf(buf, "`BLevel %d: %ld <--- You're here.\n\r`w", ch->level, exp_per_level(ch, ch->pcdata->points));
            else
                sprintf(buf, "Level %d: %ld\n\r", ch->level, exp_per_level(ch, ch->pcdata->points));
            ch->level++;
            send_to_char(buf, ch);
        }
        ch->level=orig_level;
   } else printf ("Mobs don't level.\n\r");
} 

void do_look( CHAR_DATA *ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   
   char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    EXIT_DATA *pexit;
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    char *pdesc;
    int door;
    int number,count;

    if ( ch->desc == NULL )
	return;

    if ( ch->position < POS_SLEEPING )
    {
	send_to_char( "You can't see anything but stars!\n\r", ch );
	return;
    }

    if ( ch->position == POS_SLEEPING )
    {
	send_to_char( "You can't see anything, you're sleeping!\n\r", ch );
	return;
    }

    if ( !check_blind( ch ) )
	return;

    if ( !IS_NPC(ch)
    &&   !IS_SET(ch->act, PLR_HOLYLIGHT)
    &&   room_is_dark( ch->in_room ) )
    {
	send_to_char( "It is pitch black ... \n\r", ch );
	show_char_to_char( ch->in_room->people, ch );
	return;
    }

    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2 );
    number = number_argument(arg,arg3);
    count = 0;

    if ( arg[0] == '\0' || !str_cmp( arg, "auto" ) )
    {
	/* 'look' or 'look auto' */
	send_to_char( "`B", ch);
	send_to_char( ch->in_room->name, ch );
	send_to_char( "`w\n\r", ch );

	if ( arg[0] == '\0'
	|| ( !IS_NPC(ch) && !IS_SET(ch->comm, COMM_BRIEF) ) )
	{
	    send_to_char( "  ",ch);
	    send_to_char( ch->in_room->description, ch );
	}

        if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOEXIT) )
	{
	    send_to_char("\n\r`W",ch);
            do_exits( ch, "auto" );
            send_to_char("`w",ch);
	}
	if( ch->in_room->contents != NULL ) {
	send_to_char("Visible Objects:\n\r", ch);
	show_list_to_char( ch->in_room->contents, ch, FALSE, FALSE );
	}
	send_to_char("Visible Characters:\n\r", ch);
	show_char_to_char( ch->in_room->people,   ch );
	return;
    }

    if ( !str_cmp( arg, "i" ) || !str_cmp( arg, "in" ) )
    {
	/* 'look in' */
	if ( arg2[0] == '\0' )
	{
	    send_to_char( "Look in what?\n\r", ch );
	    return;
	}

	if ( ( obj = get_obj_here( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "You do not see that here.\n\r", ch );
	    return;
	}

	switch ( obj->item_type )
	{
	default:
	    send_to_char( "That is not a container.\n\r", ch );
	    break;

	case ITEM_DRINK_CON:
	    if ( obj->value[1] <= 0 )
	    {
		send_to_char( "It is empty.\n\r", ch );
		break;
	    }

	    sprintf( buf, "It's %s full of a %s liquid.\n\r",
		obj->value[1] <     obj->value[0] / 4
		    ? "less than" :
		obj->value[1] < 3 * obj->value[0] / 4
		    ? "about"     : "more than",
		liq_table[obj->value[2]].liq_color
		);

	    send_to_char( buf, ch );
	    break;

	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
	case ITEM_CORPSE_PC:
	    if ( IS_SET(obj->value[1], CONT_CLOSED) )
	    {
		send_to_char( "It is closed.\n\r", ch );
		break;
	    }

	    act( "$p contains:", ch, obj, NULL, TO_CHAR );
	    show_list_to_char( obj->contains, ch, TRUE, TRUE );
	    break;
	}
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) != NULL )
    {
	show_char_to_char_1( victim, ch );
	return;
    }

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( can_see_obj( ch, obj ) )
	{
	    pdesc = get_extra_descr( arg3, obj->extra_descr );
	    if ( pdesc != NULL )
	    	if (++count == number)
	    	{
		    send_to_char( pdesc, ch );
		    return;
	    	}
	    	else continue;

 	    pdesc = get_extra_descr( arg3, obj->pIndexData->extra_descr );
 	    if ( pdesc != NULL )
 	    	if (++count == number)
 	    	{	
		    send_to_char( pdesc, ch );
		    return;
	     	}
		else continue;

	    if ( is_name( arg3, obj->name ) )
	    	if (++count == number)
	    	{
	    	    send_to_char( obj->description, ch );
	    	    send_to_char( "\n\r",ch);
	    	   return;
	    	}
	}
    }

    for ( obj = ch->in_room->contents; obj != NULL; obj = obj->next_content )
    {
	if ( can_see_obj( ch, obj ) )
	{
	    pdesc = get_extra_descr( arg3, obj->extra_descr );
	    if ( pdesc != NULL )
	    	if (++count == number)
	    	{
		    send_to_char( pdesc, ch );
		    return;
	    	}

	    pdesc = get_extra_descr( arg3, obj->pIndexData->extra_descr );
	    if ( pdesc != NULL )
	    	if (++count == number)
	    	{
		    send_to_char( pdesc, ch );
		    return;
	    	}
	}

	if ( is_name( arg3, obj->name ) )
	    if (++count == number)
	    {
	    	send_to_char( obj->description, ch );
	    	send_to_char("\n\r",ch);
	    	return;
	    }
    }
    
    if (count > 0 && count != number)
    {
    	if (count == 1)
    	    sprintf(buf,"You only see one %s here.\n\r",arg3);
    	else
    	    sprintf(buf,"You only see %d %s's here.\n\r",count,arg3);
    	
    	send_to_char(buf,ch);
    	return;
    }

    pdesc = get_extra_descr( arg, ch->in_room->extra_descr );
    if ( pdesc != NULL )
    {
	send_to_char( pdesc, ch );
	return;
    }

         if ( !str_cmp( arg, "n" ) || !str_cmp( arg, "north" ) ) door = 0;
    else if ( !str_cmp( arg, "e" ) || !str_cmp( arg, "east"  ) ) door = 1;
    else if ( !str_cmp( arg, "s" ) || !str_cmp( arg, "south" ) ) door = 2;
    else if ( !str_cmp( arg, "w" ) || !str_cmp( arg, "west"  ) ) door = 3;
    else if ( !str_cmp( arg, "u" ) || !str_cmp( arg, "up"    ) ) door = 4;
    else if ( !str_cmp( arg, "d" ) || !str_cmp( arg, "down"  ) ) door = 5;
    else
    {
	send_to_char( "You do not see that here.\n\r", ch );
	return;
    }

    /* 'look direction' */
    if ( ( pexit = ch->in_room->exit[door] ) == NULL )
    {
	send_to_char( "Nothing special there.\n\r", ch );
	return;
    }

    if ( pexit->description != NULL && pexit->description[0] != '\0' )
	send_to_char( pexit->description, ch );
    else
	send_to_char( "Nothing special there.\n\r", ch );

    if ( pexit->keyword    != NULL
    &&   pexit->keyword[0] != '\0'
    &&   pexit->keyword[0] != ' ' )
    {
	if ( IS_SET(pexit->exit_info, EX_CLOSED) && !IS_SET(pexit->exit_info, EX_HIDDEN) )
	{
	    act( "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR );
	}
        else if ( ( IS_SET(pexit->exit_info, EX_ISDOOR)
                && !IS_SET(pexit->exit_info, EX_HIDDEN) )
                || ( IS_SET(pexit->exit_info, EX_ISDOOR)
                && IS_SET(pexit->exit_info, EX_HIDDEN)
                && !IS_SET(pexit->exit_info, EX_CLOSED) ) )
        {
            act( "The $d is open.",   ch, NULL, pexit->keyword, TO_CHAR );
        }
    }

    return;
}

/* RT added back for the hell of it */
void do_read (CHAR_DATA *ch, char *argument )
{
    do_look(ch,argument);
}

void do_examine( CHAR_DATA *ch, char *argument )
{
      char buf[MAX_STRING_LENGTH]; 
 
   
   char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument( argument, arg2 );

    if ( arg2[0] == '\0' )
    {
	send_to_char( "Examine what?\n\r", ch );
	return;
    }

    do_look( ch, arg2 );

    if ( ( obj = get_obj_here( ch, arg2 ) ) != NULL )
    {
	switch ( obj->item_type )
	{
	default:
	    break;

	case ITEM_DRINK_CON:
	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
	case ITEM_CORPSE_PC:
	    send_to_char( "When you look inside, you see:\n\r", ch );
	    sprintf( buf, "in %s", arg2 );
	    do_look( ch, buf );
	}
    }

    if( obj )
      oprog_examine_trigger( ch, obj );

    return;
}

/*
 * Thanks to Zrin for auto-exit part.
 */
void do_exits( CHAR_DATA *ch, char *argument )
{
      char buf[MAX_STRING_LENGTH]; 
 
   
   extern char * const dir_name[];
    EXIT_DATA *pexit;
    bool found;
    bool fAuto;
    int door;

    buf[0] = '\0';
    fAuto  = !str_cmp( argument, "auto" );

    if ( !check_blind( ch ) )
	return;

    strcpy( buf, fAuto ? "[Visible Exits:" : "Obvious exits:\n\r" );

    found = FALSE;
    for ( door = 0; door <= 5; door++ )
    {
	if ( ( pexit = ch->in_room->exit[door] ) != NULL
	&&   pexit->u1.to_room != NULL
	&&   can_see_room(ch,pexit->u1.to_room)
	&&   !IS_SET(pexit->exit_info, EX_CLOSED)
	&&   !IS_SET(pexit->exit_info, EX_HIDDEN) )
	{
	    found = TRUE;
	    if ( fAuto )
	    {
		strcat( buf, " " );
		strcat( buf, dir_name[door] );
	    }
	    else
	    {
		sprintf( buf + strlen(buf), "%-5s - %s\n\r",
		    capitalize( dir_name[door] ),
		    room_is_dark( pexit->u1.to_room )
			?  "Too dark to tell"
			: pexit->u1.to_room->name
		    );
	    }
	}
    }

    if ( !found )
	strcat( buf, fAuto ? " none" : "None.\n\r" );

    if ( fAuto )
	strcat( buf, "]\n\r" );

    send_to_char( buf, ch );
    return;
}

void do_worth( CHAR_DATA *ch, char *argument )
{
   char buf[MAX_STRING_LENGTH]; 

    if (IS_NPC(ch))
    {
	sprintf(buf,"You have %ld gold.\n\r",ch->gold);
	send_to_char(buf,ch);
	return;
    }

    sprintf(buf, "You have %ld gold, and %ld experience (%ld exp to level).\n\r",
	ch->gold, ch->exp, (exp_per_level(ch,ch->pcdata->points) - ch->exp));
    send_to_char(buf,ch);

    return;
}

char *statdiff(int normal, int modified)
{
    static char tempstr[10];
    
    strcpy(tempstr, "\0");
    if (normal < modified) sprintf(tempstr, "+%d", modified-normal);
    else if (normal > modified) sprintf(tempstr, "-%d", normal-modified);
    else if (normal == modified) sprintf(tempstr, "  ");
    return (tempstr);
}

void do_score( CHAR_DATA *ch, char *argument )
{
    
      char buf[MAX_STRING_LENGTH]; 

   char tempbuf[200];
    AFFECT_DATA *paf;
    int i,x;
    
    if (!IS_NPC(ch)) {
    sprintf( buf, "\n\r      `y/~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~/~~\\\n\r");
    send_to_char( buf, ch);
    sprintf( buf, "     |   `W%s%s", ch->name,ch->pcdata->title);
    send_to_char( buf, ch);
    strcpy(buf, "\0");
    for ( x = 0; x < 47-strlen(ch->name)-str_len(ch->pcdata->title) ; x++)
        strcat( buf, " ");
    send_to_char( buf, ch);
    sprintf( buf, "%3d years old  `y|____|\n\r", get_age(ch));
    send_to_char( buf, ch);
    sprintf( buf, "     |+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+|\n\r");
    send_to_char( buf, ch);
    sprintf( buf, "     | `YSTR:     `G%2d `W%s `y| `YRace: `G%s",ch->perm_stat[STAT_STR],
    		statdiff(ch->perm_stat[STAT_STR], get_curr_stat(ch,STAT_STR)),
    		race_table[ch->race].name);
    send_to_char( buf, ch); 
    strcpy(buf, "\0");
    for ( x = 0; x < 41-strlen(race_table[ch->race].name) ; x++)
        strcat( buf, " " );
    send_to_char(buf, ch);
    sprintf( buf, "`y|\n\r     | `YINT:     `G%2d `W%s `y| `YClass: `G%s", ch->perm_stat[STAT_INT],
    		statdiff(ch->perm_stat[STAT_INT], get_curr_stat(ch,STAT_INT)),
    		class_table[ch->class].name);
    send_to_char(buf, ch);
    strcpy(buf, "\0");
    for ( x = 0; x < 40-strlen(class_table[ch->class].name) ; x++)
    	strcat( buf, " " );
    send_to_char(buf, ch);
    sprintf( buf, "`y|\n\r     | `YWIS:     `G%2d `W%s `y| `YLevel: `G%3d                                     `y|\n\r",
    		ch->perm_stat[STAT_WIS], statdiff(ch->perm_stat[STAT_WIS],
    		get_curr_stat(ch,STAT_WIS)), ch->level);
    send_to_char(buf, ch);
    sprintf( buf, "     | `YDEX:     `G%2d `W%s `y| `YAlignment: `G%5d `W[",ch->perm_stat[STAT_DEX],
    		statdiff(ch->perm_stat[STAT_DEX], get_curr_stat(ch,STAT_DEX)),
    		ch->alignment);
    send_to_char(buf, ch);
    if ( ch->alignment >  900 )      sprintf( buf, "Angelic]                     `y|\n\r" );
    else if ( ch->alignment >  700 ) sprintf( buf, "Saintly]                     `y|\n\r" );
    else if ( ch->alignment >  350 ) sprintf( buf, "Good]                        `y|\n\r" );
    else if ( ch->alignment >  100 ) sprintf( buf, "Kind]                        `y|\n\r" );
    else if ( ch->alignment > -100 ) sprintf( buf, "Neutral]                     `y|\n\r" );
    else if ( ch->alignment > -350 ) sprintf( buf, "Mean]                        `y|\n\r" );
    else if ( ch->alignment > -700 ) sprintf( buf, "Evil]                        `y|\n\r" );
    else if ( ch->alignment > -900 ) sprintf( buf, "Demonic]                     `y|\n\r" );
    else                             sprintf( buf, "Satanic]                     `y|\n\r" );
    send_to_char(buf, ch);
    sprintf( buf, "     | `YCON:     `G%2d `W%s `y| `YSex: `G%s          `YCreation Points : `G%2d      `y|\n\r",
    		ch->perm_stat[STAT_CON], statdiff(ch->perm_stat[STAT_CON], get_curr_stat(ch,STAT_CON)),
    		ch->sex == 0 ? "Sexless" : ch->sex == 1 ? "Male  " : "female",
    		ch->pcdata->points);
    send_to_char(buf, ch);
    sprintf( buf, "     |+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+|\n\r");
    send_to_char(buf, ch);
    sprintf( buf, "     | `YItems Carried   ");
    send_to_char(buf, ch);
    sprintf( tempbuf, "`G%d`y/`G%d",ch->carry_number, can_carry_n(ch));
    send_to_char(tempbuf, ch);
    strcpy(buf, "\0");
    for ( x = 0; x < 19-strlen(tempbuf)+6 ; x++)
    	strcat(buf, " ");
    send_to_char(buf, ch);
    strcpy(buf, "\0");
    sprintf( buf, "`YArmor vs magic  : `G%4d       `y|\n\r", GET_AC(ch,AC_EXOTIC));
    send_to_char(buf, ch);
    sprintf( buf, "     | `YWeight Carried  ");
    send_to_char(buf, ch);
    sprintf( tempbuf, "`G%d`y/`G%d",ch->carry_weight, can_carry_w(ch));
    send_to_char(tempbuf, ch);
    strcpy(buf, "\0");
    for ( x = 0; x < 19-strlen(tempbuf)+6 ; x++)
    	strcat(buf, " ");
    send_to_char(buf, ch);
    strcpy(buf, "\0");
    sprintf( buf, "`YArmor vs bash   : `G%4d       `y|\n\r",GET_AC(ch,AC_BASH));
    send_to_char(buf, ch);
    sprintf( buf, "     | `YGold            ");
    send_to_char(buf, ch);
    sprintf( tempbuf, "`G%ld", ch->gold);
    send_to_char(tempbuf, ch);
    strcpy(buf, "\0");
    for ( x = 0; x < 19-strlen(tempbuf)+2 ; x++)
    	strcat(buf, " ");
    send_to_char(buf, ch);
    strcpy(buf, "\0");
    sprintf( buf,"`YArmor vs pierce : `G%4d       `y|\n\r",GET_AC(ch,AC_PIERCE));
    send_to_char(buf, ch);
    sprintf( buf, "     |                                    `YArmor vs slash  : `G%4d       `y|\n\r",
    		 GET_AC(ch,AC_SLASH));
    send_to_char(buf, ch);
    sprintf( buf, "     | `YCurrent XP       ");
    send_to_char(buf, ch);
    sprintf( tempbuf, "`G%ld", ch->exp);
    send_to_char(tempbuf, ch);
    strcpy(buf, "\0");
    for ( x = 0; x < 47-strlen(tempbuf)+2 ; x++)
    	strcat(buf, " ");
    send_to_char(buf, ch);
    strcpy(buf, "\0");
    sprintf( buf, "`y|\n\r     | `YXP to level      ");
    send_to_char(buf, ch);
    sprintf( tempbuf, "`G%ld (%d%% of normal for your level)", exp_per_level(ch,ch->pcdata->points)-ch->exp,
    		figure_difference(ch->pcdata->points));
    send_to_char(tempbuf, ch);
    strcpy(buf, "\0");
    for ( x = 0; x < 47-strlen(tempbuf)+2 ; x++)
    	strcat(buf, " ");
    send_to_char(buf, ch);
    strcpy(buf, "\0");
    sprintf( buf, "`y|\n\r     |                                     `YHitP: `G%5d `y/ `G%5d         `y|\n\r",
    	ch->hit, ch->max_hit);
    send_to_char(buf, ch);
    sprintf( buf, "     | `YBonus to Hit: `W+");
    send_to_char(buf, ch);
    sprintf( tempbuf, "%d",GET_HITROLL(ch));
    send_to_char(tempbuf, ch);
    strcpy(buf, "\0");
    for ( x = 0; x < 21-strlen(tempbuf) ; x++)
    	strcat(buf, " ");
    send_to_char(buf, ch);
    strcpy(buf, "\0");
    sprintf( buf, "`YMana: `G%5d `y/ `G%5d         `y|\n\r", ch->mana, ch->max_mana);
    send_to_char(buf, ch);
    sprintf( buf, "     | `YBonus to Dam: `W+");
    send_to_char(buf, ch);
    sprintf( tempbuf, "%d",GET_DAMROLL(ch));
    send_to_char(tempbuf, ch);
    strcpy(buf, "\0");
    for ( x = 0; x < 21-strlen(tempbuf) ; x++)
    	strcat(buf, " ");
    send_to_char(buf,ch);
    sprintf( buf, "`YMove: `G%5d `y/ `G%5d         `y|\n\r", ch->move, ch->max_move);
    send_to_char(buf, ch);
    sprintf( buf, "  /~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~/   |\n\r");
    send_to_char(buf, ch);
    sprintf( buf, "  \\________________________________________________________________\\__/`w\n\r");
    send_to_char(buf, ch);
/***/
    if( ch->pcdata->clan > 0 )
    {
    CLAN_DATA *clan;
    clan = get_clan( ch->pcdata->clan );
    if( clan != NULL )
    {
    sprintf( buf, "Your rank is %s in clan %s.\n\r", clan->rank[ch->pcdata->clan_rank-1], clan->name );
    send_to_char( buf, ch );
    }
    }

    }
    else {
    sprintf( buf,
	"You are %s%s, level %d, %d years old (%d hours).\n\r",
	ch->name,
	IS_NPC(ch) ? "" : ch->pcdata->title,
	ch->level, get_age(ch),
        ( ch->played + (int) (current_time - ch->logon) ) / 3600);
    send_to_char( buf, ch );

    if ( get_trust( ch ) != ch->level )
    {
	sprintf( buf, "You are trusted at level %d.\n\r",
	    get_trust( ch ) );
	send_to_char( buf, ch );
    }

    sprintf(buf, "Race: %s  Sex: %s  Class:  %s\n\r",
	race_table[ch->race].name,
	ch->sex == 0 ? "sexless" : ch->sex == 1 ? "male" : "female",
 	IS_NPC(ch) ? "mobile" : class_table[ch->class].name);
    send_to_char(buf,ch);
	

    sprintf( buf,
	"You have %d/%d hit, %d/%d mana, %d/%d movement.\n\r",
	ch->hit,  ch->max_hit,
	ch->mana, ch->max_mana,
	ch->move, ch->max_move);
    send_to_char( buf, ch );

    sprintf( buf,
	"You have %d practices and %d training sessions.\n\r",
	ch->practice, ch->train);
    send_to_char( buf, ch );

    sprintf( buf,
	"You are carrying %d/%d items with weight %d/%d pounds.\n\r",
	ch->carry_number, can_carry_n(ch),
	ch->carry_weight, can_carry_w(ch) );
    send_to_char( buf, ch );

    sprintf( buf,
	"Str: %d(%d)  Int: %d(%d)  Wis: %d(%d)  Dex: %d(%d)  Con: %d(%d)\n\r",
	ch->perm_stat[STAT_STR],
	get_curr_stat(ch,STAT_STR),
	ch->perm_stat[STAT_INT],
	get_curr_stat(ch,STAT_INT),
	ch->perm_stat[STAT_WIS],
	get_curr_stat(ch,STAT_WIS),
	ch->perm_stat[STAT_DEX],
	get_curr_stat(ch,STAT_DEX),
	ch->perm_stat[STAT_CON],
	get_curr_stat(ch,STAT_CON) );
    send_to_char( buf, ch );

    sprintf( buf,
	"You have scored %ld exp, and have %ld gold coins.\n\r",
	ch->exp,  ch->gold );
    send_to_char( buf, ch );

    /* RT shows exp to level */
    if (!IS_NPC(ch) && ch->level < LEVEL_HERO)
    {
      sprintf (buf, 
	"You need %ld exp to level.\n\r",
	((ch->level + 1) * exp_per_level(ch,ch->pcdata->points) - ch->exp));
      send_to_char( buf, ch );
     }

    sprintf( buf, "Wimpy set to %d hit points.\n\r", ch->wimpy );
    send_to_char( buf, ch );

    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK]   > 10 )
	send_to_char( "You are drunk.\n\r",   ch );
    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_THIRST] ==  0 )
	send_to_char( "You are thirsty.\n\r", ch );
    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_FULL]   ==  0 )
	send_to_char( "You are hungry.\n\r",  ch );

    switch ( ch->position )
    {
    case POS_DEAD:     
	send_to_char( "You are DEAD!!\n\r",		ch );
	break;
    case POS_MORTAL:
	send_to_char( "You are mortally wounded.\n\r",	ch );
	break;
    case POS_INCAP:
	send_to_char( "You are incapacitated.\n\r",	ch );
	break;
    case POS_STUNNED:
	send_to_char( "You are stunned.\n\r",		ch );
	break;
    case POS_SLEEPING:
	send_to_char( "You are sleeping.\n\r",		ch );
	break;
    case POS_RESTING:
	send_to_char( "You are resting.\n\r",		ch );
	break;
    case POS_STANDING:
	send_to_char( "You are standing.\n\r",		ch );
	break;
    case POS_FIGHTING:
	send_to_char( "You are fighting.\n\r",		ch );
	break;
    }


    /* print AC values */
    if (ch->level >= 25)
    {	
	sprintf( buf,"Armor: pierce: %d  bash: %d  slash: %d  magic: %d\n\r",
		 GET_AC(ch,AC_PIERCE),
		 GET_AC(ch,AC_BASH),
		 GET_AC(ch,AC_SLASH),
		 GET_AC(ch,AC_EXOTIC));
    	send_to_char(buf,ch);
    }

    for (i = 0; i < 4; i++)
    {
	char * temp;

	switch(i)
	{
	    case(AC_PIERCE):	temp = "piercing";	break;
	    case(AC_BASH):	temp = "bashing";	break;
	    case(AC_SLASH):	temp = "slashing";	break;
	    case(AC_EXOTIC):	temp = "magic";		break;
	    default:		temp = "error";		break;
	}
	
	send_to_char("You are ", ch);

	if      (GET_AC(ch,i) >=  101 ) 
	    sprintf(buf,"hopelessly vulnerable to %s.\n\r",temp);
	else if (GET_AC(ch,i) >= 80) 
	    sprintf(buf,"defenseless against %s.\n\r",temp);
	else if (GET_AC(ch,i) >= 60)
	    sprintf(buf,"barely protected from %s.\n\r",temp);
	else if (GET_AC(ch,i) >= 40)
	    sprintf(buf,"slighty armored against %s.\n\r",temp);
	else if (GET_AC(ch,i) >= 20)
	    sprintf(buf,"somewhat armored against %s.\n\r",temp);
	else if (GET_AC(ch,i) >= 0)
	    sprintf(buf,"armored against %s.\n\r",temp);
	else if (GET_AC(ch,i) >= -20)
	    sprintf(buf,"well-armored against %s.\n\r",temp);
	else if (GET_AC(ch,i) >= -40)
	    sprintf(buf,"very well-armored against %s.\n\r",temp);
	else if (GET_AC(ch,i) >= -60)
	    sprintf(buf,"heavily armored against %s.\n\r",temp);
	else if (GET_AC(ch,i) >= -80)
	    sprintf(buf,"superbly armored against %s.\n\r",temp);
	else if (GET_AC(ch,i) >= -100)
	    sprintf(buf,"almost invulnerable to %s.\n\r",temp);
	else
	    sprintf(buf,"divinely armored against %s.\n\r",temp);

    }


    /* RT wizinvis and holy light */
    if ( IS_IMMORTAL(ch))
    {
      send_to_char("Holy Light: ",ch);
      if (IS_SET(ch->act,PLR_HOLYLIGHT))
        send_to_char("on",ch);
      else
        send_to_char("off",ch);
 
      if (IS_SET(ch->act,PLR_WIZINVIS))
      {
        sprintf( buf, "  Invisible: level %d",ch->invis_level);
        send_to_char(buf,ch);
      }
      send_to_char("\n\r",ch);
    }

    if ( ch->level >= 15 )
    {
	sprintf( buf, "Hitroll: %d  Damroll: %d.\n\r",
	    GET_HITROLL(ch), GET_DAMROLL(ch) );
	send_to_char( buf, ch );
    }
    
    if ( ch->level >= 10 )
    {
	sprintf( buf, "Alignment: %d.  ", ch->alignment );
	send_to_char( buf, ch );
    }

    send_to_char( "You are ", ch );
         if ( ch->alignment >  900 ) send_to_char( "angelic.\n\r", ch );
    else if ( ch->alignment >  700 ) send_to_char( "saintly.\n\r", ch );
    else if ( ch->alignment >  350 ) send_to_char( "good.\n\r",    ch );
    else if ( ch->alignment >  100 ) send_to_char( "kind.\n\r",    ch );
    else if ( ch->alignment > -100 ) send_to_char( "neutral.\n\r", ch );
    else if ( ch->alignment > -350 ) send_to_char( "mean.\n\r",    ch );
    else if ( ch->alignment > -700 ) send_to_char( "evil.\n\r",    ch );
    else if ( ch->alignment > -900 ) send_to_char( "demonic.\n\r", ch );
    else                             send_to_char( "satanic.\n\r", ch );
    
    if ( ch->affected != NULL )
    {
	send_to_char( "You are affected by:\n\r", ch );
	for ( paf = ch->affected; paf != NULL; paf = paf->next )
	{
	    sprintf( buf, "Spell: '%s'", skill_table[paf->type].name );
	    send_to_char( buf, ch );

	    if ( ch->level >= 20 )
	    {
		sprintf( buf,
		    " modifies %s by %d for %d hours",
		    affect_loc_name( paf->location ),
		    paf->modifier,
		    paf->duration );
		send_to_char( buf, ch );
	    }

	    send_to_char( ".\n\r", ch );
	}
    }
    }
    return;
}



char *	const	day_name	[] =
{
    "the Moon", "the Bull", "Deception", "Thunder", "Freedom",
    "the Great Gods", "the Sun"
};

char *	const	month_name	[] =
{
    "Winter", "the Winter Wolf", "the Frost Giant", "the Old Forces",
    "the Grand Struggle", "the Spring", "Nature", "Futility", "the Dragon",
    "the Sun", "the Heat", "the Battle", "the Dark Shades", "the Shadows",
    "the Long Shadows", "the Ancient Darkness", "the Great Evil"
};

void do_time( CHAR_DATA *ch, char *argument )
{
    
         char buf[MAX_STRING_LENGTH]; 

   extern char str_boot_time[];
    char *suf;
    int day;

    day     = time_info.day + 1;

         if ( day > 4 && day <  20 ) suf = "th";
    else if ( day % 10 ==  1       ) suf = "st";
    else if ( day % 10 ==  2       ) suf = "nd";
    else if ( day % 10 ==  3       ) suf = "rd";
    else                             suf = "th";

    sprintf( buf, 
        "It is %d o'clock %s, Day of %s, %d%s the Month of %s.\n\rStarted up at %s\rThe system time is %s\r",
	(time_info.hour % 12 == 0) ? 12 : time_info.hour % 12,
	time_info.hour >= 12 ? "pm" : "am",
	day_name[day % 7],
	day, suf,
	month_name[time_info.month],
	str_boot_time,
	get_curtime()
	);

    send_to_char( buf, ch );
    return;
}

void do_weather( CHAR_DATA *ch, char *argument )
{
         char buf[MAX_STRING_LENGTH]; 
 
   static char * const sky_look[4] =
    {
	"cloudless",
	"cloudy",
	"rainy",
	"lit by flashes of lightning"
    };

    if ( !IS_OUTSIDE(ch) )
    {
	send_to_char( "You can't see the weather indoors.\n\r", ch );
	return;
    }

    sprintf( buf, "The sky is %s and %s.\n\r",
	sky_look[weather_info.sky],
	weather_info.change >= 0
	? "a warm southerly breeze blows"
	: "a cold northern gust blows"
	);
    send_to_char( buf, ch );
    return;
}

void do_help( CHAR_DATA *ch, char *argument )
{
    HELP_DATA *pHelp;
    char argall[MAX_INPUT_LENGTH],argone[MAX_INPUT_LENGTH];

    if ( argument[0] == '\0' )
	argument = "summary";

    /* this parts handles help a b so that it returns help 'a b' */
    argall[0] = '\0';
    while (argument[0] != '\0' )
    {
	argument = one_argument(argument,argone);
	if (argall[0] != '\0')
	    strcat(argall," ");
	strcat(argall,argone);
    }

    for ( pHelp = help_first; pHelp != NULL; pHelp = pHelp->next )
    {
	if ( pHelp->level > get_trust( ch ) )
	    continue;

	if ( is_name( argall, pHelp->keyword ) )
	{
	    if ( pHelp->level >= 0 && str_cmp( argall, "imotd" ) )
	    {
		send_to_char( pHelp->keyword, ch );
		send_to_char( "\n\r", ch );
	    }

	    /*
	     * Strip leading '.' to allow initial blanks.
	     */
	    if ( pHelp->text[0] == '.' )
		page_to_char( pHelp->text+1, ch );
	    else
		page_to_char( pHelp->text  , ch );
	    return;
	}
    }

    send_to_char( "No help on that word.\n\r", ch );
    return;
}

/* old whois command */
void do_whoname (CHAR_DATA *ch, char *argument)
{
      char arg[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
 
   
   char output[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    bool found = FALSE;

    one_argument(argument,arg);
  
    if (arg[0] == '\0')
    {
	send_to_char("You must provide a name.\n\r",ch);
	return;
    }

    output[0] = '\0';

    for (d = descriptor_list; d != NULL; d = d->next)
    {
	CHAR_DATA *wch;
	char const *class;

 	if (d->connected != CON_PLAYING || !can_see(ch,d->character))
	    continue;
	
	wch = ( d->original != NULL ) ? d->original : d->character;

 	if (!can_see(ch,wch))
	    continue;

	if (!str_prefix(arg,wch->name))
	{
	    found = TRUE;
	    
	    /* work out the printing */
	    class = class_table[wch->class].who_name;
	    switch(wch->level)
	    {
		case MAX_LEVEL + 3 : class = "IMP";	break;
                case MAX_LEVEL + 2 : class = "IMP";     break;
                case MAX_LEVEL + 1 : class = "IMP";     break;
		case MAX_LEVEL - 0 : class = "IMP"; 	break;
		case MAX_LEVEL - 1 : class = "CRE";	break;
		case MAX_LEVEL - 2 : class = "SUP";	break;
		case MAX_LEVEL - 3 : class = "DEI";	break;
		case MAX_LEVEL - 4 : class = "GOD";	break;
		case MAX_LEVEL - 5 : class = "IMM";	break;
		case MAX_LEVEL - 6 : class = "DEM";	break;
		case MAX_LEVEL - 7 : class = "ANG";	break;
		case MAX_LEVEL - 8 : class = "AVA";	break;
		case MAX_LEVEL - 9 : class = "HER";	break;
	    }
    
	    /* a little formatting */
	sprintf( buf, "`K[`W%3d `Y%s `G%s`K] %s%s%s`w%s%s %s%s%s\n\r",
	    (wch->level> MAX_LEVEL ? 100 : wch->level),
	    wch->race < MAX_PC_RACE ? pc_race_table[wch->race].who_name 
				    : "     ",
	    class,
	    IS_SET(wch->act, PLR_AFK) ? "`W(AFK) " : "",
	    IS_SET(wch->act, PLR_KILLER) ? "`R(PK) " : "",
	    IS_SET(wch->act, PLR_THIEF)  ? "`K(THIEF) "  : "",
	    wch->name,
	    IS_NPC(wch) ? "" : wch->pcdata->title,
/*	    IS_NPC(wch) ? 
 *		(wch->pIndexData->clan == 0) ? "" : "`W[`w" :
 *              (wch->pcdata->clan ==0) ? "" : "`W[`w", 
 *	    IS_NPC(wch) ? vis_clan(wch->pIndexData->clan)
 *	        : vis_clan(wch->pcdata->clan),
 *	    IS_NPC(wch) ? 
 *		(wch->pIndexData->clan == 0) ? "" : "`W]`w" :
 *              (wch->pcdata->clan ==0) ? "" : "`W]`w");
 */
		"","","");
	    strcat(output,buf);
	}
    }

    if (!found)
    {
	send_to_char("No one of that name is playing.\n\r",ch);
	return;
    }

    page_to_char(output,ch);
}

void insert_sort(CHAR_DATA *who_list[300], CHAR_DATA *ch, int length)
{
	while ( ( length ) && who_list[length-1]->level < ch->level) {
		who_list[length]=who_list[length-1];
		length--;
	}
	who_list[length]=ch;
}

void chaos_sort(CHAR_DATA *who_list[300], CHAR_DATA *ch, int length)
{
	while ( ( length ) && who_list[length-1]->pcdata->chaos_score < ch->pcdata->chaos_score) {
		who_list[length]=who_list[length-1];
		length--;
	}
	who_list[length]=ch;
}


char * who_clan( CHAR_DATA *ch, CHAR_DATA *looker, char *empty )
{
	CLAN_DATA *clan;

	clan = get_clan( ch->pcdata->clan );
	if( clan == NULL)
	{
		bug("NULL nonzero clan %d in who_clan.", ch->pcdata->clan);
		return empty;
	}

	if( IS_NPC( looker) )
	{
		if( !IS_SET( clan->clan_flags, CLAN_PRIVATE) && !IS_SET(clan->clan_flags, CLAN_SECRET) )
		{
			return clan->whoname;
		}
		return empty;
	}

	if( !IS_SET( clan->clan_flags, CLAN_PRIVATE) && !IS_SET(clan->clan_flags, CLAN_SECRET) )
	{
		return clan->whoname;
	}

	if( IS_SET( clan->clan_flags, CLAN_PRIVATE) && ch->pcdata->clan == looker->pcdata->clan )
	{
		return clan->whoname;
	}

	if( IS_SET(clan->clan_flags, CLAN_SECRET) && 
	(ch->pcdata->clan_rank == 1 || ch->pcdata->clan_rank == 2 
	|| !str_cmp(ch->name, clan->god) ) && ch->pcdata->clan == looker->pcdata->clan )
	{
		return clan->whoname;
	}

	if( looker->level >= MAX_LEVEL )
	{
		return clan->whoname;
	}

	return empty;
}

char * pre_clan( CHAR_DATA *ch, CHAR_DATA *looker, char *empty, char *private, char *secret )
{
	CLAN_DATA *clan;

	clan = get_clan( ch->pcdata->clan );
	if( clan == NULL)
	{
		bug("NULL nonzero clan %d in who_clan.", ch->pcdata->clan);
		return empty;
	}

	if( looker->level < MAX_LEVEL )
		return empty;

	if( IS_SET( clan->clan_flags, CLAN_PRIVATE ) )
		return private;

	if( IS_SET( clan->clan_flags, CLAN_SECRET ) )
		return secret;

	return empty;
}
		

/*
 * New 'who' command originally by Alander of Rivers of Mud.
 */
/* taken out in favor of a who command for EmberMUD by Rindar (Ron Cole) */
/* new function here: */
void do_who( CHAR_DATA *ch, char *argument )
{
      char arg[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   
   char buf2[MAX_STRING_LENGTH];
    char output[4 * MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    CHAR_DATA *who_list[300];
    int iClass;
    int iRace;
    int iLevelLower;
    int iLevelUpper;
    int nNumber;
    int nMatch;
    int length;
    int maxlength;
    int count;
    bool rgfClass[MAX_CLASS];
    bool rgfRace[MAX_PC_RACE];
    bool fClassRestrict;
    bool fRaceRestrict;
    bool fImmortalOnly;
    bool doneimmort=FALSE;
    bool donemort=FALSE;
/***/
    char empty[] = "";
    char private[] = "`m(P)`w";
    char secret[] = "`r(S)`w";

    /*
     * Set default arguments.
     */
    iLevelLower    = 0;
    iLevelUpper    = MAX_LEVEL+2;
    fClassRestrict = FALSE;
    fRaceRestrict = FALSE;
    fImmortalOnly  = FALSE;
    for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
        rgfClass[iClass] = FALSE;
    for ( iRace = 0; iRace < MAX_PC_RACE; iRace++ )
        rgfRace[iRace] = FALSE;

    /*
     * Parse arguments.
     */
    nNumber = 0;
    for ( ;; )
    {
        argument = one_argument( argument, arg );
        if ( arg[0] == '\0' )
            break;

        if ( is_number( arg ) )
        {
            switch ( ++nNumber )
            {
            case 1: iLevelLower = atoi( arg ); break;
            case 2: iLevelUpper = atoi( arg ); break;
            default:
                send_to_char( "Only two level numbers allowed.\n\r", ch );
                return;
            }
        }
        else
        {

            /*
             * Look for classes to turn on.
             */
            if ( argument == "imm" )
            {
                fImmortalOnly = TRUE;
            }
            else
            {
                iClass = class_lookup(arg);
                if (iClass == -1)
                {
                    iRace = race_lookup(arg);

                    if (iRace == 0 || iRace >= MAX_PC_RACE)
                    {
                        do_whoname(ch, arg);
                        return;
                    }
                    else
                    {
                        fRaceRestrict = TRUE;
                        rgfRace[iRace] = TRUE;
                    }
                }
                else
                {
                    fClassRestrict = TRUE;
                    rgfClass[iClass] = TRUE;
                }
            }
        }
    }
    
    length=0;
    for ( d = descriptor_list ; d ; d = d->next ) {
        if ( (d->connected == CON_PLAYING) || (d->connected == CON_NOTE_TO) ||
        (d->connected == CON_NOTE_SUBJECT) || (d->connected == CON_NOTE_EXPIRE)
        || (d->connected == CON_NOTE_TEXT) || (d->connected == CON_NOTE_FINISH)) {
            insert_sort(who_list, d->character, length);
            length++;
        }
    }

    maxlength=length;

    /*
     * Now show matching chars.
     */
    nMatch = 0;
    buf[0] = '\0';
    output[0] = '\0';
    for ( length=0 ; length < maxlength ; length++ )
    {
        char const *class;

        /*
         * Check for match against restrictions.
         * Don't use trust as that exposes trusted mortals.
         */
        if ( who_list[length]->level > MAX_LEVEL-10
        &&   can_see( ch, who_list[length] )    
        &&   doneimmort==FALSE )
        { 
                sprintf( buf, "`K[`RVisible Immortals`K]\n\r\n\r");
                doneimmort=TRUE;
                strcat( output, buf );
        } 
        else if ( ( who_list[length]->level <= MAX_LEVEL-10 ) && donemort == FALSE) 
        {
           if ( doneimmort == TRUE ) 
           {
              sprintf( buf, "\n\r");
              strcat( output, buf);
           }
           sprintf( buf, "`K[`RVisible Mortals`K]\n\r\n\r");
           donemort = TRUE;
           strcat( output, buf );
        }
        if (/*who_list[length]->desc->connected != CON_PLAYING ||*/ !can_see( ch, who_list[length] ) )
            continue;

        if ( who_list[length]->level < iLevelLower
        ||   who_list[length]->level > iLevelUpper
        || ( fImmortalOnly  && who_list[length]->level < LEVEL_HERO )
        || ( fClassRestrict && !rgfClass[who_list[length]->class] ) 
        || ( fRaceRestrict && !rgfRace[who_list[length]->race]))
            continue;

        nMatch++;

        /*
         * Figure out what to print for class.
         */

        class = class_table[who_list[length]->class].who_name;
        switch ( who_list[length]->level )
        {
        default: break;
            {
		case MAX_LEVEL + 3 : class = "IMP";	break;
                case MAX_LEVEL + 2 : class = "IMP";     break;
                case MAX_LEVEL + 1 : class = "IMP";     break;
                case MAX_LEVEL - 0 : class = "IMP";     break;
                case MAX_LEVEL - 1 : class = "CRE";     break;
                case MAX_LEVEL - 2 : class = "SUP";     break;
                case MAX_LEVEL - 3 : class = "DEI";     break;
                case MAX_LEVEL - 4 : class = "GOD";     break;
                case MAX_LEVEL - 5 : class = "IMM";     break;
                case MAX_LEVEL - 6 : class = "DEM";     break;
                case MAX_LEVEL - 7 : class = "ANG";     break;
                case MAX_LEVEL - 8 : class = "AVA";     break;
                case MAX_LEVEL - 9 : class = "HER";     break;
            }
        }
       
        /*
         * Format it up.
         */
       sprintf( buf, "`K[`W%3d `Y%s `G%s`K] %s%s%s%s%s%s%s`w%s%s%s\n\r",
               (who_list[length]->level> MAX_LEVEL ? 100 : who_list[length]->level),
               who_list[length]->pcdata != NULL &&
               who_list[length]->pcdata->who_race[0] != '\0'
               ? who_list[length]->pcdata->who_race
               : who_list[length]->race < MAX_PC_RACE ? 
               pc_race_table[who_list[length]->race].who_name 
               : "          ",
               class,
               IS_NPC(who_list[length]) ? "" : (who_list[length]->pcdata->clan == 0) ? "" : 
               pre_clan( who_list[length], ch, empty, private, secret ), 
               IS_NPC(who_list[length]) ? "" : (who_list[length]->pcdata->clan == 0) ? "" : 
               who_clan(who_list[length], ch, empty),
               "", /* <---- if you need to add something, remove this */
               IS_SET(who_list[length]->act, PLR_WIZINVIS) ? "`B(Wizi)`w " : "",
               IS_SET(who_list[length]->act, PLR_AFK) ? "`W(AFK) " : "",
               IS_SET(who_list[length]->act, PLR_KILLER) ? "`R(PK) " : "",
               IS_SET(who_list[length]->act, PLR_THIEF)  ? "`K(THIEF) "  : "",
		who_list[length]->pcdata != NULL &&
		who_list[length]->pcdata->who_prefix[0] !='\0' ?
		who_list[length]->pcdata->who_prefix : "",
		IS_NPC( who_list[length] ) ?
		who_list[length]->short_descr :
               who_list[length]->name,
               IS_NPC(who_list[length]) ? "" : who_list[length]->pcdata->title);
       
       strcat(output, buf);
    }
   
   sprintf( buf2, "\n\r`wVisible Players Shown: `W%d\n\r`w", nMatch );
   strcat(output,buf2);
   count=0;
   for ( d = descriptor_list ; d ; d = d->next ) {
       if ( d->connected == CON_PLAYING 
           && !(IS_SET(d->character->act, PLR_WIZINVIS) ) ) 
         {
            count++;
         }
       else if ( ((d->connected == CON_PLAYING) || (d->connected == CON_NOTE_TO) ||
        (d->connected == CON_NOTE_SUBJECT) || (d->connected == CON_NOTE_EXPIRE)
        || (d->connected == CON_NOTE_TEXT) || (d->connected == CON_NOTE_FINISH))
                && !(d->character->invis_level > ch->level) )
         
         { count++; }
       
    }
   
   sprintf( buf2, "`wTotal Players Online: `W%d\n\r`w", count );
   strcat(output,buf2);
   page_to_char( output, ch );
    return;
}


void do_inventory( CHAR_DATA *ch, char *argument )
{
    send_to_char( "You are carrying:\n\r", ch );
    show_list_to_char( ch->carrying, ch, TRUE, TRUE );
    return;
}

void do_equipment( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    int iWear;
    bool found;

    send_to_char( "You are using:\n\r", ch );
    found = FALSE;
    for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
    {
	if ( ( obj = get_eq_char( ch, iWear ) ) == NULL )
	    continue;

	send_to_char( where_name[iWear], ch );
	if ( can_see_obj( ch, obj ) )
	{
	    send_to_char( format_obj_to_char( obj, ch, TRUE ), ch );
	    send_to_char( "\n\r`w", ch );
	}
	else
	{
	    send_to_char( "Something.\n\r", ch );
	}
	found = TRUE;
    }

    if ( !found )
	send_to_char( "Nothing.\n\r", ch );

    return;
}

void do_compare( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj1;
    OBJ_DATA *obj2;
    int value1;
    int value2;
    char *msg;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    if ( arg1[0] == '\0' )
    {
	send_to_char( "Compare what to what?\n\r", ch );
	return;
    }

    if ( ( obj1 = get_obj_carry( ch, arg1 ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    if (arg2[0] == '\0')
    {
	for (obj2 = ch->carrying; obj2 != NULL; obj2 = obj2->next_content)
	{
	    if (obj2->wear_loc != WEAR_NONE
	    &&  can_see_obj(ch,obj2)
	    &&  obj1->item_type == obj2->item_type
	    &&  (obj1->wear_flags & obj2->wear_flags & ~ITEM_TAKE) != 0 )
		break;
	}

	if (obj2 == NULL)
	{
	    send_to_char("You aren't wearing anything comparable.\n\r",ch);
	    return;
	}
    } 

    else if ( (obj2 = get_obj_carry(ch,arg2) ) == NULL )
    {
	send_to_char("You do not have that item.\n\r",ch);
	return;
    }

    msg		= NULL;
    value1	= 0;
    value2	= 0;

    if ( obj1 == obj2 )
    {
	msg = "You compare $p to itself.  It looks about the same.";
    }
    else if ( obj1->item_type != obj2->item_type )
    {
	msg = "You can't compare $p and $P.";
    }
    else
    {
	switch ( obj1->item_type )
	{
	default:
	    msg = "You can't compare $p and $P.";
	    break;

	case ITEM_ARMOR:
	    value1 = obj1->value[0] + obj1->value[1] + obj1->value[2];
	    value2 = obj2->value[0] + obj2->value[1] + obj2->value[2];
	    break;

	case ITEM_WEAPON:
	    value1 = (1 + obj1->value[2]) * obj1->value[1];
	    value2 = (1 + obj2->value[2]) * obj2->value[1];
	    break;
	}
    }

    if ( msg == NULL )
    {
	     if ( value1 == value2 ) msg = "$p and $P look about the same.";
	else if ( value1  > value2 ) msg = "$p looks better than $P.";
	else                         msg = "$p looks worse than $P.";
    }

    act( msg, ch, obj1, obj2, TO_CHAR );
    return;
}

void do_credits( CHAR_DATA *ch, char *argument )
{
    do_help( ch, "diku" );
    return;
}

void do_where( CHAR_DATA *ch, char *argument )
{
      char arg[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
 
   CHAR_DATA *victim;
    DESCRIPTOR_DATA *d;
    bool found;
    extern bool chaos;

    one_argument( argument, arg );

	if ((chaos) && (ch->level< HERO))
  	{
   	 send_to_char( "Where? Your killer is right behind you!\n\r", ch);
	 return;
	}

    if ( arg[0] == '\0' && (ch->level > HERO) )
    {
	send_to_char( "Current players:\n\r", ch );
	found = FALSE;
	for ( d = descriptor_list; d; d = d->next )
	{
	    if ( d->connected == CON_PLAYING
	    && ( victim = d->character ) != NULL
	    &&   !IS_NPC(victim)
	    &&   victim->in_room != NULL
	    &&   can_see( ch, victim ) )
	    {
		found = TRUE;
		sprintf( buf, "%-28s [%5d] %s`w\n\r",
		    victim->name, victim->in_room->vnum, victim->in_room->name);
		send_to_char( buf, ch );
	    }
	}
	if ( !found )
	    send_to_char( "None\n\r", ch );
    }
    else if (ch->level > HERO)
    {
	found = FALSE;
	for ( victim = char_list; victim != NULL; victim = victim->next )
	{
	    if ( victim->in_room != NULL
	    &&   can_see( ch, victim )
	    &&   is_name( arg, victim->name ) )
	    {
		found = TRUE;
		sprintf( buf, "%-28s %s`w\n\r",
		    PERS(victim, ch), victim->in_room->name );
		send_to_char( buf, ch );
		break;
	    }
	}
	if ( !found )
	    act( "You didn't find any $T.", ch, NULL, arg, TO_CHAR );
    }
    else if ( arg[0] == '\0' && (ch->level<=HERO) )
    {
	found=FALSE;
	send_to_char("Players near you:\n\r",ch);
	for ( d = descriptor_list; d; d = d->next )

	{
	    if ( d->connected == CON_PLAYING
	    && ( victim = d->character ) != NULL
	    &&   !IS_NPC(victim)
	    &&   victim->in_room != NULL
	    &&   victim->in_room->area == ch->in_room->area
	    &&   can_see( ch, victim ) )
	    {
		found = TRUE;
		sprintf( buf, "%-28s %s`w\n\r",
		    victim->name, victim->in_room->name );
		send_to_char( buf, ch );
	    }
	}
	if ( !found )
	    send_to_char( "None\n\r", ch );
    }
    else
    {
	found = FALSE;
	for ( victim = char_list; victim != NULL; victim = victim->next )
	{
	    if ( victim->in_room != NULL
	    &&   victim->in_room->area == ch->in_room->area
	    &&   !IS_AFFECTED(victim, AFF_HIDE)
	    &&   !IS_AFFECTED(victim, AFF_SNEAK)
	    &&   can_see( ch, victim )
	    &&   is_name( arg, victim->name ) )
	    {
		found = TRUE;
		sprintf( buf, "%-28s %s`w\n\r",
		    PERS(victim, ch), victim->in_room->name );
		send_to_char( buf, ch );
		break;
	    }
	}
	if ( !found )
	    act( "You didn't find any $T.", ch, NULL, arg, TO_CHAR );
    }

    return;
}

void do_consider( CHAR_DATA *ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   
   CHAR_DATA *victim;
    char *msg;
    int diff;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Consider killing whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They're not here.\n\r", ch );
	return;
    }

    if (is_safe(ch,victim))
    {
	send_to_char("Don't even think about it.\n\r",ch);
	return;
    }

    diff = (victim->max_hit/ch->max_hit)*100 ;
    	 
    	 if ( diff <= 50 )  msg = "Comparing HP: $N isn't even in the ballpark.";
    else if ( diff <= 65 )  msg = "Comparing HP: $N almost makes you want to laugh.";
    else if ( diff <= 85 )  msg = "Comparing HP: $N Isn't quite up to your level.";
    else if ( diff <= 115 ) msg = "Comparing HP: You're about equal.";
    else if ( diff <= 125 ) msg = "Comparing HP: $N's just a bit tougher than you.";
    else if ( diff <= 140 ) msg = "Comparing HP: Maybe you should consider attacking something else.";
    else 		    msg = "Comparing HP: $N puts you to shame.";
    act( msg, ch, NULL, victim, TO_CHAR );

    diff = victim->level - ch->level;

         if ( diff <= -10 ) msg = "You can kill $N naked and weaponless.";
    else if ( diff <=  -5 ) msg = "$N is no match for you.";
    else if ( diff <=  -2 ) msg = "$N looks like an easy kill.";
    else if ( diff <=   1 ) msg = "The perfect match!";
    else if ( diff <=   4 ) msg = "$N says 'Do you feel lucky, punk?'.";
    else if ( diff <=   9 ) msg = "$N laughs at you mercilessly.";
    else                    msg = "Death will thank you for your gift.";
    act( msg, ch, NULL, victim, TO_CHAR );
    
    	
    return;
}

void set_title( CHAR_DATA *ch, char *title )
{
   char buf[MAX_STRING_LENGTH];
   
   if ( IS_NPC(ch) )
    {
	bug( "Set_title: NPC.", 0 );
	return;
    }

    if ( title[0] != '.' && title[0] != ',' && title[0] != '!' && title[0] != '?' )
    {
	buf[0] = ' ';
	strcpy( buf+1, title );
    }
    else
    {
	strcpy( buf, title );
    }
    
    strcat( buf, "`w");

    free_string( ch->pcdata->title );
    ch->pcdata->title = str_dup( buf );
    return;
}

void do_title( CHAR_DATA *ch, char *argument )
{
   char buf[2*MAX_STRING_LENGTH];
   
   if ( IS_NPC(ch) )
     return;
   
   if ( argument[0] == '\0' )
     {
	send_to_char( "Change your title to what?\n\r", ch );
	return;
     }
   
   if ( str_len(argument) > 45 - strlen(ch->name) )
   {
	send_to_char("Title too long, redo.\n\r", ch);
	return;
   }
   
   smash_tilde( argument );
   set_title( ch, argument );
   sprintf(buf,"Ok, you are now %s%s\n\r",ch->name,ch->pcdata->title);
   send_to_char( buf, ch );
}

void do_comment( CHAR_DATA *ch, char *argument )
{
   char buf[2*MAX_STRING_LENGTH];
   
   if ( IS_NPC(ch) )
     return;
   
   if ( argument[0] == '\0' )
     {
	send_to_char( "Whois comment deleted.\n\r", ch );
	free_string(ch->pcdata->comment);
	ch->pcdata->comment = str_dup( "(none)" );
	return;
     }
   
   if ( strlen(argument) > 60 )
     argument[60] = '\0';
   
   smash_tilde( argument );
   free_string(ch->pcdata->comment);
   ch->pcdata->comment = str_dup( argument );
   sprintf(buf,"Ok, your comment is  now -\n\r\n\r     %s\n\r",ch->pcdata->comment);
   send_to_char( buf, ch );
}

void do_email( CHAR_DATA *ch, char *argument )
{
   char last_char;
   int i, length, dots;
   bool at;
   char buf[2*MAX_STRING_LENGTH];
   
   if ( IS_NPC(ch) )
     return;
   
   if ( argument[0] == '\0' )
     {
	send_to_char( "Email address deleted.\n\r", ch );
	free_string(ch->pcdata->email);
	ch->pcdata->email = str_dup ( "(none)" );
	return;
     }
   
   if ( strlen(argument) > 45 )
     argument[45] = '\0';
   
   smash_tilde( argument );
   
   length = strlen( argument ) - 1;
   
   if (argument[0] == '@' || argument[0] == '.' || argument[length] == '@' ||
   	argument[length] == '.') {
   	send_to_char( "Invalid email address, your e-mail address must be in the\n\r"
   		      "name@host.domain format.\n\r",ch);
        return;
   }
   
   at = FALSE;
   dots = 0;
   last_char = '\0';
   
   for (i = 1; i < length ; i++) {
   	switch (argument[i]) {
   		case '@':
   			if (!at) at = TRUE;
   			else {
 			  	send_to_char( "Invalid email address, your e-mail address must be in the\n\r"
			   		      "name@host.domain format.\n\r",ch);
			        return;
			}
			break;   				
   		case '.':
   			if (last_char == '@' || last_char == '.' || !at) {
 			  	send_to_char( "Invalid email address, your e-mail address must be in the\n\r"
			   		      "name@host.domain format.\n\r",ch);
			        return;
			}
   			dots++;
			break;
		case ' ':
		  	send_to_char( "Invalid email address, your e-mail address must be in the\n\r"
		   		      "name@host.domain format.\n\r",ch);
		        return;
		        break;
   	}
   	last_char = argument[i];
   }
   
   if ( dots < 1 ) {
  	send_to_char( "Invalid email address, your e-mail address must be in the\n\r"
  		      "name@host.domain format.\n\r",ch);
        return;
   }
   
   free_string(ch->pcdata->email);
   ch->pcdata->email = str_dup( argument );
   sprintf(buf,"Ok, your email address is now: %s\n\r",ch->pcdata->email);
   send_to_char( buf, ch );
}

void do_description( CHAR_DATA *ch, char *argument )
{
   if (ch->desc != NULL)  /* only if ch has a descriptor 'cause string_append will barf */
     {
	string_append( ch, &ch->description );
	return;
     }
   return;
}

void do_report( CHAR_DATA *ch, char *argument )
{
      char buf[MAX_STRING_LENGTH];
 
   
   sprintf( buf,
	"`BYou say 'I have %d/%d hp %d/%d mana %d/%d mv %ld xp.'\n\r`w",
	ch->hit,  ch->max_hit,
	ch->mana, ch->max_mana,
	ch->move, ch->max_move,
	ch->exp   );

    send_to_char( buf, ch );

    sprintf( buf, "`B$n says 'I have %d/%d hp %d/%d mana %d/%d mv %ld xp.'`w",
	ch->hit,  ch->max_hit,
	ch->mana, ch->max_mana,
	ch->move, ch->max_move,
	ch->exp   );

    act( buf, ch, NULL, NULL, TO_ROOM );

    return;
}

void do_practice( CHAR_DATA *ch, char *argument )
{
      char buf[MAX_STRING_LENGTH];
 
   
   int sn;

    if ( IS_NPC(ch) )
	return;

    if ( argument[0] == '\0' )
    {
	int col;

	col    = 0;
	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    if ( skill_table[sn].name == NULL )
		break;
	    if ( ch->level < skill_table[sn].skill_level[ch->class] 
	      || ch->pcdata->learned[sn] < 1 /* skill is not known */)
		continue;

	    sprintf( buf, "%-18s %3d%%  ",
		skill_table[sn].name, ch->pcdata->learned[sn] );
	    send_to_char( buf, ch );
	    if ( ++col % 3 == 0 )
		send_to_char( "\n\r", ch );
	}

	if ( col % 3 != 0 )
	    send_to_char( "\n\r", ch );

	sprintf( buf, "You have %d practice sessions left.\n\r",
	    ch->practice );
	send_to_char( buf, ch );
    }
    else
    {
	CHAR_DATA *mob;
	int adept;

	if ( !IS_AWAKE(ch) )
	{
	    send_to_char( "In your dreams, or what?\n\r", ch );
	    return;
	}

	for ( mob = ch->in_room->people; mob != NULL; mob = mob->next_in_room )
	{
	    if ( IS_NPC(mob) && IS_SET(mob->act, ACT_PRACTICE) )
		break;
	}

	if ( mob == NULL )
	{
	    send_to_char( "You can't do that here.\n\r", ch );
	    return;
	}

	if ( ch->practice <= 0 )
	{
	    send_to_char( "You have no practice sessions left.\n\r", ch );
	    return;
	}

	if ( ( sn = skill_lookup( argument ) ) < 0
	|| ( !IS_NPC(ch)
	&&   (ch->level < skill_table[sn].skill_level[ch->class] 
 	||    ch->pcdata->learned[sn] < 1 /* skill is not known */
	||    skill_table[sn].rating[ch->class] == 0)))
	{
	    send_to_char( "You can't practice that.\n\r", ch );
	    return;
	}

	adept = IS_NPC(ch) ? 100 : class_table[ch->class].skill_adept;

	if ( ch->pcdata->learned[sn] >= adept )
	{
	    sprintf( buf, "You are already learned at %s.\n\r",
		skill_table[sn].name );
	    send_to_char( buf, ch );
	}
	else
	{
	    ch->practice--;
	    ch->pcdata->learned[sn] += 
		int_app[get_curr_stat(ch,STAT_INT)].learn / 
	        skill_table[sn].rating[ch->class];
	    if ( ch->pcdata->learned[sn] < adept )
	    {
		act( "You practice $T.",
		    ch, NULL, skill_table[sn].name, TO_CHAR );
		act( "$n practices $T.",
		    ch, NULL, skill_table[sn].name, TO_ROOM );
	    }
	    else
	    {
		ch->pcdata->learned[sn] = adept;
		act( "You are now learned at $T.",
		    ch, NULL, skill_table[sn].name, TO_CHAR );
		act( "$n is now learned at $T.",
		    ch, NULL, skill_table[sn].name, TO_ROOM );
	    }
	}
    }
    return;
}

/*
 * 'Wimpy' originally by Dionysos.
 */
void do_wimpy( CHAR_DATA *ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   
   int wimpy;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
	wimpy = ch->max_hit / 5;
    else
	wimpy = atoi( arg );

    if ( wimpy < 0 )
    {
	send_to_char( "Your courage exceeds your wisdom.\n\r", ch );
	return;
    }

    if ( wimpy > ch->max_hit/2 )
    {
	send_to_char( "Such cowardice ill becomes you.\n\r", ch );
	return;
    }

    ch->wimpy	= wimpy;
    sprintf( buf, "Wimpy set to %d hit points.\n\r", wimpy );
    send_to_char( buf, ch );
    return;
}

void do_password( CHAR_DATA *ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH]; 
   char arg2[MAX_INPUT_LENGTH];
    char *pArg;
    char *pwdnew;
    char *p;
    char cEnd;

    if ( IS_NPC(ch) )
	return;

    /*
     * Can't use one_argument here because it smashes case.
     * So we just steal all its code.  Bleagh.
     */
    pArg = arg;
    while ( isspace(*argument) )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

    while ( *argument != '\0' )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*pArg++ = *argument++;
    }
    *pArg = '\0';

    pArg = arg2;
    while ( isspace(*argument) )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

    while ( *argument != '\0' )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*pArg++ = *argument++;
    }
    *pArg = '\0';

    if ( arg[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Syntax: password <old> <new>.\n\r", ch );
	return;
    }

    if ( strcmp( crypt( arg, ch->pcdata->pwd ), ch->pcdata->pwd ) )
    {
	WAIT_STATE( ch, 40 );
	send_to_char( "Wrong password.  Wait 10 seconds.\n\r", ch );
	return;
    }

    if ( strlen(arg2) < 5 )
    {
	send_to_char(
	    "New password must be at least five characters long.\n\r", ch );
	return;
    }

    /*
     * No tilde allowed because of player file format.
     */
    pwdnew = crypt( arg2, ch->name );
    for ( p = pwdnew; *p != '\0'; p++ )
    {
	if ( *p == '~' )
	{
	    send_to_char(
		"New password not acceptable, try again.\n\r", ch );
	    return;
	}
    }

    free_string( ch->pcdata->pwd );
    ch->pcdata->pwd = str_dup( pwdnew );
    save_char_obj( ch );
    send_to_char( "Ok.\n\r", ch );
    return;
}

void do_search( CHAR_DATA *ch)
{
      char buf[MAX_STRING_LENGTH];
   
   extern char * const dir_name[];
    EXIT_DATA *pexit;
    int door;
    bool found;

    if ( !check_blind( ch ) )
	return; 

    send_to_char( "You start searching for secret doors.\n\r", ch );
    found=FALSE;

    for ( door = 0; door <= 5; door++ )
    {
	if ( ( pexit = ch->in_room->exit[door] ) != NULL
	&&   pexit->u1.to_room != NULL
	&&   IS_SET(pexit->exit_info, EX_CLOSED)
	&&   IS_SET(pexit->exit_info, EX_HIDDEN) )
	{
	    found=TRUE;
	sprintf( buf, "You found a secret exit %s.\n\r", dir_name[door]);       
       send_to_char( buf , ch );
	}	  
    }
	if (!found)
	{
                send_to_char( "You found no secret exits.\n\r", ch );
                return;
	}
}

void do_cwho( CHAR_DATA *ch)
{
      char buf[MAX_STRING_LENGTH];
   
   char output[4 * MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    CHAR_DATA *who_list[300];
    int length;
    int maxlength;
    extern bool chaos;

  if(!chaos)
   {
     send_to_char( "There is no `rC`RH`YA`RO`rS`w active.\n\r",ch);
	return;
   }

length=0;
    for ( d = descriptor_list ; d ; d = d->next ) 
	{
           if ( d->connected == CON_PLAYING ) 
	   {
	    chaos_sort(who_list, d->character, length);
            length++;
           }
	}

    buf[0] = '\0';
    output[0] = '\0';
    maxlength=length;
   for ( length=0 ; length < maxlength ; length++ )
    {
        sprintf( buf, "`K[`W%4d`K] `w%s\n\r",
            who_list[length]->pcdata->chaos_score,who_list[length]->name);
        strcat(output, buf);
    }
   send_to_char( output, ch );
   return;
}

void do_finger(CHAR_DATA *ch, char *argument)
{
   char buf[MAX_STRING_LENGTH];
   char arg[MAX_INPUT_LENGTH];

   static char *const he_she [] = { "It", "He", "She" };
   CHAR_DATA 	*victim;
   FILE 	*fp;
   char  	pfile[MAX_STRING_LENGTH], *title;
   char		*word, *class, *race, *comment, *email, *nemesis;
   long		played=0, logon=0;
   char         buf2[MAX_STRING_LENGTH], ltime[MAX_STRING_LENGTH];
   sh_int       sex=0, level, x, pk_kills, pk_deaths;
   /* sh_int clan; */
   
   argument = one_argument( argument, arg );
   pfile[0]='\0';
   word=NULL;
   ltime[0]='\0';
   class=NULL;
   race=NULL;
   title=NULL;
   /* clan=0; */
   level=0;
   pk_kills = 0;
   pk_deaths = 0;
   comment = NULL;
   email = NULL;
   nemesis = NULL;
   
   if ( arg[0] == '\0' || arg[0] == '.' || arg[0] == '/' || arg[0] == '~' )
     {
        send_to_char( "You want information about whom?\n\r", ch );
        return;
     }
   
   else  if ( ( victim = get_char_world( ch, arg ) ) != NULL )
     {
	if (IS_NPC(victim) )
	  { 
	     DESCRIPTOR_DATA * d;
	     victim = NULL;
	     for ( d = descriptor_list; d != NULL; d = d->next )
		{
			if( d->character && !IS_NPC(d->character) && !str_cmp(d->character->name, arg) )
			victim = d->character;
		}
		if (victim == NULL)
		{
	     goto LoadVictim;
	     return;
		}
	  }
	sprintf(buf,"\n\r      `y/~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~/~~\\\n\r");
	send_to_char(buf,ch);
	sprintf(buf,"     |   `W%s%s",victim->name,victim->pcdata->title);
	send_to_char(buf,ch);
	strcpy(buf,"\0");
	for ( x = 0; x < 62-strlen(victim->name)-str_len(victim->pcdata->title) ; x++)
	  strcat( buf, " ");
	send_to_char(buf,ch);
	sprintf(buf,"`y|____|\n\r");
	send_to_char(buf,ch);
	sprintf(buf,"     |+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+|\n\r");
	send_to_char(buf,ch);
/*	if (victim->pcdata->clan!=0)
 *	   sprintf(buf2,"%s of %s",
 *		   is_clan_leader(victim->name,clan_lookup(victim->pcdata->clan)) ?
 *		   "the leader" : "a member",
 *		   vis_clan(victim->pcdata->clan));
 *	 else sprintf(buf2,"not a member of any clan.");
 */     sprintf(buf2,"not a member of any clan.");
	sprintf(buf,"     | `GSex  : `Y%-7s   `W%s is %s`y",
		victim->sex == 0 ? "Sexless" : victim->sex == 1 ? "Male" : "Female",
		he_she[URANGE(0, victim->sex, 2)],
		buf2);
	send_to_char(buf,ch);
	x=str_len(buf);
	strcpy(buf,"\0");
	for (;x<71;x++) strcat(buf," ");
	strcat(buf,"|\n\r");
	send_to_char(buf,ch);
	sprintf(buf,"     | `GLevel: `Y%3d       `WLast login: Currently playing`y",
		UMIN(100, victim->level));
	send_to_char(buf,ch);
	x=str_len(buf);
	strcpy(buf,"\0");
	for (;x<71;x++) strcat(buf," ");
	strcat(buf,"|\n\r");
	send_to_char(buf,ch);
	sprintf(buf,"     | `GAge  :`Y%3d        `W%s is a %s %s.`y",
		get_age(victim),he_she[URANGE(0, victim->sex, 2)],
		pc_race_table[victim->race].name,
		class_table[victim->class].name);
	send_to_char(buf,ch);
	x=str_len(buf);
	strcpy(buf,"\0");
	for (;x<71;x++) strcat(buf," ");
	strcat(buf,"|\n\r");
	send_to_char(buf,ch);
	sprintf(buf,"     | `GPK kills : `Y%-5d `WLast killed by: %s`y",
		victim->pcdata->pk_kills,victim->pcdata->nemesis);
	send_to_char(buf,ch);
	x=str_len(buf);
	strcpy(buf,"\0");
	for (;x<71;x++) strcat(buf," ");
	strcat(buf,"|\n\r");
	send_to_char(buf,ch);
	sprintf(buf,"     | `GPK deaths: `Y%-5d `WEmail: %s`y",
		victim->pcdata->pk_deaths,victim->pcdata->email);
	send_to_char(buf,ch);
	x=str_len(buf);
	strcpy(buf,"\0");
	for (;x<71;x++) strcat(buf," ");
	strcat(buf,"|\n\r");
	send_to_char(buf,ch);
	sprintf(buf,"     |");
	send_to_char(buf,ch);
	x=str_len(buf);
	strcpy(buf,"\0");
	for (;x!=71;x++) strcat(buf," ");
	strcat(buf,"|\n\r");
	send_to_char(buf,ch);
	sprintf(buf,"     | `GComment: `Y%s`y",victim->pcdata->comment);
	send_to_char(buf,ch);
	x=str_len(buf);
	strcpy(buf,"\0");
	for (;x<71;x++) strcat(buf," ");
	strcat(buf,"|\n\r");
	send_to_char(buf,ch);
	send_to_char("`y  /~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~/   |\n\r",ch);
	send_to_char("`y  \\________________________________________________________________\\__/`w\n\r",ch);
	return;
     }
   else
     { 
LoadVictim:
#if defined(unix)
    /* decompress if .gz file exists */
    sprintf( pfile, "%s%s%s", PLAYER_DIR, capitalize(arg),".gz");
    if ( ( fp = fopen( pfile, "r" ) ) != NULL )
    {
	char buf[MAX_INPUT_LENGTH];
        fclose(fp);
        sprintf(buf,"gzip -dfq %s",pfile);
        system(buf);
    }
#endif
	sprintf( pfile, "%s%s", PLAYER_DIR, capitalize( arg ) );
	if ( ( fp = fopen( pfile, "r" ) ) != NULL )
	  {
	     for ( ; ; )
	       {
		  word = get_word(fp);
		  if (!str_cmp(word,"#END"))
		    {
		       break;
		    }
		  if (!str_cmp(word,"#PET"))
		    {
		       break;
		    }
		  if (!str_cmp(word,"Neme") )
		    {
		       nemesis = fread_string(fp);
		    }
		  if (!str_cmp(word,"PKdi") )
		    {
		       pk_deaths = fread_number(fp);
		    }
		  if (!str_cmp(word,"PKki") )
		    {
		       pk_kills = fread_number(fp);
		    }
		  if (!str_cmp(word,"Cmnt") )
		    {
		       comment = fread_string(fp);
		    }
		  if (!str_cmp(word,"Eml") )
		    {
		       email = fread_string(fp);
		    }
		  if (!str_cmp(word,"Log") )
		    {
		       logon=fread_number(fp);
		       sprintf(ltime,"%s at %s",get_date(logon),get_time(logon));
		    }
		  if (!str_cmp(word, "Plyd") )
		    {
		       played=fread_number(fp);
		    }
		  if (!str_cmp(word, "Levl") )
		    {
		       level=fread_number(fp);
		    }
		  if (!str_cmp(word, "Race") )
		    {
		       race = fread_string(fp);
		    }
		  if (!str_cmp(word, "Sex") )
		    {
		       sex = fread_number(fp);
		    }
		  if (!str_cmp(word, "Cla") )
		    {
		       class = (class_table[fread_number(fp)].name);
		    }
		  if (!str_cmp(word, "Titl") )
		    {
		       title = fread_string(fp);
		    }
/*		  if (!str_cmp(word, "Clan") )
 *		     {
 *			clan = fread_number(fp);
 *		     }
 */		  fread_to_eol(fp);
		  if (word) free(word);
	       }
	     fclose(fp);
	     sprintf(buf,"      `y/~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~/~~\\\n\r");
	     send_to_char(buf,ch);
	     sprintf(buf,"     |   `W%s %s",capitalize(arg),title);
	     send_to_char(buf,ch);
	     strcpy(buf,"\0");
	     for ( x = 0; x < 61-strlen(arg)-str_len(title) ; x++)
	       strcat( buf, " ");
	     send_to_char(buf,ch);
	     sprintf(buf,"`y|____|\n\r");
	     send_to_char(buf,ch);
	     sprintf(buf,"     |+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+|\n\r");
	     send_to_char(buf,ch);
/*	     if (clan!=0)
 *		sprintf(buf2,"%s of %s",
 *			is_clan_leader(capitalize(arg),clan_lookup(clan)) ?
 *			"the leader" : "a member",
 *			vis_clan(clan));
 *	      else sprintf(buf2,"not a member of any clan.");
 */	     sprintf(buf2,"not a member of any clan.");
	     sprintf(buf,"     | `GSex  : `Y%-7s   `W%s is %s`y",
		     sex == 0 ? "Sexless" : sex == 1 ? "Male" : "Female",
		     he_she[URANGE(0, sex, 2)],
		     buf2);
	     send_to_char(buf,ch);
	     x=str_len(buf);
	     strcpy(buf,"\0");
	     for (;x<71;x++) strcat(buf," ");
	     strcat(buf,"|\n\r");
	     send_to_char(buf,ch);
	     sprintf(buf,"     | `GLevel: `Y%3d       `WLast login: %s`y",
		     UMIN(100,level),(logon==0) ? "Unknown" : ltime);
	     send_to_char(buf,ch);
	     x=str_len(buf);
	     strcpy(buf,"\0");
	     for (;x<71;x++) strcat(buf," ");
	     strcat(buf,"|\n\r");
	     send_to_char(buf,ch);
	     sprintf(buf,"     | `GAge  :`Y%3ld        `W%s is a %s %s.`y",
		     (17+(played/72000)),
		     he_she[URANGE(0, sex, 2)],
		     race,class);
	     send_to_char(buf,ch);
	     x=str_len(buf);
	     strcpy(buf,"\0");
	     for (;x<71;x++) strcat(buf," ");
	     strcat(buf,"|\n\r");
	     send_to_char(buf,ch);
	sprintf(buf,"     | `GPK kills : `Y%-5d `WLast killed by: %s`y",
		pk_kills,nemesis);
	send_to_char(buf,ch);
	x=str_len(buf);
	strcpy(buf,"\0");
	for (;x<71;x++) strcat(buf," ");
	strcat(buf,"|\n\r");
	send_to_char(buf,ch);
	sprintf(buf,"     | `GPK deaths: `Y%-5d `WEmail: %s`y",
	        pk_deaths,email);
	send_to_char(buf,ch);
	x=str_len(buf);
	strcpy(buf,"\0");
	for (;x<71;x++) strcat(buf," ");
	strcat(buf,"|\n\r");
	send_to_char(buf,ch);
	sprintf(buf,"     |");
	send_to_char(buf,ch);
	x=str_len(buf);
	strcpy(buf,"\0");
	for (;x<71;x++) strcat(buf," ");
	strcat(buf,"|\n\r");
	send_to_char(buf,ch);
	sprintf(buf,"     | `GComment: `Y%s`y",comment);
	send_to_char(buf,ch);
	x=str_len(buf);
	strcpy(buf,"\0");
	for (;x<71;x++) strcat(buf," ");
	strcat(buf,"|\n\r");
	     send_to_char(buf,ch);
	     send_to_char("`y  /~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~/   |\n\r",ch);
	     send_to_char("`y  \\________________________________________________________________\\__/`w\n\r",ch);
/*	     if (class) free_string(class);
 *	     if (race) free_string(race);
 *	     if (comment) free_string(comment);
 *	     if (email) free_string(email);
 *	     if (nemesis) free_string(nemesis); */
	     return;
	  }
	send_to_char("That character does not exist on this mud.\n\r",ch);
	return;
     }
}

void do_levelgain (CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    
if( IS_NPC(ch) ) return;   
 
   if ( ch->level < LEVEL_HERO && ch->exp >= 
	exp_per_level(ch,ch->pcdata->points))
    {
    	sprintf(buf, "%s has made it to level %d!", ch->name, ch->level + 1);
    	do_sendinfo(ch, buf);
	send_to_char( "You raise a level!!  ", ch );
	advance_level( ch );
	ch->level += 1;
	save_char_obj(ch);
    } else send_to_char( "\n\rYou're not quite ready yet.\n\r", ch);
}

/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,	   *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *									   *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael	   *
 *  Chastain, Michael Quan, and Mitchell Tse.				   *
 *									   *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc	   *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.						   *
 *									   *
 *  Much time and thought has gone into this software and you are	   *
 *  benefitting.  We hope that you share your changes too.  What goes	   *
 *  around, comes around.						   *
 ***************************************************************************/
 
/***************************************************************************
*	ROM 2.4 is copyright 1993-1995 Russ Taylor			   *
*	ROM has been brought to you by the ROM consortium		   *
*	    Russ Taylor (rtaylor@pacinfo.com)				   *
*	    Gabrielle Taylor (gtaylor@pacinfo.com)			   *
*	    Brian Moore (rom@rom.efn.org)				   *
*	By using this code, you have agreed to follow the terms of the	   *
*	ROM license, in the file Rom24/doc/rom.license			   *
***************************************************************************/

/* does aliasing and other fun stuff */
void substitute_alias(DESCRIPTOR_DATA *d, char *argument)
{
char buf[MAX_STRING_LENGTH];
   CHAR_DATA *ch;
    char name[MAX_INPUT_LENGTH];
    char *point;
    int alias;

    ch = d->original ? d->original : d->character;


    if (IS_NPC(ch) || ch->pcdata->alias[0] == NULL
    ||	!str_prefix("alias",argument) || !str_prefix("una",argument) ) 
    {
	interpret(d->character,argument);
	return;
    }

    strcpy(buf,argument);

    for (alias = 0; alias < MAX_ALIAS; alias++)	 /* go through the aliases */
    {
	if (ch->pcdata->alias[alias] == NULL)
	    break;

	if (!str_prefix(ch->pcdata->alias[alias],argument))
	{
	    point = one_argument(argument,name);
	    if (!strcmp(ch->pcdata->alias[alias],name))
	    {
		buf[0] = '\0';
		strcat(buf,ch->pcdata->alias_sub[alias]);
		strcat(buf," ");
		strcat(buf,point);
		break;
	    }
	    if (strlen(buf) > MAX_INPUT_LENGTH)
	    {
		send_to_char("Alias substitution too long. Truncated.\r\n",ch);
		buf[MAX_INPUT_LENGTH -1] = '\0';
	    }
	}
    }
    interpret(d->character,buf);
}

void do_alia(CHAR_DATA *ch, char *argument)
{
    send_to_char("I'm sorry, alias must be entered in full.\n\r",ch);
    return;
}

void do_alias(CHAR_DATA *ch, char *argument)
{
char buf[MAX_STRING_LENGTH];
char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *rch;
    int pos;

    if (ch->desc == NULL)
	rch = ch;
    else
	rch = ch->desc->original ? ch->desc->original : ch;

    if (IS_NPC(rch))
	return;

    argument = one_argument(argument,arg);
    
   smash_tilde(arg);
   smash_tilde(argument);
   
    if (arg[0] == '\0')
    {

	if (rch->pcdata->alias[0] == NULL)
	{
	    send_to_char("You have no aliases defined.\n\r",ch);
	    return;
	}
	send_to_char("Your current aliases are:\n\r",ch);

	for (pos = 0; pos < MAX_ALIAS; pos++)
	{
	    if (rch->pcdata->alias[pos] == NULL
	    ||	rch->pcdata->alias_sub[pos] == NULL)
		break;

	    sprintf(buf,"    %s:  %s\n\r",rch->pcdata->alias[pos],
		    rch->pcdata->alias_sub[pos]);
	    send_to_char(buf,ch);
	}
	return;
    }

    if (!str_prefix("una",arg) || !str_cmp("alias",arg))
    {
	send_to_char("Sorry, that word is reserved.\n\r",ch);
	return;
    }

    if (argument[0] == '\0')
    {
	for (pos = 0; pos < MAX_ALIAS; pos++)
	{
	    if (rch->pcdata->alias[pos] == NULL
	    ||	rch->pcdata->alias_sub[pos] == NULL)
		break;

	    if (!str_cmp(arg,rch->pcdata->alias[pos]))
	    {
	       sprintf(buf,"%s aliases to '%s'.\n\r",rch->pcdata->alias[pos],
			rch->pcdata->alias_sub[pos]);
		send_to_char(buf,ch);
		return;
	    }
	}

	send_to_char("That alias is not defined.\n\r",ch);
	return;
    }

    if (!str_prefix(argument,"delete") )
    {
	send_to_char("That shall not be done!\n\r",ch);
	return;
    }

    for (pos = 0; pos < MAX_ALIAS; pos++)
    {
	if (rch->pcdata->alias[pos] == NULL)
	    break;

	if (!str_cmp(arg,rch->pcdata->alias[pos])) /* redefine an alias */
	{
	    free_string(rch->pcdata->alias_sub[pos]);
	    rch->pcdata->alias_sub[pos] = str_dup(argument);
	    sprintf(buf,"%s is now realiased to '%s'.\n\r",arg,argument);
	    send_to_char(buf,ch);
	    return;
	}
     }

     if (pos >= MAX_ALIAS)
     {
	send_to_char("Sorry, you have reached the alias limit.\n\r",ch);
	return;
     }
  
     /* make a new alias */
     rch->pcdata->alias[pos]		= str_dup(arg);
     rch->pcdata->alias_sub[pos]	= str_dup(argument);
     sprintf(buf,"%s is now aliased to '%s'.\n\r",arg,argument);
     send_to_char(buf,ch);
}


void do_unalias(CHAR_DATA *ch, char *argument)
{
char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *rch;
    int pos;
    bool found = FALSE;
 
    if (ch->desc == NULL)
	rch = ch;
    else
	rch = ch->desc->original ? ch->desc->original : ch;
 
    if (IS_NPC(rch))
	return;
 
    argument = one_argument(argument,arg);

    if (arg == '\0')
    {
	send_to_char("Unalias what?\n\r",ch);
	return;
    }

    for (pos = 0; pos < MAX_ALIAS; pos++)
    {
	if (rch->pcdata->alias[pos] == NULL)
	    break;

	if (found)
	{
	    rch->pcdata->alias[pos-1]		= rch->pcdata->alias[pos];
	    rch->pcdata->alias_sub[pos-1]	= rch->pcdata->alias_sub[pos];
	    rch->pcdata->alias[pos]		= NULL;
	    rch->pcdata->alias_sub[pos]		= NULL;
	    continue;
	}

	if(!strcmp(arg,rch->pcdata->alias[pos]))
	{
	    send_to_char("Alias removed.\n\r",ch);
	    free_string(rch->pcdata->alias[pos]);
	    free_string(rch->pcdata->alias_sub[pos]);
	    rch->pcdata->alias[pos] = NULL;
	    rch->pcdata->alias_sub[pos] = NULL;
	    found = TRUE;
	}
    }

    if (!found)
	send_to_char("No alias of that name to remove.\n\r",ch);
}
