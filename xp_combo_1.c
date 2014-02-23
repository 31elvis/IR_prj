/****************************************************************************
 *                                                                          *
 * File    : combo1.c                                                       *
 *                                                                          *
 * Purpose : Console mode (command line) program.                           *
 *                                                                          *
 * History : Date     Reason                                                *
 *           09/18/13 Set up remote database//take user input//control      *
 *                    Arduino ir driver                                     *
 ****************************************************************************/
//1028 xperimental version for w7 machine --test out concepts
// comm issues settled by making separate return variaboe and by removing excess comm from ard
//1013 used sepg to fix some nasty comm issues -- not sure it the ard gets 
//extra stuff or if its buffer needs to be cleared out works erratically 
//first no resp -- then gradually longer
//0919 -- nearly there -- can only send one command w/cmds and then must repoint
//there surely is a way, but I don't know it!
//so, assign cmds->send->assign hex -> send

//0920 update -- now works, but I should separate the open comport
//0920 pretty much seems to 'work' need to modify to send cmd name and hex
// also need to check that return works  

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif
#include "rs232.h"


const char * rain(const char *, const char *); //comm function
int rdme(void); //user input function
int main (int argc, char *argv[]  )
{
//this section populates the database 
   		const char * cmds[2];  // pointer for xfer of command 
	    char tmp[50] ; //working array variable
		char tmp1[10][50][2][15]; // array that holds database
		const char * rtrn; // separate return from send 	
	      
		int i; //ctr to parse string 
		int  j ; //ctr to build commands
		int k ; //first array index --device
		int l; //2nd array index -- command number
		int m; //3rd array index -- command name or command 
		int x1; //value of user input 
		int dsp1 = 0; //used to index a display of commands
		int dsp2 = 0; // need a secondary index
		int x2; // format command display 
		i =j =k =l =m =0;
		char * z;
        // filename to open
        FILE *file = fopen( "datafile1.csv", "r" );

        /* fopen returns 0, the NULL pointer, on failure */
        if ( file == 0 )
        {
            printf( "Could not open file\n" );
        }
      else 
        {
            
            /* read line and store it in the array . */
            while  ( ( z = fgets( tmp,100,file ) )  != 0 )
            {
				
				
				for (i = 0; i < strlen(tmp); i++) //work thru each line 
				{	
					if (tmp[0] == ':' ) // rezero array nums for each device
					{
						
						l=m=0;
						printf("new device \n");
					}	
					if (tmp[i] != ',' && tmp[i] != '\n')//read thru each piece of data
					{
						tmp1[k][l][m][j] = tmp[i];
						j++;
					}
					if (tmp[i] == ',' || tmp[i] =='\n')//complete new array element
					{
						cmds[m] =tmp1[k][l][m] ;
						printf("command %s %i %i %i  \n", cmds[m],k,l,m);
						m++;//this indexes the 2 elements from one input (csv)
						j=0;

					}
					
				    
			    }
			if (tmp[0] == ':')//change device index 
				k++;
			//final processing for while loop 
			l++; //next command number
			m=0; //reset index 	
        	}
		fclose( file );
 		}
		
	

		//get user imput via rdme and use snd1 to send the commands to the ard
		while((x1=rdme())!= 55) //55 is the magic quit 
			{
			
			if (x1 > 100) //device type
				k= x1-100;

				//display  code should list device and commands in 4 column display
				//iterates thru 2 for loops to get the info in the right place 
				for (dsp1 = 1;dsp1 <41; dsp1 +=4) //this is the master counter 
				{
					
					for(dsp2 =0;dsp2 <4;dsp2++) //this counts the items in each row
					{
						// adjusts  for 1 and 2 digit items and  long command names
						if((dsp1+dsp2) >9) x2 = 6;  
						if(strlen(tmp1[k][dsp1+dsp2][0])<x2)
							printf("%i %s\t\t",(dsp1+dsp2),tmp1[k][dsp1+dsp2][0]);
						else
							printf("%i %s\t",(dsp1+dsp2),tmp1[k][dsp1+dsp2][0]);
					}	
					printf("\n\n");
				}


			if (x1 <50) //command #
				{
				l = x1;
				//printf("numbers %i %i %i \n", k,l,m);
				cmds[0] = tmp1[k][l][0];
				cmds[1] = tmp1[k][l][1];
				printf("return %s\n",rain(cmds[1],rtrn)); //only sends hex 
				}
			}
}


//make the user input a function 
int rdme(void)
{	
//On to user input 
	//initialize array to all nulls 
	char rd[25];   //user input string 
	int r1;	
	int i;
	char a;        // holds each user in char 

	for(i=0 ;i<25; i++)
	{
		rd[i] = '\0';
	}
	i=0;
	printf("input\n");
	//could do this with scanf or fgets
	while((a = getchar()) != '\n')
	{	
		rd[i] =a;
		printf ("tmp1 %c %s %i \n",a,rd,i);
		i++;
	}
	  	strcat(rd, "\0");
		r1 = atoi(rd);
		return r1;		
}	



// need new name and to work on variable declarations 
const char * rain(const char * cmds, const char * rtrn)
{
  static int l;
  int i, n, k, /*needed for utility*/
  cport_nr=2,        /* com port 3  */
  bdrate=9600;       /* 9600 baud */
  unsigned char buf[4096];
  
  
  k = 1; /* used to terminate program -- hopefully */
  if (l !=1)
  {
  	if(RS232_OpenComport(cport_nr, bdrate))
  	{
    	printf("Can not open comport\n");
	    return(0);
 	}
	else 
		l = 1;
  }	
  while(k==1)
  {
    //send command  
     //printf("2nd_232 %s\n", cmds);
	 RS232_cputs(cport_nr, cmds);
     Sleep(100); /* sleep for 100 milliSeconds */ 

    //now ck for response 
    n = RS232_PollComport(cport_nr, buf, 4095);

    if(n > 0)
    {
      buf[n] = 0;   /* always put a "null" at the end of a string! */

      for(i=0; i < n; i++)
      {
        if(buf[i] < 32)  /* replace unreadable control-codes by dots */
        {
          buf[i] = '.';
        }
      }
      //printf("here\n");
      //printf("received %i bytes: %s\n", n, (char *)buf);
	  rtrn = (char*)buf; //I think that this is 'casting'
	  memset(buf,0,strlen(buf));
    }
	k = 0; 

   }

#ifdef _WIN32
    Sleep(100);
#else
    usleep(100000);  /* sleep for 100 milliSeconds */
#endif
return(cmds);

}

  






