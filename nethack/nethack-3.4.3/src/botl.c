/*	SCCS Id: @(#)botl.c	3.4	1996/07/15	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#ifdef CONF_BOTL
# include <ctype.h>
#endif

#ifdef OVL0
extern const char *hu_stat[];	/* defined in eat.c */

const char * const enc_stat[] = {
	"",
	"Burdened",
	"Stressed",
	"Strained",
	"Overtaxed",
	"Overloaded"
};

#ifndef CONF_BOTL
STATIC_DCL void NDECL(bot1);
STATIC_DCL void NDECL(bot2);
#endif /* !CONF_BOTL */
#endif /* OVL0 */

/* MAXCO must hold longest uncompressed status line, and must be larger
 * than COLNO
 *
 * longest practical second status line at the moment is
 *	Astral Plane $:12345 HP:700(700) Pw:111(111) AC:-127 Xp:30/123456789
 *	T:123456 Satiated Conf FoodPois Ill Blind Stun Hallu Overloaded
 * -- or somewhat over 130 characters
 */
#if COLNO <= 140
#define MAXCO 160
#else
#define MAXCO (COLNO+20)
#endif

#if defined(STATUS_COLORS) && defined(TEXTCOLOR)

extern const struct percent_color_option *hp_colors;
extern const struct percent_color_option *pw_colors;
extern const struct text_color_option *text_colors;

struct color_option
text_color_of(text, color_options)
const char *text;
const struct text_color_option *color_options;
{
	if (color_options == NULL) {
		struct color_option result = {NO_COLOR, 0};
		return result;
	}
	if (strstri(color_options->text, text)
	 || strstri(text, color_options->text))
		return color_options->color_option;
	return text_color_of(text, color_options->next);
}

struct color_option
percentage_color_of(value, max, color_options)
int value, max;
const struct percent_color_option *color_options;
{
	if (color_options == NULL) {
		struct color_option result = {NO_COLOR, 0};
		return result;
	}
	if (100 * value <= color_options->percentage * max)
		return color_options->color_option;
	return percentage_color_of(value, max, color_options->next);
}

void
start_color_option(color_option)
struct color_option color_option;
{
	int i;
	if (color_option.color != NO_COLOR)
		term_start_color(color_option.color);
	for (i = 0; (1 << i) <= color_option.attr_bits; ++i)
		if (i != ATR_NONE && color_option.attr_bits & (1 << i))
			term_start_attr(i);
}

void
end_color_option(color_option)
struct color_option color_option;
{
	int i;
	if (color_option.color != NO_COLOR)
		term_end_color(color_option.color);
	for (i = 0; (1 << i) <= color_option.attr_bits; ++i)
		if (i != ATR_NONE && color_option.attr_bits & (1 << i))
			term_end_attr(i);
}

void
apply_color_option(color_option, newbot2)
struct color_option color_option;
const char *newbot2;
{
	if (!iflags.use_status_colors) return;
	curs(WIN_STATUS, 1, 1);
	start_color_option(color_option);
	putstr(WIN_STATUS, 0, newbot2);
	end_color_option(color_option);
}

void
add_colored_text(text, newbot2)
const char *text;
char *newbot2;
{
	char *nb;
	struct color_option color_option;

	if (*text == '\0') return;

	if (!iflags.use_status_colors) {
		Sprintf(nb = eos(newbot2), " %s", text);
                return;
        }

	Strcat(nb = eos(newbot2), " ");
	curs(WIN_STATUS, 1, 1);
	putstr(WIN_STATUS, 0, newbot2);

	Strcat(nb = eos(nb), text);
	curs(WIN_STATUS, 1, 1);
       	color_option = text_color_of(text, text_colors);
	start_color_option(color_option);
	putstr(WIN_STATUS, 0, newbot2);
	end_color_option(color_option);
}

#endif

#ifndef OVLB
STATIC_DCL int mrank_sz;
#else /* OVLB */
STATIC_OVL NEARDATA int mrank_sz = 0; /* loaded by max_rank_sz (from u_init) */
#endif /* OVLB */

STATIC_DCL const char *NDECL(rank);

#ifdef OVL1

#ifdef CONF_BOTL
#define BOTLPARMTYP_TXT 0 /* printable text           eg. "HP: "  */
#define BOTLPARMTYP_VAR 1 /* variable and formatting  eg. "%-2.5a" */
#define BOTLPARMTYP_CMD 2 /* command          eg. "%{_blue&blink}" */

