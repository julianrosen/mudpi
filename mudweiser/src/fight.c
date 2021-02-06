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

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "merc.h"

#define MAX_DAMAGE_MESSAGE 32

/* command procedures needed */
DECLARE_DO_FUN(do_emote		);
DECLARE_DO_FUN(do_berserk	);
DECLARE_DO_FUN(do_bash		);
DECLARE_DO_FUN(do_trip		);
DECLARE_DO_FUN(do_dirt		);
DECLARE_DO_FUN(do_flee		);
DECLARE_DO_FUN(do_kick		);
DECLARE_DO_FUN(do_disarm	);
DECLARE_DO_FUN(do_get		);
DECLARE_DO_FUN(do_recall	);
DECLARE_DO_FUN(do_yell		);
DECLARE_DO_FUN(do_sacrifice	);


/*
 * Local functions.
 */
void	check_assist args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	check_dodge	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	check_killer args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	check_parry	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool  check_block args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	dam_message	args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam,
			    int dt, bool immune ) );
void	death_cry	args( ( CHAR_DATA *ch ) );
void	group_gain	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
int	xp_compute	args( ( CHAR_DATA *gch, CHAR_DATA *victim, 
			    int total_levels, int members ) );
int	hit_xp_compute	args( ( CHAR_DATA *gch, CHAR_DATA *victim, 
			    int total_levels, int members, int dam ) );
bool	is_safe		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	make_corpse	args( ( CHAR_DATA *ch ) );
void	make_pk_corpse	args( ( CHAR_DATA *ch ) );
void	one_hit		args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
void	second_one_hit  args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
bool  vorpal_kill args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam, 
				int dt, int dam_type ) );
void    mob_hit		args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
void	chaos_kill	args( ( CHAR_DATA *victim ) );
void	pk_kill		args( ( CHAR_DATA *victim ) );
void	raw_kill	args( ( CHAR_DATA *victim ) );
void	set_fighting	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	disarm		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void    chaos_log	args( ( CHAR_DATA *ch, char *argument ) );


/*
 * Control the fights going on.
 * Called periodically by update_handler.
 */
void violence_update( void )
{
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    CHAR_DATA *victim;

    for ( ch = char_list; ch != NULL; ch = ch->next )
    {
	ch_next	= ch->next;

	if ( ( victim = ch->fighting ) == NULL || ch->in_room == NULL )
	    continue;

	if ( IS_AWAKE(ch) && ch->in_room == victim->in_room )
	    multi_hit( ch, victim, TYPE_UNDEFINED );
	else {
	    stop_fighting( ch, FALSE );
	}

	if ( ( victim = ch->fighting ) == NULL )
	    continue;

           rprog_rfight_trigger( ch );

      if ( ( victim = ch->fighting ) == NULL )
	    continue;

           mprog_hitprcnt_trigger( ch, victim );

	if ( ( victim = ch->fighting ) == NULL )
	    continue;

           mprog_fight_trigger( ch, victim );

	/*
	 * Fun for the whole family!
	 */
	check_assist(ch,victim);
    }

    return;
}

/* for auto assisting */
void check_assist(CHAR_DATA *ch,CHAR_DATA *victim)
{
    CHAR_DATA *rch, *rch_next;

    for (rch = ch->in_room->people; rch != NULL; rch = rch_next)
    {
	rch_next = rch->next_in_room;
	
	if (IS_AWAKE(rch) && rch->fighting == NULL)
	{

	    /* quick check for ASSIST_PLAYER */
	    if (!IS_NPC(ch) && IS_NPC(rch) 
	    && IS_SET(rch->off_flags,ASSIST_PLAYERS)
	    &&  rch->level + 6 > victim->level
	    && !IS_SET(ch->act, PLR_KILLER))
	    {
		do_emote(rch,"screams and attacks!");
		multi_hit(rch,victim,TYPE_UNDEFINED);
		continue;
	    }

	    /* PCs next */
	    if ((!IS_NPC(ch) && IS_NPC(victim)) 
		|| (IS_AFFECTED(ch,AFF_CHARM)) )
	    {
		if ( ( (!IS_NPC(rch) && IS_SET(rch->act,PLR_AUTOASSIST))
		||     IS_AFFECTED(rch,AFF_CHARM)) 
		&&   is_same_group(ch,rch) )
		    multi_hit (rch,victim,TYPE_UNDEFINED);
		
		continue;
	    }
  	
	    /* now check the NPC cases */
	    
 	    if (IS_NPC(ch) && !IS_AFFECTED(ch,AFF_CHARM))
	
	    {
		if ( (IS_NPC(rch) && IS_SET(rch->off_flags,ASSIST_ALL))

		||   (IS_NPC(rch) && rch->race == ch->race 
		   && IS_SET(rch->off_flags,ASSIST_RACE))

		||   (IS_NPC(rch) && IS_SET(rch->off_flags,ASSIST_ALIGN)
		   &&   ((IS_GOOD(rch)    && IS_GOOD(ch))
		     ||  (IS_EVIL(rch)    && IS_EVIL(ch))
		     ||  (IS_NEUTRAL(rch) && IS_NEUTRAL(ch)))) 

		||   (rch->pIndexData == ch->pIndexData 
		   && IS_SET(rch->off_flags,ASSIST_VNUM)))

	   	{
		    CHAR_DATA *vch;
		    CHAR_DATA *target;
		    int number;

		    if (number_bits(1) == 0)
			continue;
		
		    target = NULL;
		    number = 0;
		    for (vch = ch->in_room->people; vch; vch = vch->next)
		    {
			if (can_see(rch,vch)
			&&  is_same_group(vch,victim)
			&&  number_range(0,number) == 0)
			{
			    target = vch;
			    number++;
			}
		    }

		    if (target != NULL)
		    {
			do_emote(rch,"screams and attacks!");
			multi_hit(rch,target,TYPE_UNDEFINED);
		    }
		}	
	    }
	}
    }
}


/*
 * Do one group of attacks.
 */
void multi_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt )
{
    OBJ_DATA *second_wield;
    int     chance;

    /* decrement the wait */
    if (ch->desc == NULL)
	ch->wait = UMAX(0,ch->wait - PULSE_VIOLENCE);


    /* no attacks for stunnies -- just a check */
    if (ch->position < POS_RESTING)
	return;

    if (IS_NPC(ch))
    {
	mob_hit(ch,victim,dt);
	return;
    }

    one_hit( ch, victim, dt );
    if ( (second_wield = get_eq_char( ch, WEAR_SECOND_WIELD ) ) != NULL )
        second_one_hit( ch, victim, dt);

    if (ch->fighting != victim)
	return;

    if (IS_AFFECTED(ch,AFF_HASTE))
	one_hit(ch,victim,dt);

    if ( ch->fighting != victim || dt == gsn_backstab )
	return;

    chance = get_skill(ch,gsn_second_attack);
    if ( number_percent( ) < chance )
    {
	one_hit( ch, victim, dt );
	check_improve(ch,gsn_second_attack,TRUE,5);
	if ( ch->fighting != victim )
	    return;
    }

    chance = get_skill(ch,gsn_third_attack)/2;
    if ( number_percent( ) < chance )
    {
	one_hit( ch, victim, dt );
	check_improve(ch,gsn_third_attack,TRUE,6);
	if ( ch->fighting != victim )
	    return;
    }

    return;
}

/* procedure for all mobile attacks */
void mob_hit (CHAR_DATA *ch, CHAR_DATA *victim, int dt)
{
    int chance,number;
    CHAR_DATA *vch, *vch_next;

    one_hit(ch,victim,dt);

    if (ch->fighting != victim)
	return;

    /* Area attack -- BALLS nasty! */
 
    if (IS_SET(ch->off_flags,OFF_AREA_ATTACK))
    {
	for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
	{
	    vch_next = vch->next;
	    if ((vch != victim && vch->fighting == ch))
		one_hit(ch,vch,dt);
	}
    }

    if (IS_AFFECTED(ch,AFF_HASTE) || IS_SET(ch->off_flags,OFF_FAST))
	one_hit(ch,victim,dt);

    if (ch->fighting != victim || dt == gsn_backstab)
	return;

    chance = get_skill(ch,gsn_second_attack);
    if (number_percent() < chance)
    {
	one_hit(ch,victim,dt);
	if (ch->fighting != victim)
	    return;
    }

    chance = get_skill(ch,gsn_third_attack)/2;
    if (number_percent() < chance)
    {
	one_hit(ch,victim,dt);
	if (ch->fighting != victim)
	    return;
    } 

    /* oh boy!  Fun stuff! */

    if (ch->wait > 0)
	return;

    number = number_range(0,2);

    /*if (number == 1 && IS_SET(ch->act,ACT_MAGE))
	{ mob_cast_mage(ch,victim); return; };*/

    /*if (number == 2 && IS_SET(ch->act,ACT_CLERIC))
	{ mob_cast_cleric(ch,victim); return; };*/

    /* now for the skills */

    number = number_range(0,7);

    switch(number) 
    {
    case (0) :
	if (IS_SET(ch->off_flags,OFF_BASH))
	    do_bash(ch,"");
	break;

    case (1) :
	if (IS_SET(ch->off_flags,OFF_BERSERK) && !IS_AFFECTED(ch,AFF_BERSERK))
	    do_berserk(ch,"");
	break;


    case (2) :
	if (IS_SET(ch->off_flags,OFF_DISARM) 
	|| (get_weapon_sn(ch) != gsn_hand_to_hand 
	&& (IS_SET(ch->act,ACT_WARRIOR)
   	||  IS_SET(ch->act,ACT_THIEF))))
	    do_disarm(ch,"");
	break;

    case (3) :
	if (IS_SET(ch->off_flags,OFF_KICK))
	    do_kick(ch,"");
	break;

    case (4) :
	if (IS_SET(ch->off_flags,OFF_KICK_DIRT))
	    do_dirt(ch,"");
	break;

    /*case (5) :
	if (IS_SET(ch->off_flags,OFF_TAIL))
	    do_tail(ch,"");
	break; */

    case (6) :
	if (IS_SET(ch->off_flags,OFF_TRIP))
	    do_trip(ch,"");
	break;

    /*case (7) :
	if (IS_SET(ch->off_flags,OFF_CRUSH))
	    do_crush(ch,"");
	break;*/
    }
}
	

/*
 * Hit one guy once.
 */
void one_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt )
{
    OBJ_DATA *wield;
    int victim_ac;
    int thac0;
    int thac0_00;
    int thac0_32;
    int dam;
    int diceroll;
    int sn,skill;
    int dam_type;

    sn = -1;


    /* just in case */
    if (victim == ch || ch == NULL || victim == NULL)
	return;

    /*
     * Can't beat a dead char!
     * Guard against weird room-leavings.
     */
    if ( victim->position == POS_DEAD || ch->in_room != victim->in_room )
	return;

    /*
     * Figure out the type of damage message.
     */
    wield = get_eq_char( ch, WEAR_WIELD );

    if ( dt == TYPE_UNDEFINED )
    {
	dt = TYPE_HIT;
	if ( wield != NULL && wield->item_type == ITEM_WEAPON )
	    dt += wield->value[3];
	else 
	    dt += ch->dam_type;
    }

    if (dt < TYPE_HIT)
    	if (wield != NULL)
    	    dam_type = attack_table[wield->value[3]].damage;
    	else
    	    dam_type = attack_table[ch->dam_type].damage;
    else
    	dam_type = attack_table[dt - TYPE_HIT].damage;

    if (dam_type == -1)
	dam_type = DAM_BASH;

    /* get the weapon skill */
    sn = get_weapon_sn(ch);
    skill = 20 + get_weapon_skill(ch,sn);

    /*
     * Calculate to-hit-armor-class-0 versus armor.
     */
    if ( IS_NPC(ch) )
    {
	thac0_00 = 20;
	thac0_32 = -4;   /* as good as a thief */ 
	if (IS_SET(ch->act,ACT_WARRIOR))
	    thac0_32 = -10;
	else if (IS_SET(ch->act,ACT_THIEF))
	    thac0_32 = -4;
	else if (IS_SET(ch->act,ACT_CLERIC))
	    thac0_32 = 2;
	else if (IS_SET(ch->act,ACT_MAGE))
	    thac0_32 = 6;
    }
    else
    {
	thac0_00 = class_table[ch->class].thac0_00;
	thac0_32 = class_table[ch->class].thac0_32;
    }

    thac0  = interpolate( ch->level, thac0_00, thac0_32 );

    thac0 -= GET_HITROLL(ch) * skill/100;
    thac0 += 5 * (100 - skill) / 100;

    if (dt == gsn_backstab)
	thac0 -= 10 * (100 - get_skill(ch,gsn_backstab));

	 if (dt == gsn_circle)
		  thac0 -= 10 * (100 - get_skill(ch,gsn_circle));


    switch(dam_type)
    {
	case(DAM_PIERCE):victim_ac = GET_AC(victim,AC_PIERCE)/10;	break;
	case(DAM_BASH):	 victim_ac = GET_AC(victim,AC_BASH)/10;		break;
	case(DAM_SLASH): victim_ac = GET_AC(victim,AC_SLASH)/10;	break;
	default:	 victim_ac = GET_AC(victim,AC_EXOTIC)/10;	break;
    }; 
	
    if (victim_ac < -15)
	victim_ac = (victim_ac + 15) / 5 - 15;
     
    if ( !can_see( ch, victim ) )
	victim_ac -= 4;

    if ( victim->position < POS_FIGHTING)
	victim_ac += 4;
 
    if (victim->position < POS_RESTING)
	victim_ac += 6;

    /*
     * The moment of excitement!
     */
    while ( ( diceroll = number_bits( 5 ) ) >= 20 )
	;

    if ( diceroll == 0
    || ( diceroll != 19 && diceroll < thac0 - victim_ac ) )
    {
	/* Miss. */
	damage( ch, victim, 0, dt, dam_type );
	tail_chain( );
	return;
    }

    /*
     * Hit.
     * Calc damage.
     */
    if ( IS_NPC(ch) && ( wield == NULL ) )
         dam = dice(ch->damage[DICE_NUMBER],ch->damage[DICE_TYPE]);
	
    else
    {
	if (sn != -1)
	    check_improve(ch,sn,TRUE,5);
	if ( wield != NULL )
	{
	    dam = dice(wield->value[1],wield->value[2]) * skill/100;

	    if (get_eq_char(ch,WEAR_SHIELD) == NULL)  /* no shield = more */
		dam = dam * 21/20;
	}
	else
	    dam = number_range( 1 + 4 * skill/100, 2 * ch->level/3 * skill/100);
    }

    /*
     * Bonuses.
     */
    if ( get_skill(ch,gsn_enhanced_damage) > 0 )
    {
        diceroll = number_percent();
        if (diceroll <= get_skill(ch,gsn_enhanced_damage))
        {
            check_improve(ch,gsn_enhanced_damage,TRUE,6);
            dam += dam * diceroll/100;
        }
    }

    if ( !IS_AWAKE(victim) )
	dam *= 2;
     else if (victim->position < POS_FIGHTING)
	dam = dam * 3 / 2;

	 if ( dt == gsn_backstab && wield != NULL)
    	if ( wield->value[0] != 2 )
	    dam *= 2 + ch->level / 10; 
	else 
	    dam *= 2 + ch->level / 8;

	if ( dt == gsn_circle && wield != NULL)
		if ( wield->value[0] != 2 )
		 dam *= 2+ (ch->level /15);
		else
		 dam *= 2+ (ch->level / 12);

	
    dam += GET_DAMROLL(ch) * UMIN(100,skill) /100;

    if ( dam <= 0 )
	dam = 1;

    damage( ch, victim, dam, dt, dam_type );
 /* but do we have a funky weapon? */
    
    if (wield != NULL && ch->fighting == victim)
    {


      /*  
	if (IS_WEAPON_STAT(wield,WEAPON_POISON))
        {
  
            int level;
            AFFECT_DATA *poison, af;

            if ((poison = affect_find(wield->affected,gsn_poison)) == NULL)
                level = wield->level;
            else
                level = poison->level;
        
            if (!saves_spell(level / 2,victim)) 
            {
                send_to_char("You feel poison coursing through your veins.",
                    victim);
                act("$n is poisoned by the venom on $p.",
                    victim,wield,NULL,TO_ROOM);

                
                af.type      = gsn_poison;
                af.level     = level * 3/4;
                af.duration  = level / 2;
                af.location  = APPLY_STR;
                af.modifier  = -1;
                af.bitvector = AFF_POISON;
                affect_join( victim, &af );
            }

            
            if (poison != NULL)
            {
                poison->level = UMAX(0,poison->level - 2);
                poison->duration = UMAX(0,poison->duration - 1);
        
                if (poison->level == 0 || poison->duration == 0)
                    act("The poison on $p has worn off.",ch,wield,NULL,TO_CHAR);
            }
        }
	*/

        if (IS_WEAPON_STAT(wield,WEAPON_VAMPIRIC))
        {
            dam = number_range(1, wield->level / 5 + 1);
            act("$p draws life from $n.",victim,wield,NULL,TO_ROOM);
            act("You feel $p drawing your life away.",
                victim,wield,NULL,TO_CHAR);
            new_damage(ch,victim,dam,0,DAM_NEGATIVE,FALSE);
            ch->alignment = UMAX(-1000,ch->alignment - 1);
            ch->hit += dam/2;
        }

        if (IS_WEAPON_STAT(wield,WEAPON_FLAMING))
        {
            dam = number_range(1,wield->level / 4 + 1);
            act("$n is burned by $p.",victim,wield,NULL,TO_ROOM);
            act("$p sears your flesh.",victim,wield,NULL,TO_CHAR);
            new_damage(ch,victim,dam,0,DAM_FIRE,FALSE);
        }

        if (IS_WEAPON_STAT(wield,WEAPON_FROST))
        {
            dam = number_range(1,wield->level / 6 + 2);
            act("$p freezes $n.",victim,wield,NULL,TO_ROOM);
            act("The cold touch of $p surrounds you with ice.",
                victim,wield,NULL,TO_CHAR);
            new_damage(ch,victim,dam,0,DAM_COLD,FALSE);
        }

	if (IS_WEAPON_STAT(wield,WEAPON_VORPAL) 
		&& number_range(1, 600 + MAX_LEVEL - (2 * ch->level)) == 1 
		&& IS_SET(victim->parts,PART_HEAD)
		&& check_immune(victim,dam_type) != IS_IMMUNE
		&& !IS_IMMORTAL(victim)
		&& ch != victim
		&& !is_safe(ch,victim)
		&& victim->hit < (victim->max_hit / 3))
	{
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *obj;
	char *name;
	int parts_buf;

	/* used a modified version of death_cry to make a severed head */

	name		= IS_NPC(victim) ? victim->short_descr : victim->name;
	obj		= create_object( get_obj_index( OBJ_VNUM_SEVERED_HEAD ), 0 );
	obj->timer	= number_range( 20, 30 );

	sprintf( buf, obj->short_descr, name );
	free_string( obj->short_descr );
	obj->short_descr = str_dup( buf );

	sprintf( buf, obj->description, name );
	free_string( obj->description );
	obj->description = str_dup( buf );

	if (obj->item_type == ITEM_FOOD)
	{
	    if (IS_SET(victim->form,FORM_POISON))
		obj->value[3] = 1;
	    else if (!IS_SET(victim->form,FORM_EDIBLE))
		obj->item_type = ITEM_TRASH;
	}

	obj_to_room( obj, ch->in_room );
	act("$n's head is severed from $s body by $p!",victim,wield,NULL,TO_ROOM);
	act("Your last feeling before you die is $p's weapon severing your head!",victim,ch,NULL,TO_CHAR);

	/* parts_buf is used to remove all parts of the body so the death_cry 
	   function does not create any other body parts */

	parts_buf = victim->parts;
	victim->parts = 0;
	vorpal_kill(ch,victim,dam,dt,dam_type);
	victim->parts = parts_buf;
	}


/*
        if (IS_WEAPON_STAT(wield,WEAPON_SHOCKING))
        {
            dam = number_range(1,wield->level/5 + 2);
            act("$n is struck by lightning from $p.",victim,wield,NULL,TO_ROOM);
            act("You are shocked by $p.",victim,wield,NULL,TO_CHAR);
            damage(ch,victim,dam,0,DAM_LIGHTNING);
        }
*/
    }
    tail_chain( );
    return;
}

