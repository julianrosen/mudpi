/* act_comm.c */

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "merc.h"

/* command procedures needed */
DECLARE_DO_FUN(do_quit);

/*
 * Local functions.
 */

/* RT code to delete yourself */
void do_delet( CHAR_DATA *ch, char *argument)
{
    send_to_char("You must type the full command to delete yourself.\n\r",ch);
}

void do_wiznet( CHAR_DATA *ch, char *argument )
{
   char       arg1 [ MAX_INPUT_LENGTH ];
   int level;
    if ( argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOWIZ))
      {
	send_to_char("`WImmortal channels are now ON\n\r`w",ch);
	REMOVE_BIT(ch->comm,COMM_NOWIZ);
      }
      else
      {
	send_to_char("`WImmortal channels are now OFF\n\r`w",ch);
	SET_BIT(ch->comm,COMM_NOWIZ);
      } 
      return;
    }

    REMOVE_BIT(ch->comm,COMM_NOWIZ);
    argument = one_argument( argument, arg1 );
    
    if (!is_number(arg1))
	{
		send_to_char("Syntax to send wiznet message is wiznet <minlevel> <message>.", ch);
		return;
	}
    
    level = atoi( arg1 );
    
    if ( level < LEVEL_HERO || level > MAX_LEVEL)
    {
       send_to_char( "The minimum level must be 91 to 100.\n\r", ch );
       return;
    }

   send_wiz(level, argument);

    return;
}

void send_wiz(int level, char *message)
{
	DESCRIPTOR_DATA *d;
	
	for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->connected == CON_PLAYING && 
	     d->character->level >= level &&
	     !IS_SET(d->character->comm,COMM_NOWIZ) )
	{
	    printf_to_char(d->character, "`w[`rWIZNET`w](`r%d`w):%s`G\n\r", level, message);
	}
    }
}

void do_delete( CHAR_DATA *ch, char *argument)
{
   char strsave[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   if (IS_NPC(ch))
	return;

   if (IS_HERO(ch))  {
	send_to_char("Immortals cannot delete themselves.  Ask an IMP to do it.", ch);
	return;
	}  

   if (ch->pcdata->confirm_delete)
   {
	if (argument[0] != '\0')
	{
	    send_to_char("Delete status removed.\n\r",ch);
	    ch->pcdata->confirm_delete = FALSE;
	    return;
	}
	else
	{
	    sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( ch->name ) );
	    sprintf(buf, "%s just deleted his character.", ch->name);
	    do_quit(ch,"");
	    send_wiz(94, buf);
	    unlink(strsave);
	    return;
	}
    }

    if (argument[0] != '\0')
    {
	send_to_char("Just type delete. No argument.\n\r",ch);
	return;
    }

    send_to_char("Type delete again to confirm this command.\n\r",ch);
    send_to_char("WARNING: this command is irreversible.\n\r",ch);
    send_to_char("Typing delete with an argument will undo delete status.\n\r",
	ch);
    ch->pcdata->confirm_delete = TRUE;
}
	    
void do_channels( CHAR_DATA *ch, char *argument)
{
   char buf[MAX_STRING_LENGTH];
   
   /* lists all channels and their status */
    send_to_char("`W   channel   status`w\n\r",ch);
    send_to_char("`K---------------------`w\n\r",ch);
 
    send_to_char("gossip         ",ch);
    if (!IS_SET(ch->comm,COMM_NOGOSSIP))
      send_to_char("`GON`w\n\r",ch);
    else
      send_to_char("`ROFF`w\n\r",ch);

    send_to_char("auction        ",ch);
    if (!IS_SET(ch->comm,COMM_NOAUCTION))
      send_to_char("`GON`w\n\r",ch);
    else
      send_to_char("`ROFF`w\n\r",ch);

    send_to_char("OOC            ",ch);
    if (!IS_SET(ch->comm,COMM_NO_OOC))
      send_to_char("`GON`w\n\r",ch);
    else
      send_to_char("`ROFF`w\n\r",ch);

    send_to_char("Q/A            ",ch);
    if (!IS_SET(ch->comm,COMM_NOQUESTION))
      send_to_char("`GON`w\n\r",ch);
    else
      send_to_char("`ROFF`w\n\r",ch);

    if (IS_HERO(ch))
    {
      send_to_char("imm channel    ",ch);
      if(!IS_SET(ch->comm,COMM_NOWIZ))
	send_to_char("`GON`w\n\r",ch);
      else
	send_to_char("`ROFF`w\n\r",ch);
    }

    send_to_char("shouts         ",ch);
    if (!IS_SET(ch->comm,COMM_DEAF))
      send_to_char("`GON`w\n\r",ch);
    else
      send_to_char("`ROFF`w\n\r",ch);

    send_to_char("info           ",ch);
    if (!IS_SET(ch->comm,COMM_NOINFO))
      send_to_char("`GON`w\n\r",ch);
    else
      send_to_char("`ROFF`w\n\r",ch);

    send_to_char("quiet mode     ",ch);
    if (IS_SET(ch->comm,COMM_QUIET))
      send_to_char("`GON`w\n\r",ch);
    else
      send_to_char("`ROFF`w\n\r",ch);
   
    if (ch->lines != PAGELEN)
    {
	if (ch->lines)
	{
	    sprintf(buf,"You display %d lines of scroll.\n\r",ch->lines+2);
	    send_to_char(buf,ch);
	}
	else
	    send_to_char("Scroll buffering is off.\n\r",ch);
    }

    if (IS_SET(ch->comm,COMM_NOSHOUT))
      send_to_char("You cannot shout.\n\r",ch);
  
    if (IS_SET(ch->comm,COMM_NOTELL))
      send_to_char("You cannot use tell.\n\r",ch);
 
    if (IS_SET(ch->comm,COMM_NOCHANNELS))
     send_to_char("You cannot use channels.\n\r",ch);

    if (IS_SET(ch->comm,COMM_NOEMOTE))
      send_to_char("You cannot emote.\n\r",ch);

}

/* RT deaf blocks out all shouts */
void do_deaf( CHAR_DATA *ch, char *argument)
{
    if (IS_SET(ch->comm,COMM_NOSHOUT))
    {
      send_to_char("The gods have taken away your ability to shout.\n\r",ch);
      return;
    }
    
   if (IS_SET(ch->comm,COMM_DEAF))
   {
     send_to_char("You can now hear shouts again.\n\r",ch);
     REMOVE_BIT(ch->comm,COMM_DEAF);
   }
   else 
   {
     send_to_char("From now on, you won't hear shouts.\n\r",ch);
     SET_BIT(ch->comm,COMM_DEAF);
   }
}

