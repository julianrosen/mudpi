/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'. In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Thanks to abaddon for proof-reading our comm.c and pointing out bugs.  *
 *  Any remaining bugs are, of course, our work, not his.  :)              *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting. We hope that you share your changes too. What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/**************************************************************************
 * Mudprogram's (Mobprogram, Objprogram and Roomprogram) originaly 	  *
 * by the SMAUG development team             				  *
 * Ported to EmberMUD by Thanatos and Tyrluk of ToED      		  *
 * (Temple of Eternal Death)    					  *
 * Tyrluk   - morn@telmaron.com or dajy@mindspring.com			  *
 * Thanatos - morn@telmaron.com or jonathan_w._rose@ffic.com              *
 **************************************************************************/

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

#if defined(WIN32)
#include <windows.h>
#else
#include <sys/time.h>
#include <sys/wait.h>
#endif

#if defined(cbuilder)
#include <dir.h>
#endif

#include <sys/types.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "merc.h"
#include <math.h>

/* command procedures needed */
DECLARE_DO_FUN( do_help );
DECLARE_DO_FUN( do_look );
DECLARE_DO_FUN( do_skills );
DECLARE_DO_FUN( do_outfit );
DECLARE_DO_FUN( do_afk );

/*
 * Malloc debugging stuff.
 */
#if defined(sun)
#undef MALLOC_DEBUG
#endif

#if defined(MALLOC_DEBUG)
#include <malloc.h>
extern int malloc_debug args( ( int ) );
extern int malloc_verify args( ( void ) );
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
#if     defined(unix)
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/telnet.h>
const char echo_off_str[] = { IAC, WILL, TELOPT_ECHO, '\0' };
const char echo_on_str[] = { IAC, WONT, TELOPT_ECHO, '\0' };
const char go_ahead_str[] = { IAC, GA, '\0' };
const char will_suppress_ga_str[] = { IAC, WILL, TELOPT_SGA, '\0' };
const char wont_suppress_ga_str[] = { IAC, WONT, TELOPT_SGA, '\0' };
#endif

#if     defined(WIN32)
#include "Win32Common\telnet.h"
const char echo_off_str[] = { ( char ) IAC, ( char ) WILL, TELOPT_ECHO, '\0' };
const char echo_on_str[] = { ( char ) IAC, ( char ) WONT, TELOPT_ECHO, '\0' };
const char go_ahead_str[] = { ( char ) IAC, ( char ) GA, '\0' };
const char will_suppress_ga_str[] =
    { ( char ) IAC, ( char ) WILL, TELOPT_SGA, '\0' };
const char wont_suppress_ga_str[] =
    { ( char ) IAC, ( char ) WONT, TELOPT_SGA, '\0' };
#endif

#if	defined(cbuilder)
extern void AddUser( CHAR_DATA * ch );
extern void RemoveUser( CHAR_DATA * ch );
extern bool MudDown;
#endif

/*
 * OS-dependent declarations.
 */
#if     defined(_AIX)
#include <sys/select.h>
int accept args( ( int s, struct sockaddr * addr, int *addrlen ) );
int bind args( ( int s, struct sockaddr * name, int namelen ) );
void bzero args( ( char *b, int length ) );
int getpeername args( ( int s, struct sockaddr * name, int *namelen ) );
int getsockname args( ( int s, struct sockaddr * name, int *namelen ) );
//int gettimeofday args( ( struct timeval * tp, struct timezone * tzp ) );
int gettimeofday( );
int listen args( ( int s, int backlog ) );
int setsockopt args( ( int s, int level, int optname, void *optval,
                       int optlen ) );
int socket args( ( int domain, int type, int protocol ) );
#endif

#if     defined(apollo)
#include <unistd.h>
void bzero args( ( char *b, int length ) );
#endif

#if     defined(__hpux)
int accept args( ( int s, void *addr, int *addrlen ) );
int bind args( ( int s, const void *addr, int addrlen ) );
void bzero args( ( char *b, int length ) );
int getpeername args( ( int s, void *addr, int *addrlen ) );
int getsockname args( ( int s, void *name, int *addrlen ) );
//int gettimeofday args( ( struct timeval * tp, struct timezone * tzp ) );
int gettimeofday( );
int listen args( ( int s, int backlog ) );
int setsockopt args( ( int s, int level, int optname,
                       const void *optval, int optlen ) );
int socket args( ( int domain, int type, int protocol ) );
#endif

#if     defined(interactive)
#include <net/errno.h>
#include <sys/fnctl.h>
#endif

#if     defined(linux)
int close args( ( int fd ) );
/* int     getpeername     args( ( int s, struct sockaddr *name, int *namelen ) ); */
/* int     getsockname     args( ( int s, struct sockaddr *name, int *namelen ) ); */
//int gettimeofday args( ( struct timeval * tp, struct timezone * tzp ) ); // This needs to be changed for Ubuntu
int gettimeofday( ); // OK, this should work for whatever
/* int     listen          args( ( int s, int backlog ) ); */
int select args( ( int width, fd_set * readfds, fd_set * writefds,
                   fd_set * exceptfds, struct timeval * timeout ) );
int socket args( ( int domain, int type, int protocol ) );
#endif

#if     defined(MIPS_OS)
extern int errno;
#endif

#if     defined(NeXT)
int close args( ( int fd ) );
int fcntl args( ( int fd, int cmd, int arg ) );
#if     !defined(htons)
u_short htons args( ( u_short hostshort ) );
#endif
#if     !defined(ntohl)
u_long ntohl args( ( u_long hostlong ) );
#endif
int read args( ( int fd, char *buf, int nbyte ) );
int select args( ( int width, fd_set * readfds, fd_set * writefds,
                   fd_set * exceptfds, struct timeval * timeout ) );
int write args( ( int fd, char *buf, int nbyte ) );
#endif

#if     defined(sequent)
int accept args( ( int s, struct sockaddr * addr, int *addrlen ) );
int bind args( ( int s, struct sockaddr * name, int namelen ) );
int close args( ( int fd ) );
int fcntl args( ( int fd, int cmd, int arg ) );
int getpeername args( ( int s, struct sockaddr * name, int *namelen ) );
int getsockname args( ( int s, struct sockaddr * name, int *namelen ) );
//int gettimeofday args( ( struct timeval * tp, struct timezone * tzp ) );
int gettimeofday( );
#if     !defined(htons)
u_short htons args( ( u_short hostshort ) );
#endif
int listen args( ( int s, int backlog ) );
#if     !defined(ntohl)
u_long ntohl args( ( u_long hostlong ) );
#endif
int read args( ( int fd, char *buf, int nbyte ) );
int select args( ( int width, fd_set * readfds, fd_set * writefds,
                   fd_set * exceptfds, struct timeval * timeout ) );
int setsockopt args( ( int s, int level, int optname, caddr_t optval,
                       int optlen ) );
int socket args( ( int domain, int type, int protocol ) );
int write args( ( int fd, char *buf, int nbyte ) );
#endif

/* This includes Solaris Sys V as well */
#if defined(sun)
int accept args( ( int s, struct sockaddr * addr, int *addrlen ) );
int bind args( ( int s, struct sockaddr * name, int namelen ) );
void bzero args( ( char *b, int length ) );
int close args( ( int fd ) );
int getpeername args( ( int s, struct sockaddr * name, int *namelen ) );
int getsockname args( ( int s, struct sockaddr * name, int *namelen ) );
//int gettimeofday args( ( struct timeval * tp, struct timezone * tzp ) );
int gettimeofday( );
int listen args( ( int s, int backlog ) );
int read args( ( int fd, char *buf, int nbyte ) );
int select args( ( int width, fd_set * readfds, fd_set * writefds,
                   fd_set * exceptfds, struct timeval * timeout ) );
#if defined(SYSV)
int setsockopt args( ( int s, int level, int optname,
                       const char *optval, int optlen ) );
#else
int setsockopt args( ( int s, int level, int optname, void *optval,
                       int optlen ) );
#endif
int socket args( ( int domain, int type, int protocol ) );
int write args( ( int fd, char *buf, int nbyte ) );
#endif

#if defined(ultrix)
int accept args( ( int s, struct sockaddr * addr, int *addrlen ) );
int bind args( ( int s, struct sockaddr * name, int namelen ) );
void bzero args( ( char *b, int length ) );
int close args( ( int fd ) );
int getpeername args( ( int s, struct sockaddr * name, int *namelen ) );
int getsockname args( ( int s, struct sockaddr * name, int *namelen ) );
//int gettimeofday args( ( struct timeval * tp, struct timezone * tzp ) );
int gettimeofday( );
int listen args( ( int s, int backlog ) );
int read args( ( int fd, char *buf, int nbyte ) );
int select args( ( int width, fd_set * readfds, fd_set * writefds,
                   fd_set * exceptfds, struct timeval * timeout ) );
int setsockopt args( ( int s, int level, int optname, void *optval,
                       int optlen ) );
int socket args( ( int domain, int type, int protocol ) );
int write args( ( int fd, char *buf, int nbyte ) );
#endif


// JR
void prompt_race ( DESCRIPTOR_DATA *, CHAR_DATA *, int );
void motd( CHAR_DATA * );





/*
 * Global variables.
 */
DESCRIPTOR_DATA *descriptor_free = 0;   /* Free list for descriptors    */
DESCRIPTOR_DATA *descriptor_list = 0;   /* All open descriptors         */
LAST_DATA *flast = 0;
LAST_DATA *last_list = 0;
LAST_DATA *flast_imm = 0;
LAST_DATA *last_imm = 0;
LAST_DATA *flast_admin = 0;
LAST_DATA *last_admin = 0;
LAST_DATA *flast_hero = 0;
LAST_DATA *last_hero = 0;
DESCRIPTOR_DATA *d_next = 0;    /* Next descriptor in loop       */
FILE *fpReserve = 0;            /* Reserved file handle          */
bool god;                       /* All new chars are gods!       */
bool merc_down;                 /* Shutdown                      */
bool wizlock;                   /* Game is wizlocked             */
bool newlock;                   /* Game is newlocked             */
bool chaos;                     /* Game in CHAOS!                */
bool reap_shells = FALSE;
bool silentmode;                /* Send no output to descriptors */
char str_boot_time[MAX_INPUT_LENGTH];
time_t current_time;            /* time of this pulse */
bool rolled = FALSE;
//int stat1[5], stat2[5], stat3[5], stat4[5], stat5[5];
int stat[5][NUM_ROLLS]; // JR
const char* const stat_names[] = { "Strength", "Intelligence", 
                                 "Wisdom", "Dexterity", "Constitution" };


bool fCopyOver;
int port;
bool MOBtrigger;
int control;
#ifdef HEAVY_DEBUG
char last_file[MAX_INPUT_LENGTH];
#endif

/*
 * OS-dependent local functions.
 */
int game_loop args( ( int control ) );
int init_socket args( ( int port ) );
void new_descriptor args( ( int control ) );
bool read_from_descriptor args( ( DESCRIPTOR_DATA * d, bool color ) );
bool write_to_descriptor
args( ( int desc, char *txt, int length, bool color ) );
char * wait_str( CHAR_DATA *, char * ); /* Added by JR */
/*
 * Other local functions (OS-independent).
 */
bool check_parse_name args( ( char *name ) );
bool check_reconnect args( ( DESCRIPTOR_DATA * d, char *name, bool fConn ) );
bool check_playing args( ( DESCRIPTOR_DATA * d, char *name ) );
#if defined(cbuilder)
int embermain args( ( int argc, char **argv ) );
#else
int main args( ( int argc, char **argv ) );
#endif
void nanny args( ( DESCRIPTOR_DATA * d, char *argument ) );
bool process_output args( ( DESCRIPTOR_DATA * d, bool fPrompt ) );
void read_from_buffer args( ( DESCRIPTOR_DATA * d, bool color ) );
void stop_idling args( ( CHAR_DATA * ch ) );
char *doparseprompt args( ( CHAR_DATA * ch ) );
int roll_stat args( ( int base_bonus, int stat_max ) );
void roll_stats( CHAR_DATA * ch, int penalty );
void show_stats( DESCRIPTOR_DATA * d );



bool can_read_descriptor( int fd )
{
    struct timeval tzero;
    fd_set R_SET;

    tzero.tv_sec = 0;
    tzero.tv_usec = 0;
    FD_ZERO( &R_SET );
    FD_SET( fd, &R_SET );
    while ( select( fd + 1, &R_SET, 0, 0, &tzero ) < 0 )
    {
#ifndef WIN32
        if ( errno == EINTR )
        {
            continue;
        }
        return FALSE;
#else
        int err = WSAGetLastError(  );
        fprintf( stderr, "select winsock error\n" );
        return FALSE;
#endif
    }
    return ( bool ) FD_ISSET( fd, &R_SET );
}

