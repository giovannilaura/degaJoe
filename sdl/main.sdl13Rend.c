/*
Dega Joe SDL

features added by JOE

SDL_Joystick Two joysticks but not independent
SDL_Resize up to 6x for 8 BITS BITPLANES
SDL_Audio Now Perfect



*/


#define APPNAME "degaJoe"
#define APPNAME_LONG "DegaJoe/SDL"
#define VERSION "2.0"
#define LAST_UPDATE "02.04.10"

#define GG_WIDTH 160
#define GG_HEIGHT 144

#define SMS_WIDTH 256
#define SMS_HEIGHT 192

#define DEAD_AXIS 32767
#define DEAD_OFFSET 20000		

#define FULLSCREEN_WIDTH 1280
#define FULLSCREEN_HEIGHT 1024


#include <stdio.h>
#include <unistd.h>
#include <mast.h>
#include <SDL.h>
#include <malloc.h>
#include <getopt.h>
#include <fcntl.h>
#include <stdlib.h>
#include "gtk/gtk.h"

SDL_Surface *thescreen;
SDL_Window *theWindow;
SDL_Renderer *theRenderer;
SDL_Color themap[256];
char Name[255];
int width, height;
int  doubled=0;
int tripled=0;
int quadded=0;
int fsJoe=0;
static int audio_len=0;
int vidflags=0;
SDL_Joystick* js[2];


 
static void file_ok_sel(GtkWidget* gw,GtkFileSelection *fs )
{
    strncpy(Name, gtk_file_selection_get_filename(GTK_FILE_SELECTION(fs)), 1024);;
    gtk_widget_destroy(fs);
}

void OpenFile()       {
/* Get the selected filename and print it to the console */
	unsigned char* rom;
	int romlength;

    GtkWidget *filew;
    
    
    /* Create a new file selection widget */
    filew = gtk_file_selection_new ("File selection");
    
    g_signal_connect (G_OBJECT (filew), "destroy",
	              G_CALLBACK (gtk_main_quit), NULL);
    /* Connect the ok_button to file_ok_sel function */
    g_signal_connect (G_OBJECT (GTK_FILE_SELECTION (filew)->ok_button),
		      "clicked", G_CALLBACK (file_ok_sel), (gpointer) filew);
    
    /* Connect the cancel_button to destroy the widget */
    g_signal_connect_swapped (G_OBJECT (GTK_FILE_SELECTION (filew)->cancel_button),
	                      "clicked", G_CALLBACK (gtk_widget_destroy),
			      G_OBJECT (filew));
    
    /* Lets set the filename, as if this were a save dialog, and we are giving
     a default filename */
    if (MastEx&MX_GG) { 
			gtk_file_selection_set_filename (GTK_FILE_SELECTION(filew),"/hole/emu/consoles/dega/GameGear/Sonic.gg"); 
    }
    else { 
			gtk_file_selection_set_filename (GTK_FILE_SELECTION(filew),"/hole/emu/consoles/dega/SMS/Sonic.gg"); 
    }
				     
    
    gtk_widget_show (filew);
    gtk_main ();	
        
	 
	printf("Name %s\n",Name);
	//detect game gear rom
	if(strstr(Name,".gg") || strstr(Name,".GG"))
		MastEx |= MX_GG;
	
	width=MastEx&MX_GG?GG_WIDTH:SMS_WIDTH;
	height=MastEx&MX_GG?GG_HEIGHT:SMS_HEIGHT;

	if (doubled) {
		height=height*2;
		width=width*2;
	}
	
	if (tripled) {
		if (MastEx&MX_GG) {
			height=height*6-24;
			width=width*6;
		}
		else {
			height=height*5;
			width=width*5;
		}
	}
        if (fsJoe) {
		if (MastEx&MX_GG) {
			height=height*6-24;
			width=width*6;
		}
		else {
			height=height*5;
			width=width*5;
		}
	}
	
	MastLoadRom(Name, &rom, &romlength);
	MastSetRom(rom,romlength);
	MastHardReset();
 
}
int scrlock()
{
        if(SDL_MUSTLOCK(thescreen))
        {
                if ( SDL_LockSurface(thescreen) < 0 )
                {
                        fprintf(stderr, "Couldn't lock display surface: %s\n",
                                                                SDL_GetError());
                        return -1;
                }
        }
        return 0;
}
void scrunlock(void)
{
        if(SDL_MUSTLOCK(thescreen))
                SDL_UnlockSurface(thescreen);
        SDL_UpdateRect(thescreen, 0, 0, 0, 0);
         
}