void second_one_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt )
{
    OBJ_DATA *wield;
    int victim_ac;
    int thac0;
    int thac0_00;
    int thac0_32;
    int dam;
    int diceroll;
    int sn,skill;
    int dam_type;

    sn = -1;


    /* just in case */
    if (victim == ch || ch == NULL || victim == NULL)
	return;

    /*
     * Can't beat a dead char!
     * Guard against weird room-leavings.
     */
    if ( victim->position == POS_DEAD || ch->in_room != victim->in_room )
	return;

    /*
     * Figure out the type of damage message.
     */
    wield = get_eq_char( ch, WEAR_SECOND_WIELD );

    if ( dt == TYPE_UNDEFINED )
    {
	dt = TYPE_HIT;
	if ( wield != NULL && wield->item_type == ITEM_WEAPON )
	    dt += wield->value[3];
	else 
	    dt += ch->dam_type;
    }

    if (dt < TYPE_HIT)
    	if (wield != NULL)
    	    dam_type = attack_table[wield->value[3]].damage;
    	else
    	    dam_type = attack_table[ch->dam_type].damage;
    else
    	dam_type = attack_table[dt - TYPE_HIT].damage;

    if (dam_type == -1)
	dam_type = DAM_BASH;

    /* get the weapon skill */
    sn = get_second_weapon_sn(ch);
    skill = 20 + get_weapon_skill(ch,sn);

    /*
     * Calculate to-hit-armor-class-0 versus armor.
     */
    if ( IS_NPC(ch) )
    {
	thac0_00 = 20;
	thac0_32 = -4;   /* as good as a thief */ 
	if (IS_SET(ch->act,ACT_WARRIOR))
	    thac0_32 = -10;
	else if (IS_SET(ch->act,ACT_THIEF))
	    thac0_32 = -4;
	else if (IS_SET(ch->act,ACT_CLERIC))
	    thac0_32 = 2;
	else if (IS_SET(ch->act,ACT_MAGE))
	    thac0_32 = 6;
    }
    else
    {
	thac0_00 = class_table[ch->class].thac0_00;
	thac0_32 = class_table[ch->class].thac0_32;
    }

    thac0  = interpolate( ch->level, thac0_00, thac0_32 );

    thac0 -= GET_HITROLL(ch) * skill/100;
    thac0 += 5 * (100 - skill) / 100;

    if (dt == gsn_backstab)
	thac0 -= 10 * (100 - get_skill(ch,gsn_backstab));

    if (dt == gsn_circle)
	thac0 -= 10 * (100 - get_skill(ch,gsn_circle));

    switch(dam_type)
    {
	case(DAM_PIERCE):victim_ac = GET_AC(victim,AC_PIERCE)/10;	break;
	case(DAM_BASH):	 victim_ac = GET_AC(victim,AC_BASH)/10;		break;
	case(DAM_SLASH): victim_ac = GET_AC(victim,AC_SLASH)/10;	break;
	default:	 victim_ac = GET_AC(victim,AC_EXOTIC)/10;	break;
    }; 
	
    if (victim_ac < -15)
	victim_ac = (victim_ac + 15) / 5 - 15;
     
    if ( !can_see( ch, victim ) )
	victim_ac -= 4;

    if ( victim->position < POS_FIGHTING)
	victim_ac += 4;
 
    if (victim->position < POS_RESTING)
	victim_ac += 6;

    /*
     * The moment of excitement!
     */
    while ( ( diceroll = number_bits( 5 ) ) >= 20 )
	;

    if ( diceroll == 0
    || ( diceroll != 19 && diceroll < thac0 - victim_ac ) )
    {
	/* Miss. */
	damage( ch, victim, 0, dt, dam_type );
	tail_chain( );
	return;
    }

    /*
     * Hit.
     * Calc damage.
     */
    if ( IS_NPC(ch) && ( wield == NULL ) )
        dam = dice(ch->damage[DICE_NUMBER],ch->damage[DICE_TYPE]);
	
    else
    {
	if (sn != -1)
	    check_improve(ch,sn,TRUE,5);
	if ( wield != NULL )
	{
	    dam = dice(wield->value[1],wield->value[2]) * skill/100;

	    if (get_eq_char(ch,WEAR_SHIELD) == NULL)  /* no shield = more */
		dam = dam * 21/20;
	}
	else
	    dam = number_range( 1 + 4 * skill/100, 2 * ch->level/3 * skill/100);
    }

    /*
     * Bonuses.
     */
    if ( get_skill(ch,gsn_enhanced_damage) > 0 )
    {
        diceroll = number_percent();
        if (diceroll <= get_skill(ch,gsn_enhanced_damage))
        {
            check_improve(ch,gsn_enhanced_damage,TRUE,6);
            dam += dam * diceroll/100;
        }
    }

    if ( !IS_AWAKE(victim) )
	dam *= 2;
     else if (victim->position < POS_FIGHTING)
	dam = dam * 3 / 2;

    if ( dt == gsn_backstab && wield != NULL) 
    	if ( wield->value[0] != 2 )
	    dam *= 2 + ch->level / 10; 
	else 
	    dam *= 2 + ch->level / 8;

	if ( dt == gsn_circle && wield != NULL)
		if ( wield->value[0] != 2 )
		 dam *= 2+ (ch->level /15);
		else
		 dam *= 2+ (ch->level / 12);

    dam += GET_DAMROLL(ch) * UMIN(100,skill) /100;

    if ( dam <= 0 )
	dam = 1;

    damage( ch, victim, dam, dt, dam_type );
/* but do we have a funky weapon? */
    
    if (wield != NULL && ch->fighting == victim)
    {


      /*  
	if (IS_WEAPON_STAT(wield,WEAPON_POISON))
        {
  
            int level;
            AFFECT_DATA *poison, af;

            if ((poison = affect_find(wield->affected,gsn_poison)) == NULL)
                level = wield->level;
            else
                level = poison->level;
        
            if (!saves_spell(level / 2,victim)) 
            {
                send_to_char("You feel poison coursing through your veins.",
                    victim);
                act("$n is poisoned by the venom on $p.",
                    victim,wield,NULL,TO_ROOM);

                
                af.type      = gsn_poison;
                af.level     = level * 3/4;
                af.duration  = level / 2;
                af.location  = APPLY_STR;
                af.modifier  = -1;
                af.bitvector = AFF_POISON;
                affect_join( victim, &af );
            }

            
            if (poison != NULL)
            {
                poison->level = UMAX(0,poison->level - 2);
                poison->duration = UMAX(0,poison->duration - 1);
        
                if (poison->level == 0 || poison->duration == 0)
                    act("The poison on $p has worn off.",ch,wield,NULL,TO_CHAR);
            }
        }
	*/

        if (IS_WEAPON_STAT(wield,WEAPON_VAMPIRIC))
        {
            dam = number_range(1, wield->level / 5 + 1);
            act("$p draws life from $n.",victim,wield,NULL,TO_ROOM);
            act("You feel $p drawing your life away.",
                victim,wield,NULL,TO_CHAR);
            new_damage(ch,victim,dam,0,DAM_NEGATIVE,FALSE);
            ch->alignment = UMAX(-1000,ch->alignment - 1);
            ch->hit += dam/2;
        }

        if (IS_WEAPON_STAT(wield,WEAPON_FLAMING))
        {
            dam = number_range(1,wield->level / 4 + 1);
            act("$n is burned by $p.",victim,wield,NULL,TO_ROOM);
            act("$p sears your flesh.",victim,wield,NULL,TO_CHAR);
            new_damage(ch,victim,dam,0,DAM_FIRE,FALSE);
        }

        if (IS_WEAPON_STAT(wield,WEAPON_FROST))
        {
            dam = number_range(1,wield->level / 6 + 2);
            act("$p freezes $n.",victim,wield,NULL,TO_ROOM);
            act("The cold touch of $p surrounds you with ice.",
                victim,wield,NULL,TO_CHAR);
            new_damage(ch,victim,dam,0,DAM_COLD,FALSE);
        }

	if (IS_WEAPON_STAT(wield,WEAPON_VORPAL) 
		&& number_range(1, 600 + MAX_LEVEL - (2 * ch->level)) == 1 
		&& IS_SET(victim->parts,PART_HEAD)
		&& check_immune(victim,dam_type) != IS_IMMUNE
		&& !IS_IMMORTAL(victim)
		&& ch != victim
		&& !is_safe(ch,victim)
		&& victim->hit < (victim->max_hit / 3))
	{
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *obj;
	char *name;
	int parts_buf;

	/* used a modified version of death_cry to make a severed head */

	name		= IS_NPC(victim) ? victim->short_descr : victim->name;
	obj		= create_object( get_obj_index( OBJ_VNUM_SEVERED_HEAD ), 0 );
	obj->timer	= number_range( 20, 30 );

	sprintf( buf, obj->short_descr, name );
	free_string( obj->short_descr );
	obj->short_descr = str_dup( buf );

	sprintf( buf, obj->description, name );
	free_string( obj->description );
	obj->description = str_dup( buf );

	if (obj->item_type == ITEM_FOOD)
	{
	    if (IS_SET(victim->form,FORM_POISON))
		obj->value[3] = 1;
	    else if (!IS_SET(victim->form,FORM_EDIBLE))
		obj->item_type = ITEM_TRASH;
	}

	obj_to_room( obj, ch->in_room );
	act("$n's head is severed from $s body by $p!",victim,wield,NULL,TO_ROOM);
	act("Your last feeling before you die is $p's weapon severing your head!",victim,ch,NULL,TO_CHAR);

	/* parts_buf is used to remove all parts of the body so the death_cry 
	   function does not create any other body parts */

	parts_buf = victim->parts;
	victim->parts = 0;
	vorpal_kill(ch,victim,dam,dt,dam_type);
	victim->parts = parts_buf;
	}

/*
        if (IS_WEAPON_STAT(wield,WEAPON_SHOCKING))
        {
            dam = number_range(1,wield->level/5 + 2);
            act("$n is struck by lightning from $p.",victim,wield,NULL,TO_ROOM);
            act("You are shocked by $p.",victim,wield,NULL,TO_CHAR);
            damage(ch,victim,dam,0,DAM_LIGHTNING);
        }
*/
    }

    tail_chain( );
    return;
}

bool vorpal_kill( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt, int dam_type )
{
    char buf[MAX_STRING_LENGTH]; 
    OBJ_DATA *corpse;
    extern bool chaos;
    int chaos_points;

    if ( victim->position == POS_DEAD )
	return FALSE;

    /*
     * Stop up any residual loopholes.
     */
    if ( victim == ch )
	{
		log_string("BUG: victim == ch in vorpal_kill");
		return FALSE;
	}
    
    if ( victim != ch )
    {
	/*
	 * Certain attacks are forbidden.
	 * Most other attacks are returned.
	 */
	if ( is_safe( ch, victim ) )
	    return FALSE;
	check_killer( ch, victim );

	if ( victim->position > POS_STUNNED )
	{
	    if ( victim->fighting == NULL )
		set_fighting( victim, ch );
	    if (victim->timer <= 4)
	    	victim->position = POS_FIGHTING;
	}

	if ( victim->position > POS_STUNNED )
	{
	    if ( ch->fighting == NULL )
		set_fighting( ch, victim );

	    /*
	     * If victim is charmed, ch might attack victim's master.
	     */
	    if ( IS_NPC(ch)
	    &&   IS_NPC(victim)
	    &&   IS_AFFECTED(victim, AFF_CHARM)
	    &&   victim->master != NULL
	    &&   victim->master->in_room == ch->in_room
	    &&   number_bits( 3 ) == 0 )
	    {
		stop_fighting( ch, FALSE );
		multi_hit( ch, victim->master, TYPE_UNDEFINED );
		return FALSE;
	    }
	}

	/*
	 * More charm stuff.
	 */
	if ( victim->master == ch )
	    stop_follower( victim );
    }

    /*
     * Inviso attacks ... not.
     */
    if ( IS_AFFECTED(ch, AFF_INVISIBLE) )
    {
	affect_strip( ch, gsn_invis );
	affect_strip( ch, gsn_mass_invis );
	REMOVE_BIT( ch->affected_by, AFF_INVISIBLE );
	act( "`K$n fades into existence.`w", ch, NULL, NULL, TO_ROOM );
    }
	dam = victim->max_hit;
    dam_message( ch, victim, dam, dt, FALSE );

    if (dam == 0)
	return FALSE;
	
    /* Ok, give the ch xp for his hit and add to ch->exp_stack */
    if (!IS_NPC(ch)&& IS_NPC(victim)) {
    	    int xp = 0;
    	    int members = 0;
	    int group_levels = 0;
	    CHAR_DATA *gch;
	    
	    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
	    {
		if ( is_same_group( gch, ch ) )
	        {
		    members++;
		    group_levels += gch->level;
		}
	    }
	    
	    xp = hit_xp_compute(ch, victim, group_levels, members, UMIN(dam, victim->hit + 20));
	    ch->exp_stack += xp;
		xp *= 2;
	    gain_exp(ch, xp);
    }
    
    /*
     * KILL the victim.
     * Inform the victim of his new state.
     */
    victim->hit = -20;
    update_pos( victim );

    if( victim->position == POS_DEAD) 
	{
	mprog_death_trigger( victim );
	act( "`R$n is DEAD!!`w", victim, 0, 0, TO_ROOM );
	send_to_char( "`RYou have been KILLED!!\n\r\n\r`w", victim );
    }

    /*
     * Sleep spells and extremely wounded folks.
     */
    if ( !IS_AWAKE(victim) )
	stop_fighting( victim, FALSE );

    /*
     * Payoff for killing things.
     */
    if ( victim->position == POS_DEAD )
    {
	group_gain( ch, victim );

	if ( !IS_NPC(victim) )
	{
	    sprintf( log_buf, "%s killed by %s at %d",
		victim->name,
		(IS_NPC(ch) ? ch->short_descr : ch->name),
		victim->in_room->vnum );
	    log_string( log_buf );
	    if ( !IS_IMMORTAL( ch ) ) {
	    	free_string( victim->pcdata->nemesis );
	    	victim->pcdata->nemesis = str_dup( IS_NPC(ch) ? ch->short_descr : ch->name );
 	    } 

	    /*
	     * Dying penalty:
	     * 1/2 way back to previous level.
	     */
	    if ( !chaos && victim->exp > 0 && IS_NPC(ch) )
		gain_exp( victim, -1*(victim->exp/2) );
	}
	if ( chaos )
	{
	  chaos_points = 0;

	  if (ch->level < victim->level)
		chaos_points = 2*(victim->level - ch->level);
	
	  chaos_points = chaos_points + victim->level;
	  ch->pcdata->chaos_score = chaos_points;
	}

	    if (!IS_NPC(victim)) {
            sprintf(buf, "%s has been slain by %s!", victim->name,
            IS_NPC(ch) ? ch->short_descr : ch->name);
            do_sendinfo(ch, buf);
        }

	if (chaos && !IS_NPC( victim ) )
		chaos_kill( victim );
	else if ( !( !IS_NPC( victim ) && !IS_NPC( ch ) ) )
		raw_kill( victim );
	else {
		pk_kill( victim );
		if (!IS_IMMORTAL( ch )) {
			victim->pcdata->pk_deaths++;
			ch->pcdata->pk_kills++;
		}
	}
        /* RT new auto commands */

	if ( !IS_NPC(ch) && IS_NPC(victim) )
	{
	    corpse = get_obj_list( ch, "corpse", ch->in_room->contents ); 

	    if ( IS_SET(ch->act, PLR_AUTOLOOT) &&
		 corpse && corpse->contains) /* exists and not empty */
		do_get( ch, "all corpse" );

 	    if (IS_SET(ch->act,PLR_AUTOGOLD) &&
	        corpse && corpse->contains  && /* exists and not empty */
		!IS_SET(ch->act,PLR_AUTOLOOT))
	      do_get(ch, "gold corpse");
            
	    if ( IS_SET(ch->act, PLR_AUTOSAC) )
       	      if ( IS_SET(ch->act,PLR_AUTOLOOT) && corpse && corpse->contains)
		return TRUE;  /* leave if corpse has treasure */
	      else
		do_sacrifice( ch, "corpse" );
	}

	return TRUE;
    }

    /*
     * Take care of link dead people.
     */
    if ( !IS_NPC(victim) && victim->desc == NULL )
    {
	if ( number_range( 0, victim->wait ) == 0 )
	{
	    do_recall( victim, "" );
	    return TRUE;
	}
    }

    tail_chain( );
    return TRUE;
}