int figure_difference( int points )
{
    if ( points >= 28 )
        return ( ( int ) pow( ( double ) points, ( double ) 1.2 ) );
    if ( points < 28 )
        return ( 26 + points );
    return ( 0 );
}

#if defined(cbuilder)
int embermain( int argc, char **argv )
#else
int main( int argc, char **argv )
#endif
{
    struct timeval now_time = { 0, 0 };

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

#if defined(cbuilder)
/*
    if (chdir("..\\area"))
        chdir("..\\..\\area");
*/
#endif

    gettimeofday( &now_time, NULL );
    current_time = ( time_t ) now_time.tv_sec;
    strcpy( str_boot_time, get_curtime(  ) );

    /*
     * Reserve one channel for our use.
     */
    if ( ( fpReserve = fopen( NULL_FILE, "r" ) ) == NULL )
    {
#if defined(cbuilder)
        log_string( "Error: fopen fpReserve" );
        return -1;
#else
        perror( NULL_FILE );
        exit( 1 );
#endif
    }

    /*
     * Get the port number.
     */
    port = 9000;
    fCopyOver = FALSE;
    if ( argc > 1 )
    {
        if ( !is_number( argv[1] ) )
        {
#if defined(cbuilder)
            logf_string( "Usage: %s [port #] %s", argv[0], argv[1] );
            return -1;
#else
            fprintf( stderr, "Usage: %s [port #]\n", argv[0] );
            exit( 1 );
#endif
        }
        else if ( ( port = atoi( argv[1] ) ) <= 1024 )
        {
#if defined(cbuilder)
            log_string( "Port number must be above 1024." );
            return -1;
#else
            fprintf( stderr, "Port number must be above 1024.\n" );
            exit( 1 );
#endif
        }
        if ( ( argc > 2 ) && ( argv[2] && argv[2][0] ) )
        {
            fCopyOver = TRUE;
            control = atoi( argv[3] );
        }
    }

    /*
     * Run the game.
     */

    if ( !fCopyOver )
    {
        control = init_socket( port );
        if ( control == -1 )
        {
            log_string( "Error: init_socket(), shutting down." );
            return -1;
        }
    }
    if ( boot_db(  ) )
    {
        log_string( "Error: boot_db(), shutting down." );
        return -1;
    }
    sprintf( log_buf, "EmberMUD is ready to rock on port %d.", port );
#if defined(cbuilder)
//  SetStatus("EmberMUD Running.");
#endif
    update_last( "boot_db: done", "", "" );
    log_string( log_buf );
    if ( game_loop( control ) )
    {
        log_string( "Error: game_loop(), shutting down." );
#if defined (WIN32)
        closesocket( control );
#else
        close( control );
#endif
        return -1;
    }
#if defined (WIN32)
    closesocket( control );
#else
    close( control );
#endif

    /*
     * That's all, folks.
     */
    log_string( "Normal termination of game." );
#if defined(cbuilder)
    return 0;
#else
    exit( 0 );
    return 0;
#endif
}

int init_socket( int port )
{
    static struct sockaddr_in sa_zero;
    struct sockaddr_in sa;
    int x = 1;
    int fd;

#if defined(WIN32)
    WSADATA wsaData;

    wsaData.wVersion = 2;

#if defined(cbuilder)
    char wsaError[MAX_STRING_LENGTH];
#endif

    WSAStartup( MAKEWORD( 2, 2 ), &wsaData );
#endif
    if ( ( fd = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
    {
#if defined(cbuilder)
        FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL, WSAGetLastError(  ), 0,
                       wsaError, MAX_STRING_LENGTH, NULL );
        logf_string( "Init_socket: %s", wsaError );
        merc_down = TRUE;
        return -1;
#else
        perror( "Init_socket: socket" );
        exit( 1 );
#endif
    }

    if ( setsockopt( fd, SOL_SOCKET, SO_REUSEADDR,
                     ( char * ) &x, sizeof( x ) ) < 0 )
    {
#if defined(cbuilder)
        FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL, WSAGetLastError(  ), 0,
                       wsaError, MAX_STRING_LENGTH, NULL );
        logf_string( "Init_socket: %s", wsaError );
        closesocket( fd );
        merc_down = TRUE;
        return -1;
#else
        perror( "Init_socket: SO_REUSEADDR" );
#if defined(WIN32)
        closesocket( fd );
#else
        close( fd );
#endif
        exit( 1 );
#endif
    }

#if defined(SO_DONTLINGER) && !defined(SYSV)
    {
        struct linger ld;

        ld.l_onoff = 1;
        ld.l_linger = 1000;

        if ( setsockopt( fd, SOL_SOCKET, SO_DONTLINGER,
                         ( char * ) &ld, sizeof( ld ) ) < 0 )
        {
#if defined(cbuilder)
            FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL,
                           WSAGetLastError(  ), 0, wsaError, MAX_STRING_LENGTH,
                           NULL );
            logf_string( "Init_socket: %s", wsaError );
            closesocket( fd );
            merc_down = TRUE;
            return -1;
#else
            perror( "Init_socket: SO_DONTLINGER" );
#if defined(WIN32)
            closesocket( fd );
#else
            close( fd );
#endif
            exit( 1 );
#endif
        }
    }
#endif

    sa = sa_zero;
    sa.sin_family = AF_INET;
    sa.sin_port = htons( ( u_short ) port );

    if ( bind( fd, ( struct sockaddr * ) &sa, sizeof( sa ) ) < 0 )
    {
#if defined(cbuilder)
        FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL, WSAGetLastError(  ), 0,
                       wsaError, MAX_STRING_LENGTH, NULL );
        logf_string( "Init_socket: %s", wsaError );
        closesocket( fd );
        merc_down = TRUE;
        return -1;
#else
        perror( "Init socket: bind" );
#if defined(WIN32)
        closesocket( fd );
#else
        close( fd );
#endif
        exit( 1 );
#endif
    }

    if ( listen( fd, 3 ) < 0 )
    {
#if defined(cbuilder)
        FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL, WSAGetLastError(  ), 0,
                       wsaError, MAX_STRING_LENGTH, NULL );
        logf_string( "Init_socket: %s", wsaError );
        closesocket( fd );
        merc_down = TRUE;
        return -1;
#else
        perror( "Init socket: listen" );
#if defined(WIN32)
        closesocket( fd );
#else
        close( fd );
#endif
        exit( 1 );
#endif
    }

    return fd;
}

#if defined(unix)
void sigchld_handler( int sig )
{
    pid_t child_pid;
    int status;

    if ( ( child_pid = wait( &status ) ) < 0 )
    {
        return;
    }

    reap_shells = TRUE;
    return;
}
#endif

// Count 


int game_loop( int control )
{
    static struct timeval null_time;
    struct timeval last_time;
    bool color;
    int n;
    
#if defined(unix)
    static struct sigaction sa;
    sigemptyset( &sa.sa_mask );
#ifndef SA_RESTART
#define SA_RESTART 0
#endif
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = sigchld_handler;
    if ( sigaction( SIGCHLD, &sa, 0 ) < 0 )
        return -1;
    signal( SIGPIPE, SIG_IGN );
#endif
    gettimeofday( &last_time, NULL );
    current_time = ( time_t ) last_time.tv_sec;

    /* Main loop */
    while ( !merc_down )
    {
        fd_set in_set;
        fd_set out_set;
        fd_set exc_set;
        DESCRIPTOR_DATA *d;
        int maxdesc;

#if defined(MALLOC_DEBUG)
        if ( malloc_verify(  ) != 1 )
            abort(  );
#endif

        /*
         * Poll all active descriptors.
         */
        FD_ZERO( &in_set );
        FD_ZERO( &out_set );
        FD_ZERO( &exc_set );
        FD_SET( control, &in_set );
        maxdesc = control;
        for ( d = descriptor_list; d; d = d->next )
        {
            maxdesc = UMAX( maxdesc, d->descriptor );
            FD_SET( d->descriptor, &in_set );
            FD_SET( d->descriptor, &out_set );
            FD_SET( d->descriptor, &exc_set );
        }

        if ( select( maxdesc + 1, &in_set, &out_set, &exc_set, &null_time ) <
             0 )
        {
#if defined(cbuilder)
            log_string( "Error: Game_loop: select: poll" );
            return -1;
#else
            perror( "Game_loop: select: poll" );
            exit( 1 );
#endif
        }

#ifndef WIN32
        /* Guess this is as good a place as any for shell reaping? */
        if ( reap_shells )
        {
            CHAR_DATA *ch;
            /* No one in the shell list? Then don't reap shells anymore! */
            if ( !shell_char_list )
            {
                reap_shells = FALSE;
            }
            else
            {
                /* Check the "in" pipe for each character in the list.
                 * If any of them are readable, put them back into the game. */
                for ( ch = shell_char_list; ch; ch = ch->next_in_shell )
                {
                    CHAR_DATA *prev;

                    if ( can_read_descriptor( ch->fdpair[0] ) )
                    {
                        close( ch->fdpair[0] );
                        if ( ch == shell_char_list )
                        {
                            shell_char_list = NULL;
                        }
                        else
                        {
                            for ( prev = shell_char_list;
                                  prev && prev->next != ch; prev = prev->next );

                            if ( prev )
                            {
                                prev->next = ch->next;
                            }
                            else
                            {
                                bug( "reap_shell: reaped char not found in the shell_char_list", 0 );
                            }
                        }
                        ch->next = char_list;
                        char_list = ch;
                        ch->next_player = player_list;
                        ch->next_in_shell = NULL;
                        player_list = ch;
                        send_to_char( "MUD I/O resumed.\n\r", ch );
                        ch->desc->connected = CON_PLAYING;
                        continue;
                    }
                }
            }
        }
#endif

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
                FD_CLR( ( unsigned int ) d->descriptor, &in_set );
                FD_CLR( ( unsigned int ) d->descriptor, &out_set );
                if ( d->character && d->character->level > 1 )
                    save_char_obj( d->character );
#if defined(cbuilder)
                if ( d->character && d->character->TNode )
                    RemoveUser( d->character );
#endif
                d->outtop = 0;
                close_socket( d );
            }
        }

        /*
         * Process input.
         */
        for ( d = descriptor_list; d != NULL; d = d_next )
        {
            d_next = d->next;
            d->fcommand = FALSE;
            if FD_ISSET( d->descriptor, &in_set )
            {   
                if ( d->character != NULL )
                {
                    d->character->timer = 0;
                    if ( IS_SET( d->character->act, PLR_COLOR ) )
                        color = TRUE;
                    else
                        color = FALSE;
                }
                else
                    color = FALSE;
                if ( !read_from_descriptor( d, color ) )
                {
                    FD_CLR( ( unsigned int ) d->descriptor, &out_set );
                    if ( d->character != NULL && d->character->level > 1 )
                        save_char_obj( d->character );
#if defined(cbuilder)
                    if ( d->character && d->character->TNode )
                        RemoveUser( d->character );
#endif
                    d->outtop = 0;
                    close_socket( d );
                    continue;
                }
            }

            bool wait = FALSE;
            if ( d->character != NULL && d->character->wait > 0 )
            {
                n = d->character->wait--;
                if ( is_fixed_d( d ) && ( n % PULSE_PER_SECOND == 0 || n == 1 ) ) // JR :(
                {
                    write_to_buffer( d, doparseprompt(d->character), 0 ); // JR: prompt ends up getting drawn twice
                }                                                         // Not really a problem with Tintin, but still...
                if ( !IS_NPC( d->character) && d->character->level >= LEVEL_ADMIN && 
                    d->character->wait > PULSE_MOVE )
                    d->character->wait = PULSE_MOVE; // JR: admins don't have to wait long
                //continue; // JR: testing
                wait = TRUE;
            }
            

            if ( d->connected != CON_READ_MOTD || PAUSE_MOTD)
                read_from_buffer( d, FALSE );
            else
                {d->incomm[0] = '\n';d->incomm[1] = '\0';} // JR: changed double quotes to single
            
            if ( d->incomm[0] != '\0' )
            {
                d->fcommand = TRUE;
                stop_idling( d->character );

                /* OLC */
                if ( d->showstr_point )
                    show_string( d, d->incomm );
                else if ( d->pString )
                    string_add( d->character, d->incomm, d->color_edit ); // JR
                else
                    switch ( d->connected )
                    {
                    case CON_PLAYING:
                        if ( !run_olc_editor( d ) )
                        {
                            char buf[MAX_STRING_LENGTH];
                            //substitute_alias( d, d->incomm );
                            substitute_alias_string( d, d->incomm, buf );
                            d->incomm[0] = '\0';
                            if ( is_wait_blocked( d->character, buf ) )
                                add_wait_queue( d, buf );
                            else
                            {
                                interpret( d->character, buf );
                                continue;
                            }
                        }
                        break;
                    default:
                        nanny( d, d->incomm );
                        break;
                    }

                d->incomm[0] = '\0';
                
            }
            if ( !wait )
                interpret_wait_queue( d );
        }
        /*
         * Autonomous game motion.
         */
        update_handler(  );
        /*
         * Output.
         */
        for ( d = descriptor_list; d != NULL; d = d_next )
        {
            d_next = d->next;

            if ( ( d->fcommand || d->outtop > 0 )
                 && FD_ISSET( d->descriptor, &out_set ) )
            {
                if ( !process_output( d, TRUE ) )
                {
                    if ( d->character != NULL && d->character->level > 1 )
                        save_char_obj( d->character );
#if defined(cbuilder)
                    if ( d->character && d->character->TNode )
                        RemoveUser( d->character );
#endif
                    d->outtop = 0;
                    close_socket( d );
                }
            }
        }
