/* Code specifically for the new skill system */

#if defined(macintosh)
#include <types.h>
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "magic.h"
#include <math.h>

/* command procedures needed */
DECLARE_DO_FUN(do_groups	);
DECLARE_DO_FUN(do_help		);
DECLARE_DO_FUN(do_say		);


/* used to get new skills */
void do_gain(CHAR_DATA *ch, char *argument)
{
char arg[MAX_INPUT_LENGTH];
char buf[MAX_STRING_LENGTH];
   CHAR_DATA *trainer;
    int gn = 0, sn = 0;

    if (IS_NPC(ch))
	return;

    /* find a trainer */
    for ( trainer = ch->in_room->people; 
	  trainer != NULL; 
	  trainer = trainer->next_in_room)
	if (IS_NPC(trainer) && IS_SET(trainer->act,ACT_GAIN))
	    break;

    if (trainer == NULL || !can_see(ch,trainer))
    {
	send_to_char("You can't do that here.\n\r",ch);
	return;
    }

    one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	do_say(trainer,"Pardon me?");
	return;
    }

    if (!str_prefix(arg,"list"))
    {
	int col;
	
	col = 0;

	sprintf(buf, "%-18s %-5s %-18s %-5s %-18s %-5s\n\r",
	             "group","cost","group","cost","group","cost");
	send_to_char(buf,ch);

	for (gn = 0; gn < MAX_GROUP; gn++)
	{
	    if (group_table[gn].name == NULL)
		break;

	    if (!ch->pcdata->group_known[gn]
	    &&  group_table[gn].rating[ch->class] > 0)
	    {
		sprintf(buf,"%-18s %-5d ",
		    group_table[gn].name,group_table[gn].rating[ch->class]);
		send_to_char(buf,ch);
		if (++col % 3 == 0)
		    send_to_char("\n\r",ch);
	    }
	}
	if (col % 3 != 0)
	    send_to_char("\n\r",ch);
	
	send_to_char("\n\r",ch);		

	col = 0;

        sprintf(buf, "%-18s %-5s %-18s %-5s %-18s %-5s\n\r",
                     "skill","cost","skill","cost","skill","cost");
        send_to_char(buf,ch);
 
        for (sn = 0; sn < MAX_SKILL; sn++)
        {
            if (skill_table[sn].name == NULL)
                break;
 
            if (!ch->pcdata->learned[sn]
            &&  skill_table[sn].rating[ch->class] > 0
	    &&  skill_table[sn].spell_fun == spell_null)
            {
                sprintf(buf,"%-18s %-5d ",
                    skill_table[sn].name,skill_table[sn].rating[ch->class]);
                send_to_char(buf,ch);
                if (++col % 3 == 0)
                    send_to_char("\n\r",ch);
            }
        }
        if (col % 3 != 0)
            send_to_char("\n\r",ch);
	return;
    }

    if (!str_prefix(arg,"convert"))
    {
	if (ch->practice < 6)
	{
	    act("$N tells you 'You are not yet ready.'",
		ch,NULL,trainer,TO_CHAR);
	    return;
	}

	act("$N helps you apply your practice to training",
		ch,NULL,trainer,TO_CHAR);
	ch->practice -= 6;
	ch->train +=1 ;
	return;
    }

    if (!str_prefix(arg,"points"))
    {
	if (ch->train < 2)
	{
	    act("$N tells you 'You are not yet ready.'",
		ch,NULL,trainer,TO_CHAR);
	    return;
	}

	if (ch->pcdata->points <= 20)
	{
	    act("$N tells you 'There would be no point in that.'",
		ch,NULL,trainer,TO_CHAR);
	    return;
	}

	act("$N trains you, and you feel more at ease with your skills.",
	    ch,NULL,trainer,TO_CHAR);

	ch->train -= 2;
	ch->pcdata->points -= 1;
/* why? */
/*
	ch->exp = 0;
*/
	return;
    }

    /* else add a group/skill */

    gn = group_lookup(argument);
    if (gn > 0)
    {
	if (ch->pcdata->group_known[gn])
	{
	    act("$N tells you 'You already know that group!'",
		ch,NULL,trainer,TO_CHAR);
	    return;
	}

	if (group_table[gn].rating[ch->class] <= 0)
	{
	    act("$N tells you 'That group is beyond your powers.'",
		ch,NULL,trainer,TO_CHAR);
	    return;
	}

	if (ch->train < group_table[gn].rating[ch->class])
	{
	    act("$N tells you 'You are not yet ready for that group.'",
		ch,NULL,trainer,TO_CHAR);
	    return;
	}

	/* add the group */
	gn_add(ch,gn);
	act("$N trains you in the art of $t",
	    ch,group_table[gn].name,trainer,TO_CHAR);
	ch->train -= group_table[gn].rating[ch->class];
	return;
    }

    sn = skill_lookup(argument);
    if (sn > -1)
    {
	if (skill_table[sn].spell_fun != spell_null)
	{
	    act("$N tells you 'You must learn the full group.'",
		ch,NULL,trainer,TO_CHAR);
	    return;
	}
	    

        if (ch->pcdata->learned[sn])
        {
            act("$N tells you 'You already know that skill!'",
                ch,NULL,trainer,TO_CHAR);
            return;
        }
 
        if (skill_table[sn].rating[ch->class] <= 0)
        {
            act("$N tells you 'That skill is beyond your powers.'",
                ch,NULL,trainer,TO_CHAR);
            return;
        }
 
        if (ch->train < skill_table[sn].rating[ch->class])
        {
            act("$N tells you 'You are not yet ready for that skill.'",
                ch,NULL,trainer,TO_CHAR);
            return;
        }
 
        /* add the skill */
	ch->pcdata->learned[sn] = 1;
        act("$N trains you in the art of $t",
            ch,skill_table[sn].name,trainer,TO_CHAR);
        ch->train -= skill_table[sn].rating[ch->class];
        return;
    }

    act("$N tells you 'I do not understand...'",ch,NULL,trainer,TO_CHAR);
}
    