/*
 * Inflict damage from a hit.
 */
bool damage( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt, int dam_type )
{
    char buf[MAX_STRING_LENGTH]; 
	char buf2[MAX_STRING_LENGTH];
    OBJ_DATA *corpse;
    bool immune;
    extern bool chaos;
    int chaos_points;

    if ( victim->position == POS_DEAD )
	return FALSE;

    /*
     * Stop up any residual loopholes.
     */
    if ( dam > 2000 && !IS_IMMORTAL(ch) && !IS_NPC(ch))
    {
	sprintf( buf2, "%s just did %d damage to %s.", ch->name, dam, victim->name );
	send_wiz( 93, buf2);
	log_string(buf2);
	dam = 0;
	if (!IS_IMMORTAL(ch))
	{
	    OBJ_DATA *obj;
	    obj = get_eq_char( ch, WEAR_WIELD );
	    send_to_char("You really shouldn't cheat.\n\r",ch);
	    extract_obj(obj);
	}

    }
    
    if ( victim != ch )
    {
	/*
	 * Certain attacks are forbidden.
	 * Most other attacks are returned.
	 */
	if ( is_safe( ch, victim ) )
	    return FALSE;
	check_killer( ch, victim );

	if ( victim->position > POS_STUNNED )
	{
	    if ( victim->fighting == NULL )
		set_fighting( victim, ch );
	    if (victim->timer <= 4)
	    	victim->position = POS_FIGHTING;
	}

	if ( victim->position > POS_STUNNED )
	{
	    if ( ch->fighting == NULL )
		set_fighting( ch, victim );

	    /*
	     * If victim is charmed, ch might attack victim's master.
	     */
	    if ( IS_NPC(ch)
	    &&   IS_NPC(victim)
	    &&   IS_AFFECTED(victim, AFF_CHARM)
	    &&   victim->master != NULL
	    &&   victim->master->in_room == ch->in_room
	    &&   number_bits( 3 ) == 0 )
	    {
		stop_fighting( ch, FALSE );
		multi_hit( ch, victim->master, TYPE_UNDEFINED );
		return FALSE;
	    }
	}

	/*
	 * More charm stuff.
	 */
	if ( victim->master == ch )
	    stop_follower( victim );
    }

    /*
     * Inviso attacks ... not.
     */
    if ( IS_AFFECTED(ch, AFF_INVISIBLE) )
    {
	affect_strip( ch, gsn_invis );
	affect_strip( ch, gsn_mass_invis );
	REMOVE_BIT( ch->affected_by, AFF_INVISIBLE );
	act( "`K$n fades into existence.`w", ch, NULL, NULL, TO_ROOM );
    }

    /*
     * Damage modifiers.
     */
    if ( IS_AFFECTED(victim, AFF_SANCTUARY) )
	dam /= 2;

    if ( IS_AFFECTED(victim, AFF_PROTECT) && IS_EVIL(ch) )
	dam -= dam / 4;

    immune = FALSE;


    /*
     * Check for parry, and dodge.
     */
    if ( dt >= TYPE_HIT && ch != victim)
    {
        if ( check_parry( ch, victim ) )
	    return FALSE;
	if ( check_dodge( ch, victim ) )
	    return FALSE;
	if ( check_block( ch, victim ) )
            return FALSE;
    }

    switch(check_immune(victim,dam_type))
    {
	case(IS_IMMUNE):
	    immune = TRUE;
	    dam = 0;
	    break;
	case(IS_RESISTANT):	
	    dam -= dam/3;
	    break;
	case(IS_VULNERABLE):
	    dam += dam/2;
	    break;
    }

    dam_message( ch, victim, dam, dt, immune );

    if (dam == 0)
	return FALSE;
	
    /* Ok, give the ch xp for his hit and add to ch->exp_stack */
    if (!IS_NPC(ch) && IS_NPC(victim)) {
    	    int xp = 0;
    	    int members = 0;
	    int group_levels = 0;
	    CHAR_DATA *gch;
	    
	    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
	    {
		if ( is_same_group( gch, ch ) )
	        {
		    members++;
		    group_levels += gch->level;
		}
	    }
	    
	    xp = hit_xp_compute(ch, victim, group_levels, members, UMIN(dam, victim->hit + 20));
	    ch->exp_stack += xp;
	    gain_exp(ch, xp);
    }
    
    /*
     * Hurt the victim.
     * Inform the victim of his new state.
     */
    victim->hit -= dam;
    if ( !IS_NPC(victim)
    &&   victim->level >= LEVEL_IMMORTAL
    &&   victim->hit < 1 )
	victim->hit = 1;
    update_pos( victim );

    switch( victim->position )
    {
    case POS_MORTAL:
	act( "`R$n is mortally wounded, and will die soon, if not aided.`w",
	    victim, NULL, NULL, TO_ROOM );
	send_to_char( 
	    "`RYou are mortally wounded, and will die soon, if not aided.\n\r`w",
	    victim );
	break;

    case POS_INCAP:
	act( "`R$n is incapacitated and will slowly die, if not aided.`w",
	    victim, NULL, NULL, TO_ROOM );
	send_to_char(
	    "`RYou are incapacitated and will slowly die, if not aided.\n\r`w",
	    victim );
	break;

    case POS_STUNNED:
	act( "`R$n is stunned, but will probably recover.`w",
	    victim, NULL, NULL, TO_ROOM );
	send_to_char("`RYou are stunned, but will probably recover.\n\r`w",
	    victim );
	break;

    case POS_DEAD:
	mprog_death_trigger( victim );
	act( "`R$n is DEAD!!`w", victim, 0, 0, TO_ROOM );
	send_to_char( "`RYou have been KILLED!!\n\r\n\r`w", victim );
	break;

    default:
	if ( dam > victim->max_hit / 4 )
	    send_to_char( "`RThat really did HURT`R!\n\r`w", victim );
	if ( victim->hit < victim->max_hit / 4 )
	    send_to_char( "`RYou sure are BLEEDING`R!\n\r`w", victim );
	break;
    }

    /*
     * Sleep spells and extremely wounded folks.
     */
    if ( !IS_AWAKE(victim) )
	stop_fighting( victim, FALSE );

    /*
     * Payoff for killing things.
     */
    if ( victim->position == POS_DEAD )
    {
	group_gain( ch, victim );

	if ( !IS_NPC(victim) )
	{
	    sprintf( log_buf, "%s killed by %s at %d",
		victim->name,
		(IS_NPC(ch) ? ch->short_descr : ch->name),
		victim->in_room->vnum );
	    log_string( log_buf );
	    if ( !IS_IMMORTAL( ch ) ) {
	    	free_string( victim->pcdata->nemesis );
	    	victim->pcdata->nemesis = str_dup( IS_NPC(ch) ? ch->short_descr : ch->name );
 	    } 

	    /*
	     * Dying penalty:
	     * 1/2 way back to previous level.
	     */
	    if ( !chaos && victim->exp > 0 && IS_NPC(ch) )
		gain_exp( victim, -1*(victim->exp/2) );
	}
	if ( chaos )
	{
	  chaos_points = 0;

	  if (ch->level < victim->level)
		chaos_points = 2*(victim->level - ch->level);
	
	  chaos_points = chaos_points + victim->level;
	  ch->pcdata->chaos_score = chaos_points;
	}

	    if (!IS_NPC(victim)) {
            sprintf(buf, "%s has been slain by %s!", victim->name,
            IS_NPC(ch) ? ch->short_descr : ch->name);
            do_sendinfo(ch, buf);
        }

	if (chaos && !IS_NPC( victim ) )
		chaos_kill( victim );
	else if ( !( !IS_NPC( victim ) && !IS_NPC( ch ) ) )
		raw_kill( victim );
	else {
		pk_kill( victim );
		if (!IS_IMMORTAL( ch )) {
			victim->pcdata->pk_deaths++;
			ch->pcdata->pk_kills++;
		}
	}
        /* RT new auto commands */

	if ( !IS_NPC(ch) && IS_NPC(victim) )
	{
	    corpse = get_obj_list( ch, "corpse", ch->in_room->contents ); 

	    if ( IS_SET(ch->act, PLR_AUTOLOOT) &&
		 corpse && corpse->contains) /* exists and not empty */
		do_get( ch, "all corpse" );

 	    if (IS_SET(ch->act,PLR_AUTOGOLD) &&
	        corpse && corpse->contains  && /* exists and not empty */
		!IS_SET(ch->act,PLR_AUTOLOOT))
	      do_get(ch, "gold corpse");
            
	    if ( IS_SET(ch->act, PLR_AUTOSAC) )
       	      if ( IS_SET(ch->act,PLR_AUTOLOOT) && corpse && corpse->contains)
		return TRUE;  /* leave if corpse has treasure */
	      else
		do_sacrifice( ch, "corpse" );
	}

	return TRUE;
    }

    if ( victim == ch )
	return TRUE;

    /*
     * Take care of link dead people.
     */
    if ( !IS_NPC(victim) && victim->desc == NULL )
    {
	if ( number_range( 0, victim->wait ) == 0 )
	{
	    do_recall( victim, "" );
	    return TRUE;
	}
    }

    /*
     * Wimp out?
     */
    if ( IS_NPC(victim) && dam > 0 && victim->wait < PULSE_VIOLENCE / 2)
    {
	if ( ( IS_SET(victim->act, ACT_WIMPY) && number_bits( 2 ) == 0
	&&   victim->hit < victim->max_hit / 5) 
	||   ( IS_AFFECTED(victim, AFF_CHARM) && victim->master != NULL
	&&     victim->master->in_room != victim->in_room ) )
	    do_flee( victim, "" );
    }

    if ( !IS_NPC(victim)
    &&   victim->hit > 0
    &&   victim->hit <= victim->wimpy
    &&   victim->wait < PULSE_VIOLENCE / 2 )
	do_flee( victim, "" );

    tail_chain( );
    return TRUE;
}