/* RT quiet blocks out all communication */
void do_quiet ( CHAR_DATA *ch, char * argument)
{
    char buf[MAX_STRING_LENGTH];

    if (IS_SET(ch->comm,COMM_QUIET))
    {
      send_to_char("Quiet mode removed.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_QUIET);
      if (ch->pcdata->message != NULL) {
	  sprintf(buf, "You have `W%d message%s`w waiting, type `Wmessages`w to read them.\n\r",
		ch->pcdata->messages, (ch->pcdata->messages > 1) ? "s" : "" );
	  send_to_char(buf, ch);
      }      
    }
   else
   {
      send_to_char("From now on, you will only hear says and emotes.\n\r",ch);
      send_to_char("Messages are being recorded.\n\r",ch);
      SET_BIT(ch->comm,COMM_QUIET);
   }
}

/* RT auction rewritten in ROM style */
void do_auction( CHAR_DATA *ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   
   DESCRIPTOR_DATA *d;

    if (argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOAUCTION))
      {
	send_to_char("`mAuction channel is now ON.\n\r`w",ch);
	REMOVE_BIT(ch->comm,COMM_NOAUCTION);
      }
      else
      {
	send_to_char("`mAuction channel is now OFF.\n\r`w",ch);
	SET_BIT(ch->comm,COMM_NOAUCTION);
      }
    }
    else  /* auction message sent, turn auction on if it is off */
    {
	if (IS_SET(ch->comm,COMM_QUIET))
	{
	  send_to_char("You must turn off quiet mode first.\n\r",ch);
	  return;
	}

	if (IS_SET(ch->comm,COMM_NOCHANNELS))
	{
	  send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
	  return;

	REMOVE_BIT(ch->comm,COMM_NOAUCTION);
      }

       sprintf( buf, "`mYou auction '%s`m'\n\r`w", argument );
       send_to_char( buf, ch );
       sprintf(buf, " `mauctions '%s`m'`w\n\r", argument);
/*       
add2last(buf,ch);
*/
       for ( d = descriptor_list; d != NULL; d = d->next )
	 {
	    CHAR_DATA *victim;
	    
	    victim = d->original ? d->original : d->character;
	    if ( d->connected == CON_PLAYING &&
		d->character != ch &&
		!IS_SET(victim->comm,COMM_NOAUCTION) &&
		!IS_SET(victim->comm,COMM_QUIET) )
	      {
		 act_new("`m$n auctions '$t`m'`w",
			 ch,argument,d->character,TO_VICT,POS_DEAD);
	      }
	 }
    }
}

/* RT chat replaced with ROM gossip */
void do_gossip( CHAR_DATA *ch, char *argument )
{
   DESCRIPTOR_DATA *d;
   char buf[MAX_STRING_LENGTH];
   
    if (argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOGOSSIP))
      {
	send_to_char("`BGossip channel is now ON.\n\r`w",ch);
	REMOVE_BIT(ch->comm,COMM_NOGOSSIP);
      }
      else
      {
	send_to_char("`BGossip channel is now OFF.\n\r`w",ch);
	SET_BIT(ch->comm,COMM_NOGOSSIP);
      }
    }
    else  /* gossip message sent, turn gossip on if it isn't already */
    {
	if (IS_SET(ch->comm,COMM_QUIET))
	{
	  send_to_char("You must turn off quiet mode first.\n\r",ch);
	  return;
	}
 
	if (IS_SET(ch->comm,COMM_NOCHANNELS))
	{
	  send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
	  return;
 
	}

      REMOVE_BIT(ch->comm,COMM_NOGOSSIP);
 
       sprintf( buf, "`BYou gossip '%s`B'`w\n\r", argument );
       send_to_char( buf, ch );
       sprintf(buf, " `Bgossips '%s`B'`w\n\r", argument);
/*       
add2last(buf,ch);
*/       
       for ( d = descriptor_list; d != NULL; d = d->next )
	 {
	    CHAR_DATA *victim;
	    
	    victim = d->original ? d->original : d->character;
	    if ( d->connected == CON_PLAYING &&
		d->character != ch &&
		!IS_SET(victim->comm,COMM_NOGOSSIP) &&
		!IS_SET(victim->comm,COMM_QUIET) &&
		d->character->position >= POS_SLEEPING )
	      {/* Act_new kept giving me bugs from memory junk... -Kyle */
		printf_to_char(d->character, "`B%s gossips '%s`B'`w\n\r",
		IS_NPC(ch) ? ch->short_descr : ch->name, argument);
/*
		 act_new( "`B$n gossips '$t`B'`w", 
			 ch,argument, d->character, TO_VICT,POS_SLEEPING );
*/
	      }
	 }
    }
}

/* RT question channel */
void do_question( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
   char buf[MAX_STRING_LENGTH];
   
    if (argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOQUESTION))
      {
	send_to_char("`rQ/A channel is now ON.\n\r`w",ch);
	REMOVE_BIT(ch->comm,COMM_NOQUESTION);
      }
      else
      {
	send_to_char("`rQ/A channel is now OFF.\n\r`w",ch);
	SET_BIT(ch->comm,COMM_NOQUESTION);
      }
    }
    else  /* question sent, turn Q/A on if it isn't already */
    {
	if (IS_SET(ch->comm,COMM_QUIET))
	{
	  send_to_char("You must turn off quiet mode first.\n\r",ch);
	  return;
	}
 
	if (IS_SET(ch->comm,COMM_NOCHANNELS))
	{
	  send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
	  return;
	}
 
	REMOVE_BIT(ch->comm,COMM_NOQUESTION);
       
       sprintf( buf, "`rYou question '%s`r'\n\r`w", argument );
       send_to_char( buf, ch );
       sprintf(buf, " `rquestions '%s`r'`w\n\r", argument);
/*       
add2last(buf,ch);
*/	
       for ( d = descriptor_list; d != NULL; d = d->next )
	 {
	    CHAR_DATA *victim;
	    
	    victim = d->original ? d->original : d->character;
	    if ( d->connected == CON_PLAYING &&
		d->character != ch &&
		!IS_SET(victim->comm,COMM_NOQUESTION) &&
		!IS_SET(victim->comm,COMM_QUIET) )
	      {
		 act_new("`r$n questions '$t`r'`w",
			 ch,argument,d->character,TO_VICT,POS_SLEEPING);
	      }
	 }
    }
}

/* RT answer channel - uses same line as questions */
void do_answer( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
   char buf[MAX_STRING_LENGTH];
   
    if (argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOQUESTION))
      {
	send_to_char("`rQ/A channel is now ON.\n\r`w",ch);
	REMOVE_BIT(ch->comm,COMM_NOQUESTION);
      }
      else
      {
	send_to_char("`rQ/A channel is now OFF.\n\r`w",ch);
	SET_BIT(ch->comm,COMM_NOQUESTION);
      }
    }
    else  /* answer sent, turn Q/A on if it isn't already */
    {
	if (IS_SET(ch->comm,COMM_QUIET))
	{
	  send_to_char("You must turn off quiet mode first.\n\r",ch);
	  return;
	}
 
	if (IS_SET(ch->comm,COMM_NOCHANNELS))
	{
	  send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
	  return;
	}
 
	REMOVE_BIT(ch->comm,COMM_NOQUESTION);
 
       sprintf( buf, "`rYou answer '%s`r'\n\r`w", argument );
       send_to_char( buf, ch );
       sprintf(buf, "`r answers '%s`r'`w\n\r", argument);
/*       
add2last(buf,ch);
*/       
       for ( d = descriptor_list; d != NULL; d = d->next )
	 {
	    CHAR_DATA *victim;
	    
	    victim = d->original ? d->original : d->character;
	    if ( d->connected == CON_PLAYING &&
		d->character != ch &&
		!IS_SET(victim->comm,COMM_NOQUESTION) &&
		!IS_SET(victim->comm,COMM_QUIET) )
	      {
		 act_new("`r$n answers '$t`r'`w",
			 ch,argument,d->character,TO_VICT,POS_SLEEPING);
	      }
	 }
    }
}