/* RT spells and skills show the players spells (or skills) */

void do_spells(CHAR_DATA *ch, char *argument)
{
char buf[MAX_STRING_LENGTH];
    char spell_list[LEVEL_HERO][MAX_STRING_LENGTH];
    char spell_columns[LEVEL_HERO];
    int sn,lev,mana;
    bool found = FALSE;

    if (IS_NPC(ch))
      return;

    /* initilize data */
    for (lev = 0; lev < LEVEL_HERO; lev++)
    {
	spell_columns[lev] = 0;
	spell_list[lev][0] = '\0';
    }
 
    for (sn = 0; sn < MAX_SKILL; sn++)
    {
      if (skill_table[sn].name == NULL)
        break;

      if (skill_table[sn].skill_level[ch->class] < LEVEL_HERO &&
	  skill_table[sn].spell_fun != spell_null &&
          ch->pcdata->learned[sn] > 0)
      {
	found = TRUE;
	lev = skill_table[sn].skill_level[ch->class];
	if (ch->level < lev)
	  sprintf(buf,"%-18s  n/a      ", skill_table[sn].name);
	else
	{
	  mana = UMAX(skill_table[sn].min_mana,
		      100/(2 + ch->level - lev));
	  sprintf(buf,"%-18s  %3d mana  ",skill_table[sn].name,mana);
	}
	
	if (spell_list[lev][0] == '\0')
	  sprintf(spell_list[lev],"\n\rLevel %2d: %s",lev,buf);
        else /* append */
	{
	  if ( ++spell_columns[lev] % 2 == 0)
            strcat(spell_list[lev],"\n\r          ");
	  strcat(spell_list[lev],buf);
        }
      }
    }

    /* return results */
 
    if (!found)
    {
      send_to_char("You know no spells.\n\r",ch);
      return;
    }
    
    for (lev = 0; lev < LEVEL_HERO; lev++)
      if (spell_list[lev][0] != '\0')
	send_to_char(spell_list[lev],ch);
    send_to_char("\n\r",ch);
}