bool new_damage( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt, int dam_type, bool show ) 
{
    char buf[MAX_STRING_LENGTH]; 
    char buf2[MAX_STRING_LENGTH]; 
    OBJ_DATA *corpse;
    bool immune;
    extern bool chaos;
    int chaos_points;

    if ( victim->position == POS_DEAD )
	return FALSE;

    /*
     * Stop up any residual loopholes.
     */
    if ( dam > 2000 && !IS_IMMORTAL(ch) )
    {
	bug( "Damage: %d: more than 2000 points!", dam );
	sprintf( buf2, "%s just did %d damage to %s.", ch->name, dam, victim->name );
	send_wiz( 93, buf2);
	dam = 0;
	if (!IS_IMMORTAL(ch))
	{
	    OBJ_DATA *obj;
	    obj = get_eq_char( ch, WEAR_WIELD );
	    send_to_char("You really shouldn't cheat.\n\r",ch);
	    extract_obj(obj);
	}

    }

    
    if ( victim != ch )
    {
	/*
	 * Certain attacks are forbidden.
	 * Most other attacks are returned.
	 */
	if ( is_safe( ch, victim ) )
	    return FALSE;
	check_killer( ch, victim );

	if ( victim->position > POS_STUNNED )
	{
	    if ( victim->fighting == NULL )
		set_fighting( victim, ch );
	    if (victim->timer <= 4)
	    	victim->position = POS_FIGHTING;
	}

	if ( victim->position > POS_STUNNED )
	{
	    if ( ch->fighting == NULL )
		set_fighting( ch, victim );

	    /*
	     * If victim is charmed, ch might attack victim's master.
	     */
	    if ( IS_NPC(ch)
	    &&   IS_NPC(victim)
	    &&   IS_AFFECTED(victim, AFF_CHARM)
	    &&   victim->master != NULL
	    &&   victim->master->in_room == ch->in_room
	    &&   number_bits( 3 ) == 0 )
	    {
		stop_fighting( ch, FALSE );
		multi_hit( ch, victim->master, TYPE_UNDEFINED );
		return FALSE;
	    }
	}

	/*
	 * More charm stuff.
	 */
	if ( victim->master == ch )
	    stop_follower( victim );
    }

    /*
     * Inviso attacks ... not.
     */
    if ( IS_AFFECTED(ch, AFF_INVISIBLE) )
    {
	affect_strip( ch, gsn_invis );
	affect_strip( ch, gsn_mass_invis );
	REMOVE_BIT( ch->affected_by, AFF_INVISIBLE );
	act( "`K$n fades into existence.`w", ch, NULL, NULL, TO_ROOM );
    }

    /*
     * Damage modifiers.
     */
    if ( IS_AFFECTED(victim, AFF_SANCTUARY) )
	dam /= 2;

    if ( IS_AFFECTED(victim, AFF_PROTECT) && IS_EVIL(ch) )
	dam -= dam / 4;

    immune = FALSE;


    /*
     * Check for parry, and dodge.
     */
    if ( dt >= TYPE_HIT && ch != victim)
    {
        if ( check_parry( ch, victim ) )
	    return FALSE;
	if ( check_dodge( ch, victim ) )
	    return FALSE;
	if ( check_block( ch, victim ) )
            return FALSE;
    }

    switch(check_immune(victim,dam_type))
    {
	case(IS_IMMUNE):
	    immune = TRUE;
	    dam = 0;
	    break;
	case(IS_RESISTANT):	
	    dam -= dam/3;
	    break;
	case(IS_VULNERABLE):
	    dam += dam/2;
	    break;
    }

    if (show)
    dam_message( ch, victim, dam, dt, immune );

    if (dam == 0)
	return FALSE;

    /* Ok, give the ch xp for his hit and add to ch->exp_stack */
    if ((!IS_NPC(ch)) && IS_NPC(victim)) /* not NPCs, and no xp for hitting 
self */
     {
    	    int xp = 0;
    	    int members = 0;
	    int group_levels = 0;
	    CHAR_DATA *gch;
	    
	    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
	    {
		if ( is_same_group( gch, ch ) )
	        {
		    members++;
		    group_levels += gch->level;
		}
	    }
	    
	    xp = hit_xp_compute(ch, victim, group_levels, members, UMIN(dam, victim->hit + 20));
	    ch->exp_stack += xp;
	    gain_exp(ch, xp);
    }

    /*
     * Hurt the victim.
     * Inform the victim of his new state.
     */
    victim->hit -= dam;
    if ( !IS_NPC(victim)
    &&   victim->level >= LEVEL_IMMORTAL
    &&   victim->hit < 1 )
	victim->hit = 1;
    update_pos( victim );

    switch( victim->position )
    {
    case POS_MORTAL:
	act( "`R$n is mortally wounded, and will die soon, if not aided.`w",
	    victim, NULL, NULL, TO_ROOM );
	send_to_char( 
	    "`RYou are mortally wounded, and will die soon, if not aided.\n\r`w",
	    victim );
	break;

    case POS_INCAP:
	act( "`R$n is incapacitated and will slowly die, if not aided.`w",
	    victim, NULL, NULL, TO_ROOM );
	send_to_char(
	    "`RYou are incapacitated and will slowly die, if not aided.\n\r`w",
	    victim );
	break;

    case POS_STUNNED:
	act( "`R$n is stunned, but will probably recover.`w",
	    victim, NULL, NULL, TO_ROOM );
	send_to_char("`RYou are stunned, but will probably recover.\n\r`w",
	    victim );
	break;

    case POS_DEAD:
	mprog_death_trigger( victim );
	act( "`R$n is DEAD!!`w", victim, 0, 0, TO_ROOM );
	send_to_char( "`RYou have been KILLED!!\n\r\n\r`w", victim );
	break;

    default:
	if ( dam > victim->max_hit / 4 )
	    send_to_char( "`RThat really did HURT`R!\n\r`w", victim );
	if ( victim->hit < victim->max_hit / 4 )
	    send_to_char( "`RYou sure are BLEEDING`R!\n\r`w", victim );
	break;
    }

    /*
     * Sleep spells and extremely wounded folks.
     */
    if ( !IS_AWAKE(victim) ) 
	stop_fighting( victim, FALSE );

    /*
     * Payoff for killing things.
     */
    if ( victim->position == POS_DEAD )
    {
	group_gain( ch, victim );

	if ( !IS_NPC(victim) )
	{
	    sprintf( log_buf, "%s killed by %s at %d",
		victim->name,
		(IS_NPC(ch) ? ch->short_descr : ch->name),
		victim->in_room->vnum );
	    log_string( log_buf );
	    if ( !IS_IMMORTAL( ch ) ) {
	    	free_string(victim->pcdata->nemesis);
	   	victim->pcdata->nemesis = str_dup( IS_NPC(ch) ? ch->short_descr : ch->name );
	   }

	    /*
	     * Dying penalty:
	     * 1/2 way back to previous level.
	     */
	    if ( !chaos && victim->exp > 0 && IS_NPC(ch) )
		gain_exp( victim, -1*(victim->exp/2) );
	}
	if ( chaos )
	{
	  chaos_points = 0;

	  if (ch->level < victim->level)
		chaos_points = 2*(victim->level - ch->level);
	
	  chaos_points = chaos_points + victim->level;
	  ch->pcdata->chaos_score = chaos_points;
	}

	    if (!IS_NPC(victim)) {
            sprintf(buf, "%s has been slain by %s!", victim->name,
            IS_NPC(ch) ? ch->short_descr : ch->name);
            do_sendinfo(ch, buf);
        }

	if (chaos && !IS_NPC( victim ) )
		chaos_kill( victim );
	else if ( !( !IS_NPC( victim ) && !IS_NPC( ch ) ) )
		raw_kill( victim );
	else {
		pk_kill( victim );
		if (!IS_IMMORTAL( ch )) {
			victim->pcdata->pk_deaths++;
			ch->pcdata->pk_kills++;
		}
	}

        /* RT new auto commands */

	if ( !IS_NPC(ch) && IS_NPC(victim) )
	{
	    corpse = get_obj_list( ch, "corpse", ch->in_room->contents ); 

	    if ( IS_SET(ch->act, PLR_AUTOLOOT) &&
		 corpse && corpse->contains) /* exists and not empty */
		do_get( ch, "all corpse" );

 	    if (IS_SET(ch->act,PLR_AUTOGOLD) &&
	        corpse && corpse->contains  && /* exists and not empty */
		!IS_SET(ch->act,PLR_AUTOLOOT))
	      do_get(ch, "gold corpse");
            
	    if ( IS_SET(ch->act, PLR_AUTOSAC) )
       	      if ( IS_SET(ch->act,PLR_AUTOLOOT) && corpse && corpse->contains)
		return TRUE;  /* leave if corpse has treasure */
	      else
		do_sacrifice( ch, "corpse" );
	}

	return TRUE;
    }

    if ( victim == ch )
	return TRUE;

    /*
     * Take care of link dead people.
     */
    if ( !IS_NPC(victim) && victim->desc == NULL )
    {
	if ( number_range( 0, victim->wait ) == 0 )
	{
	    do_recall( victim, "" );
	    return TRUE;
	}
    }

    /*
     * Wimp out?
     */
    if ( IS_NPC(victim) && dam > 0 && victim->wait < PULSE_VIOLENCE / 2)
    {
	if ( ( IS_SET(victim->act, ACT_WIMPY) && number_bits( 2 ) == 0
	&&   victim->hit < victim->max_hit / 5) 
	||   ( IS_AFFECTED(victim, AFF_CHARM) && victim->master != NULL
	&&     victim->master->in_room != victim->in_room ) )
	    do_flee( victim, "" );
    }

    if ( !IS_NPC(victim)
    &&   victim->hit > 0
    &&   victim->hit <= victim->wimpy
    &&   victim->wait < PULSE_VIOLENCE / 2 )
	do_flee( victim, "" );

    tail_chain( );
    return TRUE;
}

bool is_safe(CHAR_DATA *ch, CHAR_DATA *victim )
{

/* no killing NPCs with ACT_NO_KILL */

if (IS_NPC(victim) && IS_SET(victim->act,ACT_NO_KILL))
{
 send_to_char("I don't think the gods would approve.\n\r",ch);
 return TRUE;
}

    /* no fighting in safe rooms */
    if (IS_SET(ch->in_room->room_flags,ROOM_SAFE))
    {
	send_to_char("Not in this room.\n\r",ch);
	return TRUE;
    }

    if (victim->fighting == ch)
	return FALSE;

    if (IS_NPC(ch))
    {
 	/* charmed mobs and pets cannot attack players */
	if (!IS_NPC(victim) && (IS_AFFECTED(ch,AFF_CHARM)
			    ||  IS_SET(ch->act,ACT_PET)))
	    return TRUE;

      	return FALSE;
     }

     else /* Not NPC */
     {	
	if (IS_IMMORTAL(ch))
	    return FALSE;

	/* no pets */
	if (IS_NPC(victim) && IS_SET(victim->act,ACT_PET))
	{
            act("But $N looks so cute and cuddly...",ch,NULL,victim,TO_CHAR);
            return TRUE;
	}

	/* no charmed mobs unless char is the the owner */
	if (IS_AFFECTED(victim,AFF_CHARM) && ch != victim->master)
	{
            send_to_char("You don't own that monster.\n\r",ch);
	    return TRUE;
	}

	return FALSE;
    }
}

bool is_safe_spell(CHAR_DATA *ch, CHAR_DATA *victim, bool area )
{
    /* can't zap self (crash bug) */
    if (ch == victim)
	return TRUE;

    /* immortals not hurt in area attacks */
    if (IS_IMMORTAL(victim) &&  area)
	return TRUE;

    /* no killing NO_KILL mobiles */
    if (IS_NPC(victim) && IS_SET(victim->act,ACT_NO_KILL))
	return TRUE;

    /* no fighting in safe rooms */
    if (IS_SET(ch->in_room->room_flags,ROOM_SAFE))
        return TRUE;

    if (victim->fighting == ch)
	return FALSE;
 
    if (IS_NPC(ch))
    {
        /* charmed mobs and pets cannot attack players */
        if (!IS_NPC(victim) && (IS_AFFECTED(ch,AFF_CHARM)
                            ||  IS_SET(ch->act,ACT_PET)))
            return TRUE;
	
	/* area affects don't hit other mobiles */
        if (IS_NPC(victim) && area)
            return TRUE;
 
        return FALSE;
    }
 
    else /* Not NPC */
    {
        if (IS_IMMORTAL(ch) && !area)
            return FALSE;
 
        /* no pets */
        if (IS_NPC(victim) && IS_SET(victim->act,ACT_PET))
            return TRUE;
 
        /* no charmed mobs unless char is the the owner */
        if (IS_AFFECTED(victim,AFF_CHARM) && ch != victim->master)
            return TRUE;
 
        /* no player killing */
        if ( !IS_NPC(victim) )
    	{
         if ( !IS_SET(victim->act, PLR_KILLER) || !IS_SET(ch->act, PLR_KILLER) )
          {
            send_to_char( "You can only kill other player killers.\n\r", ch );
            return TRUE;
          }
	}

	/* cannot use spells if not in same group */
	if ( victim->fighting != NULL && !is_same_group(ch,victim->fighting))
	    return TRUE;
 
        return FALSE;
    }
}

/*
 * See if an attack justifies a KILLER flag.
 */
void check_killer( CHAR_DATA *ch, CHAR_DATA *victim )
{
       char buf[MAX_STRING_LENGTH]; 
/*
     * Follow charm thread to responsible character.
     * Attacking someone's charmed char is hostile!
     */
    while ( IS_AFFECTED(victim, AFF_CHARM) && victim->master != NULL )
	victim = victim->master;

    /*
     * NPC's are fair game.
     * So are killers and thieves.
     */
    if ( IS_NPC(victim)
    ||   IS_SET(victim->act, PLR_KILLER)
    ||   IS_SET(victim->act, PLR_THIEF) )
	return;

    /*
     * Charm-o-rama.
     */
    if ( IS_SET(ch->affected_by, AFF_CHARM) )
    {
	if ( ch->master == NULL )
	{
	    sprintf( buf, "Check_killer: %s bad AFF_CHARM",
		IS_NPC(ch) ? ch->short_descr : ch->name );
	    bug( buf, 0 );
	    affect_strip( ch, gsn_charm_person );
	    REMOVE_BIT( ch->affected_by, AFF_CHARM );
	    return;
	}

	stop_follower( ch );
	return;
    }

    /*
     * NPC's are cool of course (as long as not charmed).
     * Hitting yourself is cool too (bleeding).
     * So is being immortal (Alander's idea).
     * And current killers stay as they are.
     */
    if ( IS_NPC(ch)
    ||   ch == victim
    ||   ch->level >= LEVEL_IMMORTAL
    ||   IS_SET(ch->act, PLR_KILLER) )
	return;

    send_to_char( "*** You are now a KILLER!! ***\n\r", ch );
    SET_BIT(ch->act, PLR_KILLER);
    save_char_obj( ch );
    return;
}



/*
 * Check for parry.
 */
bool check_parry( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;

    if ( !IS_AWAKE(victim) )
	return FALSE;

    if ( IS_NPC(victim) )
    {
	chance	= UMIN( 30, victim->level );
    }
    else
    {
	if ( get_eq_char( victim, WEAR_WIELD ) == NULL )
	    return FALSE;
	chance	= victim->pcdata->learned[gsn_parry] / 2;
    }

    if ( number_percent( ) >= chance + victim->level - ch->level )
	return FALSE;

    act( "`BYou parry $n's attack.`w",  ch, NULL, victim, TO_VICT    );
    act( "`B$N parries your attack.`w", ch, NULL, victim, TO_CHAR    );
    check_improve(victim,gsn_parry,TRUE,6);
    return TRUE;
}

/* Shield block code by Rindar(Ron Cole) of Everhaven MUD */
bool check_block( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chancea;
    int chance;

    if ( !IS_AWAKE(victim) )
        return FALSE;

    if ( get_eq_char( victim, WEAR_SHIELD ) == NULL )
        return FALSE;

    if ( IS_NPC(victim) )
    {
        chance = UMIN( 30, victim->level );
    }
    else
    {
        chance = victim->pcdata->learned[gsn_shield_block] / 4;
    }
    
    /* Must get a successful check before a parry can be attempted */ 
    if ( !IS_NPC(victim) )
    {
       if ( number_percent( ) > victim->pcdata->learned[gsn_shield_block])
          return FALSE;
    }

    chancea = 0;
    
    if (!can_see(victim,ch))    
    chance  -= 25; 

    if (!can_see(ch,victim))    
    chancea  -= 25; 

    chance  += get_curr_stat(victim,STAT_DEX)/4;
    chancea += (get_curr_stat(ch,STAT_DEX)/4) + ((ch->level)/2)+(get_curr_stat(ch,STAT_WIS)/3);


/* A high chance is good.  A low chance means a failed parry */
    if ( number_percent( ) >= chance + ((victim->level)/2) - chancea )
        return FALSE;

    act( "`BYou block $n's attack.`w",  ch, NULL, victim, TO_VICT    );
    act( "`B$N blocks your attack.`w", ch, NULL, victim, TO_CHAR    );

    if ( ((victim->level) - 5)>(ch->level) )
    return TRUE;   

    check_improve(victim,gsn_shield_block,TRUE,6);
    return TRUE;
}
/*
 * Check for dodge.
 */
bool check_dodge( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;

    if ( !IS_AWAKE(victim) )
	return FALSE;

    if ( IS_NPC(victim) )
        chance  = UMIN( 30, victim->level );
    else
        chance  = victim->pcdata->learned[gsn_dodge] / 2;

    if ( number_percent( ) >= chance + victim->level - ch->level )
        return FALSE;

    act( "`BYou dodge $n's attack.`w", ch, NULL, victim, TO_VICT    );
    act( "`B$N dodges your attack.`w", ch, NULL, victim, TO_CHAR    );
    check_improve(victim,gsn_dodge,TRUE,6);
    return TRUE;
}



/*
 * Set position of a victim.
 */
void update_pos( CHAR_DATA *victim )
{
    if ( victim->hit > 0 )
    {
    	if ( victim->position <= POS_STUNNED )
	    victim->position = POS_STANDING;
	return;
    }

    if ( IS_NPC(victim) && victim->hit < 1 )
    {
	victim->position = POS_DEAD;
	return;
    }

    if ( victim->hit <= -11 )
    {
	victim->position = POS_DEAD;
	return;
    }

         if ( victim->hit <= -6 ) victim->position = POS_MORTAL;
    else if ( victim->hit <= -3 ) victim->position = POS_INCAP;
    else                          victim->position = POS_STUNNED;

    return;
}



/*
 * Start fights.
 */
void set_fighting( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( ch->fighting != NULL )
    {
	bug( "Set_fighting: already fighting", 0 );
	return;
    }

    if ( IS_AFFECTED(ch, AFF_SLEEP) )
	affect_strip( ch, gsn_sleep );

    ch->fighting = victim;
    ch->position = POS_FIGHTING;

    return;
}



/*
 * Stop fights.
 */
void stop_fighting( CHAR_DATA *ch, bool fBoth )
{
    CHAR_DATA *fch;

    for ( fch = char_list; fch != NULL; fch = fch->next )
    {
	if ( fch == ch || ( fBoth && fch->fighting == ch ) )
	{
	    fch->fighting	= NULL;
	    fch->position	= IS_NPC(fch) ? ch->default_pos : POS_STANDING;
	    if (fch->exp_stack > 0) {
	    	char buf [MAX_STRING_LENGTH];
	    	
	    	sprintf(buf, "`WYou receive %ld experience points.\n\r`w", fch->exp_stack);
	    	send_to_char(buf, fch);
	    	fch->exp_stack = 0;
	    }	    	
	    update_pos( fch );
	}
    }

    return;
}



/*
 * Make a corpse out of a character.
 */
void make_corpse( CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH]; 
    OBJ_DATA *corpse;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    char *name;

    if ( IS_NPC(ch) )
    {
	name		= ch->short_descr;
	corpse		= create_object(get_obj_index(OBJ_VNUM_CORPSE_NPC), 0);
	corpse->timer	= number_range( 3, 6 );
	if ( ch->gold > 0 )
	{
	    obj_to_obj( create_money( ch->gold ), corpse );
	    ch->gold = 0;
	}
	corpse->cost = 0;
    }
    else
    {
	name		= ch->name;
	corpse		= create_object(get_obj_index(OBJ_VNUM_CORPSE_PC), 0);
	corpse->timer	= number_range( 25, 40 );
	REMOVE_BIT(ch->act,PLR_CANLOOT);
	if ( !IS_SET(ch->act,PLR_THIEF) )
	    corpse->owner = str_dup(ch->name);
	else
	    corpse->owner = NULL;
	corpse->cost = 0;
    }

    corpse->level = ch->level;

    sprintf( buf, corpse->short_descr, name );
    free_string( corpse->short_descr );
    corpse->short_descr = str_dup( buf );

    sprintf( buf, corpse->description, name );
    free_string( corpse->description );
    corpse->description = str_dup( buf );

    for ( obj = ch->carrying; obj != NULL; obj = obj_next )
    {
	obj_next = obj->next_content;
	obj_from_char( obj );
	if (obj->item_type == ITEM_POTION)
	    obj->timer = number_range(500,1000);
	if (obj->item_type == ITEM_SCROLL)
	    obj->timer = number_range(1000,2500);
	if (IS_SET(obj->extra_flags,ITEM_ROT_DEATH))
	    obj->timer = number_range(5,10);
	REMOVE_BIT(obj->extra_flags,ITEM_VIS_DEATH);
	REMOVE_BIT(obj->extra_flags,ITEM_ROT_DEATH);

	if ( IS_SET( obj->extra_flags, ITEM_INVENTORY ) )
	    extract_obj( obj );
	else
	    obj_to_obj( obj, corpse );
    }
    if(IS_NPC(ch))
    obj_to_room( corpse, ch->in_room );
    else
    obj_to_room(corpse, get_room_index(ROOM_VNUM_MORGUE));
    return;
}