/* Kyle OOC channel */
void do_ooc( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
   char buf[MAX_STRING_LENGTH];
   
    if (argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NO_OOC))
      {
	send_to_char("`COOC channel is now ON.\n\r`w",ch);
	REMOVE_BIT(ch->comm,COMM_NO_OOC);
      }
      else
      {
	send_to_char("`COOC channel is now OFF.\n\r`w",ch);
	SET_BIT(ch->comm,COMM_NO_OOC);
      }
    }
    else  /* OOC sent, turn OOC on if it isn't already */
    {
	if (IS_SET(ch->comm,COMM_QUIET))
	{
	  send_to_char("You must turn off quiet mode first.\n\r",ch);
	  return;
	}
 
	if (IS_SET(ch->comm,COMM_NOCHANNELS))
	{
	  send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
	  return;
	}
 
	REMOVE_BIT(ch->comm,COMM_NO_OOC);
 
       sprintf( buf, "`CYou OOC: '%s`C'\n\r`w", argument );
       send_to_char( buf, ch );
       sprintf(buf, "`C OOC: '%s`C'`w\n\r", argument);
/*       
add2last(buf,ch);
*/       
       for ( d = descriptor_list; d != NULL; d = d->next )
	 {
	    CHAR_DATA *victim;
	    
	    victim = d->original ? d->original : d->character;
	    if ( d->connected == CON_PLAYING &&
		d->character != ch &&
		!IS_SET(victim->comm,COMM_NO_OOC) &&
		!IS_SET(victim->comm,COMM_QUIET) )
	      {
		 act_new("`C$n OOC: '$t`C'`w",
			 ch,argument,d->character,TO_VICT,POS_SLEEPING);
	      }
	 }
    }
}

void do_immtalk( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
   
    if ( argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOWIZ))
      {
	send_to_char("`WImmortal channels are now ON\n\r`w",ch);
	REMOVE_BIT(ch->comm,COMM_NOWIZ);
      }
      else
      {
	send_to_char("`WImmortal channels are now OFF\n\r`w",ch);
	SET_BIT(ch->comm,COMM_NOWIZ);
      } 
      return;
    }

    REMOVE_BIT(ch->comm,COMM_NOWIZ);
/*
   sprintf( buf, "`W$n`W: %s`w", argument );
   act_new("`W$n`W: $t`w",ch,argument,NULL,TO_CHAR,POS_DEAD);
   sprintf( buf, "`W: %s`w\n",argument);   
add2last_imm(buf,ch);
*/
   for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->connected == CON_PLAYING && 
	     IS_IMMORTAL(d->character) && 
	     !IS_SET(d->character->comm,COMM_NOWIZ) )
	{/* act new gives me occasional bugs... */
/*
	    act_new("`W$n: $t`w",ch,argument,d->character,TO_VICT,POS_DEAD);
*/
	    printf_to_char(d->character, "`W%s: %s`w\n\r", ch->name, argument);
	}
    }

    return;
}

void do_say( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA * old_room;
    if ( argument[0] == '\0' )
    {
	send_to_char( "Say what?\n\r", ch );
	return;
    }

    act( "`G$n says '$T`G'`w", ch, NULL, argument, TO_ROOM );
    act( "`GYou say '$T`G'`w", ch, NULL, argument, TO_CHAR );
    old_room = ch->in_room;

    oprog_speech_trigger( argument, ch );
    if ( !ch )
	return;

    if ( ch->in_room != old_room )
	return;

    mprog_speech_trigger( argument, ch );
    if ( !ch )
	return;
    
    if ( ch->in_room != old_room )
	return;
         
    rprog_speech_trigger( argument, ch );
	
                   
    return;
}

void do_shout( CHAR_DATA *ch, char *argument )
{
   char buf [MAX_STRING_LENGTH];
   DESCRIPTOR_DATA *d;
   
    if ( IS_SET(ch->comm, COMM_NOSHOUT) )
    {
	send_to_char( "You can't shout.\n\r", ch );
	return;
    }

    if ( IS_SET(ch->comm, COMM_DEAF))
    {
	send_to_char( "Deaf people can't shout.\n\r",ch);
	return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "Shout what?\n\r", ch );
	return;
    }

   WAIT_STATE( ch, 12 );
   
   act( "`WYou shout '$T`W'`w", ch, NULL, argument, TO_CHAR );
   sprintf(buf, "`W shouts '%s`W'`w\n\r", argument);
/*   
add2last(buf,ch);
*/   
   for ( d = descriptor_list; d != NULL; d = d->next )
     {
	CHAR_DATA *victim;
	
	victim = d->original ? d->original : d->character;
	
	if ( d->connected == CON_PLAYING &&
	    d->character != ch &&
	    !IS_SET(victim->comm, COMM_DEAF) &&
	    !IS_SET(victim->comm, COMM_QUIET) ) 
	  {
	     act("`W$n shouts '$t`W'`w",ch,argument,d->character,TO_VICT);
	}
     }
   
   return;
}

void do_sendinfo( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
   char buf[MAX_STRING_LENGTH];
   
    if (argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOINFO))
      {
	send_to_char("`BInfo channel is now ON.\n\r`w",ch);
	REMOVE_BIT(ch->comm,COMM_NOINFO);
      }
      else
      {
	send_to_char("`BInfo channel is now OFF.\n\r`w",ch);
	SET_BIT(ch->comm,COMM_NOINFO);
      }
      return;
    }
   strcpy(buf, "`WINFO: `R");
   strcat(buf, argument);
   strcat(buf, "\n`r\r`w");
/*
     {
	CHAR_DATA *tmp=alloc_mem(sizeof(CHAR_DATA));
	tmp->level = 0; tmp->name=strdup(" ");
	add2last(buf,tmp);
	free_string(tmp->name);
	free_mem(tmp,sizeof(CHAR_DATA));
     }
*/
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	CHAR_DATA *victim;

	victim = d->original ? d->original : d->character;
	
	if ( d->connected == CON_PLAYING &&
	     !IS_SET(victim->comm, COMM_DEAF) &&
	     !IS_SET(victim->comm, COMM_NOINFO) &&
	     !IS_SET(victim->comm, COMM_QUIET) )
	{
	    send_to_char(buf,d->character);
	}
    }

    return;
}

void do_beep( CHAR_DATA *ch, char *argument )
{
   if (ch->beep)
   {
     ch->beep = FALSE;
     send_to_char("`RBeeps are now off\n\r",ch);
     return;
   }
   ch->beep = TRUE;
   send_to_char("`RBeeps are now on\n\r",ch);
   return;
}

void do_tell( CHAR_DATA *ch, char *argument )
{
   char buf [MAX_STRING_LENGTH];
   char arg [MAX_INPUT_LENGTH]; 
   CHAR_DATA *victim;
   
    if ( IS_SET(ch->comm, COMM_NOTELL) )
    {
	send_to_char( "Your message didn't get through.\n\r", ch );
	return;
    }

    if ( IS_SET(ch->comm, COMM_QUIET) )
    {
	send_to_char( "You must turn off quiet mode first.\n\r", ch);
	return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Tell whom what?\n\r", ch );
	return;
    }

    /*
     * Can tell to PC's anywhere, but NPC's only in same room.
     * -- Furey
     */
    if ( ( victim = get_char_world( ch, arg ) ) == NULL
    || ( IS_NPC(victim) && victim->in_room != ch->in_room ) )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->desc == NULL && !IS_NPC(victim))
    {
	act("$N seems to have misplaced $S link...please try again later.",
	    ch,NULL,victim,TO_CHAR);
	return;
    }

    if ( !IS_IMMORTAL(ch) && !IS_AWAKE(victim) )
    {
	act( "$E can't hear you, but your message has been recorded.", 
		ch, 0, victim, TO_CHAR );
	add2queue(ch, victim, argument);
	return;
    }
  
    if ( IS_SET(victim->comm,COMM_QUIET) && !IS_IMMORTAL(ch) )
    {
	act( "$E is not receiving tells, but your message has been recorded.", 
		ch, 0, victim, TO_CHAR );
	add2queue(ch, victim, argument);
	return;
    }

    if ( IS_SET(victim->act,PLR_AFK) )
    {
	act( "$E is AFK and can't hear you, but your message has been recorded.", 
		ch, 0, victim, TO_CHAR );
	add2queue(ch, victim, argument);
    }

    act( "`RYou tell $N '$t`R'`w", ch, argument, victim, TO_CHAR );