void MsndCall(void* data, Uint8* stream, int len)
{
	if(audio_len < len)
	{
		memcpy(stream,data,audio_len);
		audio_len=0;
		//printf("resetting audio_len\n");
		return;
	}
	//printf("audio_len=%d\n",audio_len);
	memcpy(stream,data,len);
	audio_len-=len;
	memcpy(data,(unsigned char*)data+len,audio_len);
}

void usage(void)
{
	printf("\nUsage: %s [OPTION]... [ROM file]\n",APPNAME);
	printf("\nOptions:\n");
	printf("     --help\tprint what you're reading now\n");
	printf("  -v --version\tprint version no.\n");
	printf("  -g --gamegear\tforce Game Gear emulation (default autodetect)\n");
	printf("  -m --sms\tforce Master System emulation (default autodetect)\n");
	printf("  -p --pal\tuse PAL mode (default NTSC)\n");
	printf("  -s --nosound\tdisable sound\n");
	printf("  -f --fullscreen\tfullscreen display\n");
	printf("\n" APPNAME_LONG " version " VERSION " by Ulrich Hecht <uli@emulinks.de>\n");
	printf("based on Win32 version by Dave <dave@finalburn.com>\n");
	exit (0);
}
	                

int main(int argc, char** argv)
{
	unsigned char* rom;
	int romlength;
	int done=0;
 	SDL_Event event;
	int key;
	SDL_AudioSpec aspec;
	unsigned char* audiobuf;

	// options
	int framerate=50;
	int autodetect=1;
	int sound=1;
	

	//init gtk
	gtk_init(&argc, &argv);
	//open the joy
	
	
	while(1)
	{
		int option_index=0;
		int copt;
		
		static struct option long_options[] = {
			{"help",0,0,0},
			{"version",no_argument,NULL,'v'},
			{"gamegear",no_argument,NULL,'g'},
			{"sms",no_argument,NULL,'m'},
			{"pal",no_argument,NULL,'p'},
			{"nosound",no_argument,NULL,'s'},
			{"fullscreen",no_argument,NULL,'f'},
			{"doubled",no_argument,NULL,'2'},
			{"tripled",no_argument,NULL,'3'},
			{"joed",no_argument,NULL,'j'},
	
			{0,0,0,0}
		};
		
		copt=getopt_long(argc,argv,"vgmpsf234j",long_options,&option_index);
		if(copt==-1) break;
		switch(copt)
		{
			case 0:
				if(strcmp(long_options[option_index].name,"help")==0) usage();
				break;
			
			case 'v':
				printf("%s",VERSION "\n");
				exit(0);
			
			case 'g':
				autodetect=0;
				MastEx |= MX_GG;
				break;
			
			case 'm':
				autodetect=0;
				MastEx &= !MX_GG;
				break;
			
			case 'p':
				MastEx |= MX_PAL;
				framerate=50;
				break;
			
			case 's':
				sound=0;
				break;

			case 'f':
				vidflags |= SDL_FULLSCREEN;
				break;
				
			case '2' : 
				doubled=1;
				break;
			
			case '3':
				tripled=1;
				break;
                        case 'j':
				fsJoe=1;
                                vidflags |= SDL_FULLSCREEN;
				
                                 break;
			
				
			case '?':
				usage();
				break;
		}
	}
	
	if(optind==argc)
	{
		fprintf(stderr,APPNAME ": no ROM image specified.\n");
		exit(1);
	}

	//detect GG ROM
	if(strstr(argv[optind],".gg") || strstr(argv[optind],".GG"))
		MastEx |= MX_GG;
				
	atexit(SDL_Quit);

	if(sound)
		SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO | SDL_INIT_JOYSTICK);
	else
		SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);

	js[0] =SDL_JoystickOpen(0);
	js[1] =SDL_JoystickOpen(1);
	
	width=MastEx&MX_GG?GG_WIDTH:SMS_WIDTH;
	height=MastEx&MX_GG?GG_HEIGHT:SMS_HEIGHT;
	
	if (doubled) {
		height=height*2;
		width=width*2;
	}
	
	if (tripled) {
		if (MastEx&MX_GG) {
			height=height*6-64;
                        //height=height*4 - 76;

			width=width*6;
		}
		else {
			height=height*5;
			width=width*5;
		}
	}
        if (fsJoe) {
		if (MastEx&MX_GG) {
			height=FULLSCREEN_HEIGHT;
			width=FULLSCREEN_WIDTH;
		}
		else {
			height=FULLSCREEN_HEIGHT;
			width=FULLSCREEN_WIDTH;
		}
	}

 
   
    
    /* or using the default masks for the depth: */
        if (fsJoe == 1) {
            thescreen=SDL_CreateRGBSurface(SDL_HWSURFACE ,FULLSCREEN_WIDTH, FULLSCREEN_HEIGHT,8,0,0,0,0);
            theWindow=SDL_CreateWindow("joe",0,0,FULLSCREEN_WIDTH,FULLSCREEN_HEIGHT,SDL_WINDOW_FULLSCREEN | SDL_WINDOW_OPENGL);
            theRenderer=SDL_CreateRenderer(theWindow,-1,SDL_RENDERER_ACCELERATED);
            
            
        }
        
        else if (MastEx&MX_GG && fsJoe == 0) {
	   thescreen=SDL_SetVideoMode(width, height, 8, vidflags | SDL_ASYNCBLIT | SDL_HWACCEL| SDL_HWSURFACE);
        }
        else if (fsJoe == 0) {
            thescreen=SDL_SetVideoMode(width, height, 8, vidflags | SDL_ASYNCBLIT | SDL_HWACCEL| SDL_HWSURFACE);
        }
        
        
        MastInit();
	MastLoadRom(argv[optind], &rom, &romlength);
	MastSetRom(rom,romlength);
	MastHardReset();
	memset(&MastInput,0,sizeof(MastInput));
	
	if(sound)	
	{
		MsndRate=48000; MsndLen=(MsndRate+(framerate>>1))/framerate; //guess
		aspec.freq=MsndRate;
		aspec.format=AUDIO_S16;
		aspec.channels=2;
		aspec.samples=1024;//was 1024 modified by joe fix crappy sound
		audiobuf=malloc(aspec.samples*aspec.channels*2*4);
		memset(audiobuf,0,aspec.samples*aspec.channels*2);
		aspec.callback=MsndCall;
		pMsndOut=malloc(MsndLen*aspec.channels*2);
		aspec.userdata=audiobuf;
		if(SDL_OpenAudio(&aspec,NULL)) {
			fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
			sound=0;
		}
		SDL_PauseAudio(0);
		MsndInit();
	}
	else
	{
		pMsndOut=NULL;
	}

	MastDrawDo=1;
	while(!done)
	{       
		
		scrlock();
                MastFrame();
		scrunlock();
		if(sound)
		{
			SDL_LockAudio();
			memcpy(audiobuf+audio_len,pMsndOut,MsndLen*aspec.channels*2);
			audio_len+=MsndLen*aspec.channels*2;
			printf("audio_len %d\n",audio_len);
			SDL_UnlockAudio();
		}
		
                int h=0;	
                while(SDL_PollEvent(&event))
		{
                         
			switch (event.type)
                        {
			case SDL_JOYAXISMOTION:
                                
				{
					 
 				        if (event.jaxis.value < -DEAD_OFFSET  && event.jaxis.axis==0) {
						//left
      						 
						MastInput[0]|=0x04;break;
                                          
					}
					
					if (event.jaxis.value > DEAD_OFFSET && event.jaxis.axis==0) {
						//right
						 
						MastInput[0]|=0x08;break;
                                                
					}
					
					if (event.jaxis.value < -DEAD_OFFSET && event.jaxis.axis==1) {
						//up
						MastInput[0]|=0x01;break;
					}
					
					if (event.jaxis.value > DEAD_OFFSET && event.jaxis.axis==1) {
						//down
						MastInput[0]|=0x02;break;
					}
                                        

					
					if ((event.jaxis.value >= -DEAD_OFFSET) && (event.jaxis.value <= DEAD_OFFSET)) {
						if (event.jaxis.axis==1) {
        						MastInput[0]&=0xfe;
                                                	MastInput[0]&=0xfd;
							break;
						}
						if (event.jaxis.axis==0) {
                               				MastInput[0]&=0xfb;
                               				MastInput[0]&=0xf7;
							break;
						}
                                        }
				}
					
				case SDL_JOYBUTTONUP:
				if (event.jbutton.button == 0) {
					MastInput[0]&=0xef;
                                 	break;
				}
				
				if (event.jbutton.button == 1) {
					MastInput[0]&=0xdf;
					break;
				}
				if (event.jbutton.button == 9) {
					MastInput[0]&=0x7f;
					break;
				}
				
				case SDL_JOYBUTTONDOWN:
				
				if (event.jbutton.button == 0) {
					MastInput[0]|=0x10;
					break;
				}
				
				if (event.jbutton.button == 1) {
					MastInput[0]|=0x20;
					break;
				}
				if (event.jbutton.button == 9) {
					MastInput[0]|=0x80;
					break;
				}
				
		        case SDL_KEYDOWN:
                                key=event.key.keysym.sym;
                                if(key==SDLK_ESCAPE) {done=1;break;}
                                if(key==SDLK_UP) {MastInput[0]|=0x01;break;}
                                if(key==SDLK_DOWN) {MastInput[0]|=0x02;break;}
                                if(key==SDLK_LEFT) {MastInput[0]|=0x04;break;}
                                if(key==SDLK_RIGHT) {MastInput[0]|=0x08;break;}
                                if(key==SDLK_z || key==SDLK_y) {MastInput[0]|=0x10;break;}
                                if(key==SDLK_x) {MastInput[0]|=0x20;break;}
                                if(key==SDLK_c) {MastInput[0]|=0x80;break;}
                                break;
                        case SDL_KEYUP:
                                key=event.key.keysym.sym;
                                if(key==SDLK_ESCAPE) {done=1;break;}
                                if(key==SDLK_UP) {MastInput[0]&=0xfe;break;}
                                if(key==SDLK_DOWN) {MastInput[0]&=0xfd;break;}
                                if(key==SDLK_LEFT) {MastInput[0]&=0xfb;break;}
                                if(key==SDLK_RIGHT) {MastInput[0]&=0xf7;break;}
                                if(key==SDLK_z || key==SDLK_y) {MastInput[0]&=0xef;break;}
                                if(key==SDLK_x) {MastInput[0]&=0xdf;break;}
                                if(key==SDLK_c) {MastInput[0]&=0x7f;break;}
				if(key==SDLK_r) {OpenFile();break;}
                                break;
                        case SDL_QUIT:
                                done = 1;
                                break;
                        default:
					                                 
			break;
		}
					
                }
                 
		if(sound) while(audio_len>aspec.samples*aspec.channels*2*4) usleep(50);

                SDL_Texture *tex=SDL_CreateTextureFromSurface(theRenderer,thescreen);
                SDL_RenderClear(theRenderer);
                SDL_RenderCopy(theRenderer,tex,NULL,NULL);
                SDL_RenderPresent(theRenderer);
                SDL_DestroyTexture(tex);
	}
}