void make_pk_corpse( CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH]; 
    OBJ_DATA *corpse;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    char *name;

    name		= ch->name;
    corpse		= create_object(get_obj_index(OBJ_VNUM_CORPSE_PC), 0);
    corpse->timer	= number_range( 3, 10 );
    REMOVE_BIT(ch->act,PLR_CANLOOT);
    if (!IS_SET(ch->act,PLR_THIEF))
	    corpse->owner = str_dup(ch->name);
	else
	    corpse->owner = NULL;
	corpse->cost = 0;

    corpse->level = ch->level;

    sprintf( buf, corpse->short_descr, name );
    free_string( corpse->short_descr );
    corpse->short_descr = str_dup( buf );

    sprintf( buf, corpse->description, name );
    free_string( corpse->description );
    corpse->description = str_dup( buf );

    if (IS_SET(ch->act,PLR_THIEF))
    for ( obj = ch->carrying; obj != NULL; obj = obj_next )
    {
	obj_next = obj->next_content;
	obj_from_char( obj );
	if (obj->item_type == ITEM_POTION)
	    obj->timer = number_range(500,1000);
	if (obj->item_type == ITEM_SCROLL)
	    obj->timer = number_range(1000,2500);
	if (IS_SET(obj->extra_flags,ITEM_ROT_DEATH))
	    obj->timer = number_range(5,10);
	REMOVE_BIT(obj->extra_flags,ITEM_VIS_DEATH);
	REMOVE_BIT(obj->extra_flags,ITEM_ROT_DEATH);

	if ( IS_SET( obj->extra_flags, ITEM_INVENTORY ) )
	    extract_obj( obj );
	else
	    obj_to_obj( obj, corpse );
    }
    if(IS_NPC(ch))
    obj_to_room( corpse, ch->in_room );
    else
    obj_to_room( corpse, get_room_index(ROOM_VNUM_MORGUE));
    return;
}


/*
 * Improved Death_cry contributed by Diavolo.
 */
void death_cry( CHAR_DATA *ch )
{
   char buf[MAX_STRING_LENGTH]; 
    ROOM_INDEX_DATA *was_in_room;
    char *msg;
    int door;
    int vnum;

    vnum = 0;
    msg = "`YYou hear $n's death cry.`w";

    switch ( number_bits(4))
    {
    case  0: msg  = "$n hits the ground ... DEAD.";			break;
    case  1: 
	if (ch->material == 0)
	{
	    msg  = "`R$n splatters blood on your armor.`w";		
	    break;
	}
    case  2: 							
	if (IS_SET(ch->parts,PART_GUTS))
	{
	    msg = "`R$n spills $s guts all over the floor.`w";
	    vnum = OBJ_VNUM_GUTS;
	}
	break;
    case  3: 
	if (IS_SET(ch->parts,PART_HEAD))
	{
	    msg  = "`R$n's severed head plops on the ground.`w";
	    vnum = OBJ_VNUM_SEVERED_HEAD;				
	}
	break;
    case  4: 
	if (IS_SET(ch->parts,PART_HEART))
	{
	    msg  = "`R$n's heart is torn from $s chest.`w";
	    vnum = OBJ_VNUM_TORN_HEART;				
	}
	break;
    case  5: 
	if (IS_SET(ch->parts,PART_ARMS))
	{
	    msg  = "`R$n's arm is sliced from $s dead body.`w";
	    vnum = OBJ_VNUM_SLICED_ARM;				
	}
	break;
    case  6: 
	if (IS_SET(ch->parts,PART_LEGS))
	{
	    msg  = "`R$n's leg is sliced from $s dead body.`w";
	    vnum = OBJ_VNUM_SLICED_LEG;				
	}
	break;
    case 7:
	if (IS_SET(ch->parts,PART_BRAINS))
	{
	    msg = "`R$n's head is shattered, and $s brains splash all over you.`w";
	    vnum = OBJ_VNUM_BRAINS;
	}
    }

    act( msg, ch, NULL, NULL, TO_ROOM );

    if ( vnum != 0 )
    {
	OBJ_DATA *obj;
	char *name;

	name		= IS_NPC(ch) ? ch->short_descr : ch->name;
	obj		= create_object( get_obj_index( vnum ), 0 );
	obj->timer	= number_range( 4, 7 );

	sprintf( buf, obj->short_descr, name );
	free_string( obj->short_descr );
	obj->short_descr = str_dup( buf );

	sprintf( buf, obj->description, name );
	free_string( obj->description );
	obj->description = str_dup( buf );

	if (obj->item_type == ITEM_FOOD)
	{
	    if (IS_SET(ch->form,FORM_POISON))
		obj->value[3] = 1;
	    else if (!IS_SET(ch->form,FORM_EDIBLE))
		obj->item_type = ITEM_TRASH;
	}

	obj_to_room( obj, ch->in_room );
    }

    if ( IS_NPC(ch) )
	msg = "`RYou hear something's death cry.`w";
    else
	msg = "`RYou hear someone's death cry.`w";

    was_in_room = ch->in_room;
    for ( door = 0; door <= 5; door++ )
    {
	EXIT_DATA *pexit;

	if ( ( pexit = was_in_room->exit[door] ) != NULL
	&&   pexit->u1.to_room != NULL
	&&   pexit->u1.to_room != was_in_room )
	{
	    ch->in_room = pexit->u1.to_room;
	    act( msg, ch, NULL, NULL, TO_ROOM );
	}
    }
    ch->in_room = was_in_room;

    return;
}

void chaos_kill( CHAR_DATA *victim)
{
   char buf[MAX_STRING_LENGTH]; 
   OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    DESCRIPTOR_DATA *d;

        stop_fighting( victim, TRUE );
	for ( obj = victim->carrying; obj != NULL; obj = obj_next )
        {
            obj_next = obj->next_content;
            obj_from_char( obj );
            obj_to_room( obj, victim->in_room );
        }
     	act( "`B$n's corpse is sucked into the ground!!`w", victim, 0, 0, TO_ROOM );
	if ( !IS_NPC(victim) )
	{	
 	   sprintf(buf, "was slain with %d chaos points.", victim->pcdata->chaos_score);
    	   chaos_log( victim, buf);
	}
    d = victim->desc;
    extract_char( victim, TRUE );
    if ( d != NULL )
        close_socket( d );
  return;
}


void raw_kill( CHAR_DATA *victim )
{
    int i;

    make_corpse( victim );
    stop_fighting( victim, TRUE );

    if ( IS_NPC(victim) )
    {
	victim->pIndexData->killed++;
	kill_table[URANGE(0, victim->level, MAX_LEVEL-1)].killed++;
	extract_char( victim, TRUE );
	return;
    }

    extract_char( victim, FALSE );
    while ( victim->affected )
	affect_remove( victim, victim->affected );
    victim->affected_by	= 0;
    for (i = 0; i < 4; i++)
    	victim->armor[i]= 100;
    victim->position	= POS_RESTING;
    victim->hit		= UMAX( 1, victim->hit  );
    victim->mana	= UMAX( 1, victim->mana );
    victim->move	= UMAX( 1, victim->move );
    /* RT added to prevent infinite deaths */
    REMOVE_BIT(victim->act, PLR_THIEF);
    REMOVE_BIT(victim->act, PLR_BOUGHT_PET);
/*  save_char_obj( victim ); */
/* Add back race affects */
    victim->affected_by = victim->affected_by|race_table[victim->race].aff;
    return;
}

void pk_kill( CHAR_DATA *victim )
{
    int i;

    make_pk_corpse( victim );
    stop_fighting( victim, TRUE );

    pk_extract_char( victim, FALSE );
    while ( victim->affected )
	affect_remove( victim, victim->affected );
    victim->affected_by	= 0;
    for (i = 0; i < 4; i++)
    	victim->armor[i]= 100;
    victim->position	= POS_RESTING;
    victim->hit		= UMAX( 1, victim->hit  );
    victim->mana	= UMAX( 1, victim->mana );
    victim->move	= UMAX( 1, victim->move );
    /* RT added to prevent infinite deaths */
    REMOVE_BIT(victim->act, PLR_THIEF);
    REMOVE_BIT(victim->act, PLR_BOUGHT_PET);
    /*  save_char_obj( victim ); */
    /* Add back race affects */
    victim->affected_by = victim->affected_by|race_table[victim->race].aff;
    /* Fix 'em up! -Kyle */
    reset_char(victim);
    return;
}


void group_gain( CHAR_DATA *ch, CHAR_DATA *victim )
{
    char buf[MAX_STRING_LENGTH]; 
    CHAR_DATA *gch;
    CHAR_DATA *lch;
    int xp = 0;
    int members;
    int group_levels;

    /*
     * Monsters don't get kill xp's or alignment changes.
     * P-killing doesn't help either.
     * Dying of mortal wounds or poison doesn't give xp to anyone!
     */
    if ( !IS_NPC(victim) || victim == ch )
	return;
    
    members = 0;
    group_levels = 0;
    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ( is_same_group( gch, ch ) )
        {
	    members++;
	    group_levels += gch->level;
	}
    }

    if ( members == 0 )
    {
	bug( "Group_gain: members.", members );
	members = 1;
	group_levels = ch->level ;
    }

    lch = (ch->leader != NULL) ? ch->leader : ch;

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;

	if ( !is_same_group( gch, ch ) || IS_NPC(gch))
	    continue;

	if ( gch->exp_stack < 1 ) xp = xp_compute( gch, victim, group_levels, members );
      	if (gch->exp_stack > 0)
      		sprintf( buf, "`WYou receive %ld experience points.\n\r`w", gch->exp_stack );
      	else
      		sprintf( buf, "`WYou receive %d experience points.\n\r`w", xp);
	if ( gch->level < LEVEL_HERO && (gch->exp + xp) >= 
	exp_per_level(gch,gch->pcdata->points) &&
	gch->exp < exp_per_level(gch,gch->pcdata->points))
	strcat( buf, "`WYou're ready to `CLevel`w!\n\r");        
	send_to_char( buf, gch );
	if ( gch->exp_stack < 1 ) gain_exp( gch, xp );
      	gch->exp_stack = 0;
	
	

	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    if ( obj->wear_loc == WEAR_NONE )
		continue;

	    if ( ( IS_OBJ_STAT(obj, ITEM_ANTI_EVIL)    && IS_EVIL(ch)    )
	    ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_GOOD)    && IS_GOOD(ch)    )
	    ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch) ) )
	    {
		act( "`WYou are zapped by $p.`w", ch, obj, NULL, TO_CHAR );
		act( "`W$n is zapped by $p.`w",   ch, obj, NULL, TO_ROOM );
		obj_from_char( obj );
		obj_to_room( obj, ch->in_room );
		oprog_zap_trigger( ch, obj );
	    }
	}
    }

    return;
}



/*
 * Compute xp for a kill.
 * Also adjust alignment of killer.
 * Edit this function to change xp computations.
 */
int xp_compute( CHAR_DATA *gch, CHAR_DATA *victim, int total_levels, int members )
{
    int xp,base_exp=0;
    int align;
    int change;

    /* compute the base exp */
    switch (victim->level)
    {
    	case 0  :	base_exp =   50;		break;
 	case 1  : 	base_exp =   100;		break;
	case 2  :	base_exp =   200;		break;
	case 3  :	base_exp =   250;		break;
	case 4  :	base_exp =   350;		break;
	case 5  : 	base_exp =   550;		break;
	case 6  :	base_exp =   1000;		break;
	case 7  :	base_exp =   3000;		break;
	case 8  :	base_exp =   5000;		break;
	case 9  :	base_exp =   7500;		break;
	case 10 :	base_exp =   10000;		break;
	case 11 :	base_exp =   15000;		break;
	case 12 :	base_exp =   23000;		break;
	case 13 :	base_exp =   35000;		break;
	case 14 :	base_exp =   50000;		break;
	case 15 :	base_exp =   65000;		break;
 	case 16 : 	base_exp =   80000;		break;
	case 17 :	base_exp =   95000;		break;
	case 18 :	base_exp =   110000;		break;
	case 19 :	base_exp =   135000;		break;
	case 20 : 	base_exp =   150000;		break;
	case 21 :	base_exp =   165000;		break;
	case 22 :	base_exp =   180000;		break;
	case 23 :	base_exp =   200000;		break;
	case 24 :	base_exp =   220000;		break;
	case 25 :	base_exp =   240000;		break;
	case 26 :	base_exp =   260000;		break;
	case 27 :	base_exp =   280000;		break;
	case 28 :	base_exp =   300000;		break;
	case 29 :	base_exp =   320000;		break;
	case 30 :	base_exp =   340000;		break;
 	case 31 : 	base_exp =   360000;		break;
	case 32 :	base_exp =   380000;		break;
	case 33 :	base_exp =   400000;		break;
	case 34 :	base_exp =   420000;		break;
	case 35 : 	base_exp =   440000;		break;
	case 36 :	base_exp =   460000;		break;
	case 37 :	base_exp =   480000;		break;
	case 38 :	base_exp =   500000;		break;
	case 39 :	base_exp =   520000;		break;
	case 40 :	base_exp =   540000;		break;
	case 41 :	base_exp =   560000;		break;
	case 42 :	base_exp =   580000;		break;
	case 43 :	base_exp =   600000;		break;
	case 44 :	base_exp =   620000;		break;
	case 45 :	base_exp =   640000;		break;
 	case 46 : 	base_exp =   660000;		break;
	case 47 :	base_exp =   680000;		break;
	case 48 :	base_exp =   700000;		break;
	case 49 :	base_exp =   720000;		break;
	case 50 : 	base_exp =   740000;		break;
	case 51 :	base_exp =   760000;		break;
	case 52 :	base_exp =   780000;		break;
	case 53 :	base_exp =   800000;		break;
	case 54 :	base_exp =   820000;		break;
	case 55 :	base_exp =   840000;		break;
	case 56 :	base_exp =   860000;		break;
	case 57 :	base_exp =   880000;		break;
	case 58 :	base_exp =   900000;		break;
	case 59 :	base_exp =   920000;		break;
	case 60 :	base_exp =   940000;		break;
	case 61 :	base_exp =   960000;		break;
	case 62 :	base_exp =   1000000;		break;
	case 63 :	base_exp =   1100000;		break;
	case 64 :	base_exp =   1200000;		break;
	case 65 :	base_exp =   1300000;		break;
	case 66 :	base_exp =   1400000;		break;
	case 67 :	base_exp =   1500000;		break;
 	case 68 : 	base_exp =   1600000;		break;
	case 69 :	base_exp =   1700000;		break;
	case 70 :	base_exp =   1800000;		break;
	case 71 :	base_exp =   1900000;		break;
	case 72 : 	base_exp =   2000000;		break;
	case 73 :	base_exp =   2100000;		break;
	case 74 :	base_exp =   2200000;		break;
	case 75 :	base_exp =   2300000;		break;
	case 76 :	base_exp =   2400000;		break;
	case 77 :	base_exp =   2500000;		break;
	case 78 :	base_exp =   2600000;		break;
	case 79 :	base_exp =   2700000;		break;
	case 80 :	base_exp =   2800000;		break;
	case 81 :	base_exp =   2900000;		break;
	case 82 :	base_exp =   3000000;		break;
	case 83 :	base_exp =   3100000;		break;
	case 84 :	base_exp =   3200000;		break;
	case 85 : 	base_exp =   3300000;		break;
	case 86 :	base_exp =   3400000;		break;
	case 87 :	base_exp =   3500000;		break;
	case 88 :	base_exp =   3600000;		break;
	case 89 :	base_exp =   3700000;		break;
	case 90 :	base_exp =   3800000;		break;
	case 91 :	base_exp =   4000000;		break;
	case 92 :	base_exp =   4500000;		break;
	case 93 :	base_exp =   5000000;		break;
	case 94 :	base_exp =   5500000;		break;
	case 95 :	base_exp =   6000000;		break;
	case 96 :	base_exp =   6500000;		break;
	case 97 :	base_exp =   7000000;		break;
	case 98 :	base_exp =   7500000;		break;
	case 99 :	base_exp =   8000000;		break;
	case 100 :	base_exp =   10000000;		break;
    } 
    
    /* do alignment computations */
   
    align = victim->alignment / 50;

    if (IS_SET(victim->act,ACT_NOALIGN))
    {
	/* no change */
    }

    else if (align > 0) /* monster is more good than slayer */
    {
	change = align  *  (gch->level/total_levels+(1/members))/2; 
	change = UMAX(1,change);
        gch->alignment = UMAX(-1000,gch->alignment - change);
    }

    else if (align < 0) /* monster is more evil than slayer */
    {
	change =  ( -1 * align ) * (gch->level/total_levels+(1/members))/2;
	change = UMAX(1,change);
	gch->alignment = UMIN(1000,gch->alignment + change);
    }

    else /* improve this someday */
    {
	change =  0;  
	gch->alignment -= change;
    }
    
    /* calculate exp multiplier */
    xp = base_exp;

    if (IS_SET(victim->act,ACT_NOALIGN))
    {
	/* no change */
    }
    else if (gch->alignment > 500)  /* for goodie two shoes */
    {
	if (victim->alignment < -750)
	    xp = base_exp * 4/3;
   
 	else if (victim->alignment < -500)
	    xp = base_exp * 5/4;

        else if (victim->alignment > 250)
	    xp = base_exp * 3/4; 

        else if (victim->alignment > 750)
	    xp = base_exp / 4;

   	else if (victim->alignment > 500)
	    xp = base_exp / 2;

	else
	    xp = base_exp;
    }

    else if (gch->alignment < -500) /* for baddies */
    {
	if (victim->alignment > 750)
	    xp = base_exp * 5/4;
	
  	else if (victim->alignment > 500)
	    xp = base_exp * 11/10; 

   	else if (victim->alignment < -750)
	    xp = base_exp * 1/2;

	else if (victim->alignment < -500)
	    xp = base_exp * 3/4;

	else if (victim->alignment < -250)
	    xp = base_exp * 9/10;

	else
	    xp = base_exp;
    }

    else if (gch->alignment > 200)  /* a little good */
    {

	if (victim->alignment < -500)
	    xp = base_exp * 6/5;

 	else if (victim->alignment > 750)
	    xp = base_exp * 1/2;

	else if (victim->alignment > 0)
	    xp = base_exp * 3/4; 
	
	else
	    xp = base_exp;
    }

    else if (gch->alignment < -200) /* a little bad */
    {
	if (victim->alignment > 500)
	    xp = base_exp * 6/5;
 
	else if (victim->alignment < -750)
	    xp = base_exp * 1/2;

	else if (victim->alignment < 0)
	    xp = base_exp * 3/4;

	else
	    xp = base_exp;
    }

    else /* neutral */
    {

	if (victim->alignment > 500 || victim->alignment < -500)
	    xp = base_exp * 4/3;

	else if (victim->alignment < 200 || victim->alignment > -200)
	    xp = base_exp * 1/2;

 	else
	    xp = base_exp;
   }

   /* randomize the rewards */
   xp = number_range (xp * 9/10, xp * 11/10);

   /* adjust for grouping */
   if (members > 1 ) xp = (gch->level * (xp / total_levels));
   if (xp > (exp_per_level(gch, gch->pcdata->points)/2) )
   	xp = (exp_per_level(gch,gch->pcdata->points)/2);
   xp = xp * 1/10; /* This function only runs if you didn't hit the mob
   		      so you don't gain much experience.  But hey, it's
   		      better than nothing. */
   
   return xp;
}