struct botlparam {
   struct botlparam *nextparam;
   xchar bptyp;  /* one of BOTLPARMTYP_foo */
   char valuechar; /* the variable */
   char *params;         /* variable formatting string */
   int parami1, parami2;
};

#define MAX_BOTLINES 2

static struct botlparam *botlbase[MAX_BOTLINES] = { NULL, NULL };

static const struct {
   const char *name;
   const int color;
} colornames[] = {
   {"black", CLR_BLACK},
   {"red", CLR_RED},
   {"green", CLR_GREEN},
   {"brown", CLR_BROWN},
   {"blue", CLR_BLUE},
   {"magenta", CLR_MAGENTA},
   {"cyan", CLR_CYAN},
   {"gray", CLR_GRAY},
   {"orange", CLR_ORANGE},
   {"lightgreen", CLR_BRIGHT_GREEN},
   {"yellow", CLR_YELLOW},
   {"lightblue", CLR_BRIGHT_BLUE},
   {"lightmagenta", CLR_BRIGHT_MAGENTA},
   {"lightcyan", CLR_BRIGHT_CYAN},
   {"white", CLR_WHITE}
};

static const struct {
   const char *name;
   const int attr;
} attrnames[] = {
     {"none", ATR_NONE},
     {"bold", ATR_BOLD},
     {"dim", ATR_DIM},
     {"underline", ATR_ULINE},
     {"blink", ATR_BLINK},
     {"inverse", ATR_INVERSE}

};
#endif /* CONF_BOTL */

/* convert experience level (1..30) to rank index (0..8) */
int
xlev_to_rank(xlev)
int xlev;
{
	return (xlev <= 2) ? 0 : (xlev <= 30) ? ((xlev + 2) / 4) : 8;
}

#if 0	/* not currently needed */
/* convert rank index (0..8) to experience level (1..30) */
int
rank_to_xlev(rank)
int rank;
{
	return (rank <= 0) ? 1 : (rank <= 8) ? ((rank * 4) - 2) : 30;
}
#endif

const char *
rank_of(lev, monnum, female)
	int lev;
	short monnum;
	boolean female;
{
	register struct Role *role;
	register int i;


	/* Find the role */
	for (role = (struct Role *) roles; role->name.m; role++)
	    if (monnum == role->malenum || monnum == role->femalenum)
	    	break;
	if (!role->name.m)
	    role = &urole;

	/* Find the rank */
	for (i = xlev_to_rank((int)lev); i >= 0; i--) {
	    if (female && role->rank[i].f) return (role->rank[i].f);
	    if (role->rank[i].m) return (role->rank[i].m);
	}

const char *
encstat()
{
    int cap = near_capacity();
    if(cap > UNENCUMBERED)
	return enc_stat[cap];
    else return "";
}

const char *
hunstat()
{
    if (strcmp(hu_stat[u.uhs], "        "))
	return hu_stat[u.uhs];
    else return "";
}

	/* Try the role name, instead */
	if (female && role->name.f) return (role->name.f);
	else if (role->name.m) return (role->name.m);
	return ("Player");
}


STATIC_OVL const char *
rank()
{
	return(rank_of(u.ulevel, Role_switch, flags.female));
}

int
title_to_mon(str, rank_indx, title_length)
const char *str;

#ifdef REALTIME_ON_BOTL
  if(iflags.showrealtime) {
    time_t currenttime = get_realtime();
    Sprintf(nb = eos(nb), " %d:%2.2d", currenttime / 3600, 
                                       (currenttime % 3600) / 60);
  }
#endif

int *rank_indx, *title_length;
{
	register int i, j;


	/* Loop through each of the roles */
	for (i = 0; roles[i].name.m; i++)
	    for (j = 0; j < 9; j++) {
	    	if (roles[i].rank[j].m && !strncmpi(str,
	    			roles[i].rank[j].m, strlen(roles[i].rank[j].m))) {
	    	    if (rank_indx) *rank_indx = j;
	    	    if (title_length) *title_length = strlen(roles[i].rank[j].m);
	    	    return roles[i].malenum;
	    	}
	    	if (roles[i].rank[j].f && !strncmpi(str,
	    			roles[i].rank[j].f, strlen(roles[i].rank[j].f))) {
	    	    if (rank_indx) *rank_indx = j;
	    	    if (title_length) *title_length = strlen(roles[i].rank[j].f);
	    	    return ((roles[i].femalenum != NON_PM) ?
	    	    		roles[i].femalenum : roles[i].malenum);
	    	}
	    }
	return NON_PM;
}