void do_skills(CHAR_DATA *ch, char *argument)
{
char buf[MAX_STRING_LENGTH];
    char skill_list[LEVEL_HERO][MAX_STRING_LENGTH];
    char skill_columns[LEVEL_HERO];
    int sn,lev;
    bool found = FALSE;
 
    if (IS_NPC(ch))
      return;
 
    /* initilize data */
    for (lev = 0; lev < LEVEL_HERO; lev++)
    {
        skill_columns[lev] = 0;
        skill_list[lev][0] = '\0';
    }
 
    for (sn = 0; sn < MAX_SKILL; sn++)
    {
      if (skill_table[sn].name == NULL )
        break;

 
      if (skill_table[sn].skill_level[ch->class] < LEVEL_HERO &&
	  skill_table[sn].spell_fun == spell_null &&
	  ch->pcdata->learned[sn] > 0)
      {
        found = TRUE;
        lev = skill_table[sn].skill_level[ch->class];
        if (ch->level < lev)
          sprintf(buf,"%-18s n/a      ", skill_table[sn].name);
        else
          sprintf(buf,"%-18s %3d%%      ",skill_table[sn].name,
					 ch->pcdata->learned[sn]);
 
        if (skill_list[lev][0] == '\0')
          sprintf(skill_list[lev],"\n\rLevel %2d: %s",lev,buf);
        else /* append */
        {
          if ( ++skill_columns[lev] % 2 == 0)
            strcat(skill_list[lev],"\n\r          ");
          strcat(skill_list[lev],buf);
        }
      }
    }
 
    /* return results */
 
    if (!found)
    {
      send_to_char("You know no skills.\n\r",ch);
      return;
    }
 
    for (lev = 0; lev < LEVEL_HERO; lev++)
      if (skill_list[lev][0] != '\0')
        send_to_char(skill_list[lev],ch);
    send_to_char("\n\r",ch);
}


/* shows skills, groups and costs (only if not bought) */
void list_group_costs(CHAR_DATA *ch)
{
char buf[MAX_STRING_LENGTH];
    int gn,sn,col;

    if (IS_NPC(ch))
	return;

    col = 0;

    sprintf(buf,"%-18s %-5s %-18s %-5s %-18s %-5s\n\r","group","cp","group","cp","group","cp");
    send_to_char(buf,ch);

    for (gn = 0; gn < MAX_GROUP; gn++)
    {
	if (group_table[gn].name == NULL)
	    break;

        if (!ch->gen_data->group_chosen[gn] 
	&&  !ch->pcdata->group_known[gn]
	&&  group_table[gn].rating[ch->class] > 0)
	{
	    sprintf(buf,"%-18s %-5d ",group_table[gn].name,
				    group_table[gn].rating[ch->class]);
	    send_to_char(buf,ch);
	    if (++col % 3 == 0)
		send_to_char("\n\r",ch);
	}
    }
    if ( col % 3 != 0 )
        send_to_char( "\n\r", ch );
    send_to_char("\n\r",ch);

    col = 0;
 
    sprintf(buf,"%-18s %-5s %-18s %-5s %-18s %-5s\n\r","skill","cp","skill","cp","skill","cp");
    send_to_char(buf,ch);
 
    for (sn = 0; sn < MAX_SKILL; sn++)
    {
        if (skill_table[sn].name == NULL)
            break;
 
        if (!ch->gen_data->skill_chosen[sn] 
	&&  ch->pcdata->learned[sn] == 0
	&&  skill_table[sn].spell_fun == spell_null
	&&  skill_table[sn].rating[ch->class] > 0)
        {
            sprintf(buf,"%-18s %-5d ",skill_table[sn].name,
                                    skill_table[sn].rating[ch->class]);
            send_to_char(buf,ch);
            if (++col % 3 == 0)
                send_to_char("\n\r",ch);
        }
    }
    if ( col % 3 != 0 )
        send_to_char( "\n\r", ch );
    send_to_char("\n\r",ch);

    sprintf(buf,"Creation points: %d\n\r",ch->pcdata->points);
    send_to_char(buf,ch);
    sprintf(buf,"Experience modifier: %d%% (Percent of difference from the norm)\n\r",
	    figure_difference(ch->gen_data->points_chosen));
    send_to_char(buf,ch);
    return;
}