if (victim->beep)
    act_new("`R$n \atells you '$t`R'`w",ch,argument,victim,TO_VICT,POS_DEAD);
else
    act_new("`R$n tells you '$t`R'`w",ch,argument,victim,TO_VICT,POS_DEAD);
    sprintf(buf, "`R%s tells you '%s`R'`w\n\r",ch->name, argument);
/*    
add2tell(ch, victim, buf);
*/  
  victim->reply       = ch;

    return;
}

void do_whisper( CHAR_DATA *ch, char *argument )
{
   char arg [MAX_INPUT_LENGTH]; 
   CHAR_DATA *victim;

    if ( IS_SET(ch->comm, COMM_QUIET) )
    {
	send_to_char( "You must turn off quiet mode first.\n\r", ch);
	return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Whisper what to who?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL
    || ( IS_NPC(victim) && victim->in_room != ch->in_room ) )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->desc == NULL && !IS_NPC(victim))
    {
	act("$N seems to have misplaced $S link...please try again
later.",
	    ch,NULL,victim,TO_CHAR);
	return;
    }

    act("`R$n whispers, '$t`R' to you.`w",ch,argument,victim,TO_VICT );
    act( "`RYou whisper, '$t'`R to $N`w", ch, argument, victim, TO_CHAR
);
    act("`R$n whispers somthing to $N", ch,NULL,victim,TO_NOTVICT );
    return;
}