#endif /* OVL1 */
#ifdef OVLB

void
max_rank_sz()
{
	register int i, r, maxr = 0;
	for (i = 0; i < 9; i++) {
	    if (urole.rank[i].m && (r = strlen(urole.rank[i].m)) > maxr) maxr = r;
	    if (urole.rank[i].f && (r = strlen(urole.rank[i].f)) > maxr) maxr = r;
	}
	mrank_sz = maxr;
	return;
}

#endif /* OVLB */
#ifdef OVL0

#ifdef SCORE_ON_BOTL
long
botl_score()
{
    int deepest = deepest_lev_reached(FALSE);
#ifndef GOLDOBJ
    long ugold = u.ugold + hidden_gold();

    if ((ugold -= u.ugold0) < 0L) ugold = 0L;
    return ugold + u.urexp + (long)(50 * (deepest - 1))
#else
    long umoney = money_cnt(invent) + hidden_gold();

    if ((umoney -= u.umoney0) < 0L) umoney = 0L;
    return umoney + u.urexp + (long)(50 * (deepest - 1))
#endif
			  + (long)(deepest > 30 ? 10000 :
				   deepest > 20 ? 1000*(deepest - 20) : 0);
}
#endif

#ifndef CONF_BOTL
STATIC_OVL void
bot1()
{
	char newbot1[MAXCO];
	register char *nb;
	register int i,j;

	Strcpy(newbot1, plname);
	if('a' <= newbot1[0] && newbot1[0] <= 'z') newbot1[0] += 'A'-'a';
	newbot1[10] = 0;
	Sprintf(nb = eos(newbot1)," the ");

	if (Upolyd) {
		char mbot[BUFSZ];
		int k = 0;

		Strcpy(mbot, mons[u.umonnum].mname);
		while(mbot[k] != 0) {
		    if ((k == 0 || (k > 0 && mbot[k-1] == ' ')) &&
					'a' <= mbot[k] && mbot[k] <= 'z')
			mbot[k] += 'A' - 'a';
		    k++;
		}
		Sprintf(nb = eos(nb), mbot);
	} else
		Sprintf(nb = eos(nb), rank());

	Sprintf(nb = eos(nb),"  ");
	i = mrank_sz + 15;
	j = (nb + 2) - newbot1; /* aka strlen(newbot1) but less computation */
	if((i - j) > 0)
		Sprintf(nb = eos(nb),"%*s", i-j, " ");	/* pad with spaces */
	if (ACURR(A_STR) > 18) {
		if (ACURR(A_STR) > STR18(100))
		    Sprintf(nb = eos(nb),"St:%2d ",ACURR(A_STR)-100);
		else if (ACURR(A_STR) < STR18(100))
		    Sprintf(nb = eos(nb), "St:18/%02d ",ACURR(A_STR)-18);
		else
		    Sprintf(nb = eos(nb),"St:18/** ");
	} else
		Sprintf(nb = eos(nb), "St:%-1d ",ACURR(A_STR));
	Sprintf(nb = eos(nb),
		"Dx:%-1d Co:%-1d In:%-1d Wi:%-1d Ch:%-1d",
		ACURR(A_DEX), ACURR(A_CON), ACURR(A_INT), ACURR(A_WIS), ACURR(A_CHA));
	Sprintf(nb = eos(nb), (u.ualign.type == A_CHAOTIC) ? "  Chaotic" :
			(u.ualign.type == A_NEUTRAL) ? "  Neutral" : "  Lawful");
#ifdef SCORE_ON_BOTL
	if (flags.showscore)
	    Sprintf(nb = eos(nb), " S:%ld", botl_score());
#endif
	curs(WIN_STATUS, 1, 0);
	putstr(WIN_STATUS, 0, newbot1);
}
#endif /* !CONF_BOTL */

/* provide the name of the current level for display by various ports */
int
describe_level(buf)
char *buf;
{
	int ret = 1;

	/* TODO:	Add in dungeon name */
	if (Is_knox(&u.uz))
#ifndef CONF_BOTL
		Sprintf(buf, "%s ", dungeons[u.uz.dnum].dname);
#else
		Sprintf(buf, "%s", dungeons[u.uz.dnum].dname);
#endif
	else if (In_quest(&u.uz))
#ifndef CONF_BOTL
		Sprintf(buf, "Home %d ", dunlev(&u.uz));
#else
		Sprintf(buf, "Home %d", dunlev(&u.uz));
#endif
	else if (In_endgame(&u.uz))
		Sprintf(buf,
#ifndef CONF_BOTL
			Is_astralevel(&u.uz) ? "Astral Plane " : "End Game ");
#else
			Is_astralevel(&u.uz) ? "Astral Plane" : "End Game");