#if defined(cbuilder)
        /*
         * Checks for GUI commands, updates GUI stats.
         */
        update_gui(  );
#endif

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
            usecDelta =
                ( ( int ) last_time.tv_usec ) - ( ( int ) now_time.tv_usec ) +
                1000000 / PULSE_PER_SECOND;
            secDelta =
                ( ( int ) last_time.tv_sec ) - ( ( int ) now_time.tv_sec );
            
            
#if defined(WIN32)
            if ( usecDelta > 0 )
                Sleep( UMIN( usecDelta, 250000 ) / 1000 );
#else
            while ( usecDelta < 0 )
            {
                usecDelta += 1000000;
                secDelta -= 1;
            }

            while ( usecDelta >= 1000000 )
            {
                usecDelta -= 1000000;
                secDelta += 1;
            }

            if ( secDelta > 0 || ( secDelta == 0 && usecDelta > 0 ) )
            {
                struct timeval stall_time;

                stall_time.tv_usec = usecDelta;
                stall_time.tv_sec = secDelta;
                if ( select( 0, NULL, NULL, NULL, &stall_time ) < 0 )
                {
#if defined(cbuilder)
                    log_string( "Error: Game_loop: select: stall" );
                    return -1;
#else
                    if ( errno != EINTR )
                    {
                        perror( "Game_loop: select: stall" );
                        exit( 1 );
                    }

#endif
                }
            }
#endif
        }

        gettimeofday( &last_time, NULL );
        current_time = ( time_t ) last_time.tv_sec;
    }
    return 0;
}

void new_descriptor( int control )
{
    char buf[MAX_STRING_LENGTH];
    static DESCRIPTOR_DATA d_zero;
    DESCRIPTOR_DATA *dnew;
    struct sockaddr_in sock;
#ifndef NO_RDNS
    //struct hostent *from;
#endif
    int desc;
    unsigned int size;
#if defined(WIN32)
    int OptVal;
#endif

    size = sizeof( sock );
    getsockname( control, ( struct sockaddr * ) &sock, &size );
    if ( ( desc = accept( control, ( struct sockaddr * ) &sock, &size ) ) < 0 )
    {
        perror( "New_descriptor: accept" );
        return;
    }

#if !defined(FNDELAY)
#define FNDELAY O_NDELAY
#endif

#if defined(WIN32)
    if ( setsockopt( desc, IPPROTO_TCP, TCP_NODELAY, ( char * ) &OptVal,
                     sizeof( int ) ) )
    {
        perror( "New_descriptor: setsockopt: TCP_NODELAY" );
        return;
    }
#else
    if ( fcntl( desc, F_SETFL, FNDELAY ) == -1 )
    {
        perror( "New_descriptor: fcntl: FNDELAY" );
        return;
    }
#endif

    /*
     * Cons a new descriptor.
     */
    if ( descriptor_free == NULL )
    {
        dnew = alloc_perm( sizeof( *dnew ) );
    }
    else
    {
        dnew = descriptor_free;
        descriptor_free = descriptor_free->next;
    }

    *dnew = d_zero;
    dnew->descriptor = desc;
    dnew->connected = CON_GET_ANSI; /* Removed by JR*/
    dnew->showstr_head = NULL;
    dnew->showstr_point = NULL;
    dnew->outsize = 2000;
    dnew->pEdit = NULL;         /* OLC */
    dnew->pString = NULL;       /* OLC */
    dnew->editor = 0;           /* OLC */
    dnew->outbuf = alloc_mem( dnew->outsize );
    dnew->tintin = FALSE; // JR
    dnew->newline = FALSE; // JR

    size = sizeof( sock );
    if ( getpeername( desc, ( struct sockaddr * ) &sock, &size ) < 0 )
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
                 ( addr >> 8 ) & 0xFF, ( addr ) & 0xFF );
        sprintf( log_buf, "Sock.sinaddr:  %s", buf );
        log_string( log_buf );
        dnew->host = str_dup( buf );

    }

    /*
     * Swiftest: I added the following to ban sites. I don't
     * endorse banning of sites, but Copper has few descriptors now
     * and some people from certain sites keep abusing access by
     * using automated 'autodialers' and leaving connections hanging.
     *
     * Furey: added suffix check by request of Nickel of HiddenWorlds.
     */

    /*
     * Init descriptor data.
     */
    dnew->next = descriptor_list;
    descriptor_list = dnew;
    //write_to_buffer( dnew, CFG_CONNECT_MSG, 0 );
    write_to_buffer( dnew, CFG_ASK_ANSI, 0 ); /* Removed by JR to streamline web interface */
    return;
}

void close_socket( DESCRIPTOR_DATA * dclose )
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
        /* If ch is switched, unswitch them */
        if ( dclose->original && dclose->original != ch )
        {
            dclose->character = dclose->original;
            dclose->original = NULL;
            dclose->character->desc = dclose;
            ch = dclose->character;
        }

        sprintf( log_buf, "Closing link to %s.", ch->name );
        log_string( log_buf );

        /* If ch is writing note or playing, just lose link otherwise clear char */
        if ( ( dclose->connected == CON_PLAYING ) ||
             ( ( dclose->connected >= CON_NOTE_TO ) &&
               ( dclose->connected <= CON_NOTE_FINISH ) ) )
        {
            act( "$n has lost $s link.", ch, NULL, NULL, TO_ROOM );
            ch->pcdata->ticks = 1;
            if ( IS_SET( ch->act, PLR_BUILDING ) )
            {
                REMOVE_BIT( ch->act, PLR_BUILDING );
            }

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

#if defined(WIN32)
    closesocket( dclose->descriptor );
#else
    close( dclose->descriptor );
#endif
    free_string( &dclose->host );
    /* RT socket leak fix -- I hope */
    free_mem( &dclose->outbuf );
    dclose->next = descriptor_free;
    descriptor_free = dclose;
    return;
}

bool read_from_descriptor( DESCRIPTOR_DATA * d, bool color )
{
    int iStart;
    bool bOverflow = FALSE;

    /* Don't read from people in shells */
    if ( d->connected == CON_SHELL )
        return TRUE;

    /* Hold horses if pending command already. */
    if ( d->incomm[0] != '\0' )
        return TRUE;

    /* Check for overflow. */
    iStart = strlen( d->inbuf );
    if ( iStart >= sizeof( d->inbuf ) - 10 )
    {
        sprintf( log_buf, "%s input overflow!", d->host );
        log_string( log_buf );
        write_to_descriptor( d->descriptor,
                             "\n\r*** PUT A LID ON IT!!! ***\n\r", 0, color );
        return FALSE;
    }

    /* Snarf input. */

    for ( ;; )
    {
        int nRead;
        int nBufSize;

        nBufSize = sizeof( d->inbuf ) - 10 - iStart;

#if defined(WIN32)
        nRead = recv( d->descriptor, d->inbuf + iStart, nBufSize, 0 );
#else
        nRead = read( d->descriptor, d->inbuf + iStart, nBufSize );
#endif
        if ( nRead > 0 )
        {
            iStart += nRead;
            if ( bOverflow )
            {
                iStart = 0;
                if ( nRead < sizeof( d->inbuf ) - 10 )
                    break;
            }
            else if ( d->inbuf[iStart - 1] == '\n'
                      || d->inbuf[iStart - 1] == '\r' || nRead < nBufSize )
                /* If we hit a CR/LF or read less than the buffer size then return */
                break;
            else if ( iStart >= sizeof( d->inbuf ) - 10 )
            {
                if ( iStart - nRead > 0 )
                    iStart -= nRead;
                else
                    iStart = 0;
                bOverflow = TRUE;
            }
        }
        else if ( nRead == 0 )
        {
            log_string( "EOF encountered on read." );
            return FALSE;
        }
#if defined(unix)
        else if ( errno == EWOULDBLOCK )
            break;
#endif
        else
        {
            perror( "Read_from_descriptor" );
            return FALSE;
        }
    }

    d->inbuf[iStart] = '\0';

    if ( bOverflow )
    {
        sprintf( log_buf, "%s input overflow!", d->host );
        log_string( log_buf );
        write_to_descriptor( d->descriptor,
                             "\n\rLine too long, ignored.\n\r", 0, color );
    }

    return TRUE;
}

/*
 * Transfer one line from input buffer to input line.
 */
void read_from_buffer( DESCRIPTOR_DATA * d, bool color )
{
    int i, j, k;

    /* Don't read from people in shells. */
    if ( d->connected == CON_SHELL )
        return;

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
            write_to_descriptor( d->descriptor, "Line too long.\n\r", 0,
                                 color );

            /* skip the rest of the line */
            for ( ; d->inbuf[i] != '\0'; i++ )
            {
                if ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
                    break;
            }
            d->inbuf[i] = '\n';
            d->inbuf[i + 1] = '\0';
            break;
        }

        if ( d->inbuf[i] == '\b' && k > 0 )
            --k;
        else if ( isascii( d->inbuf[i] ) && isprint( d->inbuf[i] ) )
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
    for ( j = 0; ( d->inbuf[j] = d->inbuf[i + j] ) != '\0'; j++ )
        ;
    return;
}





/* Writen by JR */
/* Returns a string indicating player wait time */
char * wait_str( CHAR_DATA * ch, char *buf )
{
    int n = ch->wait;
    if ( ch->level >= LEVEL_ADMIN )
        strcpy( buf, "");
    else if ( is_fixed( ch ) )
    {
        if ( n <= PULSE_MOVE ) // Wait for moving a room
            sprintf( buf, "  ");
        else
            sprintf( buf, "%d ", 1 + (ch->wait-1) / PULSE_PER_SECOND );
    }
    else
    {
        if ( n <= PULSE_MOVE )
            strcpy( buf, "   " );
        else if ( n < 1*PULSE_VIOLENCE )
            strcpy( buf, "  ." );
        else if ( n < 1.5*PULSE_VIOLENCE )
            strcpy( buf, "  *" );
        else if ( n < 2*PULSE_VIOLENCE )
            strcpy( buf, " .*" );
        else if ( n < 2.5*PULSE_VIOLENCE )
            strcpy( buf, " **" );
        else if ( n < 3*PULSE_VIOLENCE )
            strcpy( buf, ".**" );
        else
            strcpy( buf, "***" );
    }
    return buf;
}




/*
 * Low level output function.
 */
