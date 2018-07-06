/* bounce_aio.c
 *	purpose	animation with user control, using aio_read() etc
 *	note	set_ticker() sends SIGALRM, handler does animation
 *		keyboard sends SIGIO, main only calls pause()
 *	compile cc bounce_aio.c set_ticker.c -lrt  -lcurses -o bounce_aio
 */

#include <stdio.h>
#include <curses.h>
#include <signal.h>
#include <aio.h>
#include <string.h>
#include <unistd.h>


/* The state of the game 			*/

#define	MESSAGE	"hello"
#define	BLANK	"     "

int	row   = 10;	/* current row		*/
int	col   =  0;	/* current column	*/
int	dir   =  1;	/* where we are going	*/
int	delay = 200;	/* how long to wait	*/
int	done  = 0;

struct aiocb kbcbuf;

extern int set_ticker(int);

int main()
{
    void	on_alarm(int);	/* handler for alarm	*/
    void	on_input(int);	/* handler for keybd    */
    void	setup_aio_buffer();
    
    initscr();		/* set up screen */
    crmode();
    noecho();
    clear();

    signal(SIGIO, on_input);
    setup_aio_buffer();
    aio_read(&kbcbuf);
    
    signal(SIGALRM, on_alarm);
    set_ticker(delay);
    
    mvaddstr(row, col, MESSAGE);
    
    while (!done)
        pause();
    endwin();
}

/*
 * handler called when aio_read() has stuff to read
 *   First check for any error codes, and if ok, then get the return code
 */
void on_input()
{
    int c;
    char *cp = (char *) kbcbuf.aio_buf;
    
    if (aio_error(&kbcbuf) != 0)
        perror("reading failed");
    else
        if (aio_return(&kbcbuf) == 1) {
            c = *cp;
            if (c == 'q' || c == EOF)
                done = 1;
            else if (c == ' ')
                dir = -dir;
        }
    
    /* place a new request */
    aio_read(&kbcbuf);
}

void on_alarm()
{
    signal(SIGALRM, on_alarm);
    mvaddstr(row, col, BLANK);
    col += dir;
    mvaddstr(row, col, MESSAGE);
    refresh();
    
    /*
     * now handle borders
     */
    if ( dir == -1 && col <= 0 )
        dir = 1;
    else if ( dir == 1 && col+strlen(MESSAGE) >= COLS )
        dir = -1;
}


/*
 * set members of struct.
 *   First specify args like those for read(fd, buf, num)  and offset
 *   Then  specify what to do (send signal) and what signal (SIGIO)
 */

void setup_aio_buffer()
{
    static char input[1];
    
    kbcbuf.aio_fildes = 0;
    kbcbuf.aio_buf = input;
    kbcbuf.aio_nbytes = 1;
    kbcbuf.aio_offset = 0;
    
    kbcbuf.aio_sigevent.sigev_notify = SIGEV_SIGNAL;
    kbcbuf.aio_sigevent.sigev_signo = SIGIO;
}