void do_reply( CHAR_DATA *ch, char *argument )
{
    char buf [MAX_STRING_LENGTH];
    CHAR_DATA *victim;
   
    if ( IS_SET(ch->comm, COMM_NOTELL) )
    {
	send_to_char( "Your message didn't get through.\n\r", ch );
	return;
    }

    if ( ( victim = ch->reply ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->desc == NULL && !IS_NPC(victim))
    {
	act("$N seems to have misplaced $S link...please try again later.",
	    ch,NULL,victim,TO_CHAR);
	return;
    }

    if ( !IS_IMMORTAL(ch) && !IS_AWAKE(victim) )
    {
	act( "$E can't hear you, but your message has been recorded.", 
		ch, 0, victim, TO_CHAR );
	add2queue(ch, victim, argument);
	return;
    }

    if ( IS_SET(victim->comm,COMM_QUIET) && !IS_IMMORTAL(ch))
    {
	act( "$E is not receiving tells, but your message has been recorded.", 
		ch, 0, victim, TO_CHAR );
	add2queue(ch, victim, argument);
	return;
    }

    if ( IS_SET(victim->act, PLR_AFK) )
    {
	act( "$E is AFK and can't hear you, but your message has been recorded.", 
		ch, 0, victim, TO_CHAR );
	add2queue(ch, victim, argument);
    }

    act("`RYou tell $N '$t`R'`w",ch,argument,victim,TO_CHAR);
if (victim->beep)
    act_new("`R$n \atells you '$t`R'`w",ch,argument,victim,TO_VICT,POS_DEAD);
else
    act_new("`R$n tells you '$t`R'`w",ch,argument,victim,TO_VICT,POS_DEAD);
    sprintf(buf, "`R%s tells you '%s`R'`w\n\r",ch->name, argument);
/*
    add2tell(ch, victim, buf);
*/
    victim->reply       = ch;

    return;
}

void add2queue( CHAR_DATA *ch, CHAR_DATA *victim, char * argument)
{

	if IS_NPC( ch ) return;

	if (victim->pcdata->message == NULL) {
		victim->pcdata->message = alloc_mem (sizeof(QUEUE_DATA));
		victim->pcdata->message->next = NULL;
		victim->pcdata->message->sender = str_dup(ch->name);
		victim->pcdata->message->text = str_dup(argument);
		victim->pcdata->messages++;
		victim->pcdata->fmessage = victim->pcdata->message;
	} else {
		victim->pcdata->message->next = alloc_mem (sizeof(QUEUE_DATA));
		victim->pcdata->message = victim->pcdata->message->next;
		victim->pcdata->message->next = NULL;
		victim->pcdata->message->sender = str_dup(ch->name);
		victim->pcdata->message->text = str_dup(argument);
		victim->pcdata->messages++;
	}
}

void add2tell( CHAR_DATA *ch, CHAR_DATA *victim, char * argument)
{

   if IS_NPC( ch ) return; /* not for NPCs */
   
   if (victim->pcdata == NULL) return; 
   /* Only happens when vict is a switched imm I think */
   
   if (victim->pcdata->tells+1 > MAX_LAST_LENGTH)
     {
	QUEUE_DATA *tmp;
	tmp = victim->pcdata->ftell_q;
	free_mem(tmp,sizeof(QUEUE_DATA));
	victim->pcdata->ftell_q=victim->pcdata->ftell_q->next;
	victim->pcdata->tells--;
     }
   
   if (victim->pcdata->tell_q == NULL) {
      victim->pcdata->tell_q = alloc_mem (sizeof(QUEUE_DATA));
      victim->pcdata->tell_q->next = NULL;
      victim->pcdata->tell_q->sender = str_dup(ch->name);
      victim->pcdata->tell_q->text = str_dup(argument);
      victim->pcdata->tells++;
      victim->pcdata->ftell_q = victim->pcdata->tell_q;
   } else {
      victim->pcdata->tell_q->next = alloc_mem (sizeof(QUEUE_DATA));
      victim->pcdata->tell_q = victim->pcdata->tell_q->next;
      victim->pcdata->tell_q->next = NULL;
      victim->pcdata->tell_q->sender = str_dup(ch->name);
      victim->pcdata->tell_q->text = str_dup(argument);
      victim->pcdata->tells++;
   }
}
/*
void add2last( char *argument, CHAR_DATA *ch )
{
   LAST_DATA *  tmp;
   
   if (last_list == NULL) 
     {
	last_list = alloc_mem(sizeof(LAST_DATA));
	last_list->next = NULL;
	flast = last_list;
	last_list->msg = str_dup(argument);
	last_list->level = ch->invis_level;
	last_list->hidden = (IS_SET(ch->act,PLR_WIZINVIS)) ? TRUE : FALSE;
	last_list->sender = str_dup(ch->name);
	last_length++;
     } 
   else 
     {
	if (last_length >= MAX_LAST_LENGTH) 
	  {
	     tmp = flast;
	     flast = flast->next;
	     if (strlen(tmp->msg) > 0 && tmp->msg!=NULL) free_string(tmp->msg);
//         if (strlen(tmp->sender)>0) free_string(tmp->sender); //
	 if (strlen(tmp->sender)>0 && tmp->sender!=NULL) free_string(tmp->sender);
	 free_mem(tmp, sizeof(LAST_DATA));
	     last_length--;
	  }
	last_list->next = alloc_mem(sizeof(LAST_DATA));
	last_list = last_list->next;
	last_list->next = NULL;
	last_list->msg = str_dup(argument);
	last_list->sender = str_dup(ch->name);
	last_list->level = ch->invis_level;
	last_list->hidden = (IS_SET(ch->act,PLR_WIZINVIS)) ? TRUE : FALSE;
	last_length++;
     }
}
*/
/*
void add2last_imm( char *argument, CHAR_DATA *ch)
{
   LAST_DATA *  tmp;
   
   if (last_imm == NULL) 
     {
	last_imm = alloc_mem(sizeof(LAST_DATA));
	last_imm->next = NULL;
	flast_imm = last_imm;
	last_imm->msg = str_dup(argument);
	last_imm->sender = str_dup(ch->name);
	last_imm->level = ch->invis_level;
	last_imm->hidden = (IS_SET(ch->act,PLR_WIZINVIS)) ? TRUE : FALSE;
	last_imm_length++;
     } 
   else 
     {
	if (last_imm_length >= MAX_LAST_LENGTH) 
	  {
	     tmp = flast_imm;
	     flast_imm = flast_imm->next;
	     free_string(tmp->msg);
//         if (tmp->sender!=NULL) free_string(tmp->sender); //
	 if (strlen(tmp->sender)>0 && tmp->sender!=NULL) free_string(tmp->sender);
	     free_mem(tmp, sizeof(LAST_DATA));
	     last_imm_length--;
	  }
	last_imm->next = alloc_mem(sizeof(LAST_DATA));
	last_imm = last_imm->next;
	last_imm->next = NULL;
	last_imm->msg = str_dup(argument);
	last_imm->sender = str_dup(ch->name);
	last_imm->level = ch->invis_level;
	last_imm->hidden = (IS_SET(ch->act,PLR_WIZINVIS)) ? TRUE : FALSE;
	last_imm_length++;
     }
}
*/
void do_messages ( CHAR_DATA *ch, char *argument )
{
	QUEUE_DATA      *mq;
	char buf[MAX_STRING_LENGTH];
	
	if IS_NPC( ch ) return;

	if (ch->pcdata->fmessage == NULL) {
		send_to_char( "You have no messages waiting.\n\r", ch );
		return;
	}

	sprintf(buf, "`R%s tells you '%s`R'`w\n\r",ch->pcdata->fmessage->sender,
		ch->pcdata->fmessage->text);
	send_to_char(buf, ch);
	
	ch->reply       = get_char_world (ch, ch->pcdata->fmessage->sender);
	
	mq = ch->pcdata->fmessage;
	ch->pcdata->fmessage = ch->pcdata->fmessage->next;
	free_string(mq->text);
	free_string(mq->sender);
	free_mem(mq, sizeof(QUEUE_DATA));
	if (ch->pcdata->fmessage == NULL)
		ch->pcdata->message = NULL;

	ch->pcdata->messages--;
	if (ch->pcdata->messages > 0) {
		sprintf(buf, "You still have `C%d`w message%s in your queue.\n\r",
			ch->pcdata->messages, (ch->pcdata->messages > 1) ? "s" : "" );
		send_to_char(buf, ch);
	}
	
	return;
}

void do_tq ( CHAR_DATA *ch, char *argument )
{
   QUEUE_DATA   *mq;
   char buf[MAX_STRING_LENGTH];
   
   if IS_NPC( ch ) return;
   
   if (ch->pcdata->ftell_q == NULL) {
      send_to_char( "You have recieved no private messages.\n\r", ch );
      return;
   }
   
   for (mq = ch->pcdata->ftell_q; mq != NULL; mq = mq->next)
     {
	sprintf(buf,"%s`w",mq->text);
	send_to_char(buf,ch);
     }
	
   return;
}
/*
void do_last( CHAR_DATA *ch, char *argument )
{
   LAST_DATA * tlast;
   char buf[MAX_STRING_LENGTH];
   
   if (last_length == 0) 
     {
	send_to_char("Nothing has been recorded yet.\n\r", ch);
	return;
     }
   
   for ( tlast = flast; tlast != NULL; tlast = tlast->next)
     {
	if (tlast->hidden)
	  {
	     if (tlast->level > ch->level)
	       sprintf(buf,"%s%s","someone",tlast->msg);
	     else sprintf(buf,"%s%s",tlast->sender,tlast->msg);
	  }
	else sprintf(buf,"%s%s",tlast->sender,tlast->msg);
	send_to_char(buf, ch);
     }
}

void do_lastimm( CHAR_DATA *ch, char *argument )
{
   LAST_DATA * tlast;
   char buf[MAX_STRING_LENGTH];
   
   if (last_imm_length == 0) 
     {
	send_to_char("Nothing has been recorded yet.\n\r", ch);
	return;
     }
   
   for ( tlast = flast_imm; tlast != NULL; tlast = tlast->next)
     {
	if (tlast->hidden)
	  {
	     if (tlast->level > ch->level)
	       sprintf(buf,"%s%s","someone",tlast->msg);
	     else sprintf(buf,"%s%s",tlast->sender,tlast->msg);
	  }
	else sprintf(buf,"%s%s",tlast->sender,tlast->msg);
	send_to_char(buf, ch);
     }
}
*/
void do_yell( CHAR_DATA *ch, char *argument )
{
    char buf [MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
   
    if ( IS_SET(ch->comm, COMM_NOSHOUT) )
    {
	send_to_char( "You can't yell.\n\r", ch );
	return;
    }
 
    if ( argument[0] == '\0' )
    {
	send_to_char( "Yell what?\n\r", ch );
	return;
    }


    act("`YYou yell '$t`Y'`w",ch,argument,NULL,TO_CHAR);
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->connected == CON_PLAYING
	&&   d->character != ch
	&&   d->character->in_room != NULL
	&&   d->character->in_room->area == ch->in_room->area 
	&&   !IS_SET(d->character->comm,COMM_QUIET) )
	{
	    act("`Y$n yells '$t`Y'`w",ch,argument,d->character,TO_VICT);
	    sprintf(buf, "`Y%s yells '%s`Y'`w\n\r",ch->name,argument);
	}
    }

    return;
}

void do_emote( CHAR_DATA *ch, char *argument )
{
   if ( !IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE) )
    {
	send_to_char( "You can't emote.\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "Emote what?\n\r", ch );
	return;
    }

    act( "$n $T`w", ch, NULL, argument, TO_ROOM );
    act( "$n $T`w", ch, NULL, argument, TO_CHAR );
    return;
}

void do_info( CHAR_DATA *ch)
{
      if (IS_SET(ch->comm,COMM_NOINFO))
      {
	send_to_char("`BInfo channel is now ON.\n\r`w",ch);
	REMOVE_BIT(ch->comm,COMM_NOINFO);
      }
      else
      {
	send_to_char("`BInfo channel is now OFF.\n\r`w",ch);
	SET_BIT(ch->comm,COMM_NOINFO);
      }
      return;
}

/*
 * All the posing stuff.
 */
struct  pose_table_type
{
    char *      message[2*MAX_CLASS];
};

