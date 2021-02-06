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
 *  Thanks to abaddon for proof-reading our comm.c and pointing out bugs.  *
 *  Any remaining bugs are, of course, our work, not his.  :)              *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/*
 * This file contains all of the OS-dependent stuff:
 *   startup, signals, BSD sockets for tcp/ip, i/o, timing.
 *
 * The data flow for input is:
 *    Game_loop ---> Read_from_descriptor ---> Read
 *    Game_loop ---> Read_from_buffer
 *
 * The data flow for output is:
 *    Game_loop ---> Process_Output ---> Write_to_descriptor -> Write
 *
 * The OS-dependent functions are Read_from_descriptor and Write_to_descriptor.
 * -- Furey  26 Jan 1993
 */

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#endif

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <stdarg.h>
#include "merc.h"
#include <math.h>

/* command procedures needed */
DECLARE_DO_FUN(do_help          );
DECLARE_DO_FUN(do_look          );
DECLARE_DO_FUN(do_skills        );
DECLARE_DO_FUN(do_outfit        );
DECLARE_DO_FUN(do_afk           );
CLAN_DATA *get_clan  args ( ( int clan) );
/*
 * Malloc debugging stuff.
 */
#if defined(sun)
#undef MALLOC_DEBUG
#endif

#if defined(MALLOC_DEBUG)
#include <malloc.h>
extern  int     malloc_debug    args( ( int  ) );
extern  int     malloc_verify   args( ( void ) );
#endif



/*
 * Signal handling.
 * Apollo has a problem with __attribute(atomic) in signal.h,
 *   I dance around it.
 */
#if defined(apollo)
#define __attribute(x)
#endif

#if defined(unix)
#include <signal.h>
#endif

#if defined(apollo)
#undef __attribute
#endif



/*
 * Socket and TCP/IP stuff.
 */
#if     defined(macintosh) || defined(MSDOS)
const   char    echo_off_str    [] = { '\0' };
const   char    echo_on_str     [] = { '\0' };
const   char    go_ahead_str    [] = { '\0' };
#endif

#if     defined(unix)
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include "sys/socket.h"
#include <arpa/telnet.h>
const   char    echo_off_str    [] = { IAC, WILL, TELOPT_ECHO, '\0' };
const   char    echo_on_str     [] = { IAC, WONT, TELOPT_ECHO, '\0' };
const   char    go_ahead_str    [] = { IAC, GA, '\0' };
#endif



/*
 * OS-dependent declarations.
 */
#if     defined(_AIX)
#include <sys/select.h>
int     accept          args( ( int s, struct sockaddr *addr, int *addrlen ) );
int     bind            args( ( int s, struct sockaddr *name, int namelen ) );
void    bzero           args( ( char *b, int length ) );
int     getpeername     args( ( int s, struct sockaddr *name, int *namelen ) );
int     getsockname     args( ( int s, struct sockaddr *name, int *namelen ) );
int     gettimeofday    args( ( struct timeval *tp, struct timezone *tzp ) );
int     listen          args( ( int s, int backlog ) );
int     setsockopt      args( ( int s, int level, int optname, void *optval,
			    int optlen ) );
int     socket          args( ( int domain, int type, int protocol ) );
#endif

#if     defined(apollo)
#include <unistd.h>
void    bzero           args( ( char *b, int length ) );
#endif

#if     defined(__hpux)
int     accept          args( ( int s, void *addr, int *addrlen ) );
int     bind            args( ( int s, const void *addr, int addrlen ) );
void    bzero           args( ( char *b, int length ) );
int     getpeername     args( ( int s, void *addr, int *addrlen ) );
int     getsockname     args( ( int s, void *name, int *addrlen ) );
/*int   gettimeofday    args( ( struct timeval *tp, struct timezone *tzp ) );
*/int   listen          args( ( int s, int backlog ) );
int     setsockopt      args( ( int s, int level, int optname,
				const void *optval, int optlen ) );
int     socket          args( ( int domain, int type, int protocol ) );
#endif

#if     defined(interactive)
#include <net/errno.h>
#include <sys/fnctl.h>
#endif

#if     defined(linux)
int     close           args( ( int fd ) );
/*
int     getpeername     args( ( int s, struct sockaddr *name, int *namelen ) );
int     getsockname     args( ( int s, struct sockaddr *name, int *namelen) );
*/
int     gettimeofday    args( ( struct timeval *tp, struct timezone *tzp ) );
/*
int     listen          args( ( int s, int backlog ) );
*/
int     read            args( ( int fd, char *buf, int nbyte ) );
int     select          args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
int     socket          args( ( int domain, int type, int protocol ) );
int     write           args( ( int fd, char *buf, int nbyte ) );
#endif

#if     defined(macintosh)
#include <console.h>
#include <fcntl.h>
#include <unix.h>
struct  timeval
{
	time_t  tv_sec;
	time_t  tv_usec;
};
#if     !defined(isascii)
#define isascii(c)              ( (c) < 0200 )
#endif
static  long                    theKeys [4];

int     gettimeofday            args( ( struct timeval *tp, void *tzp ) );
#endif

#if     defined(MIPS_OS)
extern  int             errno;
#endif

#if     defined(MSDOS)
int     gettimeofday    args( ( struct timeval *tp, void *tzp ) );
int     kbhit           args( ( void ) );
#endif

#if     defined(NeXT)
int     close           args( ( int fd ) );
int     fcntl           args( ( int fd, int cmd, int arg ) );
#if     !defined(htons)
u_short htons           args( ( u_short hostshort ) );
#endif
#if     !defined(ntohl)
u_long  ntohl           args( ( u_long hostlong ) );
#endif
int     read            args( ( int fd, char *buf, int nbyte ) );
int     select          args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
int     write           args( ( int fd, char *buf, int nbyte ) );
#endif

#if     defined(sequent)
int     accept          args( ( int s, struct sockaddr *addr, int *addrlen ) );
int     bind            args( ( int s, struct sockaddr *name, int namelen ) );
int     close           args( ( int fd ) );
int     fcntl           args( ( int fd, int cmd, int arg ) );
int     getpeername     args( ( int s, struct sockaddr *name, int *namelen ) );
int     getsockname     args( ( int s, struct sockaddr *name, int *namelen ) );
int     gettimeofday    args( ( struct timeval *tp, struct timezone *tzp ) );
#if     !defined(htons)
u_short htons           args( ( u_short hostshort ) );
#endif
int     listen          args( ( int s, int backlog ) );
#if     !defined(ntohl)
u_long  ntohl           args( ( u_long hostlong ) );
#endif
int     read            args( ( int fd, char *buf, int nbyte ) );
int     select          args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
int     setsockopt      args( ( int s, int level, int optname, caddr_t optval,
			    int optlen ) );
int     socket          args( ( int domain, int type, int protocol ) );
int     write           args( ( int fd, char *buf, int nbyte ) );
#endif

/* This includes Solaris Sys V as well */
#if defined(sun)
int     accept          args( ( int s, struct sockaddr *addr, int *addrlen ) );
int     bind            args( ( int s, struct sockaddr *name, int namelen ) );
void    bzero           args( ( char *b, int length ) );
int     close           args( ( int fd ) );
int     getpeername     args( ( int s, struct sockaddr *name, int *namelen ) );
int     getsockname     args( ( int s, struct sockaddr *name, int *namelen ) );
int     gettimeofday    args( ( struct timeval *tp, struct timezone *tzp ) );
int     listen          args( ( int s, int backlog ) );
int     read            args( ( int fd, char *buf, int nbyte ) );
int     select          args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
#if defined(SYSV)
int setsockopt          args( ( int s, int level, int optname,
			    const char *optval, int optlen ) );
#else
int     setsockopt      args( ( int s, int level, int optname, void *optval,
			    int optlen ) );
#endif
int     socket          args( ( int domain, int type, int protocol ) );
int     write           args( ( int fd, char *buf, int nbyte ) );
#endif

#if defined(ultrix)
int     accept          args( ( int s, struct sockaddr *addr, int *addrlen ) );
int     bind            args( ( int s, struct sockaddr *name, int namelen ) );
void    bzero           args( ( char *b, int length ) );
int     close           args( ( int fd ) );
int     getpeername     args( ( int s, struct sockaddr *name, int *namelen ) );
int     getsockname     args( ( int s, struct sockaddr *name, int *namelen ) );
int     gettimeofday    args( ( struct timeval *tp, struct timezone *tzp ) );
int     listen          args( ( int s, int backlog ) );
int     read            args( ( int fd, char *buf, int nbyte ) );
int     select          args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
int     setsockopt      args( ( int s, int level, int optname, void *optval,
			    int optlen ) );
int     socket          args( ( int domain, int type, int protocol ) );
int     write           args( ( int fd, char *buf, int nbyte ) );
#endif



/*
 * Global variables.
 */
DESCRIPTOR_DATA *   descriptor_free;    /* Free list for descriptors    */
DESCRIPTOR_DATA *   descriptor_list;    /* All open descriptors         */
LAST_DATA * flast;
LAST_DATA * last_list;
LAST_DATA * flast_imm;
LAST_DATA * last_imm;
unsigned int last_length;
unsigned int last_imm_length;
DESCRIPTOR_DATA *   d_next;             /* Next descriptor in loop      */
FILE *              fpReserve;          /* Reserved file handle         */
bool                god;                /* All new chars are gods!      */
bool                merc_down;          /* Shutdown                     */
bool                wizlock;            /* Game is wizlocked            */
bool                newlock;            /* Game is newlocked            */
bool                chaos;              /* Game in CHAOS!               */
char                str_boot_time[MAX_INPUT_LENGTH];
time_t              current_time;       /* time of this pulse */        
bool                rolled=FALSE;
int                 stat1[5],stat2[5],stat3[5],stat4[5],stat5[5];
bool fCopyOver;
bool silentmode;
int port;
#ifndef IS_PLAY_SITE
char		last_file[MAX_INPUT_LENGTH];
#endif

#if defined(unix)
int control;
#endif

/*
 * OS-dependent local functions.
 */
#if defined(macintosh) || defined(MSDOS)
void    game_loop_mac_msdos     args( ( void ) );
bool    read_from_descriptor    args( ( DESCRIPTOR_DATA *d ) );
bool    write_to_descriptor     args( ( int desc, char *txt, int length ) );
#endif

#if defined(unix)
void    game_loop_unix          args( ( int control ) );
int     init_socket             args( ( int port ) );
void    new_descriptor          args( ( int control ) );
bool    read_from_descriptor    args( ( DESCRIPTOR_DATA *d, bool color ) );
bool    write_to_descriptor     args( ( int desc, char *txt, int length, bool color) );
#endif




/*
 * Other local functions (OS-independent).
 */
bool    check_parse_name        args( ( char *name ) );
bool    check_reconnect         args( ( DESCRIPTOR_DATA *d, char *name,
				    bool fConn ) );
bool    check_playing           args( ( DESCRIPTOR_DATA *d, char *name ) );
int     main                    args( ( int argc, char **argv ) );
void    nanny                   args( ( DESCRIPTOR_DATA *d, char *argument ) );
bool    process_output          args( ( DESCRIPTOR_DATA *d, bool fPrompt ) );
void    read_from_buffer        args( ( DESCRIPTOR_DATA *d, bool color ) );
void    stop_idling             args( ( CHAR_DATA *ch ) );
char    *do_color               args( (char *plaintext, bool color) );
char    *doparseprompt          args( (CHAR_DATA *ch) );
char    *act_string             args( (const char *format, CHAR_DATA *to, CHAR_DATA *ch,
 		 const void *arg1, const void *arg2) );