#endif
	else {
		/* ports with more room may expand this one */
#ifndef CONF_BOTL
		Sprintf(buf, "Dlvl:%-2d ", depth(&u.uz));
#else
		Sprintf(buf, "Dlvl:%-2d", depth(&u.uz));
#endif
		ret = 0;
	}
	return ret;
}

#ifndef CONF_BOTL
STATIC_OVL void
bot2()
{
	char  newbot2[MAXCO];
	register char *nb;
	int hp, hpmax;
	int cap = near_capacity();
#if defined(STATUS_COLORS) && defined(TEXTCOLOR)
	struct color_option color_option;
	int save_botlx = flags.botlx;
#endif

	hp = Upolyd ? u.mh : u.uhp;
	hpmax = Upolyd ? u.mhmax : u.uhpmax;

	if(hp < 0) hp = 0;
	(void) describe_level(newbot2);
	Sprintf(nb = eos(newbot2), "%c:%-2ld", oc_syms[COIN_CLASS],
#ifndef GOLDOBJ
		u.ugold
#else
		money_cnt(invent)
#endif
	       );

#if defined(STATUS_COLORS) && defined(TEXTCOLOR)
	Strcat(nb = eos(newbot2), " HP:");
	curs(WIN_STATUS, 1, 1);
	putstr(WIN_STATUS, 0, newbot2);
	flags.botlx = 0;

	Sprintf(nb = eos(nb), "%d(%d)", hp, hpmax);
	apply_color_option(percentage_color_of(hp, hpmax, hp_colors), newbot2);
#else
	Sprintf(nb = eos(nb), " HP:%d(%d)", hp, hpmax);
#endif
#if defined(STATUS_COLORS) && defined(TEXTCOLOR)
	Strcat(nb = eos(nb), " Pw:");
	curs(WIN_STATUS, 1, 1);
	putstr(WIN_STATUS, 0, newbot2);

	Sprintf(nb = eos(nb), "%d(%d)", u.uen, u.uenmax);
	apply_color_option(percentage_color_of(u.uen, u.uenmax, pw_colors), newbot2);
#else
	Sprintf(nb = eos(nb), " Pw:%d(%d)", u.uen, u.uenmax);
#endif
	Sprintf(nb = eos(nb), " AC:%-2d", u.uac);
	if (Upolyd)
		Sprintf(nb = eos(nb), " HD:%d", mons[u.umonnum].mlevel);
#ifdef EXP_ON_BOTL
	else if(flags.showexp)
		Sprintf(nb = eos(nb), " Xp:%u/%-1ld", u.ulevel,u.uexp);
#endif
	else
		Sprintf(nb = eos(nb), " Exp:%u", u.ulevel);

	if(flags.time)
	    Sprintf(nb = eos(nb), " T:%ld", moves);
	if(strcmp(hu_stat[u.uhs], "        "))
#if defined(STATUS_COLORS) && defined(TEXTCOLOR)
	     	add_colored_text(hu_stat[u.uhs], newbot2);
#else
		Sprintf(nb = eos(nb), " %s", hu_stat[u.uhs]);
#endif
	if(Confusion)
#if defined(STATUS_COLORS) && defined(TEXTCOLOR)
	     	add_colored_text("Conf", newbot2);
#else
		Strcat(nb = eos(nb), " Conf");
#endif
	if(Sick) {
		if (u.usick_type & SICK_VOMITABLE)
#if defined(STATUS_COLORS) && defined(TEXTCOLOR)
			add_colored_text("FoodPois", newbot2);
#else
			Strcat(nb = eos(nb), " FoodPois");
#endif
		if (u.usick_type & SICK_NONVOMITABLE)
#if defined(STATUS_COLORS) && defined(TEXTCOLOR)
			add_colored_text("Ill", newbot2);
#else
			Strcat(nb = eos(nb), " Ill");
#endif
	}
	if(Blind)
#if defined(STATUS_COLORS) && defined(TEXTCOLOR)
	     	add_colored_text("Blind", newbot2);
#else
		Strcat(nb = eos(nb), " Blind");
#endif
	if(Stunned)
#if defined(STATUS_COLORS) && defined(TEXTCOLOR)
	     	add_colored_text("Stun", newbot2);
#else
		Strcat(nb = eos(nb), " Stun");
#endif
	if(Hallucination)
#if defined(STATUS_COLORS) && defined(TEXTCOLOR)
	     	add_colored_text("Hallu", newbot2);
#else
		Strcat(nb = eos(nb), " Hallu");
#endif
	if(Slimed)
#if defined(STATUS_COLORS) && defined(TEXTCOLOR)
	     	add_colored_text("Slime", newbot2);
#else
		Strcat(nb = eos(nb), " Slime");
#endif
	if(cap > UNENCUMBERED)
#if defined(STATUS_COLORS) && defined(TEXTCOLOR)
		add_colored_text(enc_stat[cap], newbot2);
#else
		Sprintf(nb = eos(nb), " %s", enc_stat[cap]);
#endif
	curs(WIN_STATUS, 1, 1);
	putstr(WIN_STATUS, 0, newbot2);
	flags.botlx = save_botlx;
}