void list_group_chosen(CHAR_DATA *ch)
{
char buf[MAX_STRING_LENGTH];
    int gn,sn,col;
 
    if (IS_NPC(ch))
        return;
 
    col = 0;
 
    sprintf(buf,"%-18s %-5s %-18s %-5s %-18s %-5s","group","cp","group","cp","group","cp\n\r");
    send_to_char(buf,ch);
 
    for (gn = 0; gn < MAX_GROUP; gn++)
    {
        if (group_table[gn].name == NULL)
            break;
 
        if (ch->gen_data->group_chosen[gn] 
	&&  group_table[gn].rating[ch->class] > 0)
        {
            sprintf(buf,"%-18s %-5d ",group_table[gn].name,
                                    group_table[gn].rating[ch->class]);
            send_to_char(buf,ch);
            if (++col % 3 == 0)
                send_to_char("\n\r",ch);
        }
    }
    if ( col % 3 != 0 )
        send_to_char( "\n\r", ch );
    send_to_char("\n\r",ch);
 
    col = 0;
 
    sprintf(buf,"%-18s %-5s %-18s %-5s %-18s %-5s","skill","cp","skill","cp","skill","cp\n\r");
    send_to_char(buf,ch);
 
    for (sn = 0; sn < MAX_SKILL; sn++)
    {
        if (skill_table[sn].name == NULL)
            break;
 
        if (ch->gen_data->skill_chosen[sn] 
	&&  skill_table[sn].rating[ch->class] > 0)
        {
            sprintf(buf,"%-18s %-5d ",skill_table[sn].name,
                                    skill_table[sn].rating[ch->class]);
            send_to_char(buf,ch);
            if (++col % 3 == 0)
                send_to_char("\n\r",ch);
        }
    }
    if ( col % 3 != 0 )
        send_to_char( "\n\r", ch );
    send_to_char("\n\r",ch);
 
    sprintf(buf,"Creation points: %d\n\r",ch->gen_data->points_chosen);
    send_to_char(buf,ch);
    sprintf(buf,"Experience modifier: %d%% (Percent of difference from the norm)\n\r",
	    figure_difference(ch->gen_data->points_chosen));
    send_to_char(buf,ch);
    return;
}