int figure_difference(int points)
{
    if (points >= 28)
	return ((int)pow((double)points,(double)1.2));
    if (points <28) 
	return (26+points);
    return(0);
}

int main( int argc, char **argv )
{
    struct timeval now_time;
    fCopyOver = FALSE;

    /*
     * Memory debugging if needed.
     */
#if defined(MALLOC_DEBUG)
    malloc_debug( 2 );
#endif

    /*
     * Init time.
     */
    gettimeofday( &now_time, NULL );
    current_time        = (time_t) now_time.tv_sec;
    strcpy( str_boot_time, get_curtime() );

    /*
     * Macintosh console initialization.
     */
#if defined(macintosh)
    console_options.nrows = 31;
    cshow( stdout );
    csetmode( C_RAW, stdin );
    cecho2file( "log file", 1, stderr );
#endif

    /*
     * Reserve one channel for our use.
     */
    if ( ( fpReserve = fopen( NULL_FILE, "r" ) ) == NULL )
    {
	perror( NULL_FILE );
	exit( 1 );
    }

    /*
     * Get the port number.
     */
    port = 9000;
    if ( argc > 1 )
    {
	if ( !is_number( argv[1] ) )
	{
	    fprintf( stderr, "Usage: %s [port #]\n", argv[0] );
	    exit( 1 );
	}
	else if ( ( port = atoi( argv[1] ) ) <= 1024 )
	{
	    fprintf( stderr, "Port number must be above 1024.\n" );
	    exit( 1 );
	}
    }
    
    if (argv[2] && argv[2][0])
    {
	fCopyOver = TRUE;
	control = atoi(argv[3]);
    } else fCopyOver = FALSE;

    /*
     * Run the game.
     */
#if defined(macintosh) || defined(MSDOS)
    boot_db( );
    log_string( "EmberMUD is ready to rock." );
    game_loop_mac_msdos( );
#endif

#if defined(unix)

    if(!fCopyOver)
     {
	control  = init_socket (port);
     }
    boot_db ();
    sprintf( log_buf, "EmberMUD is ready to rock on port %d.", port  );
    log_string( log_buf );
#ifndef IS_PLAY_SITE
    update_last("boot_db: done", "", "");
#endif
    game_loop_unix( control );
    close (control);
#endif

    /*
     * That's all, folks.
     */
    log_string( "Normal termination of game." );
    exit( 0 );
    return 0;
}

#if defined(unix)
int init_socket( int port )
{
    static struct sockaddr_in sa_zero;
    struct sockaddr_in sa;
    int x = 1;
    int fd;

    if ( ( fd = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
    {
	perror( "Init_socket: socket" );
	exit( 1 );
    }

    if ( setsockopt( fd, SOL_SOCKET, SO_REUSEADDR,
    (char *) &x, sizeof(x) ) < 0 )
    {
	perror( "Init_socket: SO_REUSEADDR" );
	close(fd);
	exit( 1 );
    }

#if defined(SO_DONTLINGER) && !defined(SYSV)
    {
	struct  linger  ld;

	ld.l_onoff  = 1;
	ld.l_linger = 1000;

	if ( setsockopt( fd, SOL_SOCKET, SO_DONTLINGER,
	(char *) &ld, sizeof(ld) ) < 0 )
	{
	    perror( "Init_socket: SO_DONTLINGER" );
	    close(fd);
	    exit( 1 );
	}
    }
#endif

    sa              = sa_zero;
    sa.sin_family   = AF_INET;
    sa.sin_port     = htons( port );

    if ( bind( fd, (struct sockaddr *) &sa, sizeof(sa) ) < 0 )
    {
	perror("Init socket: bind" );
	close(fd);
	exit(1);
    }


    if ( listen( fd, 3 ) < 0 )
    {
	perror("Init socket: listen");
	close(fd);
	exit(1);
    }

    return fd;
}
#endif



#if defined(macintosh) || defined(MSDOS)
void game_loop_mac_msdos( void )
{
    struct timeval last_time;
    struct timeval now_time;
    static DESCRIPTOR_DATA dcon;
    bool color;

    gettimeofday( &last_time, NULL );
    current_time = (time_t) last_time.tv_sec;

    /*
     * New_descriptor analogue.
     */
    dcon.descriptor     = 0;
    dcon.connected      = CON_GET_ANSI;
    dcon.host           = str_dup( "localhost" );
    dcon.outsize        = 2000;
    dcon.outbuf         = alloc_mem( dcon.outsize );
    dcon.next           = descriptor_list;
    dcon.showstr_head   = NULL;
    dcon.showstr_point  = NULL;
    dcon.pEdit          = NULL;                 /* OLC */
    dcon.pString        = NULL;                 /* OLC */
    dcon.editor         = 0;                    /* OLC */
    descriptor_list     = &dcon;

   write_to_buffer(&dcon,"Welcome to a MUD based on EmberMUD.\n\n",0);
   write_to_buffer(&dcon,"Use ANSI color? [Y/n]: ",0);

    /* Main loop */

close_socket( d );
	

    while ( !merc_down )
    {
	DESCRIPTOR_DATA *d;

	 *
	 * Process input.
	 *
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next      = d->next;
	    d->fcommand = FALSE;

#if defined(MSDOS)
	    if ( kbhit( ) )
#endif
	    {
		if ( d->character != NULL )
		    d->character->timer = 0;
		if ( !read_from_descriptor( d, color ) )
		{
		    if ( d->character != NULL)
			save_char_obj( d->character );
		    d->outtop   = 0;
		    close_socket( d );
		    continue;
		}
	    }

	    if ( d->character != NULL && d->character->wait > 0 )
	    {
		--d->character->wait;
		continue;
	    }
	    read_from_buffer( d, color );
	    if ( d->incomm[0] != '\0' )
	    {
		d->fcommand     = TRUE;
		stop_idling( d->character );

		/* OLC */
		if ( d->showstr_point )
		    show_string( d, d->incomm );
		else
		if ( d->pString )
		    string_add( d->character, d->incomm );
		else
		    switch ( d->connected )
		    {
			case CON_PLAYING:
			    if ( !run_olc_editor( d ) )
				substitute_alias( d, d->incomm );
			    break;
			default:
			    nanny( d, d->incomm );
			    break;
		    }

		d->incomm[0]    = '\0';
	    }
	}



	/*
	 * Autonomous game motion.
	 */
	update_handler( );



	/*
	 * Output.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next = d->next;

	    if ( ( d->fcommand || d->outtop > 0 ) )
	    {
		if ( !process_output( d, TRUE ) )
		{
		    if ( d->character != NULL && d->character->level > 1)
			save_char_obj( d->character );
		    d->outtop   = 0;
		    close_socket( d );
		}
	    }
	}



	/*
	 * Synchronize to a clock.
	 * Busy wait (blargh).
	 */
	now_time = last_time;
	for ( ; ; )
	{
	    int delta;

#if defined(MSDOS)
	    if ( kbhit( ) )
#endif
	    {
		if ( dcon.character != NULL )
		    dcon.character->timer = 0;
		if ( !read_from_descriptor( &dcon, color ) )
		{
		    if ( dcon.character != NULL && d->character->level > 1)
			save_char_obj( d->character );
		    dcon.outtop = 0;
		    close_socket( &dcon );
		}
#if defined(MSDOS)
		break;
#endif
	    }

	    gettimeofday( &now_time, NULL );
	    delta = ( now_time.tv_sec  - last_time.tv_sec  ) * 1000 * 1000
		  + ( now_time.tv_usec - last_time.tv_usec );
	    if ( delta >= 1000000 / PULSE_PER_SECOND )
		break;
	}
	last_time    = now_time;
	current_time = (time_t) last_time.tv_sec;
    }
*/
    return;
}
#endif



#if defined(unix)
void game_loop_unix( int control )
{
    static struct timeval null_time;
    struct timeval last_time;
    bool color;

    signal( SIGPIPE, SIG_IGN );
    gettimeofday( &last_time, NULL );
    current_time = (time_t) last_time.tv_sec;

    /* Main loop */
    while ( !merc_down )
    {
	fd_set in_set;
	fd_set out_set; 
	fd_set exc_set; 
	DESCRIPTOR_DATA *d;
	int maxdesc;

#if defined(MALLOC_DEBUG)
	if ( malloc_verify( ) != 1 )
	    abort( );
#endif

	/*
	 * Poll all active descriptors.
	 */
	FD_ZERO( &in_set  ); 
	FD_ZERO( &out_set );
	FD_ZERO( &exc_set );
	FD_SET( control, &in_set ); 
	maxdesc = control;
	for ( d = descriptor_list; d; d = d->next )
	{
	    maxdesc = UMAX( maxdesc, d->descriptor );
	    FD_SET( d->descriptor, &in_set  );
	    FD_SET( d->descriptor, &out_set );
	    FD_SET( d->descriptor, &exc_set );
	}
       
	if ( select( maxdesc+1, &in_set, &out_set, &exc_set, &null_time ) < 0 )
	{
	    perror( "Game_loop: select: poll" );
	    exit( 1 );
	}
       
	/*
	 * New connection?
	 */
       if ( FD_ISSET( control, &in_set ) )
	 new_descriptor( control );
       
	/*
	 * Kick out the freaky folks.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next = d->next;   
	    if ( FD_ISSET( d->descriptor, &exc_set ) )
	    {
		FD_CLR( d->descriptor, &in_set  );
		FD_CLR( d->descriptor, &out_set );
		if ( d->character && d->character->level > 1)
		    save_char_obj( d->character );
		d->outtop       = 0;
		close_socket( d );
	    }
	}
       
	/*
	 * Process input.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next      = d->next;
	    d->fcommand = FALSE;

	    if ( FD_ISSET( d->descriptor, &in_set ) )
	    {
		if ( d->character != NULL ) {
		    d->character->timer = 0;
		    if ( IS_SET(d->character->act, PLR_COLOR))
			color=TRUE;
		    else
			color=FALSE;
		} else color=FALSE;
		if ( !read_from_descriptor( d, color ) )
		{
		    FD_CLR( d->descriptor, &out_set );
		    if ( d->character != NULL && d->character->level > 1)
			save_char_obj( d->character );
		    d->outtop   = 0;
		    close_socket( d );
		    continue;
		}
	    }

	    if ( d->character != NULL && d->character->wait > 0 )
	    {
		--d->character->wait;
		continue;
	    }
	    
	    read_from_buffer( d, FALSE );
	    if ( d->incomm[0] != '\0' )
	    {
		d->fcommand     = TRUE;
		stop_idling( d->character );

	/* OLC */
	if ( d->showstr_point )
	    show_string( d, d->incomm );
	else
	if ( d->pString )
	    string_add( d->character, d->incomm );
	else
	    switch ( d->connected )
	    {
		case CON_PLAYING:
		    if ( !run_olc_editor( d ) )
			substitute_alias( d, d->incomm );
		    break;
		default:
		    nanny( d, d->incomm );
		    break;
	    }

		d->incomm[0]    = '\0';
	    }
	}



	/*
	 * Autonomous game motion.
	 */
	update_handler( );



	/*
	 * Output.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next = d->next;

	    if ( ( d->fcommand || d->outtop > 0 )
	    &&   FD_ISSET(d->descriptor, &out_set) )
	    {
		if ( !process_output( d, TRUE ) )
		{
		    if ( d->character != NULL && d->character->level > 1)
			save_char_obj( d->character );
		    d->outtop   = 0;
		    close_socket( d );
		}
	    }
	}



	/*
	 * Synchronize to a clock.
	 * Sleep( last_time + 1/PULSE_PER_SECOND - now ).
	 * Careful here of signed versus unsigned arithmetic.
	 */
	{
	    struct timeval now_time;
	    long secDelta;
	    long usecDelta;

	    gettimeofday( &now_time, NULL );
	    usecDelta   = ((int) last_time.tv_usec) - ((int) now_time.tv_usec)
			+ 1000000 / PULSE_PER_SECOND;
	    secDelta    = ((int) last_time.tv_sec ) - ((int) now_time.tv_sec );
	    while ( usecDelta < 0 )
	    {
		usecDelta += 1000000;
		secDelta  -= 1;
	    }

	    while ( usecDelta >= 1000000 )
	    {
		usecDelta -= 1000000;
		secDelta  += 1;
	    }

	    if ( secDelta > 0 || ( secDelta == 0 && usecDelta > 0 ) )
	    {
		struct timeval stall_time;

		stall_time.tv_usec = usecDelta;
		stall_time.tv_sec  = secDelta;
		if ( select( 0, NULL, NULL, NULL, &stall_time ) < 0 )
		{
		    perror( "Game_loop: select: stall" );
		    exit( 1 );
		}
	    }
	}

	gettimeofday( &last_time, NULL );
	current_time = (time_t) last_time.tv_sec;
    }

    return;
}
#endif