void
bot()
{
	bot1();
	bot2();
	flags.botl = flags.botlx = 0;
}

#else /* CONF_BOTL is defined */

/* Deallocates memory used by botlbase[botlnum] */
void 
deallocbotlparam(botlnum)
  const int botlnum;
{
   if ((botlnum >= 0) && (botlnum < MAX_BOTLINES)) {
      struct botlparam *botlp = 0;
      struct botlparam *tmpbotlp = botlbase[botlnum];
      while (tmpbotlp) {
	 botlp = tmpbotlp;
	 tmpbotlp = tmpbotlp->nextparam;
	 free(botlp->params);
	 free(botlp);
      }
      botlbase[botlnum] = 0;
   }
}

/* Frees all the memory taken by botlines */
void
free_botlines()
{
   int line;

   for (line = 0; line < MAX_BOTLINES; line++) 
     deallocbotlparam(line);
}
 
/* flips the chain in botlbase[basenum], because parsebotl() inserts
   the params in wrong order into the chain */
void
flipbotlbase(basenum)
int basenum;
{
   struct botlparam *b1 = 0;
   
   if ((basenum < 0) || (basenum >= MAX_BOTLINES)) return;

   while (botlbase[basenum]) {
      struct botlparam *tmpb = botlbase[basenum]->nextparam;
      botlbase[basenum]->nextparam = b1;
      b1 = botlbase[basenum];
      botlbase[basenum] = tmpb;
   }
   botlbase[basenum] = b1;
}