int hit_xp_compute( CHAR_DATA *gch, CHAR_DATA *victim, int total_levels, int members, int dam )
{
    int xp,base_exp=0;
    int align;
    int change;

    /* compute the base exp */
    switch (victim->level)
    {
    	case 0  :	base_exp =   50;		break;
 	case 1  : 	base_exp =   100;		break;
	case 2  :	base_exp =   200;		break;
	case 3  :	base_exp =   250;		break;
	case 4  :	base_exp =   350;		break;
	case 5  : 	base_exp =   550;		break;
	case 6  :	base_exp =   1000;		break;
	case 7  :	base_exp =   3000;		break;
	case 8  :	base_exp =   5000;		break;
	case 9  :	base_exp =   7500;		break;
	case 10 :	base_exp =   10000;		break;
	case 11 :	base_exp =   15000;		break;
	case 12 :	base_exp =   23000;		break;
	case 13 :	base_exp =   35000;		break;
	case 14 :	base_exp =   50000;		break;
	case 15 :	base_exp =   65000;		break;
 	case 16 : 	base_exp =   80000;		break;
	case 17 :	base_exp =   95000;		break;
	case 18 :	base_exp =   110000;		break;
	case 19 :	base_exp =   135000;		break;
	case 20 : 	base_exp =   150000;		break;
	case 21 :	base_exp =   165000;		break;
	case 22 :	base_exp =   180000;		break;
	case 23 :	base_exp =   200000;		break;
	case 24 :	base_exp =   220000;		break;
	case 25 :	base_exp =   240000;		break;
	case 26 :	base_exp =   260000;		break;
	case 27 :	base_exp =   280000;		break;
	case 28 :	base_exp =   300000;		break;
	case 29 :	base_exp =   320000;		break;
	case 30 :	base_exp =   340000;		break;
 	case 31 : 	base_exp =   360000;		break;
	case 32 :	base_exp =   380000;		break;
	case 33 :	base_exp =   400000;		break;
	case 34 :	base_exp =   420000;		break;
	case 35 : 	base_exp =   440000;		break;
	case 36 :	base_exp =   460000;		break;
	case 37 :	base_exp =   480000;		break;
	case 38 :	base_exp =   500000;		break;
	case 39 :	base_exp =   520000;		break;
	case 40 :	base_exp =   540000;		break;
	case 41 :	base_exp =   560000;		break;
	case 42 :	base_exp =   580000;		break;
	case 43 :	base_exp =   600000;		break;
	case 44 :	base_exp =   620000;		break;
	case 45 :	base_exp =   640000;		break;
 	case 46 : 	base_exp =   660000;		break;
	case 47 :	base_exp =   680000;		break;
	case 48 :	base_exp =   700000;		break;
	case 49 :	base_exp =   720000;		break;
	case 50 : 	base_exp =   740000;		break;
	case 51 :	base_exp =   760000;		break;
	case 52 :	base_exp =   780000;		break;
	case 53 :	base_exp =   800000;		break;
	case 54 :	base_exp =   820000;		break;
	case 55 :	base_exp =   840000;		break;
	case 56 :	base_exp =   860000;		break;
	case 57 :	base_exp =   880000;		break;
	case 58 :	base_exp =   900000;		break;
	case 59 :	base_exp =   920000;		break;
	case 60 :	base_exp =   940000;		break;
	case 61 :	base_exp =   960000;		break;
	case 62 :	base_exp =   1000000;		break;
	case 63 :	base_exp =   1100000;		break;
	case 64 :	base_exp =   1200000;		break;
	case 65 :	base_exp =   1300000;		break;
	case 66 :	base_exp =   1400000;		break;
	case 67 :	base_exp =   1500000;		break;
 	case 68 : 	base_exp =   1600000;		break;
	case 69 :	base_exp =   1700000;		break;
	case 70 :	base_exp =   1800000;		break;
	case 71 :	base_exp =   1900000;		break;
	case 72 : 	base_exp =   2000000;		break;
	case 73 :	base_exp =   2100000;		break;
	case 74 :	base_exp =   2200000;		break;
	case 75 :	base_exp =   2300000;		break;
	case 76 :	base_exp =   2400000;		break;
	case 77 :	base_exp =   2500000;		break;
	case 78 :	base_exp =   2600000;		break;
	case 79 :	base_exp =   2700000;		break;
	case 80 :	base_exp =   2800000;		break;
	case 81 :	base_exp =   2900000;		break;
	case 82 :	base_exp =   3000000;		break;
	case 83 :	base_exp =   3100000;		break;
	case 84 :	base_exp =   3200000;		break;
	case 85 : 	base_exp =   3300000;		break;
	case 86 :	base_exp =   3400000;		break;
	case 87 :	base_exp =   3500000;		break;
	case 88 :	base_exp =   3600000;		break;
	case 89 :	base_exp =   3700000;		break;
	case 90 :	base_exp =   3800000;		break;
	case 91 :	base_exp =   4000000;		break;
	case 92 :	base_exp =   4500000;		break;
	case 93 :	base_exp =   5000000;		break;
	case 94 :	base_exp =   5500000;		break;
	case 95 :	base_exp =   6000000;		break;
	case 96 :	base_exp =   6500000;		break;
	case 97 :	base_exp =   7000000;		break;
	case 98 :	base_exp =   7500000;		break;
	case 99 :	base_exp =   8000000;		break;
	case 100 :	base_exp =   10000000;		break;
    } 
    
    /* do alignment computations */
   
    align = victim->alignment / 20;

    if (IS_SET(victim->act,ACT_NOALIGN))
    {
	/* no change */
    }

    else if (align > 0) 
    {
	change = align  *  (gch->level/total_levels+(1/members))/2; 
	change = UMAX(1,change);
        gch->alignment = UMAX(-1000,gch->alignment - change);
    }

    else if (align < 0 ) 
    {
	change = -1 * align * (gch->level/total_levels+(1/members))/2;
	change = UMAX(1,change);
	gch->alignment = UMIN(1000,gch->alignment + change);
    }

    else /* improve this someday */
    {
	/* nothing */
    }
    
    /* calculate exp multiplier */
    xp = base_exp;

    if (IS_SET(victim->act,ACT_NOALIGN) || victim == gch )
    {
	/* no change */
    }
    else if (gch->alignment > 500)  /* for goodie two shoes */
    {
	if (victim->alignment < -750)
	    xp = base_exp * 4/3;
   
 	else if (victim->alignment < -500)
	    xp = base_exp * 5/4;

        else if (victim->alignment > 250)
	    xp = base_exp * 3/4; 

        else if (victim->alignment > 750)
	    xp = base_exp / 4;

   	else if (victim->alignment > 500)
	    xp = base_exp / 2;

	else
	    xp = base_exp;
    }

    else if (gch->alignment < -500) /* for baddies */
    {
	if (victim->alignment > 750)
	    xp = base_exp * 5/4;
	
  	else if (victim->alignment > 500)
	    xp = base_exp * 11/10; 

   	else if (victim->alignment < -750)
	    xp = base_exp * 1/2;

	else if (victim->alignment < -500)
	    xp = base_exp * 3/4;

	else if (victim->alignment < -250)
	    xp = base_exp * 9/10;

	else
	    xp = base_exp;
    }

    else if (gch->alignment > 200) /* a little good */
    {

	if (victim->alignment < -500)
	    xp = base_exp * 6/5;

 	else if (victim->alignment > 750)
	    xp = base_exp * 1/2;

	else if (victim->alignment > 0)
	    xp = base_exp * 3/4; 
	
	else
	    xp = base_exp;
    }

    else if (gch->alignment < -200) /* a little bad */
    {
	if (victim->alignment > 500)
	    xp = base_exp * 6/5;
 
	else if (victim->alignment < -750)
	    xp = base_exp * 1/2;

	else if (victim->alignment < 0)
	    xp = base_exp * 3/4;

	else
	    xp = base_exp;
    }

    else /* neutral */
    {

	if (victim->alignment > 500 || victim->alignment < -500)
	    xp = base_exp * 4/3;

	else if (victim->alignment < 200 || victim->alignment > -200)
	    xp = base_exp * 1/2;

 	else
	    xp = base_exp;
   }

   /* randomize the rewards */
   xp = number_range (xp * 9/10, xp * 11/10);

   /* adjust for grouping */
   xp = ( xp * dam/victim->max_hit );
   if (members > 1 ) xp = xp * 1.15;
   
   return xp;
}


void dam_message( CHAR_DATA *ch, CHAR_DATA *victim,int dam,int dt,bool immune )
{
    char buf1[256], buf2[256], buf3[256];
    const char *vs;
    const char *vp;
    const char *attack;
    char punct;
    int damp;
    
    damp = 1 + dam / 2;

	 if ( dam  ==   0 ) { vs = "miss";	vp = "misses";		}
    else if ( damp <=   2 ) { vs = "scratch";	vp = "scratches";	}
    else if ( damp <=   4 ) { vs = "graze";	vp = "grazes";		}
    else if ( damp <=   6 ) { vs = "hit";	vp = "hits";		}
    else if ( damp <=   8 ) { vs = "injure";	vp = "injures";		}
    else if ( damp <=  10 ) { vs = "wound";	vp = "wounds";		}
    else if ( damp <=  12 ) { vs = "maul";       vp = "mauls";		}
    else if ( damp <=  14 ) { vs = "decimate";	vp = "decimates";	}
    else if ( damp <=  16 ) { vs = "devastate";	vp = "devastates";	}
    else if ( damp <=  18 ) { vs = "maim";	vp = "maims";		}
    else if ( damp <=  20 ) { vs = "MUTILATE";	vp = "MUTILATES";	}
    else if ( damp <=  22 ) { vs = "PULVERIZE";	vp = "PULVERIZES";	}
    else if ( damp <=  24 ) { vs = "DISMEMBER";	vp = "DISMEMBERS";	}
    else if ( damp <=  26 ) { vs = "MASSACRE";	vp = "MASSACRES";	}
    else if ( damp <=  28 ) { vs = "MANGLE";	vp = "MANGLES";		}
    else if ( damp <=  30 ) { vs = "*** DEMOLISH ***";
			     vp = "*** DEMOLISHES ***";			}
    else if ( damp <=  37 ) { vs = "*** DEVASTATE ***";
			     vp = "*** DEVASTATES ***";			}
    else if ( damp <=  50 ) { vs = "=== OBLITERATE ===";
			     vp = "=== OBLITERATES ===";		}
    else if ( damp <=  63 ) { vs = ">>> ANNIHILATE <<<";
			     vp = ">>> ANNIHILATES <<<";		}
    else if ( damp <=  75 ) { vs = "<<< ERADICATE >>>";
			     vp = "<<< ERADICATES >>>";			}
    else if ( damp <=  83 ) { vs = "<><><> BUTCHER <><><>";
    			     vp = "<><><> BUTCHERS <><><>";		}
    else if ( damp <=  93 ) { vs = "<><><> DISEMBOWEL <><><>";
    			     vp = "<><><> DISEMBOWELS <><><>";		}
    else                   { vs = "do UNSPEAKABLE things to";
			     vp = "does UNSPEAKABLE things to";		}

    punct   = (damp <= 24) ? '.' : '!';

    if ( dt == TYPE_HIT )
    {
        if (dam > 0)
	    if (ch  == victim)
   	    {
	        sprintf( buf1, "`w$n `R%s`Y $melf%c",vp,punct);
	        sprintf( buf2, "`wYou `R%s`Y yourself%c",vs,punct);
	    }
	    else
	    {
	        sprintf( buf1, "`w$n `R%s`w $N%c[%d]",  vp, punct, dam );
	        sprintf( buf2, "`wYou `R%s`w $N%c[%d]", vs, punct, dam );
	        sprintf( buf3, "`w$n `R%s`w you%c[%d]", vp, punct, dam );
	    }
	else if (ch == victim)
        {
	    sprintf( buf1, "`B$n %s $melf%c`w",vp,punct);
	    sprintf( buf2, "`BYou %s yourself%c`w",vs,punct);
	}
	else
	{
	    sprintf( buf1, "`B$n %s $N%c`w[%d]",  vp, punct, dam );
	    sprintf( buf2, "`BYou %s $N%c`w[%d]", vs, punct, dam );
	    sprintf( buf3, "`B$n %s you%c`w[%d]", vp, punct, dam );
	}
    }
    else
    {
	if ( dt >= 0 && dt < MAX_SKILL )
	    attack	= skill_table[dt].noun_damage;
	else if ( dt >= TYPE_HIT
	&& dt <= TYPE_HIT + MAX_DAMAGE_MESSAGE) 
	    attack	= attack_table[dt - TYPE_HIT].name;
	else
	{
	    bug( "Dam_message: bad dt %d.", dt );
	logf_string("BUG: ^^ ch: %s victim: %s ", ch->name, victim->name);
	    dt  = TYPE_HIT;
	    attack  = attack_table[0].name;
	}

	if (immune)
	{
	    if (ch == victim)
	    {
		sprintf(buf1,"`B$n is unaffected by $s own %s.`w",attack);
		sprintf(buf2,"`BLuckily, you are immune to that.`w");
	    } 
	    else
	    {
	    	sprintf(buf1,"`B$N is unaffected by $n's %s!`w",attack);
	    	sprintf(buf2,"`B$N is unaffected by your %s!`w",attack);
	    	sprintf(buf3,"`B$n's %s is powerless against you.`w",attack);
	    }
	}
	else
	{
	    if (dam > 0)
	        if (ch == victim)
	        {
		    sprintf( buf1, "`w$n's %s `R%s`w $m%c",attack,vp,punct);
		    sprintf( buf2, "`wYour %s `R%s`w you%c",attack,vp,punct);
	        }
	        else
	        {
	    	    sprintf( buf1, "`w$n's %s `R%s`w $N%c[%d]",  attack, vp, punct, dam );
	    	    sprintf( buf2, "`wYour %s `R%s`w $N%c[%d]",  attack, vp, punct, dam );
	    	    sprintf( buf3, "`w$n's %s `R%s`w you%c[%d]", attack, vp, punct, dam );
	        }
	    else if (ch == victim)
	    {
	        sprintf( buf1, "`B$n's %s %s $m%c`w",attack,vp,punct);
	        sprintf( buf2, "`BYour %s %s you%c`w",attack,vp,punct);
	    }
	    else
	    {
	        sprintf( buf1, "`B$n's %s %s $N%c`w[%d]",  attack, vp, punct, dam );
	        sprintf( buf2, "`BYour %s %s $N%c`w[%d]",  attack, vp, punct, dam );
	        sprintf( buf3, "`B$n's %s %s you%c`w[%d]", attack, vp, punct, dam );
	    }
	}
    }

    if (ch == victim)
    {
	act(buf1,ch,NULL,NULL,TO_ROOM);
	act(buf2,ch,NULL,NULL,TO_CHAR);
    }
    else
    {
    	act( buf1, ch, NULL, victim, TO_NOTVICT );
    	act( buf2, ch, NULL, victim, TO_CHAR );
    	act( buf3, ch, NULL, victim, TO_VICT );
    }

    return;
}