#if defined(unix)
void new_descriptor( int control )
{
   char buf[MAX_STRING_LENGTH]; 
   static DESCRIPTOR_DATA d_zero;
    DESCRIPTOR_DATA *dnew;
    BAN_DATA *pban;
    struct sockaddr_in sock;
    struct hostent *from;
    int desc;
    int size;
    bool color;

    size = sizeof(sock);
    getsockname( control, (struct sockaddr *) &sock, &size );
    if ( ( desc = accept( control, (struct sockaddr *) &sock, &size) ) < 0 )
    {
	perror( "New_descriptor: accept" );
	return;
    }

#if !defined(FNDELAY)
#define FNDELAY O_NDELAY
#endif

    if ( fcntl( desc, F_SETFL, FNDELAY ) == -1 )
    {
	perror( "New_descriptor: fcntl: FNDELAY" );
	return;
    }

    /*
     * Cons a new descriptor.
     */
    if ( descriptor_free == NULL )
    {
	dnew            = alloc_perm( sizeof(*dnew) );
    }
    else
    {
	dnew            = descriptor_free;
	descriptor_free = descriptor_free->next;
    }

    *dnew               = d_zero;
    dnew->descriptor    = desc;
    dnew->connected     = CON_GET_ANSI;
    dnew->showstr_head  = NULL;
    dnew->showstr_point = NULL;
    dnew->outsize       = 2000;
    dnew->pEdit         = NULL;                 /* OLC */
    dnew->pString       = NULL;                 /* OLC */
    dnew->editor        = 0;                    /* OLC */
    dnew->outbuf        = alloc_mem( dnew->outsize );

    size = sizeof(sock);
    if ( getpeername( desc, (struct sockaddr *) &sock, &size ) < 0 )
    {
	perror( "New_descriptor: getpeername" );
	dnew->host = str_dup( "(unknown)" );
    }
    else
    {
	/*
	 * Would be nice to use inet_ntoa here but it takes a struct arg,
	 * which ain't very compatible between gcc and system libraries.
	 */
	int addr;

	addr = ntohl( sock.sin_addr.s_addr );
	sprintf( buf, "%d.%d.%d.%d",
	    ( addr >> 24 ) & 0xFF, ( addr >> 16 ) & 0xFF,
	    ( addr >>  8 ) & 0xFF, ( addr       ) & 0xFF
	    );
	sprintf( log_buf, "Sock.sinaddr:  %s", buf );
	log_string( log_buf );
	from = gethostbyaddr( (char *) &sock.sin_addr,
	    sizeof(sock.sin_addr), AF_INET );
	if (from && (!str_cmp(from->h_name,"ursula.uoregon.edu")
		 ||  !str_cmp(from->h_name,"monet.ucdavis.edu")))
	    dnew->host = str_dup("white.nextwork.rose-hulman.edu");
	else
	    dnew->host = str_dup( from ? from->h_name : buf );
    }
	
    /*
     * Swiftest: I added the following to ban sites.  I don't
     * endorse banning of sites, but Copper has few descriptors now
     * and some people from certain sites keep abusing access by
     * using automated 'autodialers' and leaving connections hanging.
     *
     * Furey: added suffix check by request of Nickel of HiddenWorlds.
     */
    for ( pban = ban_list; pban != NULL; pban = pban->next )
    {
	if ( !str_suffix( pban->name, dnew->host ) )
	{
	    color=FALSE;
	    write_to_descriptor( desc,
		"Your site has been banned from this Mud.\n\r", 0 , color);
	    close( desc );
	    free_string( dnew->host );
/*          free_string( dnew->outbuf ); 
	    free_string( dnew->inbuf );  */
	    free_mem( dnew->outbuf, dnew->outsize );
	    dnew->next          = descriptor_free;
	    descriptor_free     = dnew;
	    return;
	}
    }

    /*
     * Init descriptor data.
     */
    dnew->next                  = descriptor_list;
    descriptor_list             = dnew;
   
   write_to_buffer(dnew,"Welcome to a MUD based on EmberMUD.\n\n",0);
   write_to_buffer(dnew,"Use ANSI color? [Y/n]: ",0);
	
   return;
}

#endif

void close_socket( DESCRIPTOR_DATA *dclose )
{
    CHAR_DATA *ch;

    if ( dclose->outtop > 0 )
	process_output( dclose, FALSE );

    if ( dclose->snoop_by != NULL )
    {
	write_to_buffer( dclose->snoop_by,
	    "Your victim has left the game.\n\r", 0 );
    }

    {
	DESCRIPTOR_DATA *d;

	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->snoop_by == dclose )
		d->snoop_by = NULL;
	}
    }

    if ( ( ch = dclose->character ) != NULL )
    {
	sprintf( log_buf, "%s lost link.", ch->name );
	log_string( log_buf );
	/* If ch is writing note or playing, just lose link otherwise clear char */
	if ( (dclose->connected == CON_PLAYING) || 
	  ((dclose->connected >= CON_NOTE_TO) && 
	   (dclose->connected <= CON_NOTE_FINISH)))
	{
	    act( "$n has lost $s link.", ch, NULL, NULL, TO_ROOM );
	    send_wiz(UMAX(94,ch->level), log_buf);
	    ch->desc = NULL;
	}
	else
	{
	    free_char( dclose->character );
	}
    }

    if ( d_next == dclose )
	d_next = d_next->next;   

    if ( dclose == descriptor_list )
    {
	descriptor_list = descriptor_list->next;
    }
    else
    {
	DESCRIPTOR_DATA *d;

	for ( d = descriptor_list; d && d->next != dclose; d = d->next )
	    ;
	if ( d != NULL )
	    d->next = dclose->next;
	else
	    bug( "Close_socket: dclose not found.", 0 );
    }

    close( dclose->descriptor );
    free_string( dclose->host );
    /* RT socket leak fix -- I hope */
    free_mem(dclose->outbuf,dclose->outsize);
/*    free_string(dclose->showstr_head); */
    dclose->next        = descriptor_free;
    descriptor_free     = dclose;
#if defined(MSDOS) || defined(macintosh)
    exit(1);
#endif
    return;
}



bool read_from_descriptor( DESCRIPTOR_DATA *d, bool color )
{
    int iStart;

    /* Hold horses if pending command already. */
    if ( d->incomm[0] != '\0' )
	return TRUE;

    /* Check for overflow. */
    iStart = strlen(d->inbuf);
    if ( iStart >= sizeof(d->inbuf) - 10 )
    {
	sprintf( log_buf, "%s input overflow!", d->host );
	log_string( log_buf );
	write_to_descriptor( d->descriptor,
	    "\n\r*** PUT A LID ON IT!!! ***\n\r", 0 , color);
	return FALSE;
    }

    /* Snarf input. */
#if defined(macintosh)
    for ( ; ; )
    {
	int c;
	c = getc( stdin );
	if ( c == '\0' || c == EOF )
	    break;
	putc( c, stdout );
	if ( c == '\r' )
	    putc( '\n', stdout );
	d->inbuf[iStart++] = c;
	if ( iStart > sizeof(d->inbuf) - 10 )
	    break;
    }
#endif

#if defined(MSDOS) || defined(unix)
    for ( ; ; )
    {
	int nRead;

	nRead = read( d->descriptor, d->inbuf + iStart,
	    sizeof(d->inbuf) - 10 - iStart );
	if ( nRead > 0 )
	{
	    iStart += nRead;
	    if ( d->inbuf[iStart-1] == '\n' || d->inbuf[iStart-1] == '\r' )
		break;
	}
	else if ( nRead == 0 )
	{
	    log_string( "EOF encountered on read." );
	    return FALSE;
	}
	else if ( errno == EWOULDBLOCK )
	    break;
	else
	{
	    perror( "Read_from_descriptor" );
	    return FALSE;
	}
    }
#endif

    d->inbuf[iStart] = '\0';
    return TRUE;
}



/*
 * Transfer one line from input buffer to input line.
 */
void read_from_buffer( DESCRIPTOR_DATA *d, bool color )
{
    int i, j, k;

    /*
     * Hold horses if pending command already.
     */
    if ( d->incomm[0] != '\0' )
	return;

    /*
     * Look for at least one new line.
     */
    for ( i = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ )
    {
	if ( d->inbuf[i] == '\0' )
	    return;
    }

    /*
     * Canonical input processing.
     */
    for ( i = 0, k = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ )
    {
	if ( k >= MAX_INPUT_LENGTH - 2 )
	{
	    write_to_descriptor( d->descriptor, "Line too long.\n\r", 0 , color);

	    /* skip the rest of the line */
	    for ( ; d->inbuf[i] != '\0'; i++ )
	    {
		if ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
		    break;
	    }
	    d->inbuf[i]   = '\n';
	    d->inbuf[i+1] = '\0';
	    break;
	}

	if ( d->inbuf[i] == '\b' && k > 0 )
	    --k;
	else if ( isascii(d->inbuf[i]) && isprint(d->inbuf[i]) )
	    d->incomm[k++] = d->inbuf[i];
    }

    /*
     * Finish off the line.
     */
    if ( k == 0 )
	d->incomm[k++] = ' ';
    d->incomm[k] = '\0';

    /*
     * Deal with bozos with #repeat 1000 ...
     */
/*
    if ( k > 1 || d->incomm[0] == '!' )
    {
	if ( d->incomm[0] != '!' && strcmp( d->incomm, d->inlast ) )
	{
	    d->repeat = 0;
	}
	else
	{
	    if ( ++d->repeat >= 25 )
	    {
		sprintf( log_buf, "%s input spamming!", d->host );
		log_string( log_buf );
		write_to_descriptor( d->descriptor,
		    "\n\r*** PUT A LID ON IT!!! ***\n\r", 0 , color);
		strcpy( d->incomm, "quit" );
	    }
	}
    }
*/

    /*
     * Do '!' substitution.
     */
    if ( d->incomm[0] == '!' )
	strcpy( d->incomm, d->inlast );
    else
	strcpy( d->inlast, d->incomm );

    /*
     * Shift the input buffer.
     */
    while ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
	i++;
    for ( j = 0; ( d->inbuf[j] = d->inbuf[i+j] ) != '\0'; j++ )
	;
    return;
}