long exp_per_level(CHAR_DATA *ch, int points)
{
    long expl=0;

    if (IS_NPC(ch))
	return 1000; 

    switch(ch->level) {
    	case 1: expl=750;
    		break;
    	case 2: expl=1500;
    		break;
    	case 3: expl=3000;
    		break;
    	case 4: expl=6000;
    		break;
    	case 5: expl=12000;
    		break;
    	case 6: expl=24000;
    		break;
    	case 7: expl=48000;
    		break;
    	case 8: expl=96000;
    		break;
    	case 9: expl=192000;
    		break;
    	case 10: expl=384000;
    		break;
    	case 11: expl=768000;
    		break;
    	case 12: expl=1536000;
    		break;
    	case 13: expl=1996800;
    		break;
    	case 14: expl=2595840;
    		break;
    	case 15: expl=3374590;
    		break;
    	case 16: expl=4386970;
    		break;
    	case 17: expl=5703060;
    		break;
    	case 18: expl=7413970;
    		break;
    	case 19: expl=9638170;
    		break;
    	case 20: expl=12529620;
    		break;
    	case 21: expl=16288510;
    		break;
    	case 22: expl=19546214;
    		break;
    	case 23: expl=23455450;
    		break;
    	case 24: expl=28146540;
    		break;
    	case 25: expl=33775850;
    		break;
    	case 26: expl=37153440;
    		break;
    	case 27: expl=40868780;
    		break;
    	case 28: expl=44955660;
    		break;
    	case 29: expl=49451230;
    		break;
    	case 30: expl=53523100;
    		break;
    	case 31: expl=57500000;
    		break;
    	case 32: expl=61506090;
    		break;
    	case 33: expl=65571300;
    		break;
    	case 34: expl=69696969;
    		break;
    	case 35: expl=74530130;
    		break;
    	case 36: expl=79590170;
    		break;
    	case 37: expl=84567800;
    		break;
    	case 38: expl=89012000;
    		break;
    	case 39: expl=94500600;
    		break;
    	case 40: expl=99503030;
    		break;
    	case 41: expl=104000000;
    		break;
    	case 42: expl=108000000;
    		break;
    	case 43: expl=112000000;
    		break;
    	case 44: expl=116000000;
    		break;
    	case 45: expl=120000000;
    		break;
    	case 46: expl=124000000;
    		break;
    	case 47: expl=128000000;
    		break;
    	case 48: expl=132000000;
    		break;
    	case 49: expl=136000000;
    		break;
    	case 50: expl=140000000;
    		break;
     	case 51: expl=150000000;
    		break;
    	case 52: expl=160000000;
    		break;
    	case 53: expl=170000000;
    		break;
    	case 54: expl=180000000;
    		break;
    	case 55: expl=190000000;
    		break;
    	case 56: expl=200000000;
    		break;
    	case 57: expl=210000000;
    		break;
    	case 58: expl=220000000;
    		break;
    	case 59: expl=230000000;
    		break;
    	case 60: expl=240000000;
    		break;

    	case 61: expl=250000000;
    		break;
    	case 62: expl=260000000;
    		break;
    	case 63: expl=270000000;
    		break;
    	case 64: expl=280000000;
    		break;
    	case 65: expl=290000000;
    		break;
    	case 66: expl=300000000;
    		break;
    	case 67: expl=310000000;
    		break;
    	case 68: expl=320000000;
    		break;
    	case 69: expl=330000000;
    		break;
    	case 70: expl=340000000;
    		break;
    	case 71: expl=350000000;
    		break;
    	case 72: expl=360000000;
    		break;
    	case 73: expl=370000000;
    		break;
    	case 74: expl=380000000;
    		break;
    	case 75: expl=390000000;
    		break;
    	case 76: expl=400000000;
    		break;
    	case 77: expl=410000000;
    		break;
    	case 78: expl=420000000;
    		break;
    	case 79: expl=430000000;
    		break;
    	case 80: expl=460000000;
    		break;
    	case 81: expl=480000000;
    		break;
    	case 82: expl=500000000;
    		break;
    	case 83: expl=520000000;
    		break;
    	case 84: expl=540000000;
    		break;
    	case 85: expl=560000000;
    		break;
    	case 86: expl=580000000;
    		break;
    	case 87: expl=600000000;
    		break;
    	case 88: expl=620000000;
    		break;
    	case 89: expl=640000000;
    		break;
    	case 90: expl=660000000;
    		break;
    	case 91: expl=700000000;
    		break;
    	case 92: expl=999999999;
    		break;
    	case 93: expl=999999999;
    		break;
    	case 94: expl=999999999;
    		break;
    	case 95: expl=999999999;
    		break;
    	case 96: expl=999999999;
    		break;
    	case 97: expl=999999999;
    		break;
    	case 98: expl=999999999;
    		break;
    	case 99: expl=999999999;
    		break;
    	case 100: expl=999999999;
    		break;
                                                                                                                                                                                                                                                               


                     }
/*    if (points > 110) expl=(long)pow((double)110,(double)1.2)*(expl/100); */
    if (points >= 28) expl=(long)pow((double)points,(double)1.2)*(expl/100);
/*    && (points <= 110)) */
    if (points < 28) expl*=((26.0+points)/100.0);
/*    if (points < 40)
	return 1000 * pc_race_table[ch->race].class_mult[ch->class]/100;

    / processing /
    points -= 40;

    while (points > 9)
    {
	expl += inc;
        points -= 10;
        if (points > 9)
	{
	    expl += inc;
	    inc *= 2;
	    points -= 10;
	}
    }

    expl += points * inc / 10;*/

    return expl * (pc_race_table[ch->race].class_mult[ch->class]/100);
}