const   struct  pose_table_type pose_table      []      =
{
    {
	{
	    "You sizzle with energy.",
	    "$n sizzles with energy.",
	    "You feel very holy.",
	    "$n looks very holy.",
	    "You perform a small card trick.",
	    "$n performs a small card trick.",
	    "You show your bulging muscles.",
	    "$n shows $s bulging muscles."
	}
    },

    {
	{
	    "You turn into a butterfly, then return to your normal shape.",
	    "$n turns into a butterfly, then returns to $s normal shape.",
	    "You nonchalantly turn wine into water.",
	    "$n nonchalantly turns wine into water.",
	    "You wiggle your ears alternately.",
	    "$n wiggles $s ears alternately.",
	    "You crack nuts between your fingers.",
	    "$n cracks nuts between $s fingers."
	}
    },

    {
	{
	    "Blue sparks fly from your fingers.",
	    "Blue sparks fly from $n's fingers.",
	    "A halo appears over your head.",
	    "A halo appears over $n's head.",
	    "You nimbly tie yourself into a knot.",
	    "$n nimbly ties $mself into a knot.",
	    "You grizzle your teeth and look mean.",
	    "$n grizzles $s teeth and looks mean."
	}
    },

    {
	{
	    "Little red lights dance in your eyes.",
	    "Little red lights dance in $n's eyes.",
	    "You recite words of wisdom.",
	    "$n recites words of wisdom.",
	    "You juggle with daggers, apples, and eyeballs.",
	    "$n juggles with daggers, apples, and eyeballs.",
	    "You hit your head, and your eyes roll.",
	    "$n hits $s head, and $s eyes roll."
	}
    },

    {
	{
	    "A slimy green monster appears before you and bows.",
	    "A slimy green monster appears before $n and bows.",
	    "Deep in prayer, you levitate.",
	    "Deep in prayer, $n levitates.",
	    "You steal the underwear off every person in the room.",
	    "Your underwear is gone!  $n stole it!",
	    "Crunch, crunch -- you munch a bottle.",
	    "Crunch, crunch -- $n munches a bottle."
	}
    },

    {
	{
	    "You turn everybody into a little pink elephant.",
	    "You are turned into a little pink elephant by $n.",
	    "An angel consults you.",
	    "An angel consults $n.",
	    "The dice roll ... and you win again.",
	    "The dice roll ... and $n wins again.",
	    "... 98, 99, 100 ... you do pushups.",
	    "... 98, 99, 100 ... $n does pushups."
	}
    },

    {
	{
	    "A small ball of light dances on your fingertips.",
	    "A small ball of light dances on $n's fingertips.",
	    "Your body glows with an unearthly light.",
	    "$n's body glows with an unearthly light.",
	    "You count the money in everyone's pockets.",
	    "Check your money, $n is counting it.",
	    "Arnold Schwarzenegger admires your physique.",
	    "Arnold Schwarzenegger admires $n's physique."
	}
    },

    {
	{
	    "Smoke and fumes leak from your nostrils.",
	    "Smoke and fumes leak from $n's nostrils.",
	    "A spot light hits you.",
	    "A spot light hits $n.",
	    "You balance a pocket knife on your tongue.",
	    "$n balances a pocket knife on your tongue.",
	    "Watch your feet, you are juggling granite boulders.",
	    "Watch your feet, $n is juggling granite boulders."
	}
    },

    {
	{
	    "The light flickers as you rap in magical languages.",
	    "The light flickers as $n raps in magical languages.",
	    "Everyone levitates as you pray.",
	    "You levitate as $n prays.",
	    "You produce a coin from everyone's ear.",
	    "$n produces a coin from your ear.",
	    "Oomph!  You squeeze water out of a granite boulder.",
	    "Oomph!  $n squeezes water out of a granite boulder."
	}
    },

    {
	{
	    "Your head disappears.",
	    "$n's head disappears.",
	    "A cool breeze refreshes you.",
	    "A cool breeze refreshes $n.",
	    "You step behind your shadow.",
	    "$n steps behind $s shadow.",
	    "You pick your teeth with a spear.",
	    "$n picks $s teeth with a spear."
	}
    },

    {
	{
	    "A fire elemental singes your hair.",
	    "A fire elemental singes $n's hair.",
	    "The sun pierces through the clouds to illuminate you.",
	    "The sun pierces through the clouds to illuminate $n.",
	    "Your eyes dance with greed.",
	    "$n's eyes dance with greed.",
	    "Everyone is swept off their foot by your hug.",
	    "You are swept off your feet by $n's hug."
	}
    },

    {
	{
	    "The sky changes color to match your eyes.",
	    "The sky changes color to match $n's eyes.",
	    "The ocean parts before you.",
	    "The ocean parts before $n.",
	    "You deftly steal everyone's weapon.",
	    "$n deftly steals your weapon.",
	    "Your karate chop splits a tree.",
	    "$n's karate chop splits a tree."
	}
    },

    {
	{
	    "The stones dance to your command.",
	    "The stones dance to $n's command.",
	    "A thunder cloud kneels to you.",
	    "A thunder cloud kneels to $n.",
	    "The Grey Mouser buys you a beer.",
	    "The Grey Mouser buys $n a beer.",
	    "A strap of your armor breaks over your mighty thews.",
	    "A strap of $n's armor breaks over $s mighty thews."
	}
    },

    {
	{
	    "The heavens and grass change colour as you smile.",
	    "The heavens and grass change colour as $n smiles.",
	    "The Burning Man speaks to you.",
	    "The Burning Man speaks to $n.",
	    "Everyone's pocket explodes with your fireworks.",
	    "Your pocket explodes with $n's fireworks.",
	    "A boulder cracks at your frown.",
	    "A boulder cracks at $n's frown."
	}
    },

    {
	{
	    "Everyone's clothes are transparent, and you are laughing.",
	    "Your clothes are transparent, and $n is laughing.",
	    "An eye in a pyramid winks at you.",
	    "An eye in a pyramid winks at $n.",
	    "Everyone discovers your dagger a centimeter from their eye.",
	    "You discover $n's dagger a centimeter from your eye.",
	    "Mercenaries arrive to do your bidding.",
	    "Mercenaries arrive to do $n's bidding."
	}
    },

    {
	{
	    "A black hole swallows you.",
	    "A black hole swallows $n.",
	    "Valentine Michael Smith offers you a glass of water.",
	    "Valentine Michael Smith offers $n a glass of water.",
	    "Where did you go?",
	    "Where did $n go?",
	    "Four matched Percherons bring in your chariot.",
	    "Four matched Percherons bring in $n's chariot."
	}
    },

    {
	{
	    "The world shimmers in time with your whistling.",
	    "The world shimmers in time with $n's whistling.",
	    "The great gods give you a staff.",
	    "The great gods give $n a staff.",
	    "Click.",
	    "Click.",
	    "Atlas asks you to relieve him.",
	    "Atlas asks $n to relieve him."
	}
    }
};

void do_pose( CHAR_DATA *ch, char *argument )
{
    int level;
    int pose;

    if ( IS_NPC(ch) )
	return;

    level = UMIN( ch->level, sizeof(pose_table) / sizeof(pose_table[0]) - 1 );
    pose  = number_range(0, level);

    act( pose_table[pose].message[2*ch->class+0], ch, NULL, NULL, TO_CHAR );
    act( pose_table[pose].message[2*ch->class+1], ch, NULL, NULL, TO_ROOM );

    return;
}

void do_bug( CHAR_DATA *ch, char *argument )
{
    append_file( ch, BUG_FILE, argument );
    send_to_char( "Bug logged.\n\r", ch );
    return;
}

void do_idea( CHAR_DATA *ch, char *argument )
{
    append_file( ch, IDEA_FILE, argument );
    send_to_char( "Idea logged. This is NOT an identify command.\n\r", ch );
    return;
}

void do_typo( CHAR_DATA *ch, char *argument )
{
    append_file( ch, TYPO_FILE, argument );
    send_to_char( "Typo logged.\n\r", ch );
    return;
}

void do_qui( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to QUIT, you have to spell it out.\n\r", ch );
    return;
}