/* Returns a string variable depending on valchar */
char *
botlvaluechar(valchar, buf)
const char valchar;
char *buf;
{
   int hp = Upolyd ? u.mh : u.uhp;
   int hpmax = Upolyd ? u.mhmax : u.uhpmax;
   int cap = near_capacity();

   buf[0] = '\0';
   
   switch (valchar) {
    case '%': /* percent sign */
        Strcpy(buf, "%");
        break;
    case '$': /* gold symbol */
        Sprintf(buf, "%c", oc_syms[COIN_CLASS]);
	break;
    case 'a': /* Player's name */
  	Strcpy(buf, plname);
	if('a' <= buf[0] && buf[0] <= 'z') buf[0] += 'A'-'a';
        break;
    case 'b': /* Player's rank */
    	if (Upolyd) {
	   char mbot[BUFSZ];
	   int k = 0;

	   Strcpy(mbot, mons[u.umonnum].mname);
	   while(mbot[k] != 0) {
	      if ((k == 0 || (k > 0 && mbot[k-1] == ' ')) &&
		  'a' <= mbot[k] && mbot[k] <= 'z')
		mbot[k] += 'A' - 'a';
	      k++;
	   }
	   Sprintf(buf, "%s", mbot);
	} else Sprintf(buf, "%s", rank());
        break;
    case 'c': /* current Str */
      	if (ACURR(A_STR) > 18) {
	   if (ACURR(A_STR) > STR18(100))
	     Sprintf(buf,"%2d",ACURR(A_STR)-100);
	   else if (ACURR(A_STR) < STR18(100))
	     Sprintf(buf, "18/%02d",ACURR(A_STR)-18);
	   else
	     Sprintf(buf,"18/**");
	} else Sprintf(buf, "%-1d",ACURR(A_STR));
        break;
    case 'd': /* current Dex */ 
        Sprintf(buf, "%-1d", ACURR(A_DEX));
        break;
    case 'e': /* current Con */ 
        Sprintf(buf, "%-1d", ACURR(A_CON));
        break;
    case 'f': /* current Int */ 
        Sprintf(buf, "%-1d", ACURR(A_INT));
        break;
    case 'g': /* current Wis */ 
        Sprintf(buf, "%-1d", ACURR(A_WIS));
        break;
    case 'h': /* current Cha */ 
        Sprintf(buf, "%-1d", ACURR(A_CHA));
        break;  
    case 'i': /* current Alignment */
	Sprintf(buf, (u.ualign.type == A_CHAOTIC) ? "Chaotic" :
			(u.ualign.type == A_NEUTRAL) ? "Neutral" : "Lawful");
        break;
    case 'j': /* dungeon depth */
        Sprintf(buf, "%-1d", depth(&u.uz));
        break;
    case 'k': /* dungeon name */
        (void) describe_level(buf);
        break;
    case 'l': /* gold */
        Sprintf(buf, "%-1ld",
#ifndef GOLDOBJ
		u.ugold
#else
		money_cnt(invent)
#endif
		);
        break;
    case 'm': /* max HP */
        Sprintf(buf, "%d", hpmax);
        break;
    case 'n': /* current HP */
	if (hp < 0) hp = 0;
        Sprintf(buf, "%d", hp);
        break;
    case 'o': /* max power */
        Sprintf(buf, "%d", u.uenmax);
        break;
    case 'p': /* current power */
        Sprintf(buf, "%d", u.uen);
        break;
    case 'q': /* AC */
        Sprintf(buf, "%-1d", u.uac);
        break;  
    case 'r': /* experience level */
      	if (Upolyd) Sprintf(buf, "%d", mons[u.umonnum].mlevel);
#ifdef EXP_ON_BOTL
	else if(flags.showexp) Sprintf(buf, "%u", u.ulevel);
#endif
	else Sprintf(buf, "%u", u.ulevel);
        break;
#ifdef EXP_ON_BOTL
    case 's': /* experience points */
	if(flags.showexp) Sprintf(buf, "%-1ld", u.uexp);
        break;
#endif
    case 't': /* full experience string */
 	if (Upolyd) Sprintf(buf, "HD:%d", mons[u.umonnum].mlevel);
 #ifdef EXP_ON_BOTL
 	else if(flags.showexp) Sprintf(buf, "Xp:%u/%-1ld", u.ulevel,u.uexp);
 #endif
 	else Sprintf(buf, "Exp:%u", u.ulevel);
        break;    
    case 'u': /* time value */
	if(flags.time) Sprintf(buf, "%ld", moves);
        break;
    case 'v': /* full time string */
	if(flags.time) Sprintf(buf, "T:%ld", moves);
        break;
    case 'w': /* all misc. flags */
        if (strcmp(hu_stat[u.uhs], "        "))
	    Sprintf(buf, "%s", hu_stat[u.uhs]);
	if(Confusion)	   Strcat(buf, " Conf");
 	if(Sick) {
	   if (u.usick_type & SICK_VOMITABLE)    Strcat(buf, " FoodPois");
	   if (u.usick_type & SICK_NONVOMITABLE) Strcat(buf, " Ill");
 	}
	if(Blind)	   Strcat(buf, " Blind");
	if(Stunned)	   Strcat(buf, " Stun");
	if(Hallucination)  Strcat(buf, " Hallu");
	if(Slimed)         Strcat(buf, " Slime");
	if(cap > UNENCUMBERED) {
	   Strcat(buf, " ");
	   Strcat(buf, enc_stat[cap]);
	}
        break;
#ifdef SCORE_ON_BOTL
    case 'x': /* score value */
	if (flags.showscore) Sprintf(buf, "%ld", botl_score());
        break;
    case 'y': /* full score string */
	if (flags.showscore) Sprintf(buf, "S:%ld", botl_score());
        break;
#endif
    case 'Z': /* Hunger Status */
	if (strcmp(hu_stat[u.uhs], "        "))	
	    Sprintf(buf, "%s", hu_stat[u.uhs]);
        break;
    case 'Y': /* Confusion status */
	if (Confusion) Sprintf(buf, "%s", "Conf");
        break;
    case 'X': /* FoodPois status */
	if ((Sick) && (u.usick_type & SICK_VOMITABLE)) 
	   Sprintf(buf, "%s", "FoodPois");
        break;
    case 'W': /*Ill status */
	if ((Sick) && (u.usick_type & SICK_NONVOMITABLE)) 
	   Sprintf(buf, "%s", "Ill");
        break;
    case 'V': /* Blind status */
	if (Blind) Sprintf(buf, "%s", "Blind");
        break;
    case 'U': /* Stun status */
	if (Stunned) Sprintf(buf, "%s", "Stun");
        break;
    case 'T': /* Hallu status */
	if (Hallucination) Sprintf(buf, "%s", "Hallu");
        break;
    case 'S': /* Slimed status */
	if (Slimed) Sprintf(buf, "%s", "Slime");
        break;
    case 'R': /* Encumbrance status */
	if (cap>UNENCUMBERED) Sprintf(buf, "%s", enc_stat[cap]);
        break;    
    default:;
   }
   return buf;
}