void MdrawCall()
{
	int i,len,yoff=0;
	//if(Mdraw.Data[0]) printf("MdrawCall called, line %d, first pixel %d\n",Mdraw.Line,Mdraw.Data[0]);
	if(Mdraw.PalChange)
	{
		Mdraw.PalChange=0;
#define p(x) Mdraw.Pal[x]
		for(i=0;i<0x100;i++)
		{
			themap[i].r=(p(i)&7)<<5;
			themap[i].g=(p(i)&56)<<2;
			themap[i].b=(p(i)&448)>>1;
		}
		SDL_SetColors(thescreen, themap, 0, 256);
	}
	
    	if(MastEx&MX_GG && !doubled && !tripled && !fsJoe) {i=64; yoff=24;}
	
	else if(MastEx&MX_GG && doubled) {i=64; yoff=24;}
	//else if(MastEx&MX_GG && tripled) {i=64; yoff=24;}
        else if(MastEx&MX_GG && tripled) {i=64; yoff=32;}
        else if(MastEx&MX_GG && fsJoe) {i=64; yoff=32;}
	
	else	 {i=16;}
    	
	if(Mdraw.Line-yoff<0 || Mdraw.Line-yoff>=height) return;
	char datajoe[2024];
	int k=0;
	for (k=0;k < 2024 ; k++) 
	{
		datajoe[k]=' ';
	}
	if (!doubled && !tripled && !quadded && !fsJoe) {
		memcpy(thescreen->pixels+(Mdraw.Line-yoff)*thescreen->pitch,Mdraw.Data+i,width);
	}
	else  if (doubled) { //doubled master system
		
		for (k=0;k < 256; k++)
			{	
				
				//build the line doubling the pixels
				if (MastEx&MX_GG) {
					datajoe[k*2]=Mdraw.Data[k+yoff-4];
					datajoe[k*2+1]=Mdraw.Data[k+yoff-4];
				}
				else {
					datajoe[k*2]=Mdraw.Data[k+i];
					datajoe[k*2+1]=Mdraw.Data[k+i];
				
				}

			}
		
		char* a=thescreen->pixels;
		a = a + (Mdraw.Line-yoff)*2*thescreen->pitch;
		memcpy(a,datajoe + i + yoff ,width);
		
		a=thescreen->pixels;
		a = a + (((Mdraw.Line-yoff)*2)+1)*thescreen->pitch;
		memcpy(a,datajoe + i + yoff ,width);
		
		//memcpy(thescreen->pixels+(Mdraw.Line-yoff)*thescreen->pitch,datajoe+ i + yoff,width);
	}
	else  if (tripled) { //tripled
		
		for (k=0;k <256; k++)
			{	
			
				//build the line doubling the pixels
				if (MastEx&MX_GG) {
					datajoe[k*5 + yoff]=Mdraw.Data[k+yoff];
					datajoe[k*5+1 + yoff]=Mdraw.Data[k+yoff];
					datajoe[k*5+2 + yoff]=Mdraw.Data[k+yoff];
					datajoe[k*5+3 + yoff]=Mdraw.Data[k+yoff];
					datajoe[k*5+4 + yoff]=Mdraw.Data[k+yoff];
					 
				}
				else {
					datajoe[k*5]=Mdraw.Data[k+i];
					datajoe[k*5+1]=Mdraw.Data[k+i];
					datajoe[k*5+2]=Mdraw.Data[k+i];
					datajoe[k*5+3]=Mdraw.Data[k+i];
					datajoe[k*5+4]=Mdraw.Data[k+i];
					
				
				}

			}
		
		char* a; 
		a=thescreen->pixels ;
		a = a + (Mdraw.Line-yoff)*5*thescreen->pitch;
		memcpy(a,datajoe  +i  + yoff ,width);
		 
		a=thescreen->pixels ;
		a = a + (((Mdraw.Line-yoff)*5)+1)*thescreen->pitch ;
		memcpy(a,datajoe  +i  + yoff ,width);
		
		a=thescreen->pixels;
		a = a + (((Mdraw.Line-yoff)*5)+2)*thescreen->pitch  ;
		memcpy(a,datajoe +i  + yoff ,width);

                a=thescreen->pixels;
		a = a + (((Mdraw.Line-yoff)*5)+3)*thescreen->pitch  ;
		memcpy(a,datajoe +i  + yoff ,width);

	        a=thescreen->pixels;
		a = a + (((Mdraw.Line-yoff)*5)+4)*thescreen->pitch  ;
		memcpy(a,datajoe +i  + yoff ,width);
 
		}
	
	
               else  if (fsJoe) { //FULL SCREEN 1280x1024 con ingrandimento GG 6 e SMS 5
	             for (k=0;k <256; k++) //Every line has 256 bytes
			{	
			
				//build the line sextupling the pixels
				if (MastEx&MX_GG) {
					datajoe[k*6 + yoff]=Mdraw.Data[k+yoff];
                                        datajoe[k*6+1 + yoff]=Mdraw.Data[k+yoff];
					datajoe[k*6+2 + yoff]=Mdraw.Data[k+yoff];
					datajoe[k*6+3 + yoff]=Mdraw.Data[k+yoff];
					datajoe[k*6+4 + yoff]=Mdraw.Data[k+yoff];
					datajoe[k*6+5 + yoff]=Mdraw.Data[k+yoff];
					  
				}
				else {
					datajoe[k*5]=Mdraw.Data[k+i];
					datajoe[k*5+1]=Mdraw.Data[k+i];
					datajoe[k*5+2]=Mdraw.Data[k+i];
					datajoe[k*5+3]=Mdraw.Data[k+i];
					datajoe[k*5+4]=Mdraw.Data[k+i];
			 		
				
				}

			}
		
		char* a; 
		 
                if (MastEx&&MX_GG) {
                   a=thescreen->pixels ;
		   a = a + (((Mdraw.Line-yoff)*6))*thescreen->pitch ;
		   memcpy(a,datajoe  +i  + yoff ,width);
		
		   a=thescreen->pixels ;
		   a = a + (((Mdraw.Line-yoff)*6)+1)*thescreen->pitch ;
		   memcpy(a,datajoe  +i  + yoff ,width);
		
		   a=thescreen->pixels;
		   a = a + (((Mdraw.Line-yoff)*6)+2)*thescreen->pitch  ;
		   memcpy(a,datajoe +i  + yoff ,width);

                   a=thescreen->pixels;
		   a = a + (((Mdraw.Line-yoff)*6)+3)*thescreen->pitch  ;
	           memcpy(a,datajoe +i  + yoff ,width);

	           a=thescreen->pixels;
		   a = a + (((Mdraw.Line-yoff)*6)+4)*thescreen->pitch  ;
		   memcpy(a,datajoe +i  + yoff ,width);
	        
                   a=thescreen->pixels;
		   a = a + (((Mdraw.Line-yoff)*6)+5)*thescreen->pitch  ;
		   memcpy(a,datajoe +i  + yoff ,width);
                   
                    
                }
                else {

                   a=thescreen->pixels ;
                   a = a + (((Mdraw.Line-yoff)*5))*thescreen->pitch ;
                   memset(a,0 ,width);
		   memcpy(a,datajoe  +i  + yoff ,width);
		
                   a=thescreen->pixels ;
                   a = a + (((Mdraw.Line-yoff)*5)+1)*thescreen->pitch ;
                   memset(a,0 ,width);
		   memcpy(a,datajoe  +i  + yoff ,width);
		
		   a=thescreen->pixels;
                   a = a + (((Mdraw.Line-yoff)*5)+2)*thescreen->pitch  ;
                   memset(a,0 ,width);
		   memcpy(a,datajoe +i  + yoff ,width);

                   a=thescreen->pixels;
		   a = a + (((Mdraw.Line-yoff)*5)+3)*thescreen->pitch  ;
	           memset(a,0 ,width);
		   memcpy(a,datajoe +i  + yoff ,width);

	           a=thescreen->pixels;
		   a = a + (((Mdraw.Line-yoff)*5)+4)*thescreen->pitch  ;
		   memset(a,0 ,width);
		   memcpy(a,datajoe +i  + yoff ,width);
 

                   //fill rest of screen with zeroes
	             	        
                }
                
 
        }
	
	
}