bool process_output( DESCRIPTOR_DATA * d, bool fPrompt )
{
    char buf[MAX_STRING_LENGTH];
    //char waitbuf[10];
    extern bool merc_down;
    bool color = TRUE;
    bool tintin;
    
    tintin = is_fixed_d( d );

    /* If you're in a shell, then no output for you! */
    if ( d->connected == CON_SHELL )
        return TRUE;

    /*
     * Bust a prompt.
     */
    if ( ( d->character != NULL ) && IS_SET( d->character->act, PLR_COLOR ) )
        color = TRUE;
    else
        color = FALSE;

    if ( !merc_down )
    {
        if ( d->showstr_point )
        {
            if ( d->character == NULL || !IS_SET(d->character->comm, COMM_COMPACT ) )
                write_to_buffer( d, "`\n\r", 2 );
            write_to_buffer( d, "`W[Hit Return to continue]`w\n\r", 0 );
        }
        else if ( fPrompt && d->pString && d->connected == CON_PLAYING )
            write_to_buffer( d, "> ", 2 ); // JR: may want to fiddle with this to fix olc
        else if ( fPrompt && d->connected == CON_PLAYING )
        {
            CHAR_DATA *ch;

            ch = d->original ? d->original : d->character;
            if ( !IS_SET( ch->comm, COMM_COMPACT ) )
            {
                if ( tintin )
                {
                    if ( ch != NULL )
                        d->newline = TRUE;
                }
                else
                    write_to_buffer( d, "\n\r", 2 );
            }

            if ( IS_SET( ch->comm, COMM_PROMPT ) )
            {
                ch = d->character;
                //if ( !IS_NPC( ch ) )
                    sprintf( buf, "%s", doparseprompt( ch ) );
                //else
                    /* This is the default prompt */
                //    sprintf( buf, "%s<H%d/%d M%d/%d V%d/%d>", wait_str( ch, waitbuf ), ch->hit,
                //             ch->max_hit, ch->mana, ch->max_mana, ch->move,
                //             ch->max_move );
                write_to_buffer( d, buf, 0 );
            }

            if ( IS_SET( ch->comm, COMM_TELNET_GA ) )
                write_to_buffer( d, go_ahead_str, 0 );
        }
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
        if ( d->character != NULL )
            write_to_buffer( d->snoop_by, d->character->name, 0 );
        write_to_buffer( d->snoop_by, "> ", 2 );
        write_to_buffer( d->snoop_by, d->outbuf, d->outtop );
    }
    /*
     * OS-dependent output.
     */
    if ( !write_to_descriptor( d->descriptor, d->outbuf, d->outtop, color ) )
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
void write_to_buffer( DESCRIPTOR_DATA * d, const char *txt, int length )
{
    extern bool silentmode;
    bool b,tintin;
    
    char buf[MAX_OUTPUT_BUFFER],buf2[MAX_OUTPUT_BUFFER],*tmp;
    
    strcpy( buf, txt );

    /*
     * Find length in case caller didn't.
     */
    
    if ( length <= 0 )
        length = strlen( txt );
    tintin = is_fixed_d( d );
    tmp = buf;
    
    if ( tintin && d->newline ) //JR temp debug
    {
        b = TRUE;
        for (int n=0; n+1<length; n++ )
        {
            strcpy( buf2, txt+n );
            buf2[2] = '\0';
            if ( !str_prefix( buf2, PROMPT_TOP ) )
            {
                b = FALSE;
                break;
            }
            if ( txt[n] == '\n' )
            {
                break;
            }
        }
        if ( b )
        {
            *tmp = '\n';
            *(tmp+1) = '\r';
            tmp += 2;
            length += 2;
            d->newline = FALSE;
        }
    }
    
    strcpy( tmp, txt );
    

        
    /*while ( strlen(buf) > 1 && 
        (buf[strlen(buf)-1] == '\n' || buf[strlen(buf)-1] == '\r') )
    {
        printf("removed newline from end, ");
        buf[strlen(buf)-2] = '\0';
        newline = TRUE;
        length -= 2;
        break;
    }
    printf("\n\n");*/


    /* Don't do ANYTHING if global silentmode is on. */
    if ( silentmode )
        return;


    /*
     * Initial \n\r if needed.
     */
    if ( d->outtop == 0 && !tintin && !d->fcommand ) // JR tintin
    {
        d->outbuf[0] = '\n';
        d->outbuf[1] = '\r';
        d->outtop = 2;
    }

    /*
     * Expand the buffer as needed.
     */
    while ( d->outtop + length >= d->outsize )
    {
        char *outbuf;

        if ( d->outsize + 2048 > MAX_OUTPUT_BUFFER )
            return;

        outbuf = alloc_mem( d->outsize + 2048 );
        strncpy( outbuf, d->outbuf, d->outtop );
        free_mem( &d->outbuf );
        d->outbuf = outbuf;
        d->outsize += 2048;
    }
    /*
     * Copy.
     */
    strcpy( d->outbuf + d->outtop, buf );
    d->outtop += length;
    return;
}

/*
 * Lowest level output function.
 * Write a block of text to the file descriptor.
 * If this gives errors on very long blocks (like 'ofind all'),
 *   try lowering the max block size.
 */
bool write_to_descriptor( int desc, char *txt, int length, bool color )
{
    int iStart;
    int nWrite;
    int nBlock;
    static char colorbuf[MAX_OUTPUT_BUFFER];
    
    /* Run through the color filter */
    do_color( txt, length, colorbuf, sizeof( colorbuf ), color );
    length = strlen( colorbuf );

    for ( iStart = 0; iStart < length; iStart += nWrite )
    {
        nBlock = UMIN( length - iStart, sizeof( colorbuf ) );
#if defined(WIN32)
        if ( ( nWrite = send( desc, colorbuf + iStart, nBlock, 0 ) ) < 0 )
        {
            perror( "Write_to_descriptor" );
            return FALSE;
        }
#else
        if ( ( nWrite = write( desc, colorbuf + iStart, nBlock ) ) < 0 )
        {
            perror( "Write_to_descriptor" );
            return FALSE;
        }
#endif
    }

    return TRUE;
}

/*
 * Deal with sockets that haven't logged in yet.
 */
void nanny( DESCRIPTOR_DATA * d, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *ch;
    BAN_DATA *pban;
    int ban_type;
    char *pwdnew;
    char *p;
    int iClass, race, i, x, max_len;
    bool fOld;
    DESCRIPTOR_DATA *d_old;

    /* Delete leading spaces UNLESS character is writing a note */
    if ( d->connected != CON_NOTE_TEXT )
    {
        while ( isspace( *argument ) )
            argument++;
    }

    ch = d->character;
    update_last( "Nanny:", ch ? ch->name : "new", argument );
    switch ( d->connected )
    {

    default:
        bug( "Nanny: bad d->connected %d.", d->connected );
#if defined(cbuilder)
        if ( d->character && d->character->TNode )
            RemoveUser( d->character );
#endif
        close_socket( d );
        return;

    case CON_GET_ANSI:
        if ( ( argument[0] == 'n' ) || ( argument[0] == 'N' ) )
            d->ansi = FALSE;
        else
            d->ansi = TRUE;
            
        if ( !strcmp( argument, "tintin" ) )
            d->tintin = TRUE;
        else
            d->tintin = FALSE;

        /*
         * Send the greeting.
         */
        {
            extern char *help_greeting;
            /* extern char *ansi_greeting; */
            
            write_to_buffer(d,"\n\r",0); /* Added by JR (fixed display problem with TinTin*/
            /*if ( d->ansi ) // JR: fixed this nonsense
                write_to_buffer( d, ansi_greeting, 0 );
            else*/
            {
                if ( help_greeting[0] == '.' )
                    p = help_greeting + 1;
                else
                    p = help_greeting;
                do_color( p, strlen(p), buf, MAX_STRING_LENGTH, TRUE );
                write_to_buffer( d, buf, 0);
            }
        }
        d->connected = CON_GET_NAME;
        return;

    case CON_GET_NAME:
        if ( argument[0] == '\0' )
        {
            close_socket( d );
            return;
        }

        argument[0] = UPPER( argument[0] );
        if ( !check_parse_name( argument ) )
        {
            write_to_buffer( d, "Illegal name, try another.\n\rName: ", 0 );
            return;
        }

        fOld = load_char_obj( d, argument );
        ch = d->character;

/* I just could NOT get the check_ban function from the rom ban.c file to
work, so I got fed up with it and wrote some new ban checks. They are not
the best in the world, but it was alot quicker then writting a new
check_ban function.  
		-Lancelight 
*/

        for ( pban = ban_list; pban != NULL; pban = pban->next )
        {
            ban_type = pban->ban_flags;
            if ( !str_suffix( ( pban->name ), d->host ) )
            {
                if ( IS_SET( pban->ban_flags, BAN_PERMANENT )
                     || IS_SET( pban->ban_flags, BAN_ALL ) )
                {
                    write_to_buffer( d,
                                     "Your site has been banned from this mud.\n\r",
                                     0 );
                    close_socket( d );
                    return;
                }
            }

        }
        for ( pban = ban_list; pban != NULL; pban = pban->next )
        {
            ban_type = pban->ban_flags;
            if ( !str_suffix( ( pban->name ), d->host ) )
            {
                if ( ban_type == 16 && !IS_SET( ch->act, PLR_PERMIT ) )
                {
                    sprintf( buf,
                             "You do not have permission to play on this mud.\n\r Please email %s to get permission to play here.",
                             MUD_ADMIN_EMAIL );
                    send_to_char( buf, ch );
                    close_socket( d );
                    return;
                }
            }

        }
        if ( IS_SET( ch->act, PLR_DENY ) )
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
            if ( wizlock && !IS_HERO( ch ) )
            {
                write_to_buffer( d, "The game is wizlocked.\n\r", 0 );
                close_socket( d );
                return;
            }
            else if ( chaos && !IS_HERO( ch ) )
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
            if ( newlock )
            {
                write_to_buffer( d, "The game is newlocked.\n\r", 0 );
                close_socket( d );
                return;
            }
            for ( pban = ban_list; pban != NULL; pban = pban->next )
            {
                ban_type = pban->ban_flags;
                if ( !str_suffix( ( pban->name ), d->host ) )
                {
                    if ( ban_type == 4 )
                    {
                        write_to_buffer( d,
                                         "New players from your site have been banned from this mud.\n\r",
                                         0 );
                        close_socket( d );
                        return;
                    }
                }

            }
/*
    for ( pban = ban_list; pban != NULL; pban = pban->next )
    {
	if ( !str_suffix( pban->name, d->host ) && 
	     (pban->ban_flags,BAN_NEWBIES))
	    {
		write_to_buffer(d,
		    "New players are not allowed from your site.\n\r",0);
		close_socket(d);
		return;
	    }
	else 
		write_to_buffer(d,
		    "I dont work.\n\r",0);
	
	
}*/
            sprintf( buf, "\n\rDid I get that right, %s (Y/N): ", argument );
            write_to_buffer( d, buf, 0 );
            d->connected = CON_CONFIRM_NEW_NAME;
            return;
        }

        if ( check_playing( d, ch->name ) )
            return;

        break;

    case CON_GET_OLD_PASSWORD:
        //write_to_buffer( d, "\n\r", 2 );

        if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ) )
        {
            write_to_buffer( d, "Wrong password.\n\r", 0 );
            close_socket( d );
            return;
        }

        if ( ch->pcdata->pwd[0] == 0 )
        {
            write_to_buffer( d, "Warning! Null password!\n\r", 0 );
            write_to_buffer( d, "Please report old password with bug.\n\r", 0 );
            write_to_buffer( d,
                             "Type 'password null <new password>' to fix.\n\r",
                             0 );
        }

        write_to_buffer( d, echo_on_str, 0 );

        if ( check_reconnect( d, ch->name, TRUE ) )  // JR
            return;

        if ( check_playing( d, ch->name ) )
            return;

        sprintf( log_buf, "%s@%s has connected.", ch->name, d->host );
        log_string( log_buf );
        if ( IS_SET( ch->act, PLR_BUILDING ) )
        {
            REMOVE_BIT( ch->act, PLR_BUILDING );
        }
        
        if ( IS_HERO( ch ) && ENABLE_IMOTD )
        {
            do_help( ch, "imotd" );
            d->connected = CON_READ_IMOTD;
        }
        else
        {
            motd( ch );
            d->connected = CON_READ_MOTD;
        }
        break;