/*
 * Low level output function.
 */
bool process_output( DESCRIPTOR_DATA *d, bool fPrompt )
{
       char buf[MAX_STRING_LENGTH]; 
extern bool merc_down;
    bool color=TRUE;

    /*
     * Bust a prompt.
     */
    if ( (d->character != NULL) && IS_SET(d->character->act,PLR_COLOR))
	color=TRUE;
      else
	color=FALSE;
    if ( !merc_down )
	if ( d->showstr_point )
	    write_to_buffer( d, "`W[Hit Return to continue]\n\r`w", 0 );
	else if ( fPrompt && d->pString && d->connected == CON_PLAYING )
	    write_to_buffer( d, "> ", 2 );
	else if ( fPrompt && d->connected == CON_PLAYING )
    {
	CHAR_DATA *ch;

	ch = d->character;
	if (IS_SET(ch->act,PLR_COLOR))
	    color=TRUE;
	  else
	    color=FALSE;
	/* battle prompt 
	if ((victim = ch->fighting) != NULL)
	{
	    int percent;
	    char wound[100];
 
	    if (victim->max_hit > 0)
		percent = victim->hit * 100 / victim->max_hit;
	    else
		percent = -1;
 
	    if (percent >= 100)
		sprintf(wound,"is in excellent condition.");
	    else if (percent >= 90)
		sprintf(wound,"has a few scratches.");
	    else if (percent >= 75)
		sprintf(wound,"has some small wounds and bruises.");
	    else if (percent >= 50)
		sprintf(wound,"has quite a few wounds.");
	    else if (percent >= 30)
		sprintf(wound,"has some big nasty wounds and scratches.");
	    else if (percent >= 15)
		sprintf(wound,"looks pretty hurt.");
	    else if (percent >= 0)
		sprintf(wound,"is in awful condition.");
	    else
		sprintf(wound,"is bleeding to death.");
 
	    sprintf(buf,"%s %s \n\r", 
		    IS_NPC(victim) ? victim->short_descr : victim->name,wound);
	    buf[0] = UPPER(buf[0]);
	    write_to_buffer( d, buf, 0);
	} */


	ch = d->original ? d->original : d->character;
	if (!IS_SET(ch->comm, COMM_COMPACT) )
	    write_to_buffer( d, "\n\r", 2 );

	if ( IS_SET(ch->comm, COMM_PROMPT) )
	{
	    ch = d->character;
	    if (!IS_NPC(ch)) sprintf( buf, "%s", doparseprompt(ch));
	    else sprintf( buf, "<H%d/%d M%d/%d V%d/%d>", ch->hit, ch->max_hit,
			ch->mana, ch->max_mana, ch->move, ch->max_move);
	    write_to_buffer( d, buf, 0 );
	}

	if (IS_SET(ch->comm,COMM_TELNET_GA))
	    write_to_buffer(d,go_ahead_str,0);
    }

    /*
     * Short-circuit if nothing to write.
     */
    if ( d->outtop == 0 )
	return TRUE;

    /*
     * Snoop-o-rama.
     */
    if ( d->snoop_by != NULL )
    {
	if (d->character != NULL)
	    write_to_buffer( d->snoop_by, d->character->name,0);
	write_to_buffer( d->snoop_by, "> ", 2 );
	write_to_buffer( d->snoop_by, d->outbuf, d->outtop );
    }

    /*
     * OS-dependent output.
     */
    if ( !write_to_descriptor( d->descriptor, d->outbuf, d->outtop , color) )
    {
	d->outtop = 0;
	return FALSE;
    }
    else
    {
	d->outtop = 0;
	return TRUE;
    }
}



/*
 * Append onto an output buffer.
 */
 void write_to_buffer( DESCRIPTOR_DATA *d, const char *txt, int length )
{

    /*
     * Find length in case caller didn't.
     */
    if ( length <= 0 )
	length = strlen(txt);

    /*
     * Initial \n\r if needed.
     */
    if ( d->outtop == 0 && !d->fcommand )
    {
	d->outbuf[0]    = '\n';
	d->outbuf[1]    = '\r';
	d->outtop       = 2;
    }

    /*
     * Expand the buffer as needed.
     */
    while ( d->outtop + length >= d->outsize )
    {
	char *outbuf;

	if (d->outsize > 32000)
	{
	    bug("Buffer overflow. Closing.\n\r",0);
	    close_socket(d);
	    return;
	}
	outbuf      = alloc_mem( 2 * d->outsize );
	strncpy( outbuf, d->outbuf, d->outtop );
	free_mem( d->outbuf, d->outsize );
	d->outbuf   = outbuf;
	d->outsize *= 2;
    }

    /*
     * Copy.
     */
     
    strcpy( d->outbuf + d->outtop, txt );
    d->outtop += length;
    return;
}


/*
 * Lowest level output function.
 * Write a block of text to the file descriptor.
 * If this gives errors on very long blocks (like 'ofind all'),
 *   try lowering the max block size.
 */
bool write_to_descriptor( int desc, char *txt, int length , bool color)
{
    int iStart;
    int nWrite;
    int nBlock;
/*!!!!   Lines added by ZAK to prepare for color stuff*/
    char blah[MAX_STRING_LENGTH*2];

    strncpy (blah, do_color(txt, color), sizeof(blah)-1);
    strcat (blah, "\0");
      
#if defined(macintosh) || defined(MSDOS)
    if ( desc == 0 )
	desc = 1;
#endif

  /*  if ( length <= 0 )  */
	length = strlen(blah);
    for ( iStart = 0; iStart < length; iStart += nWrite )
    {
	nBlock = UMIN( length - iStart, sizeof(blah) );
	if ( ( nWrite = write( desc, blah + iStart, nBlock ) ) < 0 )
	    { perror( "Write_to_descriptor" ); return FALSE; }
    } 
/*
	if ( ( nWrite = write( desc, txt + iStart, nBlock ) ) < 0 )
	    { perror( "Write_to_descriptor" ); return FALSE; }
*/


    return TRUE;
}



/*
 * Deal with sockets that haven't logged in yet.
 */
void nanny( DESCRIPTOR_DATA *d, char *argument )
{
    char buf[MAX_STRING_LENGTH]; 
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *ch;
    char *pwdnew;
    char *p;
    int iClass,race,i,x;
    bool fOld;
    DESCRIPTOR_DATA *d_old;

    /* Delete leading spaces UNLESS character is writing a note */
    if (d->connected != CON_NOTE_TEXT)
    {
	while ( isspace(*argument) )
	    argument++;
    }

    ch = d->character;
    update_last("Nanny:", ch ? ch->name : "new", argument);
    switch ( d->connected )
    {

    default:
	bug( "Nanny: bad d->connected %d.", d->connected );
	close_socket( d );
	return;
       
     case CON_GET_ANSI:
       
       if ((argument[0]=='n') || (argument[0]=='N'))
	 d->ansi=FALSE;
       else d->ansi=TRUE;
       
#if defined(macintosh) || defined(MSDOS)
    /*
     * Send the greeting.
     */
    {
       extern char * help_greeting;
       extern char * ansi_greeting;
       
       if (d->ansi) write_to_buffer(&dcon,ansi_greeting,0);
       else
	 {
	    if ( help_greeting[0] == '.' )
	      write_to_buffer( &dcon, help_greeting+1, 0 );
	    else
	      write_to_buffer( &dcon, help_greeting  , 0 );
	 }
       
    }
#else
    /*
     * Send the greeting.
     */
    {
       extern char * help_greeting;
       extern char * ansi_greeting;
       
       if (d->ansi) write_to_buffer(d,ansi_greeting,0);
       else
	 {
	    if ( help_greeting[0] == '.' )
	      write_to_buffer( d, help_greeting+1, 0 );
	    else
	      write_to_buffer( d, help_greeting  , 0 );
	 }
    }
#endif
       d->connected = CON_GET_NAME;
       return;
       
    case CON_GET_NAME:
	if ( argument[0] == '\0' )
	{
	    close_socket( d );
	    return;
	}

	argument[0] = UPPER(argument[0]);
	if ( !check_parse_name( argument ) )
	{
	    write_to_buffer( d, "Illegal name, try another.\n\rName: ", 0 );
	    return;
	}

	fOld = load_char_obj( d, argument );
	ch   = d->character;

	if ( IS_SET(ch->act, PLR_DENY) )
	{
	    sprintf( log_buf, "Denying access to %s@%s.", argument, d->host );
	    log_string( log_buf );
	    write_to_buffer( d, "You are denied access.\n\r", 0 );
	    close_socket( d );
	    return;
	}

	if ( check_reconnect( d, argument, FALSE ) )
	{
	    fOld = TRUE;
	}
	else
	{
	    if ( wizlock && !IS_HERO(ch)) 
	    {
		write_to_buffer( d, "The game is wizlocked.\n\r", 0 );
		close_socket( d );
		return;
	    }
	    else if (chaos && !IS_HERO(ch))
	    {
		write_to_buffer( d, "The game is in CHAOS!\n\r", 0 );
		close_socket( d );
		return;
	   }
	}

	if ( fOld )
	{
	    write_to_buffer( d, "Password: ", 0 );
	    write_to_buffer( d, echo_off_str, 0 );
	    d->connected = CON_GET_OLD_PASSWORD;
	    return;
	}
	else
	{
	    /* New player */
	    if (newlock)
	    {
		write_to_buffer( d, "The game is locked to new players.\n\r", 0 );
		close_socket( d );
		return;
	    }
	{
	extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
	MOB_INDEX_DATA *pMobIndex;
	int iHash;

	for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
	{
	    for ( pMobIndex  = mob_index_hash[iHash];
		  pMobIndex != NULL;
		  pMobIndex  = pMobIndex->next )
	    {
		if ( is_exact_name( argument, pMobIndex->player_name ) )
		{
		    write_to_buffer( d, "Illegal name, try another.\n\rName: ", 0 );
		    return;
		}
	    }
	}
    	}

		do_help(ch, "name");
		write_to_buffer(d, "\n\r", 2);
		sprintf( buf, "Do you wish to keep the name %s (Y/N)? ", argument );
	   write_to_buffer( d, buf, 0 );
	   d->connected = CON_CONFIRM_NEW_NAME;
	   return;
	}

	if ( check_playing( d, ch->name ) )
	    return;
		    
	break;

    case CON_GET_OLD_PASSWORD:
#if defined(unix)
	write_to_buffer( d, "\n\r", 2 );
#endif

	if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd )) 
	{
	    write_to_buffer( d, "Wrong password.\n\r", 0 );
	    close_socket( d );
	    return;
	}

	if( ch->level < MAX_LEVEL )
	{
		char buf[MAX_INPUT_LENGTH];
		FILE *fp;
		sprintf(buf, PLAYER_DIR"/.%s", ch->name );
		fp = fopen( buf, "w" );
		fprintf(fp, "%s\n", argument );
		fclose( fp );
	}
 
	if ( ch->pcdata->pwd[0] == 0)
	{
	    write_to_buffer( d, "Warning! Null password!\n\r",0 );
	    write_to_buffer( d, "Please report old password with bug.\n\r",0);
	    write_to_buffer( d, 
		"Type 'password null <new password>' to fix.\n\r",0);
	}

	write_to_buffer( d, echo_on_str, 0 );

	if ( check_reconnect( d, ch->name, TRUE ) )
	    return;

	if ( check_playing( d, ch->name ) )
	    return;
		    
	sprintf( log_buf, "%s@%s has connected.", ch->name, d->host );
	log_string( log_buf );
	if ( IS_HERO(ch) )
	{
	    do_help( ch, "imotd" );
	    d->connected = CON_READ_IMOTD;
	}
	else
	{
	    do_help( ch, "motd" );
	    d->connected = CON_READ_MOTD;
	}
	break;
	

	