void do_quit( CHAR_DATA *ch, char *argument )
{
   DESCRIPTOR_DATA *d;
   DESCRIPTOR_DATA *d_next;
   
   char *name;
   char buf[MAX_STRING_LENGTH];
   
   send_to_char( "\n\r", ch );
   
   if ( IS_NPC(ch) )
     return;
   
   if ( ch->position == POS_FIGHTING )
     {
	send_to_char( "No way! You are fighting.\n\r", ch );
	return;
     }
   
   if ( ch->position  < POS_STUNNED  )
     {
	send_to_char( "You're not DEAD yet.\n\r", ch );
	return;
     }
   
   send_to_char("Alas, all good things must come to an end.\n\r",ch);
   act( "$n has left the game.", ch, NULL, NULL, TO_ROOM );
   sprintf( log_buf, "%s has quit.", ch->name );
   log_string( log_buf );
   if(ch->desc != NULL) {
   sprintf( buf, "%s has left the game.", ch->name);
if( !str_cmp(argument, "hidden") && get_trust(ch) > LEVEL_HERO)
	{} else 
if( IS_SET(ch->act, PLR_NO_ANNOUNCE) )
      {} else       {
   do_sendinfo( ch, buf);
	}
   sprintf(buf, "%s@%s[%d] has quit.", ch->name, ch->desc->host, UMIN(100, ch->level));
   send_wiz(URANGE(92, ch->level, 102), buf);
   }
   /*
    * After extract_char the ch is no longer valid!
    */
   save_char_obj( ch );

	/* Free note that might be there somehow */        
     if (ch->pcdata->in_progress)
	free_note (ch->pcdata->in_progress);

   name = strdup(ch->name);
   d = ch->desc;
   extract_char( ch, TRUE );
   if ( d != NULL ) close_socket( d );
   
   for (d = descriptor_list; d!=NULL; d = d_next)
     {
	CHAR_DATA *tch;
	
	d_next=d->next;
	tch = d->original ? d->original : d->character;
	if ((tch) && (!strcmp(tch->name,name)) && (ch != tch))
	  {
	     extract_char(ch,TRUE);
	     close_socket(d);
	  }
     }
	
   return;
}

void do_save( CHAR_DATA *ch, char *argument )
{
   extern bool chaos;
   
   if ( IS_NPC(ch) )
     return;

   if ( chaos )
     {
	send_to_char("Saving is not allowed during `rC`RH`YA`RO`rS.\n\r`w",ch);
	return;
     }
   
   save_char_obj( ch );
   send_to_char("Saving.\n\r", ch );
   return;
}

void do_follow( CHAR_DATA *ch, char *argument )
{
/* RT changed to allow unlimited following and follow the NOFOLLOW rules */
   char arg[MAX_INPUT_LENGTH]; 
   
   CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Follow whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master != NULL )
    {
	act( "But you'd rather follow $N!", ch, NULL, ch->master, TO_CHAR );
	return;
    }

    if ( victim == ch )
    {
	if ( ch->master == NULL )
	{
	    send_to_char( "You already follow yourself.\n\r", ch );
	    return;
	}
	stop_follower(ch);
	return;
    }

    if (!IS_NPC(victim) && IS_SET(victim->act,PLR_NOFOLLOW) && !IS_HERO(ch))
    {
	act("$N doesn't seem to want any followers.\n\r",
	     ch,NULL,victim, TO_CHAR);
	return;
    }

    REMOVE_BIT(ch->act,PLR_NOFOLLOW);
    
    if ( ch->master != NULL )
	stop_follower( ch );

    add_follower( ch, victim );
    return;
}

void add_follower( CHAR_DATA *ch, CHAR_DATA *master )
{
    if ( ch->master != NULL )
    {
	bug( "Add_follower: non-null master.", 0 );
	return;
    }

    ch->master        = master;
    ch->leader        = NULL;

    if ( can_see( master, ch ) )
	act( "$n now follows you.", ch, NULL, master, TO_VICT );

    act( "You now follow $N.",  ch, NULL, master, TO_CHAR );

    return;
}

void stop_follower( CHAR_DATA *ch )
{
    if ( ch->master == NULL )
    {
	bug( "Stop_follower: null master.", 0 );
	return;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM) )
    {
	REMOVE_BIT( ch->affected_by, AFF_CHARM );
	affect_strip( ch, gsn_charm_person );
    }

    if ( can_see( ch->master, ch ) && ch->in_room != NULL)
    {
	act( "$n stops following you.",     ch, NULL, ch->master, TO_VICT    );
	act( "You stop following $N.",      ch, NULL, ch->master, TO_CHAR    );
    }
    if (ch->master->pet == ch)
	ch->master->pet = NULL;

    ch->master = NULL;
    ch->leader = NULL;
    return;
}

/* nukes charmed monsters and pets */
void nuke_pets( CHAR_DATA *ch )
{    
    CHAR_DATA *pet;

    if ((pet = ch->pet) != NULL)
    {
	stop_follower(pet);
	if (pet->in_room != NULL)
	    act("$N slowly fades away.",ch,NULL,pet,TO_NOTVICT);
	extract_char(pet,TRUE);
    }
    ch->pet = NULL;

    return;
}

void die_follower( CHAR_DATA *ch )
{
    CHAR_DATA *fch;

    if ( ch->master != NULL )
    {
	if (ch->master->pet == ch)
	    ch->master->pet = NULL;
	stop_follower( ch );
    }

    ch->leader = NULL;

    for ( fch = char_list; fch != NULL; fch = fch->next )
    {
	if ( fch->master == ch )
	    stop_follower( fch );
	if ( fch->leader == ch )
	    fch->leader = fch;
    }

    return;
}