/* this procedure handles the input parsing for the skill generator */
bool parse_gen_groups(CHAR_DATA *ch,char *argument)
{
char arg[MAX_INPUT_LENGTH];
char buf[MAX_STRING_LENGTH];
    int gn,sn,i;
 
    if (argument[0] == '\0')
	return FALSE;

    argument = one_argument(argument,arg);

    if (!str_prefix(arg,"help"))
    {
	if (argument[0] == '\0')
	{
	    do_help(ch,"group help");
	    return TRUE;
	}

        do_help(ch,argument);
	return TRUE;
    }

    if (!str_prefix(arg,"add"))
    {
	if (argument[0] == '\0')
	{
	    send_to_char("You must provide a skill name.\n\r",ch);
	    return TRUE;
	}

	gn = group_lookup(argument);
	if (gn != -1)
	{
	    if (ch->gen_data->group_chosen[gn]
	    ||  ch->pcdata->group_known[gn])
	    {
		send_to_char("You already know that group!\n\r",ch);
		return TRUE;
	    }

	    if (group_table[gn].rating[ch->class] < 1)
	    {
	  	send_to_char("That group is not available.\n\r",ch);
	 	return TRUE;
	    }

	    sprintf(buf,"%s group added\n\r",group_table[gn].name);
	    send_to_char(buf,ch);
	    ch->gen_data->group_chosen[gn] = TRUE;
	    ch->gen_data->points_chosen += group_table[gn].rating[ch->class];
	    gn_add(ch,gn);
	    ch->pcdata->points += group_table[gn].rating[ch->class];
	    return TRUE;
	}

	sn = skill_lookup(argument);
	if (sn != -1)
	{
	    if (ch->gen_data->skill_chosen[sn]
	    ||  ch->pcdata->learned[sn] > 0)
	    {
		send_to_char("You already know that skill!\n\r",ch);
		return TRUE;
	    }

	    if (skill_table[sn].rating[ch->class] < 1
	    ||  skill_table[sn].spell_fun != spell_null)
	    {
		send_to_char("That skill is not available.\n\r",ch);
		return TRUE;
	    }
	    sprintf(buf, "%s skill added\n\r",skill_table[sn].name);
	    send_to_char(buf,ch);
	    ch->gen_data->skill_chosen[sn] = TRUE;
	    ch->gen_data->points_chosen += skill_table[sn].rating[ch->class];
	    ch->pcdata->learned[sn] = 1;
	    ch->pcdata->points += skill_table[sn].rating[ch->class];
	    return TRUE;
	}

	send_to_char("No skills or groups by that name...\n\r",ch);
	return TRUE;
    }

    if (!strcmp(arg,"drop"))
    {
	if (argument[0] == '\0')
  	{
	    send_to_char("You must provide a skill to drop.\n\r",ch);
	    return TRUE;
	}

	gn = group_lookup(argument);
	if (gn != -1 && ch->gen_data->group_chosen[gn])
	{
	    send_to_char("Group dropped.\n\r",ch);
	    ch->gen_data->group_chosen[gn] = FALSE;
	    ch->gen_data->points_chosen -= group_table[gn].rating[ch->class];
	    gn_remove(ch,gn);
	    for (i = 0; i < MAX_GROUP; i++)
	    {
		if (ch->gen_data->group_chosen[gn])
		    gn_add(ch,gn);
	    }
	    ch->pcdata->points -= group_table[gn].rating[ch->class];
	    return TRUE;
	}

	sn = skill_lookup(argument);
	if (sn != -1 && ch->gen_data->skill_chosen[sn])
	{
	    send_to_char("Skill dropped.\n\r",ch);
	    ch->gen_data->skill_chosen[sn] = FALSE;
	    ch->gen_data->points_chosen -= skill_table[sn].rating[ch->class];
	    ch->pcdata->learned[sn] = 0;
	    ch->pcdata->points -= skill_table[sn].rating[ch->class];
	    return TRUE;
	}

	send_to_char("You haven't bought any such skill or group.\n\r",ch);
	return TRUE;
    }

    if (!str_prefix(arg,"premise"))
    {
	do_help(ch,"premise");
	return TRUE;
    }

    if (!str_prefix(arg,"list"))
    {
	list_group_costs(ch);
	return TRUE;
    }

    if (!str_prefix(arg,"learned"))
    {
	list_group_chosen(ch);
	return TRUE;
    }

    if (!str_prefix(arg,"info"))
    {
	do_groups(ch,argument);
	return TRUE;
    }

    return FALSE;
}
	    
	


        