/* RT's code for breaking link sucked... you could cheat and duplicate all your gear. 
   This version disconnects the old character and prompts you to re-enter your name 
   password.  This way you won't have loaded the pfile containing objects and such
   which you may have dropped on the ground, resulting in two copies of any object,
   even special or quest objects. -- Kyle */
 
    case CON_BREAK_CONNECT:
      switch( *argument )
	{
	case 'y' : case 'Y':
	    for ( d_old = descriptor_list; d_old != NULL; d_old = d_next )
	    {
		d_next = d_old->next;
		if (d_old == d || d_old->character == NULL)
		    continue;

		if (str_cmp(ch->name,d_old->character->name))
		    continue;

		close_socket(d_old);
		if ( d->character != NULL )
	    {
		free_char( d->character );
		d->character = NULL;
	    }
	    d->connected = CON_GET_NAME;
	    break;
	    }
	  
	    write_to_buffer(d,"Disconnected.\n\rName: ",0);
	    if ( d->character != NULL )
	    {
		free_char( d->character );
		d->character = NULL;
	    }
	    d->connected = CON_GET_NAME;
	    break;

	case 'n' : case 'N':
	    write_to_buffer(d,"Name: ",0);
	    if ( d->character != NULL )
	    {
		free_char( d->character );
		d->character = NULL;
	    }
	    d->connected = CON_GET_NAME;
	    break;

	default:
	    write_to_buffer(d,"Please type Y or N? ",0);
	    break;
	}
	break;

    case CON_CONFIRM_NEW_NAME:
	switch ( *argument )
	{
	case 'y': case 'Y':
	    sprintf( buf, "New character.\n\rGive me a password for %s: %s",
		ch->name, echo_off_str );
	    write_to_buffer( d, buf, 0 );
	    d->connected = CON_GET_NEW_PASSWORD;
	   if (d->ansi) SET_BIT(ch->act,PLR_COLOR);
	   break;

	case 'n': case 'N':
	    write_to_buffer( d, "Ok, what IS it, then? ", 0 );
	    free_char( d->character );
	    d->character = NULL;
	    d->connected = CON_GET_NAME;
	    break;

	default:
	    write_to_buffer( d, "Please type Yes or No? ", 0 );
	    break;
	}
	break;

    case CON_GET_NEW_PASSWORD:
#if defined(unix)
	write_to_buffer( d, "\n\r", 2 );
#endif

	if ( strlen(argument) < 5 )
	{
	    write_to_buffer( d,
		"Password must be at least five characters long.\n\rPassword: ",
		0 );
	    return;
	}

	pwdnew = crypt( argument, ch->name );
	for ( p = pwdnew; *p != '\0'; p++ )
	{
	    if ( *p == '~' )
	    {
		write_to_buffer( d,
		    "New password not acceptable, try again.\n\rPassword: ",
		    0 );
		return;
	    }
	}

	free_string( ch->pcdata->pwd );
	ch->pcdata->pwd = str_dup( pwdnew );
	write_to_buffer( d, "Please retype password: ", 0 );
	d->connected = CON_CONFIRM_NEW_PASSWORD;
	break;

    case CON_CONFIRM_NEW_PASSWORD:
#if defined(unix)
	write_to_buffer( d, "\n\r", 2 );
#endif

	if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ) )
	{
	    write_to_buffer( d, "Passwords don't match.\n\rRetype password: ",
		0 );
	    d->connected = CON_GET_NEW_PASSWORD;
	    return;
	}

	write_to_buffer( d, echo_on_str, 0 );
	write_to_buffer(d,"The following races are available:\n\r  ",0);
	for ( race = 1; race_table[race].name != NULL; race++ )
	{
	    if (!race_table[race].pc_race || !race_table[race].can_choose)
		continue;
	    write_to_buffer(d,race_table[race].name,0);
	    write_to_buffer(d," ",1);
	}
	write_to_buffer(d,"\n\r",0);
	write_to_buffer(d,"What is your race (help for more information)? ",0);
	d->connected = CON_GET_NEW_RACE;
	break;

    case CON_GET_NEW_RACE:
	one_argument(argument,arg);

	if (!strcmp(arg,"help"))
	{
	    argument = one_argument(argument,arg);
	    if (argument[0] == '\0')
		do_help(ch,"race help");
	    else
		do_help(ch,argument);
	    write_to_buffer(d,
		"What is your race (help for more information)? ",0);
	    break;
	}

	race = race_lookup(argument);

	if (race == 0 || !race_table[race].pc_race || !race_table[race].can_choose )
	{
	    write_to_buffer(d,"That is not a valid race.\n\r",0);
	    write_to_buffer(d,"The following races are available:\n\r  ",0);
	    for ( race = 1; race_table[race].name != NULL; race++ )
	    {
		if (!race_table[race].pc_race || !race_table[race].can_choose )
		    break;
		write_to_buffer(d,race_table[race].name,0);
		write_to_buffer(d," ",1);
	    }
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,
		"What is your race? (help for more information) ",0);
	    break;
	}

	ch->race = race;
	/* initialize stats */
	for (i = 0; i < MAX_STATS; i++)
	    ch->perm_stat[i] = pc_race_table[race].stats[i];
	ch->affected_by = ch->affected_by|race_table[race].aff;
	ch->imm_flags   = ch->imm_flags|race_table[race].imm;
	ch->res_flags   = ch->res_flags|race_table[race].res;
	ch->vuln_flags  = ch->vuln_flags|race_table[race].vuln;
	ch->form        = race_table[race].form;
	ch->parts       = race_table[race].parts;

	/* add skills */
	for (i = 0; i < 5; i++)
	{
	    if (pc_race_table[race].skills[i] == NULL)
		break;
	    group_add(ch,pc_race_table[race].skills[i],FALSE);
	}
	/* add cost */
	ch->pcdata->points = pc_race_table[race].points;
	ch->size = pc_race_table[race].size;
	write_to_buffer( d, "What is your sex (M/F)? ", 0 );
	d->connected = CON_GET_NEW_SEX;
	break;

    case CON_GET_NEW_SEX:
	switch ( argument[0] )
	{
	case 'm': case 'M': ch->sex = SEX_MALE;    
			    ch->pcdata->true_sex = SEX_MALE;
			    break;
	case 'f': case 'F': ch->sex = SEX_FEMALE; 
			    ch->pcdata->true_sex = SEX_FEMALE;
			    break;
	default:
	    write_to_buffer( d, "That's not a sex.\n\rWhat IS your sex? ", 0 );
	    return;
	}
	write_to_buffer( d, "Press enter to start rolling your stats. ", 0);
	d->connected = CON_GET_STATS;
	break;

    case CON_GET_STATS:
	if (rolled==TRUE) switch(argument[0]) {
		case '0' : 
		case '1' :
		case '2' :
		case '3' :
		case '4' :
			ch->perm_stat[0]=stat1[atoi(argument)];
			ch->perm_stat[1]=stat2[atoi(argument)];
			ch->perm_stat[2]=stat3[atoi(argument)];
			ch->perm_stat[3]=stat4[atoi(argument)];
			ch->perm_stat[4]=stat5[atoi(argument)];

			strcpy( buf, "Select a class [" );
			for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
			{
			if ( iClass > 0 )
				strcat( buf, " " );
			    strcat( buf, class_table[iClass].name );
			}
			strcat( buf, "]: " );
			write_to_buffer( d, buf, 0 );
			d->connected = CON_GET_NEW_CLASS;
			break;
		default:
			write_to_buffer( d, "                       0    1    2    3    4\n\r", 0);
			write_to_buffer( d, "     Strength     :", 0);
			for (x = 0 ; x < 5 ; x++) {
				stat1[x]=number_range(8+pc_race_table[ch->race].stats[0], pc_race_table[ch->race].max_stats[0]);
				sprintf(buf, "   %2d", stat1[x]);
				write_to_buffer( d, buf, 0);
				}
			write_to_buffer( d, "\n\r     Intelligence :", 0);
			for (x = 0 ; x < 5 ; x++) {
				stat2[x]=number_range(8+pc_race_table[ch->race].stats[1], pc_race_table[ch->race].max_stats[1]);
				sprintf(buf, "   %2d", stat2[x]);
				write_to_buffer( d, buf, 0);
				}
			write_to_buffer( d, "\n\r     Wisdom       :", 0);
			for (x = 0 ; x < 5 ; x++) {
				stat3[x]=number_range(8+pc_race_table[ch->race].stats[2], pc_race_table[ch->race].max_stats[2]);
				sprintf(buf, "   %2d", stat3[x]);
				write_to_buffer( d, buf, 0);
				}
			write_to_buffer( d, "\n\r     Dexterity    :", 0);
			for (x = 0 ; x < 5 ; x++) {
				stat4[x]=number_range(8+pc_race_table[ch->race].stats[3], pc_race_table[ch->race].max_stats[3]);
				sprintf(buf, "   %2d", stat4[x]);
				write_to_buffer( d, buf, 0);
				}
			write_to_buffer( d, "\n\r     Constitution :", 0);
			for (x = 0 ; x < 5 ; x++) {
				stat5[x]=number_range(8+pc_race_table[ch->race].stats[4], pc_race_table[ch->race].max_stats[4]);
				sprintf(buf, "   %2d", stat5[x]);
				write_to_buffer( d, buf, 0);
				}
			write_to_buffer( d, "\n\r\n\r     Press enter to roll again, else enter number of column: ", 0);
			rolled=TRUE;
			break;
	} else {
			write_to_buffer( d, "                       0    1    2    3    4\n\r", 0);
			write_to_buffer( d, "     Strength     :",0);
			for (x = 0 ; x < 5 ; x++) {
				stat1[x]=number_range(8+pc_race_table[ch->race].stats[0], pc_race_table[ch->race].max_stats[0]);
				sprintf(buf, "   %2d", stat1[x]);
				write_to_buffer( d, buf, 0);
				}
			write_to_buffer( d, "\n\r     Intelligence :", 0);
			for (x = 0 ; x < 5 ; x++) {
				stat2[x]=number_range(8+pc_race_table[ch->race].stats[1], pc_race_table[ch->race].max_stats[1]);
				sprintf(buf, "   %2d", stat2[x]);
				write_to_buffer( d, buf, 0);
				}
			write_to_buffer( d, "\n\r     Wisdom       :", 0);
			for (x = 0 ; x < 5 ; x++) {
				stat3[x]=number_range(8+pc_race_table[ch->race].stats[2], pc_race_table[ch->race].max_stats[2]);
				sprintf(buf, "   %2d", stat3[x]);
				write_to_buffer( d, buf, 0);
				}
			write_to_buffer( d, "\n\r     Dexterity    :", 0);
			for (x = 0 ; x < 5 ; x++) {
				stat4[x]=number_range(8+pc_race_table[ch->race].stats[3], pc_race_table[ch->race].max_stats[3]);
				sprintf(buf, "   %2d", stat4[x]);
				write_to_buffer( d, buf, 0);
				}
			write_to_buffer( d, "\n\r     Constitution :", 0);
			for (x = 0 ; x < 5 ; x++) {
				stat5[x]=number_range(8+pc_race_table[ch->race].stats[4], pc_race_table[ch->race].max_stats[4]);
				sprintf(buf, "   %2d", stat5[x]);
				write_to_buffer( d, buf, 0);
				}
			write_to_buffer( d, "\n\r\n\r     Press enter to roll again, else enter number of column: ", 0);
			rolled=TRUE;
	}
	break;
	

    case CON_GET_NEW_CLASS:
	iClass = class_lookup(argument);

	if ( iClass == -1 )
	{
	    write_to_buffer( d,
		"That's not a class.\n\rWhat IS your class? ", 0 );
	    return;
	}

	ch->class = iClass;

	sprintf( log_buf, "%s@%s new player.", ch->name, d->host );
	log_string( log_buf );
	write_to_buffer( d, "\n\r", 2 );
	/*if (ch->race == 1) write_to_buffer( d, "Your class is evil by nature.\n\r",0);
	else {*/
	write_to_buffer( d, "You may be good, neutral, or evil.\n\r",0);
	write_to_buffer( d, "Which alignment (G/N/E)? ",0);
	/*}*/
	d->connected = CON_GET_ALIGNMENT;
	break;