/*
 * Disarm a creature.
 * Caller must check for successful attack.
 */
void disarm( CHAR_DATA *ch, CHAR_DATA *victim )
{
    OBJ_DATA *obj;

    if ( ( obj = get_eq_char( victim, WEAR_WIELD ) ) == NULL )
	return;

    if ( IS_OBJ_STAT(obj,ITEM_NOREMOVE))
    {
	act("`W$S weapon won't budge!`w",ch,NULL,victim,TO_CHAR);
	act("`W$n tries to disarm you, but your weapon won't budge!`w",
	    ch,NULL,victim,TO_VICT);
	act("`W$n tries to disarm $N, but fails.`w",ch,NULL,victim,TO_NOTVICT);
	return;
    }

    act( "`W$n disarms you and sends your weapon flying!`w", 
	 ch, NULL, victim, TO_VICT    );
    act( "`WYou disarm $N!`w",  ch, NULL, victim, TO_CHAR    );
    act( "`W$n disarms $N!`w",  ch, NULL, victim, TO_NOTVICT );

    obj_from_char( obj );
    if ( IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_INVENTORY) )
	obj_to_char( obj, victim );
    else
    {
	obj_to_room( obj, victim->in_room );
	if (IS_NPC(victim) && victim->wait == 0 && can_see_obj(victim,obj))
	    get_obj(victim,obj,NULL);
    }

    return;
}

void do_berserk( CHAR_DATA *ch, char *argument)
{
    int chance, hp_percent;

    if ((chance = get_skill(ch,gsn_berserk)) == 0
    ||  (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_BERSERK))
    ||  (!IS_NPC(ch)
    &&   ch->level < skill_table[gsn_berserk].skill_level[ch->class]))
    {
	send_to_char("You turn red in the face, but nothing happens.\n\r",ch);
	return;
    }

    if (IS_AFFECTED(ch,AFF_BERSERK) || is_affected(ch,gsn_berserk)
    ||  is_affected(ch,skill_lookup("frenzy")))
    {
	send_to_char("You get a little madder.\n\r",ch);
	return;
    }

    if (IS_AFFECTED(ch,AFF_CALM))
    {
	send_to_char("You're feeling to mellow to berserk.\n\r",ch);
	return;
    }

    if (ch->mana < 50)
    {
	send_to_char("You can't get up enough energy.\n\r",ch);
	return;
    }

    /* modifiers */

    /* fighting */
    if (ch->position == POS_FIGHTING)
	chance += 10;

    /* damage -- below 50% of hp helps, above hurts */
    hp_percent = 100 * ch->hit/ch->max_hit;
    chance += 25 - hp_percent/2;

    if (number_percent() < chance)
    {
	AFFECT_DATA af;

	WAIT_STATE(ch,PULSE_VIOLENCE);
	ch->mana -= 50;
	ch->move /= 2;

	/* heal a little damage */
	ch->hit += ch->level * 2;
	ch->hit = UMIN(ch->hit,ch->max_hit);

	send_to_char("`RYour pulse races as you are consumned by rage!\n\r`w",ch);
	act("`W$n gets a wild look in $s eyes.`w",ch,NULL,NULL,TO_ROOM);
	check_improve(ch,gsn_berserk,TRUE,2);

	af.type		= gsn_berserk;
	af.level	= ch->level;
	af.duration	= number_fuzzy(ch->level / 8);
	af.modifier	= UMAX(1,ch->level/5);
	af.bitvector 	= AFF_BERSERK;

	af.location	= APPLY_HITROLL;
	affect_to_char(ch,&af);

	af.location	= APPLY_DAMROLL;
	affect_to_char(ch,&af);

	af.modifier	= UMAX(10,10 * (ch->level/5));
	af.location	= APPLY_AC;
	affect_to_char(ch,&af);
    }

    else
    {
	WAIT_STATE(ch,3 * PULSE_VIOLENCE);
	ch->mana -= 25;
	ch->move /= 2;

	send_to_char("Your pulse speeds up, but nothing happens.\n\r",ch);
	check_improve(ch,gsn_berserk,FALSE,2);
    }
}
												/* Bash Enhanced Significantly */
void do_bash( CHAR_DATA *ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH]; 
   CHAR_DATA *victim;
    int chance;

    one_argument(argument,arg);
 
    if ( (chance = get_skill(ch,gsn_bash)) == 0
    ||	 (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_BASH))
    ||	 (!IS_NPC(ch)
    &&	  ch->level < skill_table[gsn_bash].skill_level[ch->class]))
    {	
	send_to_char("Bashing? What's that?\n\r",ch);
	return;
    }
 
    if (arg[0] == '\0')
    {
	victim = ch->fighting;
	if (victim == NULL)
	{
	    send_to_char("But you aren't fighting anyone!\n\r",ch);
	    return;
	}
    }

    else if ((victim = get_char_room(ch,arg)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if ( !IS_NPC(victim) )
    {
        if ( !IS_SET(victim->act, PLR_KILLER) || !IS_SET(ch->act, PLR_KILLER) )
        {
            send_to_char( "You can only kill other player killers.\n\r", ch );
            return;
        }
    }

    if (victim->position < POS_FIGHTING)
    {
	act("You'll have to let $M get back up first.",ch,NULL,victim,TO_CHAR);
	return;
    } 

    if (victim == ch)
    {
	send_to_char("You try to bash your brains out, but fail.\n\r",ch);
	return;
    }

    if (is_safe(ch,victim))
	return;

    if ( victim->fighting != NULL && !is_same_group(ch,victim->fighting))
    {
        send_to_char("Kill stealing is not permitted.\n\r",ch);
        return;
    }

    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
	act("But $N is your friend!",ch,NULL,victim,TO_CHAR);
	return;
    }

    /* modifiers */

    /* size  and weight */
    chance += ch->carry_weight / 25;
    chance -= victim->carry_weight / 20;

    if (ch->size < victim->size)
	chance += (ch->size - victim->size) * 25;
    else
	chance += (ch->size - victim->size) * 10; 


    /* stats */
    chance += get_curr_stat(ch,STAT_STR);
    chance -= get_curr_stat(victim,STAT_DEX) * 4/3;

    /* speed */
    if (IS_SET(ch->off_flags,OFF_FAST))
	chance += 10;
    if (IS_SET(ch->off_flags,OFF_FAST))
	chance -= 20;

    /* level */
	 chance += (ch->level - victim->level) * 2;

    /* now the attack */
    if (number_percent() < chance)
    {
    
	act("`W$n sends you sprawling with a powerful bash!`w",
		ch,NULL,victim,TO_VICT);
	act("`WYou slam into $N, and send $M flying!`w",ch,NULL,victim,TO_CHAR);
	act("`W$n sends $N sprawling with a powerful bash.`w",
		ch,NULL,victim,TO_NOTVICT);
	check_improve(ch,gsn_bash,TRUE,1);

	WAIT_STATE(victim, 3 * PULSE_VIOLENCE);
	WAIT_STATE(ch,skill_table[gsn_bash].beats);
	victim->position = POS_RESTING;               /* Change right below */
	damage(ch,victim,number_range(2, get_curr_stat(ch, STAT_STR)/4 + ((2 * ch->level)/5) * ch->size + chance/10),gsn_bash,
	    DAM_BASH);
	
    }
    else
    {
	damage(ch,victim,0,gsn_bash,DAM_BASH);
	act("`BYou fall flat on your face!`w",
	    ch,NULL,victim,TO_CHAR);
	act("`B$n falls flat on $s face.`w",
	    ch,NULL,victim,TO_NOTVICT);
	act("`BYou evade $n's bash, causing $m to fall flat on $s face.`w",
	    ch,NULL,victim,TO_VICT);
	check_improve(ch,gsn_bash,FALSE,1);
	ch->position = POS_RESTING;
	WAIT_STATE(ch,skill_table[gsn_bash].beats * 3/2); 
    }
}