/* new order function by Ron Cole(Rindar) clogar@concentric.net */
void do_order( CHAR_DATA *ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   char arg[MAX_INPUT_LENGTH];
   
   char arg2[MAX_INPUT_LENGTH];
   char cmd_vi[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   CHAR_DATA *och;
   CHAR_DATA *och_next;
   bool found;
   bool fAll;
   bool fUnto;

   argument = one_argument( argument, arg );
   one_argument(argument,arg2);

   if ( arg[0] == '\0' || argument[0] == '\0' )
   {
      send_to_char( "Order whom to do what?\n\r", ch );
      return;
   }

   if ( IS_AFFECTED( ch, AFF_CHARM ) )
   {
      send_to_char( "You feel like taking, not giving, orders.\n\r", ch );
      return;
   }

   if ( !str_cmp( arg, "all" ) )
   {
      fAll   = TRUE;
      victim = NULL;
   }
   else
   {
      fAll   = FALSE;
      if ( ( victim = get_char_room( ch, arg ) ) == NULL )
      {
         send_to_char( "They aren't here.\n\r", ch );
         return;
      }

      if ( victim == ch )
      {
         send_to_char( "Aye aye, right away!\n\r", ch );
         return;
      }

      if ( !IS_AFFECTED(victim, AFF_CHARM) || victim->master != ch )
      {
         send_to_char( "Do it yourself!\n\r", ch );
         return;
      }
   }

   fUnto = FALSE;
   found = FALSE;
   for ( och = ch->in_room->people; och != NULL; och = och_next )
   {
      och_next = och->next_in_room;

      if ( IS_AFFECTED(och, AFF_CHARM)
           && och->master == ch
           && ( fAll || och == victim ) )
      {
         found = TRUE;
         strcpy( cmd_vi, argument );
/*          
chk_command( och, cmd_vi );
*/
         if (cmd_vi[0] == '\0')
            act( "$N is unable to comply with your order.", ch, NULL, och, TO_CHAR );

         else if ( (!strcmp( cmd_vi, "delete"   )) ||
                   (!strcmp( cmd_vi, "quit"     )) ||
                   (!strcmp( cmd_vi, "password" )) ||
                   (!strcmp( cmd_vi, "pk"       )) ||
                   (!strcmp( cmd_vi, "quiet"    ))  )
         {
            act( "I don't think $N appreciated that.", ch, NULL, och, TO_CHAR );
            sprintf( buf, "WATCH PLAYER!: %s attempted to make %s %s!",
                                          ch->name, och->name, cmd_vi );
            log_string( buf );
	send_wiz( 94, buf);
            stop_follower( och );
            if (!IS_NPC( och ))
               fUnto = TRUE;
         }
         else if (( (!strcmp( cmd_vi, "advance" )) && (IS_NPC( och )) ) ||
                  (!strncmp( cmd_vi, "mp", 2 )) )
         {
            sprintf( buf, "WATCH PLAYER!: %s attempted to make %s %s!",                                  ch->name, och->name, cmd_vi );
            log_string( buf );
	send_wiz(94, buf);
            act( "$N is unable to comply with your order.", ch, NULL, och, TO_CHAR );

         }
         else
         {
            if (!IS_NPC( och ))
            {
               sprintf( buf, "%s ordered %s to '%s'", ch->name, och->name, argument );
               log_string( buf );
            }
            sprintf( buf, "$n orders you to '%s'.", argument );
            act( buf, ch, NULL, och, TO_VICT );
            act( "$N does $S best to comply to your order.", ch, NULL, och, TO_CHAR );
            interpret( och, argument );
         }
      }
   }

   if ( !found )
      send_to_char( "You have no followers here.\n\r", ch );

   if (fUnto)
   {
      /* If I feel especially mean spirited one day, I'll check to see
       * if they tried to changed another's password and, if so, change
       * their's to some random string.
       */
      send_to_char( "The warders of this land have a perverse delight in poetic justice./n/r", ch );
      interpret( ch, argument );
   }

   tail_chain( );
   return;
}

void do_group( CHAR_DATA *ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   char arg[MAX_INPUT_LENGTH];
   
   CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	CHAR_DATA *gch;
	CHAR_DATA *leader;

	leader = (ch->leader != NULL) ? ch->leader : ch;
	sprintf( buf, "`K[`W%s's group`K]`w\n\r", PERS(leader, ch) );
	send_to_char( buf, ch );

	for ( gch = char_list; gch != NULL; gch = gch->next )
	{
	    if ( is_same_group( gch, ch ) )
	    {
		sprintf( buf,
		"`K[`W%3d `G%s`K] `w%-16s `W%4d`K/`W%4d hp %4d`K/`W%4d mana %4d`K/`W%4d mv %5ld xp\n\r",
		    gch->level,
		    IS_NPC(gch) ? "Mob" : class_table[gch->class].who_name,
		    capitalize( PERS(gch, ch) ),
		    gch->hit,   gch->max_hit,
		    gch->mana,  gch->max_mana,
		    gch->move,  gch->max_move,
		    gch->exp    );
		send_to_char( buf, ch );
	    }
	}
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( ch->master != NULL || ( ch->leader != NULL && ch->leader != ch ) )
    {
	send_to_char( "But you are following someone else!\n\r", ch );
	return;
    }

    if ( victim->master != ch && ch != victim )
    {
	act( "$N isn't following you.", ch, NULL, victim, TO_CHAR );
	return;
    }
    
    if (IS_AFFECTED(victim,AFF_CHARM))
    {
	send_to_char("You can't remove charmed mobs from your group.\n\r",ch);
	return;
    }
    
    if (IS_AFFECTED(ch,AFF_CHARM))
    {
	act("You like your master too much to leave $m!",ch,NULL,victim,TO_VICT);
	return;
    }

    if ( is_same_group( victim, ch ) && ch != victim )
    {
	victim->leader = NULL;
	act( "$n removes $N from $s group.",   ch, NULL, victim, TO_NOTVICT );
	act( "$n removes you from $s group.",  ch, NULL, victim, TO_VICT    );
	act( "You remove $N from your group.", ch, NULL, victim, TO_CHAR    );
	return;
    }

    victim->leader = ch;
    act( "$N joins $n's group.", ch, NULL, victim, TO_NOTVICT );
    act( "You join $n's group.", ch, NULL, victim, TO_VICT    );
    act( "$N joins your group.", ch, NULL, victim, TO_CHAR    );
    return;
}

/*
 * 'Split' originally by Gnort, God of Chaos.
 */
void do_split( CHAR_DATA *ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   
   CHAR_DATA *gch;
    int members;
    int amount;
    int share;
    int extra;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Split how much?\n\r", ch );
	return;
    }
    
    amount = atoi( arg );

    if ( amount < 0 )
    {
	send_to_char( "Your group wouldn't like that.\n\r", ch );
	return;
    }

    if ( amount == 0 )
    {
	send_to_char( "You hand out zero coins, but no one notices.\n\r", ch );
	return;
    }

    if ( ch->gold < amount )
    {
	send_to_char( "You don't have that much gold.\n\r", ch );
	return;
    }
  
    members = 0;
    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ( is_same_group( gch, ch ) && !IS_AFFECTED(gch,AFF_CHARM))
	    members++;
    }

    if ( members < 2 )
    {
	send_to_char( "Just keep it all.\n\r", ch );
	return;
    }
	    
    share = amount / members;
    extra = amount % members;

    if ( share == 0 )
    {
	send_to_char( "Don't even bother, cheapskate.\n\r", ch );
	return;
    }

    ch->gold -= amount;
    ch->gold += share + extra;

    sprintf( buf,
	"You split %d gold coins.  Your share is %d gold coins.\n\r",
	amount, share + extra );
    send_to_char( buf, ch );

    sprintf( buf, "$n splits %d gold coins.  Your share is %d gold coins.",
	amount, share );

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ( gch != ch && is_same_group( gch, ch ) && !IS_AFFECTED(gch,AFF_CHARM))
	{
	    act( buf, ch, NULL, gch, TO_VICT );
	    gch->gold += share;
	}
    }

    return;
}

void do_gtell( CHAR_DATA *ch, char *argument )
{
   char buf[MAX_STRING_LENGTH]; 
   
   CHAR_DATA *gch;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Tell your group what?\n\r", ch );
	return;
    }

    if ( IS_SET( ch->comm, COMM_NOTELL ) )
    {
	send_to_char( "Your message didn't get through!\n\r", ch );
	return;
    }

    /*
     * Note use of send_to_char, so gtell works on sleepers.
     */
    sprintf( buf, "`C%s tells the group '%s`C'.\n\r`w", ch->name, argument );
/*    
add2tell(ch, ch, buf);
*/
    for ( gch = char_list; gch != NULL; gch = gch->next )
    {
	if ( is_same_group( gch, ch ) ) {
	    send_to_char( buf, gch );
	}
    }

    return;
}

/*
 * It is very important that this be an equivalence relation:
 * (1) A ~ A
 * (2) if A ~ B then B ~ A
 * (3) if A ~ B  and B ~ C, then A ~ C
 */
bool is_same_group( CHAR_DATA *ach, CHAR_DATA *bch )
{
    if ( ach->leader != NULL ) ach = ach->leader;
    if ( bch->leader != NULL ) bch = bch->leader;
    return ach == bch;
}