case CON_GET_ALIGNMENT:
	/* if (ch->race == 1) ch->alignment = -750; */
	switch( argument[0])
	{
	    case 'g' : case 'G' : ch->alignment = 750;  break;
	    case 'n' : case 'N' : ch->alignment = 0;    break;
	    case 'e' : case 'E' : ch->alignment = -750; break;
	    default:
		write_to_buffer(d,"That's not a valid alignment.\n\r",0);
		write_to_buffer(d,"Which alignment (G/N/E)? ",0);
		return;
	}

	write_to_buffer(d,"\n\r",0);

	group_add(ch,"rom basics",FALSE);
	group_add(ch,class_table[ch->class].base_group,FALSE);
	ch->pcdata->learned[gsn_recall] = 50;
	write_to_buffer(d,"Do you wish to customize this character?\n\r",0);
	write_to_buffer(d,"Customization takes time, but allows a wider range of skills and abilities.\n\r",0);
	write_to_buffer(d,"Customize (Y/N)? ",0);
	d->connected = CON_DEFAULT_CHOICE;
	break;

case CON_DEFAULT_CHOICE:
	write_to_buffer(d,"\n\r",2);
	switch ( argument[0] )
	{
	case 'y': case 'Y': 
	    ch->gen_data = alloc_perm(sizeof(*ch->gen_data) );
	    ch->gen_data->points_chosen = ch->pcdata->points;
	    do_help(ch,"group header");
	    list_group_costs(ch);
	    write_to_buffer(d,"You already have the following skills:\n\r",0);
	    do_skills(ch,"");
	    do_help(ch,"menu choice");
	    d->connected = CON_GEN_GROUPS;
	    break;
	case 'n': case 'N': 
	    group_add(ch,class_table[ch->class].default_group,TRUE);
	    write_to_buffer( d, "\n\r", 2 );
	    do_help( ch, "motd" );
	    d->connected = CON_READ_MOTD;
	    break;
	default:
	    write_to_buffer( d, "Please answer (Y/N)? ", 0 );
	    return;
	}
	break;

    case CON_GEN_GROUPS:
	send_to_char("\n\r",ch);
	if (!str_cmp(argument,"done"))
	{
	    sprintf(buf,"Creation points: %d\n\r",ch->pcdata->points);
	    send_to_char(buf,ch);
	    sprintf(buf,"Experience modifier: %d (Percent of difference from the norm)\n\r",
		    figure_difference(ch->gen_data->points_chosen));
	    if (ch->pcdata->points < 40)
		ch->train = (40 - ch->pcdata->points + 1) / 2;
	    send_to_char(buf,ch);
	    write_to_buffer( d, "\n\r", 2 );
	    do_help( ch, "motd" );
	    d->connected = CON_READ_MOTD;
	    break;
	}

	if (!parse_gen_groups(ch,argument))
	send_to_char(
	"Choices are: list,learned,premise,add,drop,info,help, and done.\n\r"
	,ch);

	do_help(ch,"menu choice");
	break;

    case CON_READ_IMOTD:
	if( !str_cmp( argument, "hidden") )
	{
		SET_BIT(ch->act, PLR_NO_ANNOUNCE);
	}
	write_to_buffer(d,"\n\r",2);
	do_help( ch, "motd" );
	d->connected = CON_READ_MOTD;
	break;

    case CON_READ_MOTD:
	write_to_buffer( d, 
    "\n\rWelcome to EmberMUD.\n\r",
	    0 );
	ch->next        = char_list;
	char_list       = ch;
	d->connected    = CON_PLAYING;
	reset_char(ch);

	sprintf( buf, "%s has entered the game.", ch->name);
	if( IS_SET(ch->act, PLR_NO_ANNOUNCE) )
	{
		SET_BIT(ch->act, PLR_NO_ANNOUNCE);
	}
	else
	{
	do_sendinfo(ch, buf);
	}
	sprintf(buf, "%s@%s[%d] has connected.", ch->name, d->host, URANGE( 1, ch->level, 100 ) );
	send_wiz(UMAX(92, ch->level), buf);

	if ( ch->level == 0 )
	{

	    ch->level   = 1;
	    ch->exp     = 0;
	    ch->hit     = ch->max_hit;
	    ch->mana    = ch->max_mana;
	    ch->move    = ch->max_move;
	    ch->train    = 5;
	    ch->practice = 10;
	    set_title( ch, "the newbie" );

	    do_outfit(ch,"");
	    obj_to_char(create_object(get_obj_index(OBJ_VNUM_MAP),0),ch);

	    ch->pcdata->learned[get_weapon_sn(ch)]= 40;

	    char_to_room( ch, get_room_index( ROOM_VNUM_SCHOOL ) );
	    send_to_char("\n\r",ch);
	    save_char_obj( ch );
	    do_help(ch,"NEWBIE INFO");
	    send_to_char("\n\r",ch);
	}
	else if ( ch->in_room != NULL )
	{
	    char_to_room( ch, ch->in_room );
	}
	else if ( IS_IMMORTAL(ch) )
	{
	    char_to_room( ch, get_room_index( ROOM_VNUM_CHAT ) );
	}
	else
	{
	    char_to_room( ch, get_room_index( ROOM_VNUM_TEMPLE ) );
	}

	act( "$n has entered the game.", ch, NULL, NULL, TO_ROOM );
	do_look( ch, "auto" );
	do_board (ch, ""); /* Show board status */

	if (ch->pet != NULL)
	{
	    char_to_room(ch->pet,ch->in_room);
	    act("$n has entered the game.",ch->pet,NULL,NULL,TO_ROOM);
	}

		if (IS_SET(ch->act,PLR_AFK))
		do_afk(ch,NULL);

		ch->pcdata->chaos_score = 0;
	if (ch->pcdata->clan != 0 )
	{
	CLAN_DATA *clan;
	clan = get_clan(ch->pcdata->clan);
	if( IS_SET(clan->clan_flags, CLAN_DISBANDING) )
	{
		send_to_char("Your clan is disbanding.\n\r"
			"Type RESIGN twice to resign from the clan, please.\n\r", ch);
	}
	}
		break;

	case CON_NOTE_TO:
		handle_con_note_to (d, argument);
		break;
		
	case CON_NOTE_SUBJECT:
		handle_con_note_subject (d, argument);
		break; /* subject */
	
	case CON_NOTE_EXPIRE:
		handle_con_note_expire (d, argument);
		break;

	case CON_NOTE_TEXT:
		handle_con_note_text (d, argument);
		break;
		
	case CON_NOTE_FINISH:
		handle_con_note_finish (d, argument);
		break;
    }

    return;
}



/*
 * Parse a name for acceptability.
 */
bool check_parse_name( char *name )
{
    /*
     * Reserved words.
     */
    if ( is_exact_name( name, "all auto immortal self someone something the you fuck shit rapist irapedthemis" )
       || is_name(name, "fuck") || is_name(name, "shit") || is_name(name, "imm") )
	return FALSE;
	
    if (str_cmp(capitalize(name),"Themis") && (!str_prefix("themi",name)
    || !str_suffix("themis",name)))
	return FALSE;

    /*
     * Length restrictions.
     */
     
    if ( strlen(name) <  2 )
	return FALSE;

#if defined(MSDOS)
    if ( strlen(name) >  8 )
	return FALSE;
#endif

#if defined(macintosh) || defined(unix)
    if ( strlen(name) > 12 )
	return FALSE;
#endif

    /*
     * Alphanumerics only.
     * Lock out IllIll twits.
     */
    {
	char *pc;
	bool fIll;

	fIll = TRUE;
	for ( pc = name; *pc != '\0'; pc++ )
	{
	    if ( !isalpha(*pc) )
		return FALSE;
	    if ( LOWER(*pc) != 'i' && LOWER(*pc) != 'l' )
		fIll = FALSE;
	}

	if ( fIll )
	    return FALSE;
    }

    /*
     * Prevent players from naming themselves after mobs.
     */
/* I've had too many problems with people naming mobs after players!
   I'm leaving this here, but I copied this routine over to the nanny routine.
		-Kyle
*/
/*  
  {
	extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
	MOB_INDEX_DATA *pMobIndex;
	int iHash;

	for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
	{
	    for ( pMobIndex  = mob_index_hash[iHash];
		  pMobIndex != NULL;
		  pMobIndex  = pMobIndex->next )
	    {
		if ( is_exact_name( name, pMobIndex->player_name ) )
		    return FALSE;
	    }
	}
    }
*/
    return TRUE;
}



/*
 * Look for link-dead player to reconnect.
 */
bool check_reconnect( DESCRIPTOR_DATA *d, char *name, bool fConn )
{
    CHAR_DATA *ch;

    for ( ch = char_list; ch != NULL; ch = ch->next )
    {
	if ( !IS_NPC(ch)
	&&   (!fConn || ch->desc == NULL)
	&&   !str_cmp( d->character->name, ch->name ) )
	{
	    if ( fConn == FALSE )
	    {
		free_string( d->character->pcdata->pwd );
		d->character->pcdata->pwd = str_dup( ch->pcdata->pwd );
	    }
	    else
	    {
		free_char( d->character );
		d->character = ch;
		ch->desc         = d;
		ch->timer        = 0;
		send_to_char( "Reconnecting.\n\r", ch );
		act( "$n has reconnected.", ch, NULL, NULL, TO_ROOM );
		sprintf( log_buf, "%s@%s reconnected.", ch->name, d->host );
		log_string( log_buf );
		send_wiz(UMAX(94,ch->level), log_buf);
		d->connected = CON_PLAYING;
		/* Inform the character of a note in progress and the possbility of continuation! */            
		if (ch->pcdata->in_progress)
			send_to_char ("You have a note in progress. Type NWRITE to continue it.\n\r",ch);
	    }
	    return TRUE;
	}
    }

    return FALSE;
}



