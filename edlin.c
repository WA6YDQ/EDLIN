/* edlin.c - simple line editor 
 *
 * (C) 7/2023 k theis  <theis.kurt@gmail.com>
 * 
 * This is just a small, simple text line editor
 * with no baggage attached. Should work on the
 * simplest of machines with something as simple
 * as a tty device.
 *
 * If you need this as a static program (no libraries)
 * run 'make static' otherwise just run 'make'.
 *
 * Entered line lengths can be modified in defines. Files with
 * longer lines can be loaded - you will see warnings when printed.
 *
 * This program is under the MIT license. Do what
 * you want with it.
 *
 * Revisions:
 * 0.9a change line length to max length likely ever encountered
 * 0.9  buffer grows dynamically as needed
 * 0.8  added search function
 * 0.7  insert, print, delete functions w/line numbers added
 * 0.6  some cleanup
 *  
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define CBUFSIZE 8192		// initial size of text buffer
#define BUFGROW 2048		// size to grow buffer by when it gets low
#define LINESIZE 132		// size of input line
#define VERSION "0.9a"

char *buffer;			// main text buffer
int position = 0;		// counter showing current position in buffer
int first=0, last=0;		// used with line numbering
int FIRSTTIME = 1;		// show messages the first time run
int BUFSIZE;			// holds upward-growing buffer size

void append(void);		// buffer input routine
void loadfile(char*);		// load an external file into the buffer
void help(void);		// show command summary
void lineprint(int,int);	// show a block of lines
void delete(int,int);		// delete a block (or single) lines
void insert(int,int);		// insert a (or multiple) lines. stop with a single .
void search(char *);		// search for a substring in the buffer

void help(void) {		// show commands 
	printf("q		exit edlin \n");
	printf("w [filename]	save buffer to file \n");
	printf("l [filename]	load an external file into buffer \n");
	printf("n		delete existing buffer (new file) \n");
	printf("p		print (list) buffer \n");
	printf("a		append into end of buffer \n");
	printf("s [substring]	search for a substring in the buffer \n");
	printf("v		show buffer statistics \n");
	printf("[mm-nn]d	delete lines mm-nn \n");
	printf("mm]i		insert new lines before line number mm \n");
	printf("[mm-nn]p	print (list) lines mm-nn in buffer \n");
	printf("When appending/inserting, use '.' to stop \n");
	printf("\n");
	return;
}

void search(char *substring) {			// search for a substring in the buffer
	char ch, *subline;
	int n=0, pos=0, linenum=1;
	subline = malloc(LINESIZE+1*sizeof(char));
	if (subline == NULL) {
		fprintf(stderr,"Memory Error \n");
		return;
	}
	// get a line from the buffer, search, print if match
	while (1) {
		memset(subline,0,LINESIZE);
		for (n=0; n<LINESIZE; n++) {
			ch = *(buffer+pos+n);
			subline[n] = ch;
			if (ch == '\n') break;
		}
		if (ch != '\n') {
			fprintf(stderr,"WARNING - line length exceeds settings. \n");
			fprintf(stderr,"Possible corruption in line %d \n",linenum);
		}
		// substring in subline. search
		if (strstr(subline,substring) != NULL) 		// hit
			printf("%03d] %s",linenum,subline);
		// next line
		linenum++;
		pos += n+1;
		if (pos < position-1) continue;
		free(subline);
		printf("\n");
		return;
	}
}


void lineprint(int first, int last) {		// show a group of lines

	if (first > last) last=first;	
	int linecnt = 1;
	int ptr = 0; int n=0;
	char ch;
	char line[LINESIZE+1];
	while (1) {
		memset(line,0,LINESIZE);
		n = 0;
		while (n < LINESIZE) {
			ch = *(buffer+ptr+n);
			if (ch == '\0') break;
			line[n++] = ch;
			if (ch != '\n') continue;
			if (linecnt == first || (linecnt >= first && linecnt <= last)) 
				printf("%03d] %s",linecnt,line);
			break;
		}
		// test for too-long lines
		if (n >= LINESIZE) { 
			fprintf(stderr,"WARNING - line length exceeds settings \n");
			fprintf(stderr,"Possible corruption in file at line %d \n",linecnt);
		}
		linecnt++; ptr += n;
		if (ptr < position-1) continue;
		break;
	}
	return;
}

void delete(int first, int last) {		// delete a group of lines
	
	if (first > last) last = first;
	int startpos=0, endpos=0;		// where in the buffer are the lines
	int linecnt = 1, ptr = 0, n = 0;
	char ch;
	char line[LINESIZE+1];
	while (1) {
		memset(line,0,LINESIZE);
		n = 0;
		while (n < LINESIZE) {
			ch = *(buffer+ptr+n);
			if (ch == '\0') break;
			line[n++] = ch;
			if (ch != '\n') continue;	// search line by line
			if (linecnt == first) startpos = ptr;
			if (linecnt == last) endpos = ptr+n;	// skip past \n
			break;
		}
		linecnt++; ptr += n;
		if (ptr >= position-1) break;
		continue;
	}
	endpos--;		// convert to real #
	
	// got positions, now delete
	for (int n=startpos; n<=endpos; n++) *(buffer+n)='\0';

	// now shift everything down by (endpos-startpos)
	n=0;
	while (1) {
		*(buffer+startpos+n) = *(buffer+endpos+n+1);	// +1 to skip past \n
		n++;
		if ((n+endpos) == position) break;
	}

	// and clear out old code
	int shift = endpos-startpos;
	for (n=position-shift; n<=position; n++)
		*(buffer+n) = '\0';

	// and reset position
	position = position-shift-1;

	return;
}



void insert(int first,int last) {
	// insert a line of text at start address. loop until user enters '.' to stop
	
	if (first > last) last = first;
        int startpos=0, endpos=0;               // where in the buffer are the lines
        int linecnt = 1, ptr = 0, n = 0;
        char ch;
        char line[LINESIZE+1];
        while (1) {
                memset(line,0,LINESIZE);
                n = 0;
                while (n < LINESIZE) {
                        ch = *(buffer+ptr+n);
                        if (ch == '\0') break;
                        line[n++] = ch;
                        if (ch != '\n') continue;       // search line by line
                        if (linecnt == first) startpos = ptr;
                        if (linecnt == last) endpos = ptr+n;    // skip past \n
                        break;
                }
                linecnt++; ptr += n;
                if (ptr >= position-1) break;
                continue;
        }

insloop:

	// startpos points to location where the line of text will go
	char insline[LINESIZE+1];
	printf("$ ");			// get a line of text
	fgets(line,LINESIZE,stdin);
	if (line[0] == '.') return;	// quit character
	int ls = strlen(line);		// how much to insert
	
	// start at position+ls, move data up
	ptr = position+ls;		// avoid \0 at end
	if (ptr > BUFSIZE-LINESIZE) {
		char *tmpptr;
                tmpptr = realloc(buffer, (BUFSIZE+BUFGROW)*sizeof(char));
                if (tmpptr == NULL) {
                	fprintf(stderr,"Memory Error \n");
                        free(buffer);
                        exit (1);
                }
                buffer = tmpptr;
		tmpptr = NULL;
                BUFSIZE+=BUFGROW;
	}
	
	// shift buffer up
	n = 0;
	while (1) {
		*(buffer+ptr-n) = *(buffer+position-n);
		if (position-n-1 == startpos-1) break;
		n++;
	}

	// now insert line into buffer
	n=0;
	while (1) {
		*(buffer+startpos+n) = line[n];
		if (n == strlen(line)-1) break;
		n++;
	}

	// get new startpos
	startpos = startpos+n+1;

	// update end position
	position += strlen(line);
	
	goto insloop;			// continue until user enters a '.'	
}



void loadfile(char *filename) {		// load an external file
	int linex = 0;			// counter to test for overflow

	FILE *fd = fopen(filename,"r");
	if (fd == NULL) {
		fprintf(stderr,"Unable to open %s\n",filename);
		return;
	}
	char ch;
	while (1) {
		ch = fgetc(fd);
		
		linex++;
		if (linex > LINESIZE) {
			fprintf(stderr,"Warning - line length exceeded while loading! \n");
			linex=0;
		}
		if (ch == '\n') linex = 0;

		if (feof(fd)) break;
		*(buffer+position) = ch;
		position++;
		if (position > BUFSIZE-LINESIZE) {
			char *tmpptr;
			tmpptr = realloc(buffer, (BUFSIZE+BUFGROW)*sizeof(char));
			if (tmpptr == NULL) {
				fprintf(stderr,"Memory Error \n");
				free(buffer);
				exit (1);
			}
			buffer = tmpptr;
			tmpptr = NULL;
			BUFSIZE+=BUFGROW;
			continue;;
		}
		continue;
	}
	fclose(fd);
	printf("Read in %d characters \n",position);
	printf("\n");
	return;
}


int main(int argc, char **argv) {

	// set buffer size
	BUFSIZE = CBUFSIZE;		// realloc as needed

	buffer = malloc(BUFSIZE*sizeof(char));
	if (buffer == NULL) {
		fprintf(stderr,"Memory Error \n");
		exit(1);
	}
	memset(buffer,0,BUFSIZE);

	// test for input file
	if (argv[1] != NULL) 
		loadfile(argv[1]);

	char line[LINESIZE+1] = {""};

	if (FIRSTTIME) {
		printf("Press ? at the # prompt for help\n");
	}

	while (1) {		// command loop
		memset(line,0,LINESIZE);
		first=0; last=0;
		printf("# ");	// prompt
		fgets(line,LINESIZE,stdin);

		if (line[0] == 'q')  {		// exit without saving
			free(buffer);
			exit(0);
		}

		if (line[0] == 'n') {		// delete buffer (new)
			memset(buffer,0,BUFSIZE);
			position = 0;
			printf("Buffer Cleared \n");
			continue;
		}

		if (line[0] == 'a')  {		// insert into text buffer
			append();
			continue;
		}

		if (line[0] == 'w') {		// save buffer to file
			char *filename = malloc(256*sizeof(char));
			char *cmd = malloc(80*sizeof(char));
			if (filename == NULL || cmd == NULL) {		// memory error
				fprintf(stderr,"Memory Error - file not saved \n");
				exit(1);
			}
			sscanf(line,"%s %s",cmd,filename);
			// save buffer to file
			FILE *fd = fopen(filename,"w");
			if (fd == NULL) {
				fprintf(stderr,"Cannot open %s\n",filename);
				free(cmd); free(filename);
				continue;
			}
			for (int n=0; n<position; n++)
				fprintf(fd,"%c",*(buffer+n));
			fflush(fd); fclose(fd);
			// done
			printf("File %s saved \n",filename);
			free(cmd);
			free(filename);
			continue;
		}			

		if (line[0] == 'l') {		// load an external file
			char cmd[8] = {""};
			char fname[60] = {""};
			sscanf(line,"%s %s",cmd,fname);
			loadfile(fname);
			continue;
		}

		if (line[0] == 'p') {		// show buffer (simple display - show all of it)
			int linecnt = 1;
			printf("%03d] ",linecnt);
			int ccnt = 0;
			while (1) {
				printf("%c",*(buffer+ccnt));
				if (*(buffer+ccnt)=='\n') {
					linecnt++;
					printf("%03d] ",linecnt);
				}
				ccnt++;
				if (ccnt < position) continue;
				break;
			}
			printf("\r    \r");	// delete last displayed line number
			continue;

		
			for (int n=0; n<position; n++)
				printf("%c",*(buffer+n));
			printf("\n");
			continue;
		}

		if (line[0] == 's') {		// search for a substring
			char cmd[6], substring[LINESIZE+1];
			sscanf(line,"%s %s",cmd,substring);
			search(substring);
			continue;
		}


		if (line[0] == 'v') {		// show buffer size
			printf("%d characters in buffer \n",position);
			printf("version: %s\n",VERSION);
			continue;
		}


		/* format: mm-nn, mm [command] - run command on a single line or multiple lines of the buffer */
		if (isdigit(line[0])) {
			char digit[80] = {""}; int digitcnt=0; int linecnt=0;
			while (1) {		// get 1st number
				digit[digitcnt++] = line[linecnt++];
				if (linecnt > LINESIZE) break;
				if (isdigit(line[linecnt])) continue;
				else
					break;
			}
			
			// non-digit in line - get first number
			first = atoi(digit); memset(digit,0,10); digitcnt = 0;
			if (line[linecnt] == '-' || line[linecnt] == ',') {	// seperator, get next digit
				linecnt++;
				while(1) {	// get 2nd number
					digit[digitcnt++] = line[linecnt++];
					if (isdigit(line[linecnt])) continue;
					else
						break;
				}
				// non-digit in line - get 2nd number
				last = atoi(digit); 
			}
			
			// we have 1 or 2 digits and the next char needs to be tested
			while (isblank(line[linecnt])) linecnt++; 	// skip blanks
			
			// next char should be p (print), d (delete)
			if (line[linecnt] == 'p') {		// print a group of lines
				lineprint(first,last);
				continue;
			}
			if (line[linecnt] == 'd') {		// delete a group of lines
				delete(first,last);
				continue;
			}
			if (line[linecnt] == 'i') {		// insert a line
				insert(first,last);
				continue;
			}
			printf("bad format: use mm,nn or mm-nn or mm followed by p/d/i\n");
			continue;
		}


		// no other commands (show help in all cases)
		help();		// show help
		continue;
	}
}


void append(void) {		// get user input, save in buffer. 
	
	int cnt;
	char line[LINESIZE+1] = {""};
	if (FIRSTTIME) {
		printf("Type . on an empty line to exit insert mode \n");
		FIRSTTIME=0;
	}

	while (1) {		// entry loop
		memset(line,0,LINESIZE);
		printf("$ ");
		fgets(line,LINESIZE,stdin);

		if (line[0] == '.') {	// done entering
			return;
		}

		// save line to buffer - increment pointer
		cnt = 0;
		while (1) {
			if (line[cnt] == '\0') break;
			if (cnt == LINESIZE) break;
			*(buffer+position) = line[cnt];
			position++; cnt++;
		}
		if (position > BUFSIZE-LINESIZE) {		// buffer full
			char *tmpptr;
                        tmpptr = realloc(buffer, (BUFSIZE+BUFGROW)*sizeof(char));
                        if (tmpptr == NULL) {
                                fprintf(stderr,"Memory Error \n");
                                free(buffer);
                                exit (1);
                        }
                        buffer = tmpptr;
			tmpptr = NULL;
                        BUFSIZE+=BUFGROW;
			continue;
		}
		continue;
	}

}	