/* shows all groups, or the sub-members of a group */
void do_groups(CHAR_DATA *ch, char *argument)
{
char buf[MAX_STRING_LENGTH];
    int gn,sn,col;

    if (IS_NPC(ch))
	return;

    col = 0;

    if (argument[0] == '\0')
    {   /* show all groups */
	
	for (gn = 0; gn < MAX_GROUP; gn++)
        {
	    if (group_table[gn].name == NULL)
		break;
	    if (ch->pcdata->group_known[gn])
	    {
		sprintf(buf,"%-20s ",group_table[gn].name);
		send_to_char(buf,ch);
		if (++col % 3 == 0)
		    send_to_char("\n\r",ch);
	    }
        }
        if ( col % 3 != 0 )
            send_to_char( "\n\r", ch );
        sprintf(buf,"Creation points: %d\n\r",ch->pcdata->points);
	send_to_char(buf,ch);
	return;
     }

     if (!str_cmp(argument,"all"))    /* show all groups */
     {
        for (gn = 0; gn < MAX_GROUP; gn++)
        {
            if (group_table[gn].name == NULL)
                break;
	    sprintf(buf,"%-20s ",group_table[gn].name);
            send_to_char(buf,ch);
	    if (++col % 3 == 0)
            	send_to_char("\n\r",ch);
        }
        if ( col % 3 != 0 )
            send_to_char( "\n\r", ch );
	return;
     }
	
     
     /* show the sub-members of a group */
     gn = group_lookup(argument);
     if (gn == -1)
     {
	send_to_char("No group of that name exist.\n\r",ch);
	send_to_char(
	    "Type 'groups all' or 'info all' for a full listing.\n\r",ch);
	return;
     }

     for (sn = 0; sn < MAX_IN_GROUP; sn++)
     {
	if (group_table[gn].spells[sn] == NULL)
	    break;
	sprintf(buf,"%-20s ",group_table[gn].spells[sn]);
	send_to_char(buf,ch);
	if (++col % 3 == 0)
	    send_to_char("\n\r",ch);
     }
    if ( col % 3 != 0 )
        send_to_char( "\n\r", ch );
}