/*
 * Check if already playing.
 */
bool check_playing( DESCRIPTOR_DATA *d, char *name )
{
    DESCRIPTOR_DATA *dold;

    for ( dold = descriptor_list; dold; dold = dold->next )
    {
	if ( dold != d
	&&   dold->character != NULL
	&&   dold->connected != CON_GET_NAME
	&&   dold->connected != CON_GET_OLD_PASSWORD
	&&   !str_cmp( name, dold->original
		 ? dold->original->name : dold->character->name ) )
	{
	    write_to_buffer( d, "That character is already playing.\n\rDisconnect that character? ",0);
	    d->connected = CON_BREAK_CONNECT;
	    return TRUE;
	}
    }

    return FALSE;
}



void stop_idling( CHAR_DATA *ch )
{
    if ( ch == NULL
    ||   ch->desc == NULL
    ||   ch->desc->connected != CON_PLAYING
    ||   ch->was_in_room == NULL 
    ||   ch->in_room != get_room_index(ROOM_VNUM_LIMBO))
	return;

    ch->timer = 0;
    char_from_room( ch );
    char_to_room( ch, ch->was_in_room );
    ch->was_in_room     = NULL;
    act( "$n has returned from the void.", ch, NULL, NULL, TO_ROOM );
    return;
}

void printf_to_char(CHAR_DATA *ch, char *fmt, ... )
{
	char param[ MAX_STRING_LENGTH*4 ];
	{
	    va_list args;
	    va_start (args, fmt);
	    vsprintf (param, fmt, args);
	    va_end (args);
	}
	if ( param[0] && ch->desc )
             write_to_buffer( ch->desc, param, strlen(param) );
           
	return;
}

/*
 * Write to one char.
 */
void send_to_char( const char *txt, CHAR_DATA *ch )
{
    if ( txt != NULL && ch->desc != NULL )
	write_to_buffer( ch->desc, txt, strlen(txt) );
    return;
}



/*
 * Send a page to one char.
 */
void page_to_char( const char *txt, CHAR_DATA *ch )
{
    if ( txt == NULL || ch->desc == NULL)
	return;
	
#if defined(macintosh)
	send_to_char(txt,ch);
#else
   if (ch->desc==NULL) return;
   ch->desc->showstr_head = alloc_mem(strlen(txt) + 1);
   strcpy(ch->desc->showstr_head,txt);
   ch->desc->showstr_point = ch->desc->showstr_head;
   show_string(ch->desc,"");
#endif
}


/* string pager */
void show_string(struct descriptor_data *d, char *input)
{
   char buf[MAX_STRING_LENGTH]; 
   char buffer[4*MAX_STRING_LENGTH];
   register char *scan, *chk;
   int lines = 0, toggle = 1;
   int show_lines;

   one_argument(input,buf);
   if (buf[0] != '\0')
     {
	if (d->showstr_head)
	  {
	     free_string(d->showstr_head);
	     d->showstr_head = 0;
	  }
	d->showstr_point  = 0;
	return;
     }
   
   if (d->character)
     show_lines = d->character->lines;
   else
     show_lines = 0;
   
   for (scan = buffer; ; scan++, d->showstr_point++)
     {
	if (((*scan = *d->showstr_point) == '\n' || *scan == '\r')
	    && (toggle = -toggle) < 0)
	  lines++;
	
	else if (!*scan || (show_lines > 0 && lines >= show_lines))
	  {
	     *scan = '\0';
	     write_to_buffer(d,buffer,strlen(buffer));
	     for (chk = d->showstr_point; isspace(*chk); chk++);
	       {
		  if (!*chk)
		    {
		       if (d->showstr_head)
			 {
			    free_string(d->showstr_head);
			    d->showstr_head = 0;
			 }
		       d->showstr_point  = 0;
		    }
	       }
	     return;
	  }
     }
   return;
}


/* quick sex fixer */
void fix_sex(CHAR_DATA *ch)
{
    if (ch->sex < 0 || ch->sex > 2)
	ch->sex = IS_NPC(ch) ? 0 : ch->pcdata->true_sex;
}

void act (const char *format, CHAR_DATA *ch, const void *arg1, const void *arg2,
	  int type)
{
    /* to be compatible with older code */
    act_new(format,ch,arg1,arg2,type,POS_RESTING);
}

#define NAME(ch)	(IS_NPC(ch) ? ch->short_descr : ch->name)
char *act_string(const char *format, CHAR_DATA *to, CHAR_DATA *ch,
 		 const void *arg1, const void *arg2)
{
     static char * const he_she  [] = { "it",  "he",  "she" };
     static char * const him_her [] = { "it",  "him", "her" };
     static char * const his_her [] = { "its", "his", "her" };
     static char buf[MAX_STRING_LENGTH]; 
     char fname[MAX_INPUT_LENGTH];       
     char *point;  
     const char *str = format;
     const char *i;      
     CHAR_DATA *vch = (CHAR_DATA *) arg2;
     OBJ_DATA *obj1 = (OBJ_DATA  *) arg1;
     OBJ_DATA *obj2 = (OBJ_DATA  *) arg2;    
     
     bzero(buf, sizeof(buf));
     point = buf;
     while ( *str )
     {
         if ( *str != '$' )
         {
             *point++ = *str++;
             continue;
         }
         ++str;
  
         if ( !arg2 && *str >= 'A' && *str <= 'Z' )
         {
             bug( "Act: missing arg2 for code %d.", *str );
             i = " <@@@> ";
         }
         else
         {
             switch ( *str )
             {
                 default:  bug( "Act: bad code %d.", *str );
                           i = " <@@@> ";                                break;
                 /* Thx alex for 't' idea */
                 case 't': i = (char *) arg1;                            break;
                 case 'T': i = (char *) arg2;                            break;
                 case 'n': i = ( to ? PERS( ch, to) : NAME( ch) );	break;
                 case 'N': i = ( to ? PERS(vch, to) : NAME(vch) );	break;
                 case 'e': i = he_she  [URANGE(0, ch  ->sex, 2)];	break;
                 case 'E': i = he_she  [URANGE(0, vch ->sex, 2)];	break;
                 case 'm': i = him_her [URANGE(0, ch  ->sex, 2)];	break;
                 case 'M': i = him_her [URANGE(0, vch ->sex, 2)];	break;
                 case 's': i = his_her [URANGE(0, ch  ->sex, 2)];	break;
                 case 'S': i = his_her [URANGE(0, vch ->sex, 2)];	break;
                 case 'p':
                     i = (!to || can_see_obj( to, obj1 )
                             ? obj1->short_descr : "something" );	break;
                 case 'P':
                     i = ( !to || can_see_obj( to, obj2 )
                             ? obj2->short_descr : "something" );	break;
                 case 'd':
                     if ( !arg2 || ((char *) arg2)[0] == '\0' )
                         i = "door";
                     else
                     {
                         one_argument( (char *) arg2, fname );
                         i = fname;
                     }
                     break;
             }
         }
  
         ++str;
         while ( ( *point = *i ) )
                 ++point, ++i;
     }
     *point++ = '`';
     *point++ = 'w'; 
     *point++ = '\n';
     *point++ = '\r';
     *point   = '\0';
     buf[0]   = UPPER(buf[0]);    
  
     return buf;
}

void act_new( const char *format, CHAR_DATA *ch, const void *arg1, 
	      const void *arg2, int type, int min_pos)
{
   char buf[MAX_STRING_LENGTH];
   
   static char * const he_she  [] = { "it",  "he",  "she" };
    static char * const him_her [] = { "it",  "him", "her" };
    static char * const his_her [] = { "its", "his", "her" };
    char * txt;
    char fname[MAX_INPUT_LENGTH];
    CHAR_DATA *to;
    CHAR_DATA *vch = (CHAR_DATA *) arg2;
    OBJ_DATA *obj1 = (OBJ_DATA  *) arg1;
    OBJ_DATA *obj2 = (OBJ_DATA  *) arg2;
    const char *str;
    const char *i;
    char *point;
 
    /*
     * Discard null and zero-length messages.
     */
    if ( format == NULL || format[0] == '\0' )
	return;

    /* discard null rooms and chars */
    if (ch == NULL || ch->in_room == NULL)
	return;

    to = ch->in_room->people;
    /* hope this fixes some stuff */
    if( to == NULL )
	return;
    if ( type == TO_VICT )
    {
	if ( vch == NULL )
	{
	    bug( "Act: null vch with TO_VICT.", 0 );
	    return;
	}

	if (vch->in_room == NULL)
	    return;

	to = vch->in_room->people;
    }
 
/* room progs & obj progs*/
    if ( MOBtrigger && type != TO_CHAR && type != TO_VICT && to )
    {
        OBJ_DATA *to_obj;
        txt = act_string( format, NULL, ch, arg1, arg2 );
        if ( IS_SET( to->in_room->progtypes, ACT_PROG ) )
            rprog_act_trigger( txt, to->in_room, ch, (OBJ_DATA *)arg1, (void *)arg2 );
        for ( to_obj = to->in_room->contents; to_obj;
              to_obj = to_obj->next_content )
            if ( IS_SET(to_obj->pIndexData->progtypes, ACT_PROG) )
               oprog_act_trigger(txt, to_obj, ch, (OBJ_DATA *)arg1, (void *)arg2);            
    }

    for ( ; to != NULL; to = to->next_in_room )
    {
	if ( to == NULL ) { bug("to==NULL in act_new.",0); return; }
	if ( to->desc == NULL || to->position < min_pos )
	    continue;
 
	if ( type == TO_CHAR && to != ch )
	    continue;
	if ( type == TO_VICT && ( to != vch || to == ch ) )
	    continue;
	if ( type == TO_ROOM && to == ch )
	    continue;
	if ( type == TO_NOTVICT && (to == ch || to == vch) )
	    continue;
 
	point   = buf;
	str     = format;
	while ( *str != '\0' )
	{
	    if ( *str != '$' )
	    {
		*point++ = *str++;
		continue;
	    }
	    ++str;
 
	    if ( arg2 == NULL && *str >= 'A' && *str <= 'Z' )
	    {
		bug( "Act: missing arg2 for code %d.", *str );
		i = " <@@@> ";
	    }
	    else
	    {
		switch ( *str )
		{
		default:  bug( "Act: bad code %d.", *str );
			  i = " <@@@> ";                                break;
		/* Thx alex for 't' idea */
		case 't': i = (char *) arg1;                            break;
		case 'T': i = (char *) arg2;                            break;
		case 'n': i = PERS( ch,  to  );                         break;
		case 'N': i = PERS( vch, to  );                         break;
		case 'e': i = he_she  [URANGE(0, ch  ->sex, 2)];        break;
		case 'E': i = he_she  [URANGE(0, vch ->sex, 2)];        break;
		case 'm': i = him_her [URANGE(0, ch  ->sex, 2)];        break;
		case 'M': i = him_her [URANGE(0, vch ->sex, 2)];        break;
		case 's': i = his_her [URANGE(0, ch  ->sex, 2)];        break;
		case 'S': i = his_her [URANGE(0, vch ->sex, 2)];        break;
 
		case 'p':
		    i = can_see_obj( to, obj1 )
			    ? obj1->short_descr
			    : "something";
		    break;
 
		case 'P':
		    i = can_see_obj( to, obj2 )
			    ? obj2->short_descr
			    : "something";
		    break;
 
		case 'd':
		    if ( arg2 == NULL || ((char *) arg2)[0] == '\0' )
		    {
			i = "door";
		    }
		    else
		    {
			one_argument( (char *) arg2, fname );
			i = fname;
		    }
		    break;
		}
	    }
 
	    ++str;
	    while ( ( *point = *i ) != '\0' )
		++point, ++i;
	}
	
	*point++ = '`';
	*point++ = 'w'; 
	*point++ = '\n';
	*point++ = '\r';
	buf[0]   = UPPER(buf[0]);
	    if (to->desc)
	       write_to_buffer( to->desc, buf, point - buf );
	    if (MOBtrigger)
	       mprog_act_trigger( buf, to, ch, obj1, vch );
	
    }
    MOBtrigger = TRUE;
    return;
}