/* Shows the botl on screen */
void
showbotl(botlnum)
const int botlnum;
{
    char tmpstr[BUFSZ], tmpbuf[BUFSZ], buf[BUFSZ];
    struct botlparam *tmpparm;
    int x = 1;
    int color = NO_COLOR, attr = ATR_NONE;
    int do_color_slide = -1;

    if ((botlnum < 0) || (botlnum >= MAX_BOTLINES)) return;

    tmpparm = botlbase[botlnum];

    while (tmpparm) {
	tmpbuf[0] = tmpstr[0] = buf[0] = '\0';

	switch (tmpparm->bptyp) {
	    case BOTLPARMTYP_TXT: {
		curs(WIN_STATUS, x, botlnum);
		putstr(WIN_STATUS, 0, tmpparm->params);
		x += strlen(tmpparm->params);
		break;
	    }
	    case BOTLPARMTYP_VAR: {
		Sprintf(tmpstr, "%%%ss", tmpparm->params);
		Sprintf(buf, tmpstr, botlvaluechar(tmpparm->valuechar, tmpbuf));
		curs(WIN_STATUS, x, botlnum);
		putstr(WIN_STATUS, 0, buf);
		x += strlen(buf);
		break;
	    }
	    case BOTLPARMTYP_CMD: {
		switch (tmpparm->valuechar) {
		    case 'c': { /* Set color and attribute */
			if (iflags.wc_color && (color != NO_COLOR)) 
			  term_end_color();
			term_end_attr(attr);

			if (iflags.wc_color) {
			    color = tmpparm->parami1;
			    if (color != NO_COLOR) {
				term_end_color();
				term_start_color(color);
			    }
			}
			attr = tmpparm->parami2;
			term_start_attr(attr);
			break;
		    }
		    case 's': { /* Use color slider */
			char slidebuf[BUFSZ];
			long v1, v2;
			char tmpc;
			const static int slideri[5] =
			     {CLR_ORANGE, CLR_YELLOW, CLR_WHITE, 
			      NO_COLOR, NO_COLOR};

			if (iflags.wc_color && (color != NO_COLOR)) 
			  term_end_color();
			term_end_attr(attr);

			if (tmpparm->parami1 <= 0)
			  v1 = -tmpparm->parami1;
			else {
			    tmpc = tmpparm->parami1;
			    v1 = atoi(botlvaluechar(tmpc, slidebuf));
			}
			if (tmpparm->parami2 < 0)
			  v2 = -tmpparm->parami2;
			else {
			    tmpc = tmpparm->parami2;
			    v2 = atoi(botlvaluechar(tmpc, slidebuf));
			}
			if (iflags.wc_color) {
			    int idx = (v1 * 5) / v2;
			    if (idx < 0) idx = 0;
			    if (idx >= 5) idx = 5 - 1;
			    color = slideri[idx];
			    if (color != NO_COLOR) {
				term_end_color();
				term_start_color(color);
			    }
			}
			attr = ATR_NONE;
			term_start_attr(attr);
			break;
		    }
		    default: break;
		}
		break;
	    }
	    default: break;
	}
	tmpparm = tmpparm->nextparam;      
    }
    if (iflags.wc_color && (color != NO_COLOR)) term_end_color();
    term_end_attr(attr);
}