/* checks for skill improvement */
void check_improve( CHAR_DATA *ch, int sn, bool success, int multiplier )
{
char buf[MAX_STRING_LENGTH];
    int chance;

    if (IS_NPC(ch))
	return;

    if (ch->level < skill_table[sn].skill_level[ch->class]
    ||  skill_table[sn].rating[ch->class] == 0
    ||  ch->pcdata->learned[sn] == 0
    ||  ch->pcdata->learned[sn] == 100)
	return;  /* skill is not known */ 

    /* check to see if the character has a chance to learn */
    chance = 10 * int_app[get_curr_stat(ch,STAT_INT)].learn;
    chance /= (		multiplier
		*	skill_table[sn].rating[ch->class] 
		*	4);
    chance += ch->level;

    if (number_range(1,1000) > chance)
	return;

    /* now that the character has a CHANCE to learn, see if they really have */	

    if (success)
    {
	chance = URANGE(5,100 - ch->pcdata->learned[sn], 95);
	if (number_percent() < chance)
	{
	    sprintf(buf,"You have become better at %s!\n\r",
		    skill_table[sn].name);
	    send_to_char(buf,ch);
	    ch->pcdata->learned[sn]++;
	    gain_exp(ch,2 * skill_table[sn].rating[ch->class]);
	}
    }

    else
    {
	chance = URANGE(5,ch->pcdata->learned[sn]/2,30);
	if (number_percent() < chance)
	{
	    sprintf(buf,
		"You learn from your mistakes, and your %s skill improves.\n\r",
		skill_table[sn].name);
	    send_to_char(buf,ch);
	    ch->pcdata->learned[sn] += number_range(1,3);
	    ch->pcdata->learned[sn] = UMIN(ch->pcdata->learned[sn],100);
	    gain_exp(ch,2 * skill_table[sn].rating[ch->class]);
	}
    }
}

/* returns a group index number given the name */
int group_lookup( const char *name )
{
    int gn;
 
    for ( gn = 0; gn < MAX_GROUP; gn++ )
    {
        if ( group_table[gn].name == NULL )
            break;
        if ( LOWER(name[0]) == LOWER(group_table[gn].name[0])
        &&   !str_prefix( name, group_table[gn].name ) )
            return gn;
    }
 
    return -1;
}

/* recursively adds a group given its number -- uses group_add */
void gn_add( CHAR_DATA *ch, int gn)
{
    int i;
    
    ch->pcdata->group_known[gn] = TRUE;
    for ( i = 0; i < MAX_IN_GROUP; i++)
    {
        if (group_table[gn].spells[i] == NULL)
            break;
        group_add(ch,group_table[gn].spells[i],FALSE);
    }
}

/* recusively removes a group given its number -- uses group_remove */
void gn_remove( CHAR_DATA *ch, int gn)
{
    int i;

    ch->pcdata->group_known[gn] = FALSE;

    for ( i = 0; i < MAX_IN_GROUP; i ++)
    {
	if (group_table[gn].spells[i] == NULL)
	    break;
	group_remove(ch,group_table[gn].spells[i]);
    }
}
	
/* use for processing a skill or group for addition  */
void group_add( CHAR_DATA *ch, const char *name, bool deduct)
{
    int sn,gn;

    if (IS_NPC(ch)) /* NPCs do not have skills */
	return;

    sn = skill_lookup(name);

    if (sn != -1)
    {
	if (ch->pcdata->learned[sn] == 0) /* i.e. not known */
	{
	    ch->pcdata->learned[sn] = 1;
	    if (deduct)
	   	ch->pcdata->points += skill_table[sn].rating[ch->class]; 
	}
	return;
    }
	
    /* now check groups */

    gn = group_lookup(name);

    if (gn != -1)
    {
	if (ch->pcdata->group_known[gn] == FALSE)  
	{
	    ch->pcdata->group_known[gn] = TRUE;
	    if (deduct)
		ch->pcdata->points += group_table[gn].rating[ch->class];
	}
	gn_add(ch,gn); /* make sure all skills in the group are known */
    }
}

/* used for processing a skill or group for deletion -- no points back! */

void group_remove(CHAR_DATA *ch, const char *name)
{
    int sn, gn;
    
     sn = skill_lookup(name);

    if (sn != -1)
    {
	ch->pcdata->learned[sn] = 0;
	return;
    }
 
    /* now check groups */
 
    gn = group_lookup(name);
 
    if (gn != -1 && ch->pcdata->group_known[gn] == TRUE)
    {
	ch->pcdata->group_known[gn] = FALSE;
	gn_remove(ch,gn);  /* be sure to call gn_add on all remaining groups */
    }
}