/* RT's code for breaking link sucked... you could cheat and duplicate all your gear. 
   This version disconnects the old character and prompts you to re-enter your name 
   password. This way you won't have loaded the pfile containing objects and such
   which you may have dropped on the ground, resulting in two copies of any object,
   even special or quest objects. -- Kyle */

    case CON_BREAK_CONNECT:
        switch ( *argument )
        {
        case 'y':
        case 'Y':
            for ( d_old = descriptor_list; d_old != NULL; d_old = d_next )
            {
                d_next = d_old->next;
                if ( d_old == d || d_old->character == NULL )
                    continue;

                if ( str_cmp( ch->name, d_old->character->name ) )
                    continue;

                close_socket( d_old );
                if ( d->character != NULL )
                {
                    free_char( d->character );
                    d->character = NULL;
                }
                d->connected = CON_GET_NAME;
                break;
            }

            write_to_buffer( d, "Disconnected. Re-enter name: ", 0 );
            if ( d->character != NULL )
            {
                free_char( d->character );
                d->character = NULL;
            }
            d->connected = CON_GET_NAME;
            break;

        case 'n':
        case 'N':
            write_to_buffer( d, "Name: ", 0 );
            if ( d->character != NULL )
            {
                free_char( d->character );
                d->character = NULL;
            }
            d->connected = CON_GET_NAME;
            break;

        default:
            write_to_buffer( d, "Please type Y or N: ", 0 );
            break;
        }
        break;

    case CON_CONFIRM_NEW_NAME:
        switch ( *argument )
        {
        case 'y':
        case 'Y':
            sprintf( buf, "New character.\n\r\n\rGive me a password for %s: %s",
                     ch->name, echo_off_str );
            write_to_buffer( d, buf, 0 );
            d->connected = CON_GET_NEW_PASSWORD;
            if ( d->ansi )
                SET_BIT( ch->act, PLR_COLOR );
            break;

        case 'n':
        case 'N':
            write_to_buffer( d, "Ok, what IS it, then: ", 0 );
            free_char( d->character );
            d->character = NULL;
            d->connected = CON_GET_NAME;
            break;

        default:
            write_to_buffer( d, "Please type Yes or No: ", 0 );
            break;
        }
        break;

    case CON_GET_NEW_PASSWORD:
        write_to_buffer( d, "\n\r", 2 );

        
        if ( strlen( argument ) < 5 )
        {
            write_to_buffer(d,"`YCaution: password is short.\n\r`w",0);
            /*write_to_buffer( d,
                             "Password must be at least five characters long.\n\rPassword: ",
                             0 );*/
            /*return; */
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

        free_string( &ch->pcdata->pwd );
        ch->pcdata->pwd = str_dup( pwdnew );
        write_to_buffer( d, "Please retype password: ", 0 );
        d->connected = CON_CONFIRM_NEW_PASSWORD;
        break;

    case CON_CONFIRM_NEW_PASSWORD:
        write_to_buffer( d, "\n\r", 2 );

        if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ) )
        {
            write_to_buffer( d, "Passwords don't match.\n\rRetype password: ",
                             0 );
            d->connected = CON_GET_NEW_PASSWORD;
            return;
        }

        write_to_buffer( d, echo_on_str, 0 );

        prompt_race(d, ch, 3);

        d->connected = CON_GET_NEW_RACE;
        break;

    case CON_GET_NEW_RACE:
        one_argument( argument, arg );

        if ( !strcmp( arg, "help" ) )
        {
            argument = one_argument( argument, arg );
            if ( argument[0] == '\0' )
                do_help( ch, "race help" );
            else
                do_help( ch, argument );
            write_to_buffer( d,
                             "What is your race (help for more information): ",
                             0 );
            break;
        }

        race = race_lookup( argument );
        if ( race == 0 || !race_table[race].pc_race
             || ( race_table[race].remort_race
                  && !IS_SET( ch->act, PLR_REMORT ) ) )
        {
            write_to_buffer( d, "That is not a valid race.\n\r", 0 );
            prompt_race( d, ch, 3 );

            break;
        }

        ch->race = race;
            
        /* initialize stats */
        for ( i = 0; i < MAX_STATS; i++ )
            ch->perm_stat[i] = pc_race_table[race].stats[i];
        ch->affected_by = ch->affected_by | race_table[race].aff;
        ch->imm_flags = ch->imm_flags | race_table[race].imm;
        ch->res_flags = ch->res_flags | race_table[race].res;
        ch->vuln_flags = ch->vuln_flags | race_table[race].vuln;
        ch->form = race_table[race].form;
        ch->parts = race_table[race].parts;

        /* add skills */
        for ( i = 0; i < 5; i++ )
        {
            if ( pc_race_table[race].skills[i] == NULL )
                break;
            group_add( ch, pc_race_table[race].skills[i], FALSE );
        }
            
        /* add cost */
        ch->pcdata->points = pc_race_table[race].points;
        ch->size = pc_race_table[race].size;
        write_to_buffer( d, "\n\rWhat is your gender (M/F/N): ", 0 );
        d->connected = CON_GET_NEW_SEX;
        break;
    
    case CON_GET_NEW_SEX:
        switch ( argument[0] )
        {
        case 'm':
        case 'M':
            ch->sex = SEX_MALE;
            ch->pcdata->true_sex = SEX_MALE;
            break;
        case 'f':
        case 'F':
            ch->sex = SEX_FEMALE;
            ch->pcdata->true_sex = SEX_FEMALE;
            break;
        case 'n':
        case 'N':
            ch->sex = SEX_NB;
            ch->pcdata->true_sex = SEX_NB;
            break;
        default:
            write_to_buffer( d, "That's not a gender.\n\rWhat *is* your gender: ", 0 );
            return;
        }
        write_to_buffer( d, "\n\rNow rolling your stats.\n\r", 0 );
        roll_stats( ch, 0 );
        show_stats( d );
        if ( ALLOW_REROLL )
            write_to_buffer( d, "\n\rEnter a column number, or press enter to roll again: ", 0 );
        else
            write_to_buffer( d, "\n\rEnter a column number: ", 0 );
        d->connected = CON_GET_STATS;
        break;

    case CON_GET_STATS:
        if ( atoi( argument ) <= NUM_ROLLS && atoi( argument ) >= 1 )
        {
            for ( i = 0; i < 5; i++ )
            {
                ch->perm_stat[i] = stat[i][atoi( argument ) - 1];
            }

            write_to_buffer( d, "\n\rThe following classes are available:\n\r", 0 );
            max_len = 0;    

            for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
            {
                if ( !class_table[iClass].remort_class ||
                     ( class_table[iClass].remort_class
                       && IS_SET( ch->act, PLR_REMORT ) ) )
                {
                    if ( str_cmp( class_table[iClass].name, "Console" ) )
                        max_len = str_len( class_table[iClass].name ) > max_len ? str_len( class_table[iClass].name ) : max_len;
                }
            }
            x = 0;                  
            for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
            {
                if ( !class_table[iClass].remort_class ||
                     ( class_table[iClass].remort_class
                       && IS_SET( ch->act, PLR_REMORT ) ) )
                {
                    if ( str_cmp( class_table[iClass].name, "Console" ) )
                    {
                        if ( x == 3 )
                        {
                            write_to_buffer( d, "\n\r", 2 );
                            x = 0;
                        }
                        if ( x == 0 )
                            write_to_buffer( d, "   ", 3 );
                        write_to_buffer( d, class_table[iClass].name, 0 );
                        for ( i=0 ; i+str_len(class_table[iClass].name) < max_len + 3; i++)
                            write_to_buffer( d, " ", 1 );
                        x++;
                    }
                }
            }
            write_to_buffer( d, "\n\r\n\rWhat is your class ('help' for more information): ", 0 );
            d->connected = CON_GET_NEW_CLASS;
            break;
        }
        else if ( ALLOW_REROLL )
        {
            if ( argument[0] == '\0' )
            {
                roll_stats( ch, REROLL_PENALTY );
                show_stats( d );
            }
            write_to_buffer( d, "\n\rEnter a column number, or press enter to roll again: ", 0 );
        }
        else
        {
            sprintf( buf, "Please enter a column number 1 - %i: ", NUM_ROLLS );
            write_to_buffer( d, buf, 0 );
            break;
        }
        break;

    case CON_GET_NEW_CLASS:
        iClass = class_lookup( argument );

        if ( iClass == -1 || !str_cmp( argument, "console" )
             || ( class_table[iClass].remort_class
                  && !IS_SET( ch->act, PLR_REMORT ) ) )
        {
            write_to_buffer( d,
                             "That's not a class.\n\rWhat IS your class: ", 0 );
            return;
        }

        ch->Class = iClass;

        sprintf( log_buf, "%s@%s new player.", ch->name, d->host );
        log_string( log_buf );
        write_to_buffer( d, "\n\r", 2 );
        /*if (ch->race == 1) write_to_buffer( d, "Your class is evil by nature.\n\r",0);
           else { */
        write_to_buffer( d, "You may be good, neutral, or evil.\n\r", 0 );
        write_to_buffer( d, "Which alignment (G/N/E): ", 0 );
        /*} */
        d->connected = CON_GET_ALIGNMENT;
        break;

    case CON_GET_ALIGNMENT:
        /* if (ch->race == 1) ch->alignment = -750; */
        switch ( argument[0] )
        {
        case 'g':
        case 'G':
            ch->alignment = 750;
            break;
        case 'n':
        case 'N':
            ch->alignment = 0;
            break;
        case 'e':
        case 'E':
            ch->alignment = -750;
            break;
        default:
            write_to_buffer( d, "That's not a valid alignment.\n\r", 0 );
            write_to_buffer( d, "Which alignment (G/N/E): ", 0 );
            return;
        }

        write_to_buffer( d, "\n\r", 0 );

        group_add( ch, "rom basics", FALSE );
        group_add( ch, class_table[ch->Class].base_group, FALSE );
        ch->pcdata->learned[gsn_recall] = 50;
        write_to_buffer( d, "Do you wish to customize this character?\n\r", 0 );
        write_to_buffer( d,
                         "Customization takes time, but allows a wider range of skills and abilities.\n\r",
                         0 );
        write_to_buffer( d, "Customize (Y/N): ", 0 );
        d->connected = CON_DEFAULT_CHOICE;
        break;

    case CON_DEFAULT_CHOICE:
        //write_to_buffer( d, "\n\r", 2 );
        switch ( argument[0] )
        {
        case 'y':
        case 'Y':
            ch->gen_data = alloc_perm( sizeof( *ch->gen_data ) );
            ch->gen_data->points_chosen = ch->pcdata->points;
            write_to_buffer( d, "\n\r", 2 );
            do_help( ch, "group header" );
            list_group_costs( ch );
            write_to_buffer( d, "You already have the following skills:\n\r",
                             0 );
            do_skills( ch, "" );
            do_help( ch, "menu choice" );
            d->connected = CON_GEN_GROUPS;
            break;
        case 'n':
        case 'N':
            group_add( ch, class_table[ch->Class].default_group, TRUE );
            ch->train = (CP_MAX - ch->pcdata->points)/2 + STARTING_TRAINS;
            motd( ch );
            d->connected = CON_READ_MOTD;
            break;
        default:
            write_to_buffer( d, "Please answer (Y/N): ", 0 );
            return;
        }
        break;

    case CON_GEN_GROUPS:
        //send_to_char( "\n\r", ch );
        if ( !str_cmp( argument, "done" ) ) /* !str_cmp( s1, s2 ) mean s1 *is* equal to s2 */
        {
            /*if ( ch->pcdata->points < CP_MIN_CREATE )
            {
                sprintf(buf, "Continue now with 25 CP, +%d trains (Y/N):", CP_MIN_CREATE - ch->pcdata->points, CP_MIN_CREATE,
                       (CP_MIN_CREATE - ch->pcdata->points)/2);
                send_to_char( buf, ch );
                d->connected = CON_ACCEPT_CP;
                return;
                send_to_char
                ( "Choices are: list,learned,premise,add,drop,info,help, and done.\n\r",
                  ch );
                do_help( ch, "menu choice" );
                return;
            }*/

            //sprintf( buf, "Creation points: %d\n\r", ch->pcdata->points );
            //send_to_char( buf, ch );
            //sprintf( buf,
            //         "Experience modifier: %d\% (Percent of difference from the norm)\n\r",
            //         figure_difference( ch->gen_data->points_chosen ) );
            //if ( ch->pcdata->points < 40 )
            //    ch->train = ( 40 - ch->pcdata->points + 1 ) / 2; /* What is this? Seems to be overwritten anyway. */

            ch->train = (CP_MAX - ch->pcdata->points)/2 + STARTING_TRAINS;
            ch->pcdata->points = CP_MAX;
            //send_to_char( buf, ch );
            //write_to_buffer( d, "\n\r", 2 );
            motd( ch );
            d->connected = CON_READ_MOTD;
            break;
        }
        send_to_char( "\n\r", ch );
        if ( !parse_gen_groups( ch, argument ) )
            send_to_char
                ( "Invalid choice\n\r",
                  ch );

        do_help( ch, "menu choice" );
        break;
            

    case CON_BEGIN_REMORT:
        write_to_buffer( d, "Now beginning the remorting process.\n\r\n\r", 0 );
        prompt_race( d, ch, 3 );
        d->connected = CON_GET_NEW_RACE;
        break;

    case CON_READ_IMOTD:
#ifdef IMMS_CAN_HIDE
        if ( !str_cmp( argument, "hidden" ) )
        {
            SET_BIT( ch->act, PLR_NO_ANNOUNCE );
        }

        if ( !str_cmp( argument, "wizi" ) )
        {
            SET_BIT( ch->act, PLR_NO_ANNOUNCE );
            SET_BIT( ch->act, PLR_WIZINVIS );
            ch->invis_level = get_trust( ch );
        }

#endif
        motd( ch );
        
        d->connected = CON_READ_MOTD;
        break;

    case CON_READ_MOTD:
#if defined(cbuilder)
        AddUser( ch );
#endif
        write_to_buffer( d, "\n\r", 2 ); // JR asdf

        //if ( d->tintin )
        //      write_to_buffer( d, TINTIN_ON "\n\r", 6 ); // JR asdf
            
        /* Add to list of PCs and NPCs */
        ch->next = char_list;
        char_list = ch;
        /* Add to list of PCs */
        ch->next_player = player_list;
        player_list = ch;


        d->connected = CON_PLAYING;

        reset_char( ch );
        if ( ch->level == 0 )
        {

            ch->level = 1;
            ch->exp = 0;
            ch->hit = ch->max_hit;
            ch->mana = ch->max_mana;
            ch->move = ch->max_move;
            ch->start_age = number_range( pc_race_table[ch->race].age[0], pc_race_table[ch->race].age[1] ); // JR: randomize starting age
            // ch->train = STARTING_TRAINS; // Training sessions are now determined by CP
            ch->practice = STARTING_PRACTICES;
            set_title( ch, STARTING_TITLE );
            ch->visited = (char *)malloc(MAX_ROOMS*sizeof(char));
            long int counter;
            for ( counter=0; counter<MAX_ROOMS; counter++)
                ch->visited[counter] = '0';
            ch->visited[MAX_ROOMS-1] = 'X';
            ch->num_visited = 0;
            
            //sprintf( buf, "`G*******  Entering mudpi  *******`w\n\r\n\r");
            //send_to_char( buf, ch );

            do_outfit( ch, "" );
            obj_to_char( create_object( get_obj_index( OBJ_VNUM_MAP ), 0 ),
                         ch );

            ch->pcdata->learned[get_weapon_sn( ch )] = 40;

            char_to_room( ch, get_room_index( ROOM_VNUM_SCHOOL ) );
            send_to_char( "\n\r", ch );
            save_char_obj( ch );
            do_help( ch, "NEWBIE INFO" );
            send_to_char( "\n\r", ch );
        }
        else if ( ch->in_room != NULL )
        {
            char_to_room( ch, ch->in_room );
        }
        else if ( IS_IMMORTAL( ch ) )
        {
            char_to_room( ch, get_room_index( ROOM_VNUM_CHAT ) );
        }
        else
        {
            char_to_room( ch, get_room_index( ROOM_VNUM_TEMPLE ) );
        }

#ifdef ANNOUNCE_CONNECTIONS
        
#ifdef IMMS_CAN_HIDE
        if ( IS_SET( ch->act, PLR_NO_ANNOUNCE ) )
        {
            REMOVE_BIT( ch->act, PLR_NO_ANNOUNCE );
        }
        else
        {
#endif
            if ( !IS_SET( ch->act, PLR_WIZINVIS ) )
            {
                if ( ch->played == 0 && !IS_SET( ch->act, PLR_REMORT ) )
                    sprintf( buf, "%s has entered the game for the first time.",
                        ch->name );
                else
                    sprintf( buf, "%s has entered the game.", ch->name );
                do_sendinfo( ch, buf );
            }
#ifdef IMMS_CAN_HIDE
        }
#endif
#endif

        act( "$n appears in the room.", ch, NULL, NULL, TO_ROOM );
        
        write_to_buffer( d, "\n\r", 0 ); // For formatting
        do_look( ch, "auto" );
        
        if ( AUTO_BOARD )    
            do_board( ch, "" );     /* Show board status */

        if ( ch->pet != NULL )
        {
            char_to_room( ch->pet, ch->in_room );
            act( "$n appears in the room.", ch->pet, NULL, NULL, TO_ROOM );
        }

        REMOVE_BIT( ch->act, PLR_AFK );
        //if ( IS_SET( ch->act, PLR_AFK ) )
        //    do_afk( ch, NULL );

        ch->pcdata->chaos_score = 0;

        break;
    
    case CON_PAUSE: // JR: awful hack
            d->connected = CON_PLAYING;
            do_look( ch, "auto" );
            break;
    case CON_NOTE_TO:
        handle_con_note_to( d, argument );
        break;

    case CON_NOTE_SUBJECT:
        handle_con_note_subject( d, argument );
        break;                  /* subject */

    case CON_NOTE_EXPIRE:
        handle_con_note_expire( d, argument );
        break;

    case CON_NOTE_TEXT:
        handle_con_note_text( d, argument );
        break;

    case CON_NOTE_FINISH:
        handle_con_note_finish( d, argument );
        break;

    case CON_SHELL:
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
    if ( is_exact_name
         ( name, "all auto immortal self someone something the you fuck shit" )
         || is_name( name, "fuck" ) || is_name( name, "shit" )
         || is_name( name, "imm" ) )
        return FALSE;

    /*
     * Length restrictions.
     */

    if ( strlen( name ) < 2 )
        return FALSE;

    if ( strlen( name ) > 12 )
        return FALSE;

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
            if ( !isalpha( *pc ) )
                return FALSE;
            if ( LOWER( *pc ) != 'i' && LOWER( *pc ) != 'l' )
                fIll = FALSE;
        }

        if ( fIll )
            return FALSE;
    }

    /*
     * Prevent players from naming themselves after mobs.
     */
#ifdef NO_MOB_NAMES
    {
        extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
        MOB_INDEX_DATA *pMobIndex;
        int iHash;

        for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
        {
            for ( pMobIndex = mob_index_hash[iHash];
                  pMobIndex != NULL; pMobIndex = pMobIndex->next )
            {
                if ( is_exact_name( name, pMobIndex->player_name ) )
                    return FALSE;
            }
        }
    }
#endif

    return TRUE;
}

/*
 * Look for link-dead player to reconnect.
 */
bool check_reconnect( DESCRIPTOR_DATA * d, char *name, bool fConn )
{
    CHAR_DATA *ch;

    for ( ch = player_list; ch != NULL; ch = ch->next_player )
    {
        if ( ( !fConn || ch->desc == NULL )
             && !str_cmp( d->character->name, ch->name ) )
        {
            if ( fConn == FALSE )
            {
                free_string( &d->character->pcdata->pwd );
                d->character->pcdata->pwd = str_dup( ch->pcdata->pwd );
            }
            else
            {
                free_char( d->character );
                d->character = ch;
                ch->desc = d;
                ch->timer = 0;
                if ( IS_SET( ch->act, PLR_BUILDING ) )
                {
                    REMOVE_BIT( ch->act, PLR_BUILDING );
                }
                
                                
                send_to_char( "\n\rReconnecting.\n\r\n\r\n\r", ch );

                
                
                
#if defined(cbuilder)
                AddUser( ch );
#endif
                //act( "$n has reconnected.", ch, NULL, NULL, TO_ROOM ); // JR
                ch->pcdata->ticks = 0;
                sprintf( log_buf, "%s@%s reconnected.", ch->name, d->host );
                log_string( log_buf );
                
                if ( d->tintin )
                {
                    send_tt_settings( ch );
                    d->connected = CON_PAUSE;
                }
                else
                    d->connected = CON_PLAYING;
                /* Inform the character of a note in progress and the possbility of continuation! */
                if ( ch->pcdata->in_progress )
                    send_to_char
                        ( "You have a note in progress. Type NWRITE to continue it.\n\r",
                          ch );
            }
            //do_look( ch, "auto" );
            return TRUE;
        }
    }

    return FALSE;
}

/*
 * Check if already playing.
 */
bool check_playing( DESCRIPTOR_DATA * d, char *name )
{
    DESCRIPTOR_DATA *dold;

    for ( dold = descriptor_list; dold; dold = dold->next )
    {
        if ( dold != d
             && dold->character != NULL
             && dold->connected != CON_GET_NAME
             && dold->connected != CON_GET_OLD_PASSWORD
             && !str_cmp( name, dold->original
                          ? dold->original->name : dold->character->name ) )
        {
            write_to_buffer( d,
                             "That character is already playing.\n\rDisconnect that player? ",
                             0 );
            d->connected = CON_BREAK_CONNECT;
            return TRUE;
        }
    }

    return FALSE;
}

void stop_idling( CHAR_DATA * ch )
{
    if ( ch == NULL
         || ch->desc == NULL
         || ch->desc->connected != CON_PLAYING
         || ch->was_in_room == NULL
         || ch->in_room != get_room_index( ROOM_VNUM_LIMBO ) )
        return;

    ch->timer = 0;
    char_from_room( ch );
    char_to_room( ch, ch->was_in_room );
    ch->was_in_room = NULL;
    act( "$n has returned from the void.", ch, NULL, NULL, TO_ROOM );
    return;
}

/*
 * Write to one char.
 */
void send_to_char( const char *txt, CHAR_DATA * ch )
{
    if ( txt != NULL && ch->desc != NULL )
    {
        //write_to_buffer( ch->desc, "`w", 2 ); // JR: trying to fix a text color bug
        write_to_buffer( ch->desc, txt, strlen( txt ) ); // JR made 0 tem[]
    }
    return;
}

void printf_to_char( CHAR_DATA * ch, char *fmt, ... )
{
    char param[MAX_STRING_LENGTH * 4];
    {
        va_list args;
        va_start( args, fmt );
        vsprintf( param, fmt, args );
        va_end( args );
    }
    if ( param[0] && ch->desc )
        write_to_buffer( ch->desc, param, strlen( param ) );

    return;
}

/*
 * Send a page to one char.
 */
void page_to_char( const char *txt, CHAR_DATA * ch )
{
    if ( txt == NULL || ch->desc == NULL )
        return;


    ch->desc->showstr_head = alloc_mem( strlen( txt ) + 1 );
    strcpy( ch->desc->showstr_head, txt );
    ch->desc->showstr_point = ch->desc->showstr_head;
    show_string( ch->desc, "" );
}

/* string pager */
void show_string( DESCRIPTOR_DATA *d, char *input )
{
    char buf[MAX_STRING_LENGTH];
    char buffer[4 * MAX_STRING_LENGTH];
    register char *scan, *chk;
    int lines = 0, toggle = 1;
    int show_lines;

    one_argument( input, buf );
    if ( buf[0] != '\0' )
    {
        if ( d->showstr_head )
            free_mem( &d->showstr_head );
        d->showstr_point = 0;
        return;
    }
    if ( d->character )
        show_lines = d->character->lines;
    else
        show_lines = 0;
    for ( scan = buffer;; scan++, d->showstr_point++ )
    {
        if ( ( ( *scan = *d->showstr_point ) == '\n' || *scan == '\r' ) 
             && ( toggle = -toggle ) < 0 )
        {
            lines++;
        }
        else if ( !*scan || ( show_lines > 0 && lines >= show_lines ) )
        {
            *scan = '\0';
            write_to_buffer( d, buffer, 0 ); // JR: 0
            for ( chk = d->showstr_point; isspace( *chk ); chk++ ); // This needs to be here

            if ( !*chk ) // JR: removed indentation, braces
            {
                if ( d->showstr_head )
                    free_mem( &d->showstr_head );
                d->showstr_point = 0;
            }
            
            return;
        }
    }
    return;
}

/* quick sex fixer */
void fix_sex( CHAR_DATA * ch )
{
    if ( ch->sex < 0 || ch->sex > NUM_SEXES ) /* Modified by JR */
        ch->sex = IS_NPC( ch ) ? 0 : ch->pcdata->true_sex;
}

void act( const char *format, CHAR_DATA * ch, const void *arg1,
          const void *arg2, int type )
{
    /* to be compatible with older code */
    act_new( format, ch, arg1, arg2, type, POS_RESTING );
}