struct botlparam *
parsebotl_cmd(str)
char *str;
{
    if (str) {
	struct botlparam *botlp = (struct botlparam *) alloc(sizeof(struct botlparam));
	char *params = (char *)0;

	botlp->bptyp = BOTLPARMTYP_CMD;
	botlp->valuechar = str[0];
	str++;
	if (*str && (*str == ':')) str++;
	params = str;

	switch (botlp->valuechar) {
	    case 'c': { /* Set color and attribute */

		int i, clr = NO_COLOR, atr = ATR_NONE;

		if (*params) {
		    for (i = 0; i < SIZE(colornames); i++)
		      if (strstri(params, colornames[i].name)) {
			  clr = colornames[i].color;
			  break;
		      }
		    if ((i == SIZE(colornames)) && 
			(*params >= '0' && *params <= '9')) {
			clr = colornames[atoi(params) % SIZE(colornames)].color;
		    }

		    while (*params && (*params != ';')) params++;
		    if (*params) {
			for (i = 0; i < SIZE(attrnames); i++)
			  if (strstri(params, attrnames[i].name)) {
			      atr = attrnames[i].attr;
			      break;
			  }
			if ((i == SIZE(attrnames)) && 
			    (*params >= '0' && *params <= '9')) {
			    atr = attrnames[atoi(params) % SIZE(attrnames)].attr;
			}
		    }
		}

		botlp->parami1 = clr;
		botlp->parami2 = atr;
		botlp->params = (char *)0;

		break;
	    }
	    case 's': { /* Use color slide */
		if (*params) {
		    if (*params >= '0' && *params <= '9')
		      botlp->parami1 = -atoi(params);
		    else
		      botlp->parami1 = *params;
		}
		while (*params && (*params != ';')) params++;
		params++;
		if (*params) {
		    if (*params >= '0' && *params <= '9')
		      botlp->parami2 = -atoi(params);
		    else
		      botlp->parami2 = *params;
		}
		break;
	    }
	    default:  break;
	}

	return botlp;
    } else return 0;
}

/* Parse "%1.4s %d Yuck! %Z" or something like that into botlbase[botlnum] */
void
parsebotl(str, botlnum)
   char *str;
   int botlnum;
{
   char params[BUFSZ];
   int params_pos = 0;
   boolean hasquotes = FALSE;
   struct botlparam *botlp = (struct botlparam *)0;

   if ((botlnum < 0) || (botlnum >= MAX_BOTLINES)) return;

   if (botlbase[botlnum]) deallocbotlparam(botlnum);

   if (str && (strlen(str) > 1) 
       && ((*str == '"') || (*str == '\'')) 
       && (*str == str[strlen(str)-1])) {
      hasquotes = TRUE;
      str++;
   }

   while (*str) {
       if (hasquotes && (*(str+1) == '\0')) break;
       if (*str == '%') {
	   /* It's a variable or a command */
	   str++;
	   if (*str == '{') {
	       /* It's a command */
	       str++;
	       params[0] = '\0';
	       params_pos = 0;
	       while (*str && (*str != '}')) {
		   params[params_pos++] = *str;
		   params[params_pos] = '\0';
		   str++;
	       }
	       if (*str && (*str == '}')) str++;
	       botlp = parsebotl_cmd(params);
	       if (botlp) {
		   botlp->nextparam = botlbase[botlnum];
		   botlbase[botlnum] = botlp;
	       }
	   } else if (*str) {
	       /* It's a variable */
	       params[0] = '\0';
	       params_pos = 0;
	       while (*str && !isalpha(*str) && (*str != '%') && (*str != '$')) {
		   params[params_pos++] = *str;
		   params[params_pos] = '\0';
		   str++;
	       }
	       botlp = (struct botlparam *) alloc(sizeof(struct botlparam));
	       botlp->bptyp = BOTLPARMTYP_VAR;
	       botlp->params = (char *) alloc(strlen(params)+1);
	       Strcpy(botlp->params, params);
	       botlp->valuechar = *str;
	       str++;
	       botlp->nextparam = botlbase[botlnum];
	       botlbase[botlnum] = botlp;	       
	   }
       } else {
	   /* It's printable text */
	   params[0] = '\0';
	   params_pos = 0;
	   while (*str && (*str != '%')) {
	       params[params_pos++] = *str;
	       params[params_pos] = '\0';
	       str++;
	   }
	   botlp = (struct botlparam *) alloc(sizeof(struct botlparam));
	   botlp->bptyp = BOTLPARMTYP_TXT;
	   botlp->params = (char *) alloc(strlen(params)+1);
	   Strcpy(botlp->params, params);
	   botlp->nextparam = botlbase[botlnum];
	   botlbase[botlnum] = botlp;
       }
   }
   flipbotlbase(botlnum);
}

void
bot()
{
    int y;

    for (y = 0; y < MAX_BOTLINES; y++) {
	curs(WIN_STATUS, 1, y);
	showbotl(y);
    }

    flags.botl = flags.botlx = 0;
}
    
#endif /* CONF_BOTL */

#endif /* OVL0 */

/*botl.c*/