void do_dirt( CHAR_DATA *ch, char *argument )
{
       char arg[MAX_INPUT_LENGTH]; 
CHAR_DATA *victim;
    int chance;

    one_argument(argument,arg);

    if ( (chance = get_skill(ch,gsn_dirt)) == 0
    ||   (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_KICK_DIRT))
    ||   (!IS_NPC(ch)
    &&    ch->level < skill_table[gsn_dirt].skill_level[ch->class]))
    {
	send_to_char("You get your feet dirty.\n\r",ch);
	return;
    }

    if (arg[0] == '\0')
    {
	victim = ch->fighting;
	if (victim == NULL)
	{
	    send_to_char("But you aren't in combat!\n\r",ch);
	    return;
	}
    }

    else if ((victim = get_char_room(ch,arg)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if (IS_AFFECTED(victim,AFF_BLIND))
    {
	act("$e's already been blinded.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (victim == ch)
    {
	send_to_char("Very funny.\n\r",ch);
	return;
    }

    if (is_safe(ch,victim))
	return;
    if ( !IS_NPC(victim) )
    {
        if ( !IS_SET(victim->act, PLR_KILLER) || !IS_SET(ch->act, PLR_KILLER) )
        {
            send_to_char( "You can only kill other player killers.\n\r", ch );
            return;
        }
    }


    if ( victim->fighting != NULL && !is_same_group(ch,victim->fighting))
    {
        send_to_char("Kill stealing is not permitted.\n\r",ch);
        return;
    }

    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
	act("But $N is such a good friend!",ch,NULL,victim,TO_CHAR);
	return;
    }

    /* modifiers */

    /* dexterity */
    chance += get_curr_stat(ch,STAT_DEX);
    chance -= 2 * get_curr_stat(victim,STAT_DEX);

    /* speed  */
    if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
	chance += 10;
    if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
	chance -= 25;

    /* level */
    chance += (ch->level - victim->level) * 2;

    /* sloppy hack to prevent false zeroes */
    if (chance % 5 == 0)
	chance += 1;

    /* terrain */

    switch(ch->in_room->sector_type)
    {
	case(SECT_INSIDE):		chance -= 20;	break;
	case(SECT_CITY):		chance -= 10;	break;
	case(SECT_FIELD):		chance +=  5;	break;
	case(SECT_FOREST):				break;
	case(SECT_HILLS):				break;
	case(SECT_MOUNTAIN):		chance -= 10;	break;
	case(SECT_WATER_SWIM):		chance  =  0;	break;
	case(SECT_WATER_NOSWIM):	chance  =  0;	break;
	case(SECT_AIR):			chance  =  0;  	break;
	case(SECT_DESERT):		chance += 10;   break;
    }

    if (chance == 0)
    {
	send_to_char("There isn't any dirt to kick.\n\r",ch);
	return;
    }

    /* now the attack */
    if (number_percent() < chance)
    {
	AFFECT_DATA af;
	act("`W$n is blinded by the dirt in $s eyes!`w",victim,NULL,NULL,TO_ROOM);
        damage(ch,victim,number_range(2,5),gsn_dirt,DAM_NONE);
	send_to_char("`WYou can't see a thing!\n\r`w",victim);
	check_improve(ch,gsn_dirt,TRUE,2);
	WAIT_STATE(ch,skill_table[gsn_dirt].beats);

	af.type 	= gsn_dirt;
	af.level 	= ch->level;
	af.duration	= 0;
	af.location	= APPLY_HITROLL;
	af.modifier	= -4;
	af.bitvector 	= AFF_BLIND;

	affect_to_char(victim,&af);
    }
    else
    {
	damage(ch,victim,0,gsn_dirt,DAM_NONE);
	check_improve(ch,gsn_dirt,FALSE,2);
	WAIT_STATE(ch,skill_table[gsn_dirt].beats);
    }
}

void do_trip( CHAR_DATA *ch, char *argument )
{
       char arg[MAX_INPUT_LENGTH]; 
CHAR_DATA *victim;
    int chance;

    one_argument(argument,arg);

    if ( (chance = get_skill(ch,gsn_trip)) == 0
    ||   (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_TRIP))
    ||   (!IS_NPC(ch) 
	  && ch->level < skill_table[gsn_trip].skill_level[ch->class]))
    {
	send_to_char("Tripping?  What's that?\n\r",ch);
	return;
    }


    if (arg[0] == '\0')
    {
	victim = ch->fighting;
	if (victim == NULL)
	{
	    send_to_char("But you aren't fighting anyone!\n\r",ch);
	    return;
 	}
    }

    else if ((victim = get_char_room(ch,arg)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if (is_safe(ch,victim))
	return;

    if ( !IS_NPC(victim) )
    {
        if ( !IS_SET(victim->act, PLR_KILLER) || !IS_SET(ch->act, PLR_KILLER) )
        {
            send_to_char( "You can only kill other player killers.\n\r", ch );
            return;
        }
    }

    if ( victim->fighting != NULL && !is_same_group(ch,victim->fighting))
    {
	send_to_char("Kill stealing is not permitted.\n\r",ch);
	return;
    }
    
    if (IS_AFFECTED(victim,AFF_FLYING))
    {
	act("$S feet aren't on the ground.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (victim->position < POS_FIGHTING)
    {
	act("$N is already down.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (victim == ch)
    {
	send_to_char("`BYou fall flat on your face!\n\r`w",ch);
	WAIT_STATE(ch,2 * skill_table[gsn_trip].beats);
	act("$n trips over $s own feet!",ch,NULL,NULL,TO_ROOM);
	return;
    }

    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
	act("$N is your beloved master.",ch,NULL,victim,TO_CHAR);
	return;
    }

    /* modifiers */

    /* size */
    if (ch->size < victim->size)
        chance += (ch->size - victim->size) * 10;  /* bigger = harder to trip */

    /* dex */
    chance += get_curr_stat(ch,STAT_DEX);
    chance -= get_curr_stat(victim,STAT_DEX) * 3 / 2;

    /* speed */
    if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
	chance += 10;
    if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
	chance -= 20;

    /* level */
    chance += (ch->level - victim->level) * 2;


    /* now the attack */
    if (number_percent() < chance)
    {
	act("`W$n trips you and you go down!`w",ch,NULL,victim,TO_VICT);
	act("`WYou trip $N and $N goes down!`w",ch,NULL,victim,TO_CHAR);
	act("`W$n trips $N, sending $M to the ground.`w",ch,NULL,victim,TO_NOTVICT);
	check_improve(ch,gsn_trip,TRUE,1);

	WAIT_STATE(victim,2 * PULSE_VIOLENCE);
        WAIT_STATE(ch,skill_table[gsn_trip].beats);
	victim->position = POS_RESTING;
	damage(ch,victim,( ch->level/2 + victim->size + get_curr_stat(ch, 
STAT_STR) - get_curr_stat(victim, STAT_DEX) * 5/4),gsn_trip, DAM_BASH);
    }
    else
    {
	damage(ch,victim,0,gsn_trip,DAM_BASH);
	WAIT_STATE(ch,skill_table[gsn_trip].beats*2/3);
	check_improve(ch,gsn_trip,FALSE,1);
    } 
}



void do_kill( CHAR_DATA *ch, char *argument )
{
       char arg[MAX_INPUT_LENGTH]; 
CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Kill whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( !IS_NPC(victim) )
    {
        if ( !IS_SET(victim->act, PLR_KILLER) || !IS_SET(ch->act, PLR_KILLER) )
        {
            send_to_char( "You can only kill other player killers.\n\r", ch );
            return;
        }
    }

    if ( victim == ch )
    {
	send_to_char( "You hit yourself.  Ouch!\n\r", ch );
	multi_hit( ch, ch, TYPE_UNDEFINED );
	return;
    }

    if ( is_safe( ch, victim ) )
	return;

    if ( victim->fighting != NULL && !is_same_group(ch,victim->fighting))
    {
        send_to_char("Kill stealing is not permitted.\n\r",ch);
        return;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
    {
	act( "$N is your beloved master.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( ch->position == POS_FIGHTING )
    {
	send_to_char( "You do the best you can!\n\r", ch );
	return;
    }

    WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
    check_killer( ch, victim );
    multi_hit( ch, victim, TYPE_UNDEFINED );
    return;
}



void do_murde( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to MURDER, spell it out.\n\r", ch );
    return;
}



void do_murder( CHAR_DATA *ch, char *argument )
{
      char buf[MAX_STRING_LENGTH]; 
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Murder whom?\n\r", ch );
	return;
    }

    if (IS_AFFECTED(ch,AFF_CHARM) || (IS_NPC(ch) && IS_SET(ch->act,ACT_PET)))
	return;

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "Suicide is a mortal sin.\n\r", ch );
	return;
    }

    if ( is_safe( ch, victim ) )
	return;

    if ( victim->fighting != NULL && !is_same_group(ch,victim->fighting))
    {
        send_to_char("Kill stealing is not permitted.\n\r",ch);
        return;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
    {
	act( "$N is your beloved master.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( ch->position == POS_FIGHTING )
    {
	send_to_char( "You do the best you can!\n\r", ch );
	return;
    }

    WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
    if (IS_NPC(ch))
	sprintf(buf, "`YHelp! I am being attacked by %s!`w",ch->short_descr);
    else
    	sprintf( buf, "`YHelp!  I am being attacked by %s!`w", ch->name );
    do_yell( victim, buf );
    check_killer( ch, victim );
    multi_hit( ch, victim, TYPE_UNDEFINED );
    return;
}



void do_backstab( CHAR_DATA *ch, char *argument )
{
       char arg[MAX_INPUT_LENGTH]; 
CHAR_DATA *victim;
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Backstab whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
	 }
    /* This would be the dagger backstab change */
	 if ( get_weapon_sn(ch) != gsn_dagger )
	 {
	send_to_char("Try wielding a dagger to backstab.\n\r", ch);
	return;
	 }

	 if ( victim == ch )
	 {
	send_to_char( "How can you sneak up on yourself?\n\r", ch );
	return;
	 }

	 if ( is_safe( ch, victim ) )
		return;
	 if ( !IS_NPC(victim) )
	 {
		  if ( !IS_SET(victim->act, PLR_KILLER) || !IS_SET(ch->act, PLR_KILLER) )
		  {
				send_to_char( "You can only kill other player killers.\n\r", ch );
				return;
		  }
	 }

	 if ( victim->fighting != NULL && !is_same_group(ch,victim->fighting))
	 {
		  send_to_char("Kill stealing is not permitted.\n\r",ch);
		  return;
	 }

	 if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL)
	 {
	send_to_char( "You need to wield a weapon to backstab.\n\r", ch );
	return;
	 }

	 if ( victim->fighting != NULL )
	 {
	send_to_char( "You can't backstab a fighting person.\n\r", ch );
	return;
	 }

	 if ( victim->hit < (victim->max_hit/4) && !IS_AFFECTED(ch, AFF_SNEAK) )
	 {
	act( "$N is hurt and suspicious ... you can't sneak up.",
		 ch, NULL, victim, TO_CHAR );
	return;
	 }

	 check_killer( ch, victim );
	 WAIT_STATE( ch, skill_table[gsn_backstab].beats );
	 if ( !IS_AWAKE(victim)
	 ||   IS_NPC(ch)
	 ||   number_percent( ) < ch->pcdata->learned[gsn_backstab] )
	 {
	check_improve(ch,gsn_backstab,TRUE,1);
	multi_hit( ch, victim, gsn_backstab );
	 }
	 else
	 {
	check_improve(ch,gsn_backstab,FALSE,1);
	damage( ch, victim, 0, gsn_backstab,DAM_NONE );
	 }

	 return;
}

void do_circle( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    one_argument( argument, arg );
 
    if (arg[0] == '\0')
    {
        send_to_char("Circle whom?\n\r",ch);
        return;
    }

    else if ((victim = get_char_room(ch,arg)) == NULL)
    {
        send_to_char("They aren't here.\n\r",ch);
        return;
    }

    if (victim == ch)
	{
		send_to_char("You spin in circles, but nothing happens. Whee.\n\r", ch);
		return;
	}

    if ( is_safe( ch, victim ) )
      return;

    if (IS_NPC(victim) &&
         victim->fighting != NULL &&
        !is_same_group(ch,victim->fighting))
    {
        send_to_char("Kill stealing is not permitted.\n\r",ch);
        return;
    }
    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL)
    {
        send_to_char( "You need to wield a weapon to circle.\n\r", ch );
        return;
    }
    if ( ( victim = ch->fighting ) == NULL )
    {
        send_to_char( "You must be fighting in order to circle.\n\r", ch );
        return;
    }

    check_killer( ch, victim );
    WAIT_STATE( ch, skill_table[gsn_circle].beats );
    if ( number_percent( ) < get_skill(ch,gsn_circle)
    || ( get_skill(ch,gsn_circle) >= 2 && !IS_AWAKE(victim) ) )
    {
        check_improve(ch,gsn_circle,TRUE,1);
        multi_hit( ch, victim, gsn_circle );
    }
    else
    {
        check_improve(ch,gsn_circle,FALSE,1);
        new_damage( ch, victim, 0, gsn_circle,DAM_NONE,TRUE);
    }
    return;
}

void do_flee( CHAR_DATA *ch, char *argument )
{
	 ROOM_INDEX_DATA *was_in;
	 ROOM_INDEX_DATA *now_in;
    CHAR_DATA *victim;
    int attempt;
    long lost_exp;

    if ( ( victim = ch->fighting ) == NULL )
    {
        if ( ch->position == POS_FIGHTING )
            ch->position = POS_STANDING;
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    was_in = ch->in_room;
    for ( attempt = 0; attempt < 6; attempt++ )
    {
	EXIT_DATA *pexit;
	int door;

	door = number_door( );
	if ( ( pexit = was_in->exit[door] ) == 0
	||   pexit->u1.to_room == NULL
	||   IS_SET(pexit->exit_info, EX_CLOSED)
	|| ( IS_NPC(ch)
	&&   IS_SET(pexit->u1.to_room->room_flags, ROOM_NO_MOB) ) )
	    continue;

	move_char( ch, door, FALSE );
	if ( ( now_in = ch->in_room ) == was_in )
	    continue;

	ch->in_room = was_in;
	act( "$n has fled!", ch, NULL, NULL, TO_ROOM );
	ch->in_room = now_in;

	if ( !IS_NPC(ch) )
	{
	    char buf [MAX_STRING_LENGTH];
	    
	    lost_exp = 0.05 * exp_per_level(ch, ch->pcdata->points);
	    sprintf(buf, "You flee from combat!  You lose %ld exps.\n\r", lost_exp);
	    send_to_char( buf, ch );
	    gain_exp( ch, 0 - lost_exp );
	}

	stop_fighting( ch, TRUE );
	return;
    }

    send_to_char( "`RPANIC! You couldn't escape!\n\r`w", ch );
    return;
}



void do_rescue( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH]; 
    CHAR_DATA *victim;
    CHAR_DATA *fch;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Rescue whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "What about fleeing instead?\n\r", ch );
	return;
    }

    if ( !is_same_group(ch,victim))
    {
        send_to_char("Kill stealing is not permitted.\n\r",ch);
        return;
    }

    if ( !IS_NPC(ch) && IS_NPC(victim) )
    {
	send_to_char( "Doesn't need your help!\n\r", ch );
	return;
    }

    if ( ch->fighting == victim )
    {
	send_to_char( "Too late.\n\r", ch );
	return;
    }

    if ( ( fch = victim->fighting ) == NULL )
    {
	send_to_char( "That person is not fighting right now.\n\r", ch );
	return;
    }

    WAIT_STATE( ch, skill_table[gsn_rescue].beats );
    if ( !IS_NPC(ch) && number_percent( ) > ch->pcdata->learned[gsn_rescue] )
    {
	send_to_char( "`BYou fail the rescue.\n\r`w", ch );
	check_improve(ch,gsn_rescue,FALSE,1);
	return;
    }

    act( "`WYou rescue $N!`w",  ch, NULL, victim, TO_CHAR    );
    act( "`W$n rescues you!`w", ch, NULL, victim, TO_VICT    );
    act( "`W$n rescues $N!`w",  ch, NULL, victim, TO_NOTVICT );
    check_improve(ch,gsn_rescue,TRUE,1);

    stop_fighting( fch, FALSE );
    stop_fighting( victim, FALSE );

    check_killer( ch, fch );
    set_fighting( ch, fch );
    set_fighting( fch, ch );
    return;
}



void do_kick( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;

    if ( !IS_NPC(ch)
    &&   ch->level < skill_table[gsn_kick].skill_level[ch->class] )
    {
	send_to_char(
	    "You better leave the martial arts to fighters.\n\r", ch );
	return;
    }

    if (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_KICK))
	return;

    if ( ( victim = ch->fighting ) == NULL )
    {
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    if ( !IS_NPC(victim) )
    {
        if ( !IS_SET(victim->act, PLR_KILLER) || !IS_SET(ch->act, PLR_KILLER) )
        {
            send_to_char( "You can only kill other player killers.\n\r", ch );
            return;
        }
    }

    WAIT_STATE( ch, skill_table[gsn_kick].beats );
    if ( IS_NPC(ch) || number_percent( ) < ch->pcdata->learned[gsn_kick] )
    {
	damage( ch, victim, number_range( 9, ch->level + 9 ), gsn_kick,DAM_BASH );
	check_improve(ch,gsn_kick,TRUE,1);
	 }
	 else
	 {
	damage( ch, victim, 0, gsn_kick,DAM_BASH );
	check_improve(ch,gsn_kick,FALSE,1);
	 }

	 return;
}




void do_disarm( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int chance,hth,ch_weapon,vict_weapon,ch_vict_weapon;

    hth = 0;

    if ((chance = get_skill(ch,gsn_disarm)) == 0)
    {
	send_to_char( "You don't know how to disarm opponents.\n\r", ch );
	return;
    }

    if ( get_eq_char( ch, WEAR_WIELD ) == NULL 
    &&   ((hth = get_skill(ch,gsn_hand_to_hand)) == 0
    ||    (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_DISARM))))
    {
	send_to_char( "You must wield a weapon to disarm.\n\r", ch );
	return;
    }

    if ( ( victim = ch->fighting ) == NULL )
    {
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    if ( ( obj = get_eq_char( victim, WEAR_WIELD ) ) == NULL )
    {
	send_to_char( "Your opponent is not wielding a weapon.\n\r", ch );
	return;
    }

    /* find weapon skills */
    ch_weapon = get_weapon_skill(ch,get_weapon_sn(ch));
    vict_weapon = get_weapon_skill(victim,get_weapon_sn(victim));
    ch_vict_weapon = get_weapon_skill(ch,get_weapon_sn(victim));

    /* modifiers */

    /* skill */
    if ( get_eq_char(ch,WEAR_WIELD) == NULL)
	chance = chance * hth/150;
    else
	chance = chance * ch_weapon/100;

    chance += (ch_vict_weapon/2 - vict_weapon) / 2; 

    /* dex vs. strength */
    chance += get_curr_stat(ch,STAT_DEX);
    chance -= 2 * get_curr_stat(victim,STAT_STR);

    /* level */
    chance += (ch->level - victim->level) * 2;
 
    /* and now the attack */
    if (number_percent() < chance)
    {
    	WAIT_STATE( ch, skill_table[gsn_disarm].beats );
	disarm( ch, victim );
	check_improve(ch,gsn_disarm,TRUE,1);
    }
    else
    {
	WAIT_STATE(ch,skill_table[gsn_disarm].beats);
	act("`BYou fail to disarm $N.`w",ch,NULL,victim,TO_CHAR);
	act("`B$n tries to disarm you, but fails.`w",ch,NULL,victim,TO_VICT);
	act("`B$n tries to disarm $N, but fails.`w",ch,NULL,victim,TO_NOTVICT);
	check_improve(ch,gsn_disarm,FALSE,1);
    }
    return;
}



void do_sla( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to SLAY, spell it out.\n\r", ch );
    return;
}



void do_slay( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH]; 
    CHAR_DATA *victim;
    extern bool chaos;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Slay whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( ch == victim )
    {
	send_to_char( "Suicide is a mortal sin.\n\r", ch );
	return;
    }

    if ( !IS_NPC(victim) && victim->level >= get_trust(ch) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    act( "`RYou slay $M in cold blood!`w",  ch, NULL, victim, TO_CHAR    );
    act( "`R$n slays you in cold blood!`w", ch, NULL, victim, TO_VICT    );
    act( "`R$n slays $N in cold blood!`w",  ch, NULL, victim, TO_NOTVICT );

	if (chaos)
		chaos_kill( victim );
	else if ( IS_NPC( ch ) )
		raw_kill( victim );
	else
		pk_kill( victim );

    return;
}

void chaos_log( CHAR_DATA *ch, char *argument )
{
    append_file( ch, CHAOS_FILE , argument );
    return;
}


bool raf_damage( CHAR_DATA *victim, int dam, int dam_type , char *dam_name) 
{
/*    OBJ_DATA *corpse;   Gonna use this Thex?  */
   char buf[MAX_STRING_LENGTH]; 
   bool immune;
    extern bool chaos;

    if ( victim->position == POS_DEAD )
	return FALSE;

   


    /*
     * Damage modifiers.
     */
    if ( IS_AFFECTED(victim, AFF_SANCTUARY) )
	dam /= 2;

    immune = FALSE;


    switch(check_immune(victim,dam_type))
    {
	case(IS_IMMUNE):
	    immune = TRUE;
	    dam = 0;
	    break;
	case(IS_RESISTANT):	
	    dam -= dam/3;
	    break;
	case(IS_VULNERABLE):
	    dam += dam/2;
	    break;
    }

    if (dam == 0)
	return FALSE;

    /*
     * Hurt the victim.
     * Inform the victim of his new state.
     */
    victim->hit -= dam;
    if ( !IS_NPC(victim)
    &&   victim->level >= LEVEL_IMMORTAL
    &&   victim->hit < 1 )
	victim->hit = 1;
    update_pos( victim );

    switch( victim->position )
    {
    case POS_MORTAL:
	act( "`R$n is mortally wounded, and will die soon, if not aided.`w",
	    victim, NULL, NULL, TO_ROOM );
	send_to_char( 
	    "`RYou are mortally wounded, and will die soon, if not aided.\n\r`w",
	    victim );
	break;

    case POS_INCAP:
	act( "`R$n is incapacitated and will slowly die, if not aided.`w",
	    victim, NULL, NULL, TO_ROOM );
	send_to_char(
	    "`RYou are incapacitated and will slowly die, if not aided.\n\r`w",
	    victim );
	break;

    case POS_STUNNED:
	act( "`R$n is stunned, but will probably recover.`w",
	    victim, NULL, NULL, TO_ROOM );
	send_to_char("`RYou are stunned, but will probably recover.\n\r`w",
	    victim );
	break;

    case POS_DEAD:
	rprog_death_trigger( victim );
	mprog_death_trigger( victim );
	act( "`R$n is DEAD!!`w", victim, 0, 0, TO_ROOM );
	send_to_char( "`RYou have been KILLED!!\n\r\n\r`w", victim );
	break;

    default:
	if ( dam > victim->max_hit / 4 )
	    send_to_char( "`RThat really did HURT`R!\n\r`w", victim );
	if ( victim->hit < victim->max_hit / 4 )
	    send_to_char( "`RYou sure are BLEEDING`R!\n\r`w", victim );
	break;
    }


    /*
     * Payoff for killing things.
     */
    if ( victim->position == POS_DEAD )
    {

	if ( !IS_NPC(victim) )
	{
	    sprintf( log_buf, "%s killed by %s at %d",
		victim->name, dam_name,	victim->in_room->vnum );
	    log_string( log_buf );

	    /*
	     * Dying penalty:
	     * 1/2 way back to previous level.
	     */
	    if ( !chaos && victim->exp > 0 )
		gain_exp( victim, -1*(victim->exp/2) );
	}

       if (!IS_NPC(victim)) 
	 {
            sprintf(buf, "%s has been slain by %s!", victim->name, dam_name);
            do_sendinfo(victim, buf);
	 }
       
        if (chaos)
		chaos_kill( victim );
	else raw_kill( victim );
    }
   

    /*
     * Take care of link dead people.
     */
    if ( !IS_NPC(victim) && victim->desc == NULL )
    {
	if ( number_range( 0, victim->wait ) == 0 )
	{
	    do_recall( victim, "" );
	    return TRUE;
	}
    }

    /*
     * Wimp out?
     */

       if ( IS_NPC(victim) && dam > 0 && victim->wait < PULSE_VIOLENCE / 2)
	 {
	    if ( ( IS_SET(victim->act, ACT_WIMPY) && number_bits( 2 ) == 0
		  &&   victim->hit < victim->max_hit / 5) 
		||   ( IS_AFFECTED(victim, AFF_CHARM) && victim->master != NULL
		      &&     victim->master->in_room != victim->in_room ) )
	      do_flee( victim, "" );
	 }
       
       if ( !IS_NPC(victim)
	   &&   victim->hit > 0
	   &&   victim->hit <= victim->wimpy
	   &&   victim->wait < PULSE_VIOLENCE / 2 )
	 do_flee( victim, "" );
       
       tail_chain( );
       return TRUE;
}