#define NAME(ch)	(IS_NPC(ch) ? ch->short_descr : ch->name)
char *act_string( const char *format, CHAR_DATA * to, CHAR_DATA * ch,
                  const void *arg1, const void *arg2 )
{
    static char buf[MAX_STRING_LENGTH];
    char fname[MAX_INPUT_LENGTH];
    char *point;
    const char *str = format;
    const char *i;
    CHAR_DATA *vch = ( CHAR_DATA * ) arg2;
    OBJ_DATA *obj1 = ( OBJ_DATA * ) arg1;
    OBJ_DATA *obj2 = ( OBJ_DATA * ) arg2;

    bzero( buf, sizeof( buf ) );
    point = buf;
    
    *point++ = '`'; // Added by JR
    *point++ = 'w'; // Always start fresh
    
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
            default:
                bug( "Act: bad code %d.", *str );
                i = " <@@@> ";
                break;
                /* Thx alex for 't' idea */
            case 't':
                i = ( char * ) arg1;
                break;
            case 'T':
                i = ( char * ) arg2;
                break;
            case 'n':
                i = ( to ? PERS( ch, to ) : NAME( ch ) );
                break;
            case 'N':
                i = ( to ? PERS( vch, to ) : NAME( vch ) );
                break;
            case 'e':
                i = he_she( ch->sex );
                break;
            case 'E':
                i = he_she( vch->sex );
                break;
            case 'm':
                i = him_her( ch->sex );
                break;
            case 'M':
                i = him_her( vch->sex );
                break;
            case 's':
                i = his_her( ch->sex );
                break;
            case 'S':
                i = his_her( vch->sex );
                break;
            case 'p':
                i = ( !to || can_see_obj( to, obj1 )
                      ? obj1->short_descr : "something" );
                break;
            case 'P':
                i = ( !to || can_see_obj( to, obj2 )
                      ? obj2->short_descr : "something" );
                break;
            case 'd':
                if ( !arg2 || ( ( char * ) arg2 )[0] == '\0' )
                    i = "door";
                else
                {
                    one_argument( ( char * ) arg2, fname );
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
    *point = '\0';
    /* Modified by JR to fix capitalization problem with color */
    /*if ( buf[0] != '`' ) 
        buf[0] = UPPER( buf[0] );
    else
        buf[2] = UPPER( buf[2] );*/
    char *tmp = buf;
    while ( *tmp == '`' || *tmp == ' ' )
    {
        if ( *tmp == '`' )
            tmp += 2;
        else
            tmp++;
    }
    *tmp = UPPER( *tmp );
    return buf;
}

char *act_new( const char *format, CHAR_DATA * ch, const void *arg1,
               const void *arg2, int type, int min_pos )
{
    char *txt = NULL;
    CHAR_DATA *to, *third;
    CHAR_DATA *vch = ( CHAR_DATA * ) arg2;

    /*
     * Discard null and zero-length messages.
     */
    if ( !format || !format[0] )
        return NULL;

    /* discard null rooms and chars */
    if ( !ch )
    {
        bug( "Act: null ch. (%s)", format );
        return NULL;
    }

    if ( !ch->in_room )
        to = NULL;
    else if ( type == TO_CHAR )
        to = ch;
    else
        to = ch->in_room->people;

    if ( type == TO_VICT )
    {
        if ( !vch )
        {
            bug( "Act: null vch with TO_VICT." );
            bug( "%s (%s)", ch->name, format );
            return NULL;
        }

        if ( !vch->in_room )
        {
            bug( "Act: vch in NULL room!" );
            bug( "%s -> %s (%s)", ch->name, vch->name, format );
            return NULL;
        }

        to = vch;
/*        to = vch->in_room->people;*/
    }
    
    if ( type == TO_THIRD )
        to = ( CHAR_DATA * ) arg1;

    /* room progs & obj progs */
    if ( MOBtrigger && type != TO_CHAR && type != TO_VICT && to )
    {
/*        OBJ_DATA *to_obj; *//* FIXME! - Zane */
        txt = act_string( format, NULL, ch, arg1, arg2 );
        /* FIXME! - Zane */
/*        if ( IS_SET( to->in_room->progtypes, ACT_PROG ) )
            rprog_act_trigger( txt, to->in_room, ch, (OBJ_DATA *)arg1, (void *)arg2 );
        for ( to_obj = to->in_room->contents; to_obj;
              to_obj = to_obj->next_content )
            if ( IS_SET(to_obj->pIndexData->progtypes, ACT_PROG) )
               oprog_act_trigger(txt, to_obj, ch, (OBJ_DATA *)arg1, (void *)arg2);*/
    }

    for ( ; to; to = ( type == TO_CHAR || type == TO_VICT || type == TO_THIRD )
          ? NULL : to->next_in_room )
    {
        if ( ( !to->desc && ( IS_NPC( to ) &&
                              !IS_SET( to->pIndexData->progtypes, ACT_PROG ) ) )
             || to->position < min_pos )
            continue;

        if ( type == TO_CHAR && to != ch )
            continue;
        if ( type == TO_VICT && ( to != vch || to == ch ) )
            continue;
        if ( type == TO_ROOM && to == ch )
            continue;
        if ( type == TO_NOTVICT && ( to == ch || to == vch ) )
            continue;

        txt = act_string( format, to, ch, arg1, arg2 );
        if ( to->desc )
            write_to_buffer( to->desc, txt, strlen( txt ) );

        mprog_act_trigger( txt, ch );

        oprog_act_trigger( txt, ch );

        rprog_act_trigger( txt, ch );

        /* FIXME! - Zane */
/*        if (MOBtrigger)
               mprog_act_trigger( txt, to, ch, (OBJ_DATA *)arg1, vch );*/

    }

    MOBtrigger = TRUE;
    return txt;
}

void do_color( register char *inbuf, int inlen, register char *outbuf,
               int outlen, bool color )
{
    char *origin = inbuf;
    char *origout = outbuf;

    while ( *inbuf != '\0' && origin + inlen > inbuf
            && origout + outlen > outbuf )
    {
        if ( *inbuf != '`' )
        {
            *outbuf++ = *inbuf++;
            continue;
        }

        if ( origin + inlen - 1 <= ++inbuf )
        {
            *outbuf = '\0';
            return;
        }

        if ( !color )
        {
            if ( *inbuf++ == '`' )
                *outbuf++ = '`';
        }
        else
            switch ( *inbuf++ )
            {
            case '0':
                if ( origout + outlen - 7 <= outbuf )
                {
                    *outbuf = '\0';
                    return;
                }
                strcpy( outbuf, "[0m" );
                outbuf += 4;
                break;
            case 'k':
                if ( origout + outlen - 7 <= outbuf )
                {
                    *outbuf = '\0';
                    return;
                }
                strcpy( outbuf, "[0;30m" );
                outbuf += 7;
                break;
            case 'K':
                if ( origout + outlen - 7 <= outbuf )
                {
                    *outbuf = '\0';
                    return;
                }
                strcpy( outbuf, "[1;30m" );
                outbuf += 7;
                break;
            case 'r':
                if ( origout + outlen - 7 <= outbuf )
                {
                    *outbuf = '\0';
                    return;
                }
                strcpy( outbuf, "[0;31m" );
                outbuf += 7;
                break;
            case 'R':
                if ( origout + outlen - 7 <= outbuf )
                {
                    *outbuf = '\0';
                    return;
                }
                strcpy( outbuf, "[1;31m" );
                outbuf += 7;
                break;
            case 'g':
                if ( origout + outlen - 7 <= outbuf )
                {
                    *outbuf = '\0';
                    return;
                }
                strcpy( outbuf, "[0;32m" );
                outbuf += 7;
                break;
            case 'G':
                if ( origout + outlen - 7 <= outbuf )
                {
                    *outbuf = '\0';
                    return;
                }
                strcpy( outbuf, "[1;32m" );
                outbuf += 7;
                break;
            case 'y':
                if ( origout + outlen - 7 <= outbuf )
                {
                    *outbuf = '\0';
                    return;
                }
                strcpy( outbuf, "[0;33m" );
                outbuf += 7;
                break;
            case 'Y':
                if ( origout + outlen - 7 <= outbuf )
                {
                    *outbuf = '\0';
                    return;
                }
                strcpy( outbuf, "[1;33m" );
                outbuf += 7;
                break;
            case 'b':
                if ( origout + outlen - 7 <= outbuf )
                {
                    *outbuf = '\0';
                    return;
                }
                strcpy( outbuf, "[0;34m" );
                outbuf += 7;
                break;
            case 'B':
                if ( origout + outlen - 7 <= outbuf )
                {
                    *outbuf = '\0';
                    return;
                }
                strcpy( outbuf, "[1;34m" );
                outbuf += 7;
                break;
            case 'm':
                if ( origout + outlen - 7 <= outbuf )
                {
                    *outbuf = '\0';
                    return;
                }
                strcpy( outbuf, "[0;35m" );
                outbuf += 7;
                break;
            case 'M':
                if ( origout + outlen - 7 <= outbuf )
                {
                    *outbuf = '\0';
                    return;
                }
                strcpy( outbuf, "[1;35m" );
                outbuf += 7;
                break;
            case 'c':
                if ( origout + outlen - 7 <= outbuf )
                {
                    *outbuf = '\0';
                    return;
                }
                strcpy( outbuf, "[0;36m" );
                outbuf += 7;
                break;
            case 'C':
                if ( origout + outlen - 7 <= outbuf )
                {
                    *outbuf = '\0';
                    return;
                }
                strcpy( outbuf, "[1;36m" );
                outbuf += 7;
                break;
            case 'w':
                if ( origout + outlen - 7 <= outbuf )
                {
                    *outbuf = '\0';
                    return;
                }
                strcpy( outbuf, "[0;37m" );
                outbuf += 7;
                break;
            case 'W':
                if ( origout + outlen - 7 <= outbuf )
                {
                    *outbuf = '\0';
                    return;
                }
                strcpy( outbuf, "[1;37m" );
                outbuf += 7;
                break;
            case '`':
                if ( origout + outlen - 2 <= outbuf )
                {
                    *outbuf = '\0';
                    return;
                }
                *outbuf++ = '`';
                break;
            default:
                if ( origout + outlen - 2 <= outbuf )
                {
                    *outbuf = '\0';
                    return;
                }
                *outbuf++ = '`';
                *outbuf++ = *( inbuf - 1 );
                break;
            }
    }

    if ( color )
    {
        if ( origout + outlen - 7 <= outbuf )
        {
            *outbuf = '\0';
            return;
        }
        strcpy( outbuf, "[0m\0" );
    }
    else
        *outbuf = '\0';
}

char *int_to_str( int value, int length, char style)
{
    static char string[40];
    char tmp[10];
    switch ( style )
    {
        case 'r': // Right justified
            sprintf( tmp, "%%%dd", length );
            break;
        case '0': // Zero padded
            sprintf( tmp, "%%0%dd", length );
            break;
        case 'l': // Left justified
            sprintf( tmp, "%%-%dd", length );
            break;
    }
    sprintf( string, tmp, value );
    return string;
}

char *figurestate( int current, int max )
{
    static char status[40];
    int length;
    sprintf( status, "%d", max );
    length = strlen( status );
    bzero( status, sizeof( status ) );
    if ( current >= ( max / 3 * 2 ) )
        sprintf( status, "`W%s`w", int_to_str( current, length, PROMPT_STYLE ) );
    else if ( current >= max / 3 )
        sprintf( status, "`Y%s`w", int_to_str( current, length, PROMPT_STYLE ) );
    else
        sprintf( status, "`R%s`w", int_to_str ( current, length, PROMPT_STYLE ) );
    return ( status );
}

char *damstatus( CHAR_DATA * ch )
{
    int percent;
    static char wound[40];

    bzero( wound, sizeof( wound ) );
    if ( ch->max_hit > 0 )
        percent = ch->hit * 100 / ch->max_hit;
    else
        percent = -1;
    if ( percent >= 100 )
        sprintf( wound, "excellent condition" );
    else if ( percent >= 90 )
        sprintf( wound, "few scratches" );
    else if ( percent >= 75 )
        sprintf( wound, "small wounds" );
    else if ( percent >= 50 )
        sprintf( wound, "quite a few wounds" );
    else if ( percent >= 30 )
        sprintf( wound, "nasty wounds" );
    else if ( percent >= 15 )
        sprintf( wound, "pretty hurt" );
    else if ( percent >= 0 )
        sprintf( wound, "awful condition" );
    else
        sprintf( wound, "bleeding to death" );
    return ( wound );
}

char *doparseprompt( CHAR_DATA * ch )
{
    CHAR_DATA *tank, *victim;
    static char finished_prompt[240];
    char workstr[100];
    char waitbuf[10];
    int hp_dig=0, mana_dig=0, mv_dig=0;
    char *fp_point;
    char *orig_prompt;
    bool twoline = FALSE;
    bool tintin;
    
    if ( IS_NPC( ch ) )
    {
        sprintf( finished_prompt, "%s<H%d/%d M%d/%d V%d/%d>", wait_str( ch, waitbuf ), ch->hit,
                             ch->max_hit, ch->mana, ch->max_mana, ch->move,
                             ch->max_move );
        return finished_prompt;
    }
    bzero( finished_prompt, sizeof( finished_prompt ) );
    orig_prompt = ch->pcdata->prompt;
    fp_point = finished_prompt;
    strcpy( fp_point, "" );
    tintin = is_fixed( ch );
    if ( tintin )
    {
        for ( int n = 0; n + 1 < strlen(orig_prompt); n++ )
        {
            if ( orig_prompt[n] == '%' && orig_prompt[n+1] == 'r' )
            {
                twoline = TRUE;
                break;
            }
        }

        if ( twoline )
        {
            strcpy( fp_point, PROMPT_TOP );
            fp_point += strlen( PROMPT_TOP );
        }
        else
        {
            strcpy( fp_point, PROMPT_TOP "\n\r" PROMPT_BOTTOM );
            fp_point += strlen( PROMPT_TOP ) + strlen( PROMPT_BOTTOM ) + 2;
        }
    }
    twoline = FALSE;
    while ( *orig_prompt != '\0' )
    {
        if ( *orig_prompt != '%' )
        {
            *fp_point = *orig_prompt;
            orig_prompt++;
            fp_point++;
            continue;
        }
        orig_prompt++;
        switch ( *orig_prompt )
        {
        case 'h':
            if ( hp_dig == 0 )
            {
                sprintf( workstr, "%d", ch->max_hit );
                hp_dig = strlen( workstr );
            }
            strcat( finished_prompt, int_to_str( ch->hit, hp_dig, PROMPT_STYLE ) );
            fp_point += hp_dig;
            orig_prompt++;
            break;
        case 'H':
            sprintf( workstr, "%d", ch->max_hit );
            strcat( finished_prompt, workstr );
            fp_point += strlen( workstr );
            orig_prompt++;
            break;
        case 'm':
            if ( mana_dig == 0 )
            {
                sprintf( workstr, "%d", ch->max_mana );
                mana_dig = strlen( workstr );
            }
            strcat( finished_prompt, int_to_str( ch->mana, mana_dig, PROMPT_STYLE ) );
            fp_point += mana_dig;
            orig_prompt++;
            break;
        case 'M':
            sprintf( workstr, "%d", ch->max_mana );
            strcat( finished_prompt, workstr );
            fp_point += strlen( workstr );
            orig_prompt++;
            break;
        case 'v':
            if ( mv_dig == 0 )
            {
                sprintf( workstr, "%d", ch->max_move );
                mv_dig = strlen( workstr );
            }
            strcat( finished_prompt, int_to_str( ch->move, mv_dig, PROMPT_STYLE ) );
            fp_point += mv_dig;
            orig_prompt++;
            break;
        case 'V':
            sprintf( workstr, "%d", ch->max_move );
            strcat( finished_prompt, workstr );
            fp_point += strlen( workstr );
            orig_prompt++;
            break;
        case 'r':
            if ( tintin )
            {
                if ( twoline )
                {
                    printf("Bail on prompt!\n");
                    return ( finished_prompt ); // Bail out
                }
                twoline = TRUE;
                strcat( finished_prompt, "\n\r" PROMPT_BOTTOM );
                fp_point += strlen( PROMPT_BOTTOM ) + 2;
            }
            else
            {
                strcat( finished_prompt, "\n\r" );
                fp_point += 2;
            }
            orig_prompt++;
            break;
        case 'i':
            sprintf( workstr, "%s", figurestate( ch->hit, ch->max_hit ) );
            strcat( finished_prompt, workstr );
            fp_point += strlen( workstr );
            orig_prompt++;
            break;
        case 'n':
            sprintf( workstr, "%s", figurestate( ch->mana, ch->max_mana ) );
            strcat( finished_prompt, workstr );
            fp_point += strlen( workstr );
            orig_prompt++;
            break;
        case 'w':
            sprintf( workstr, "%s", figurestate( ch->move, ch->max_move ) );
            strcat( finished_prompt, workstr );
            fp_point += strlen( workstr );
            orig_prompt++;
            break;
        case 'l':
            if ( ( tank = ch->fighting ) != NULL )
                if ( ( tank = tank->fighting ) != NULL )
                {
                    sprintf( workstr, "%s", damstatus( tank ) );
                    strcat( finished_prompt, workstr );
                    fp_point += strlen( workstr );
                }
            orig_prompt++;
            break;
        case 'e':
            if ( ( victim = ch->fighting ) != NULL )
            {
                sprintf( workstr, "%s", damstatus( victim ) );
                strcat( finished_prompt, workstr );
                fp_point += strlen( workstr );
            }
            orig_prompt++;
            break;
        case 's':
            sprintf( workstr, "%s", damstatus( ch ) );
            strcat( finished_prompt, workstr );
            fp_point += strlen( workstr );
            orig_prompt++;
            break;
        case 'T':
            sprintf( workstr, "%s", get_curtime(  ) );
            strcat( finished_prompt, workstr );
            fp_point += strlen( workstr );
            orig_prompt++;
            break;
        case '#':
            if ( IS_IMMORTAL( ch ) && ch->in_room != NULL )
            {
                sprintf( workstr, "%d", ch->in_room->vnum );
                strcat( finished_prompt, workstr );
                fp_point += strlen( workstr );
                orig_prompt++;
                break;
            }
            else
                break;
        case 'a':
            sprintf( workstr, "%d", ch->alignment );
            strcat( finished_prompt, workstr );
            fp_point += strlen( workstr );
            orig_prompt++;
            break;
        case 'B':
            if ( IS_SET( ch->act, PLR_BUILDING ) )
            {
                sprintf( workstr, "`Y[`RBuilding`Y]`w " );
                strcat( finished_prompt, workstr );
                fp_point += strlen( workstr );
            }
            orig_prompt++;
            break;
        case 'A':
            if ( IS_SET( ch->act, PLR_AFK ) )
            {
                sprintf( workstr, "`W(AFK)" );
                strcat( finished_prompt, workstr );
                fp_point += strlen( workstr );
            }
            if ( IS_SET( ch->act, PLR_KILLER ) )
            {
                sprintf( workstr, "`R(PK)" );
                strcat( finished_prompt, workstr );
                fp_point += strlen( workstr );
            }
            if ( IS_SET( ch->act, PLR_THIEF ) )
            {
                sprintf( workstr, "`K(THIEF)" );
                strcat( finished_prompt, workstr );
                fp_point += strlen( workstr );
            }
            if ( IS_IMMORTAL( ch ) && IS_SET( ch->act, PLR_WIZINVIS ) )
            {
                sprintf( workstr, "`B(Wizi:%d)", ch->invis_level );
                strcat( finished_prompt, workstr );
                fp_point += strlen( workstr );
            }
            orig_prompt++;
            break;
        case 'q':
            sprintf( workstr, "%s", ch->in_room->area->name );
            strcat( finished_prompt, workstr );
            fp_point += strlen( workstr );
            orig_prompt++;
            break;
        case 'x':
            sprintf( workstr, "%ld", ch->exp );
            strcat( finished_prompt, workstr );
            fp_point += strlen( workstr );
            orig_prompt++;
            break;
        case 'X':
            sprintf( workstr, "%ld",
                     exp_per_level( ch, ch->pcdata->points ) - ch->exp );
            strcat( finished_prompt, workstr );
            fp_point += strlen( workstr );
            orig_prompt++;
            break;
        case 'W':
            wait_str( ch, waitbuf );
            sprintf( workstr, "%s", waitbuf );
            strcat( finished_prompt, workstr );
            fp_point += strlen( workstr );
            orig_prompt++;
            break;
        default:
            strcat( finished_prompt, "%" );
            fp_point++;
            break;
        }
    }
    if ( tintin )
        strcat ( finished_prompt, "`w\n\r`w" );
    return ( finished_prompt );
}

/* Roll a six sided die four times, discard the lowest roll and total the other three,
 * add the base_bonus and return the result */
int roll_stat( int base_bonus, int stat_max )
{
    int roll = 0;
    int min_roll = 99;
    int total = 0;
    int counter;

    /* Roll a six sided die four times, total the result and remember the lowest roll */
    for ( counter = 0; counter < 4; counter++ )
    {
        roll = number_range( 1, 6 );
        total += roll;

        if ( roll < min_roll )
            min_roll = roll;
    }

    /* 
     * Discard the lowest roll of the four and return the total plus the base_bonus 
     * If the result is less than 1 return 1
     */
    return UMAX( total - min_roll + base_bonus, 1 );
}

// Display list of allowable races
void prompt_race ( DESCRIPTOR_DATA * d, CHAR_DATA * ch, int columns )
{
    int max_len = 0,x=0,i,race;
    write_to_buffer( d, "\n\rThe following races are available:\n\r", 0 );
    // Compute the max length of race here
    for ( race = 1; race_table[race].name != NULL; race++ )
    {
        if ( !race_table[race].pc_race )
            break;
        if ( !race_table[race].remort_race ||
             ( race_table[race].remort_race
               && IS_SET( ch->act, PLR_REMORT ) ) )
            max_len = str_len(race_table[race].name) > max_len ? 
                str_len(race_table[race].name) : max_len;
    }
    for ( race = 1; race_table[race].name != NULL; race++ )
    {
        if ( !race_table[race].pc_race )
            break;
        if ( !race_table[race].remort_race ||
             ( race_table[race].remort_race
               && IS_SET( ch->act, PLR_REMORT ) ) )
        {
            if ( x == columns )
            {
                write_to_buffer( d, "\n\r", 2 );
                x = 0;
            }
            if ( x == 0 )
                write_to_buffer( d, "   ", 3 );
            write_to_buffer( d, race_table[race].name, 0 );
            for ( i=0 ; i+strlen(race_table[race].name) < max_len + 3; i++)
                write_to_buffer( d, " ", 1 );
            x++;
        }
    }
    write_to_buffer( d, "\n\r\n\r", 4 );
    write_to_buffer( d, "What is your race ('help' for more information): ", 0 );
}

void motd( CHAR_DATA * ch )
{
    DESCRIPTOR_DATA *d = ch->desc;
    //char buf[20];
    send_tt_settings( ch );
    write_to_buffer( d, "\n\r", 2 );
    //if ( d -> tintin )
    //    write_to_buffer( d, buf, 0 );
    do_help( ch, "motd" );
    if ( d -> tintin )
        write_to_buffer( d, "\n\r\n\r", 4 );
}

void roll_stats( CHAR_DATA * ch, int penalty )
{
    int i,n;
    for ( n = 0; n < NUM_ROLLS; n++ )
    {
        for ( i = 0; i < 5; i++ )
        {
            stat[i][n] = roll_stat( pc_race_table[ch->race].stats[i],
                                   pc_race_table[ch->race].max_stats[i] -
                                   penalty );
        }
    }
    return;
}


void show_stats( DESCRIPTOR_DATA * d )
{
    char buf[MAX_STRING_LENGTH];
    int x, i;
    strcpy( buf, "                     " );
    for ( i = 0; i < NUM_ROLLS; i++)
    {
        sprintf(buf+strlen(buf),"%2d   ",i+1);
    }
    strcat( buf, "\n\r" );    
    write_to_buffer( d, buf, 0 );
    strcpy( buf, "   ---------------" );
    for ( i = 0; i < NUM_ROLLS; i++ )
        strcat( buf, "-----" );
    strcat( buf, "\n\r" );
    write_to_buffer( d, buf, 0 );
    for ( x = 0; x < 5; x++ )
    {
        sprintf( buf, "   %s", stat_names[x] );
        lengthen( buf, 18 );
        strcat( buf, ": " );
        for ( i = 0; i < NUM_ROLLS; i++ )
            sprintf( buf+strlen(buf), " %2d |", stat[x][i] );
        sprintf( buf+strlen(buf)-1, "\n\r" );
        write_to_buffer( d, buf, 0 );
    }
}

// JR: add a line to the wait queue
void add_wait_queue( DESCRIPTOR_DATA * d, char * command )
{
    INPUT_LINE *line;
    line = malloc( sizeof(*line));
    strcpy( line->command, command );
    line->next = NULL;
    if ( d->wait_queue == NULL )
        d->wait_queue = line;
    else
        d->wait_queue_last->next = line;
    d->wait_queue_last = line;
}

// JR: execute the top item in the wait queue
void interpret_wait_queue( DESCRIPTOR_DATA * d )
{
    char *c;
    INPUT_LINE *input = d->wait_queue;
    if ( input == NULL || d->character == NULL )
        return;
    c = input->command;
    interpret( d->character, c );
    d->wait_queue = d->wait_queue->next;
    free( input );
    return;
}