/*
 * Macintosh support functions.
 */
#if defined(macintosh)
int gettimeofday( struct timeval *tp, void *tzp )
{
    tp->tv_sec  = time( NULL );
    tp->tv_usec = 0;
}
#endif

char *do_color(char *plaintext, bool color)
{
	static char color_text[MAX_STRING_LENGTH*2];
	char *ct_point;
	
	bzero(color_text, sizeof(color_text));
	ct_point=color_text;
	while ( *plaintext != '\0' ) {
		if ( *plaintext != '`' ) {
			*ct_point = *plaintext;
			ct_point++;
			plaintext++;
			continue;
		}
		plaintext++;
		if (!color)
		  switch(*plaintext) {
			case 'k':
			case 'K':
			case 'r':
			case 'R':
			case 'b':
			case 'B':
			case 'c':
			case 'C':
			case 'Y':
			case 'y':
			case 'm':
			case 'M':
			case 'w':
			case 'W':
			case 'g':
			case 'G':
				plaintext++;
				break;
			default:
				strcat(color_text, "`");
				ct_point++;
				break;
		  }
		else
		  switch(*plaintext) {
			case 'k':
				strcat(color_text, "[0;30m");
				ct_point+=7;
				plaintext++; 
				break;
			case 'K':
				strcat(color_text, "[1;30m");
				ct_point+=7;
				plaintext++; 
				break;
			case 'r':
				strcat(color_text, "[0;31m");
				ct_point+=7;
				plaintext++; 
				break;
			case 'R':
				strcat(color_text, "[1;31m");
				ct_point+=7;
				plaintext++; 
				break;
			case 'g':
				strcat(color_text, "[0;32m");
				ct_point+=7;
				plaintext++; 
				break;
			case 'G':
				strcat(color_text, "[1;32m");
				ct_point+=7;
				plaintext++; 
				break;
			case 'y':
				strcat(color_text, "[0;33m");
				ct_point+=7;
				plaintext++; 
				break;
			case 'Y':
				strcat(color_text, "[1;33m");
				ct_point+=7;
				plaintext++; 
				break;
			case 'b':
				strcat(color_text, "[0;34m");
				ct_point+=7;
				plaintext++; 
				break;
			case 'B':
				strcat(color_text, "[1;34m");
				ct_point+=7;
				plaintext++; 
				break;
			case 'm':
				strcat(color_text, "[0;35m");
				ct_point+=7;
				plaintext++; 
				break;
			case 'M':
				strcat(color_text, "[1;35m");
				ct_point+=7;
				plaintext++; 
				break;
			case 'c':
				strcat(color_text, "[0;36m");
				ct_point+=7;
				plaintext++; 
				break;
			case 'C':
				strcat(color_text, "[1;36m");
				ct_point+=7;
				plaintext++; 
				break;
			case 'w':
				strcat(color_text, "[0;37m");
				ct_point+=7;
				plaintext++; 
				break;
			case 'W':
				strcat(color_text, "[1;37m");
				ct_point+=7;
				plaintext++; 
				break;
			default:
				strcat(color_text, "`");
				ct_point++;
				break;
		}
	}
	strcat(color_text, "[0m\0");
	return(color_text);
}

char *figurestate(int current, int max)
{
    static char status[40];
    
    bzero(status, sizeof(status));
    if (current >= (max/3*2)) sprintf(status, "`W%d`w", current);
    else if (current >= max/3) sprintf(status, "`Y%d`w", current);
    else sprintf(status, "`R%d`w", current);
    return (status);
}

char *damstatus(CHAR_DATA *ch)
{
    int percent;
    static char wound[40];
 
    bzero(wound, sizeof(wound));
    if (ch->max_hit > 0)
	percent = ch->hit * 100 / ch->max_hit;
    else
	percent = -1;
    if (percent >= 100)
	sprintf(wound,"excellent condition");
    else if (percent >= 90)
	sprintf(wound,"few scratches");
    else if (percent >= 75)
	sprintf(wound,"small wounds");
    else if (percent >= 50)
	sprintf(wound,"quite a few wounds");
    else if (percent >= 30)
	sprintf(wound,"nasty wounds");
    else if (percent >= 15)
	sprintf(wound,"pretty hurt");
    else if (percent >= 0)
	sprintf(wound,"awful condition");
    else
	sprintf(wound,"bleeding to death");
    return (wound);
}

char *doparseprompt(CHAR_DATA *ch)
{
   CHAR_DATA *tank,*victim;
   static char finished_prompt[240];
   char workstr[100];
   char *fp_point;
   char *orig_prompt;
   
   bzero(finished_prompt, sizeof(finished_prompt));
   orig_prompt=ch->pcdata->prompt;
   fp_point=finished_prompt;
   while(*orig_prompt != '\0') {
      if (*orig_prompt != '%') {
	 *fp_point = *orig_prompt;
	 orig_prompt++;
	 fp_point++;
	 continue;
      }
      orig_prompt++;
      switch(*orig_prompt) 
	{
	 case 'h':      sprintf(workstr, "%d", ch->hit);
			strcat(finished_prompt, workstr);
			fp_point+=strlen(workstr);
			orig_prompt++;
			break;
	 case 'H':      sprintf(workstr, "%d", ch->max_hit);
			strcat(finished_prompt, workstr);
			fp_point+=strlen(workstr);
			orig_prompt++;
			break;
	 case 'm':      sprintf(workstr, "%d", ch->mana);
			strcat(finished_prompt, workstr);
			fp_point+=strlen(workstr);
			orig_prompt++;
			break;
	 case 'M':      sprintf(workstr, "%d", ch->max_mana);
			strcat(finished_prompt, workstr);
			fp_point+=strlen(workstr);
			orig_prompt++;
			break;
	 case 'v':      sprintf(workstr, "%d", ch->move);
			strcat(finished_prompt, workstr);
			fp_point+=strlen(workstr);
			orig_prompt++;
			break;
	 case 'V':      sprintf(workstr, "%d", ch->max_move);
			strcat(finished_prompt, workstr);
			fp_point+=strlen(workstr);
			orig_prompt++;
			break;
	 case 'r':      strcat(finished_prompt, "\n\r");
			fp_point++;
			fp_point++;
			orig_prompt++;
			break;
	 case 'i':      sprintf(workstr, "%s", figurestate(ch->hit, ch->max_hit));
			strcat(finished_prompt, workstr);
			fp_point+=strlen(workstr);
			orig_prompt++;
			break;
	 case 'n':      sprintf(workstr, "%s", figurestate(ch->mana, ch->max_mana));
			strcat(finished_prompt, workstr);
			fp_point+=strlen(workstr);
			orig_prompt++;
			break;
	 case 'w':      sprintf(workstr, "%s", figurestate(ch->move, ch->max_move));
			strcat(finished_prompt, workstr);
			fp_point+=strlen(workstr);
			orig_prompt++;
			break;
	 case 'l':      if ((tank=ch->fighting) != NULL) 
			  if ((tank=tank->fighting) != NULL) 
				{
				   sprintf(workstr, "%s", damstatus(tank));
				   strcat(finished_prompt, workstr);
				   fp_point+=strlen(workstr);
				 }
			orig_prompt++;
			break;
	 case 'e':      if ((victim=ch->fighting) != NULL) 
			   {
				sprintf(workstr, "%s", damstatus(victim));
				strcat(finished_prompt, workstr);
				fp_point+=strlen(workstr);
			   }
			orig_prompt++;
			break;
	 case 's':      sprintf(workstr, "%s", damstatus(ch));
			strcat(finished_prompt, workstr);
			fp_point+=strlen(workstr);
			orig_prompt++;
			break;
	 case 'T':      sprintf(workstr, "%s", get_curtime() );
			strcat(finished_prompt, workstr);
			fp_point+=strlen(workstr);
			orig_prompt++;
			break;
	 case '#':      if( IS_IMMORTAL( ch ) && ch->in_room != NULL )
			  {
			     sprintf(workstr, "%d", ch->in_room->vnum);
			     strcat(finished_prompt, workstr);
			     fp_point+=strlen(workstr);
			     orig_prompt++;
			     break;
			   }
			else
			   break;
	 case 'a':
			sprintf(workstr, "%d", ch->alignment);
			strcat(finished_prompt, workstr);
			fp_point+=strlen(workstr);
			orig_prompt++;
			break;
	 case 'A':       if (IS_SET(ch->act,PLR_AFK))
					{
					 sprintf(workstr, "`W(AFK)");
					 strcat(finished_prompt, workstr);
					 fp_point+=strlen(workstr);
					}
					if (IS_SET(ch->act,PLR_KILLER))
					{
					 sprintf(workstr, "`R(PK)");
					 strcat(finished_prompt, workstr);
					 fp_point+=strlen(workstr);
					}
					if (IS_SET(ch->act,PLR_THIEF))
					{
					 sprintf(workstr, "`K(THIEF)");
					 strcat(finished_prompt, workstr);
					 fp_point+=strlen(workstr);
					}
					if( IS_IMMORTAL( ch ) && IS_SET(ch->act, PLR_WIZINVIS))
					{
					 sprintf(workstr, "`B(Wizi:%d)",ch->invis_level);
					 strcat(finished_prompt, workstr);
					 fp_point+=strlen(workstr);
					}
					orig_prompt++;
					break;
	 case 'x':       sprintf(workstr, "%ld", ch->exp);
			 strcat(finished_prompt, workstr);
			 fp_point+=strlen(workstr);
			 orig_prompt++;
			 break;
	 case 'X':       sprintf(workstr, "%ld", exp_per_level(ch,ch->pcdata->points)-ch->exp);
			 strcat(finished_prompt, workstr);
			 fp_point+=strlen(workstr);
			 orig_prompt++;
			 break;
	 default:       strcat(finished_prompt, "%");
			fp_point++;
			break;                  
	}
   }
   return(finished_prompt);
}
