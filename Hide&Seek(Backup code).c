#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

#undef main

typedef struct{
    int x,y,        //coordinate
        wx,wy,
        dx,dy,      //animation direction
        animFrame,  //animation frame
        bump,       //close hiding spot
        hidden,     //where player is hidden
        killed;     //if player is killed

    float tempx,tempy;  //temporary coordinate for collision
} Man;

typedef struct{
    int x,y,status;
    int pick,dy,frame;
} Killer;

typedef struct{
    int x,y,w,h;
} Ledge;

typedef struct{
    int x,y;
} Background;

typedef struct{
    int x,y;
} Signal;

typedef struct{
    //Player
    Man man;

    //Killer
    Killer killer;

    //ledge (collision for furniture)
    Ledge ledges[10];

    Signal snD;
    //timer
    int time,min,sec,mili,count;
    int tempSec;        //temporary sec for every 10sec

    int IHearYou;       //show i hear you image
    int powpow;         //police transition status
    int timerStatus;    //if timer turns to red

    char *timerM,        //text min
         *timerS;        //text sec

    char* timerText;    //print time

    int knockS;         //knock sound status
    int hunt,           //status for the killer if its time to search
        rmSpot,         //status if the killer will remove a hiding spot(0-4)
        signalStatus,   //show signal circle or not
        signalTemp,     //temp time for signal
        signalNumTemp,  //to avoid repeated location
        signalTime;     //temporary time

    int walkSound;      //walking sound status

    char *text;
    int textW;

    char *btext;        //button text
    int statusState,    //if on 0menu/1playing/2game over/3won
        pauseGame,      //if game is paused 1 for paused
        button,         //button status //for menu(play/instruction/exit) and pause(resume/menu/exit);
        run,            //if the killer will chase the player
        textStatus,     //inGame intro text
        temp,           //for identifying if the game just started for the intro
        tempT;


    int spot[5];        //Status of hiding spot if player can still hide in it

    int bgtime,         //temp time for menu
        bgTemptime,     //temp time for menu
        bgStatus,       //bg frame status
        bgnum,          //how many frame
        bgFrame;        //what frame to use

    int thunderStatus;
    int thunderTempTime;
    int thunderTime;    //time when will have a thunder
    int volume;
    int laughVal;       //which laugh

    int musicChannel;
    Mix_Chunk *bgMusic, *rain, *thunder, *doorOpen, *doorClose, *slash, *knock, *jumpscare, *cl1, *cl2, *cl3;
    //Images
    SDL_Texture *boo[2];           //image of killer
    SDL_Texture *hear;          //i hear you image
    SDL_Texture *manFrames[6];  //walking frame for player
    SDL_Texture *bg;            //floor
    SDL_Texture *pauseImg;      //background image for pause
    SDL_Texture *furniture[10]; //hiding spot
    SDL_Texture *label;         //text hiding spot
    SDL_Texture *buttonLabel;
    SDL_Texture *signal;        //signal circle
    SDL_Texture *gameover;      //gameover image
    SDL_Texture *bgMenu[2];        //background image of menu
    SDL_Texture *police[2];     //winning blue red filter

    SDL_Texture *chase[5];     //window hiding frames

    SDL_Texture *window[5];     //window hiding frames
    SDL_Texture *sofa[5];     //window hiding frames
    SDL_Texture *table[5];     //window hiding frames
    SDL_Texture *closet[5];     //window hiding frames
    SDL_Texture *bed[5];
    SDL_Texture *lightning;       //lightning frame
    SDL_Texture *win;       //win
    SDL_Texture *instruction;


    TTF_Font *font;
    SDL_Renderer *renderer;

} GameState;

void reset(GameState *game){
    game->statusState=0;
    game->pauseGame=0;
    game->timerText="";

    game->run=0;
    game->button=1;
    game->killer.status=0;
    game->hunt=0;
    game->rmSpot=0;

    game->count=0;
    game->min=3;
    game->sec=55;
    game->mili=0;
    game->tempSec=0;
    game->timerStatus=0;
    game->volume=64;

    game->walkSound=0;

    game->textStatus=4;
    game->tempT=0;

    game->bgStatus=1;
    game->bgTemptime=0;
    game->bgtime=100;
    game->bgnum=0;

    game->signalTime=0;
    game->signalNumTemp=-1;
    game->signalTemp=0;
    game->signalStatus=0;
    game->snD.x=0;
    game->snD.y=0;

    game->man.x = 330;
    game->man.y = 350;
    game->man.wx = 0;
    game->man.wy = 0;
    game->man.dx = 0;
    game->man.dy = 1;
    game->man.tempx=0.0;
    game->man.tempy=0.0;
    game->man.animFrame = 3;
    game->man.bump = 0;
    game->man.hidden = 0;
    game->man.killed = 0;

    game->killer.frame=0;
    game->killer.pick=-1;
    game->killer.x=330;
    game->killer.y=600;

    game->ledges[0].x=140;
    game->ledges[0].y=50;
    game->ledges[0].w=180;
    game->ledges[0].h=80;

    game->ledges[1].x=140;
    game->ledges[1].y=280;
    game->ledges[1].w=40;
    game->ledges[1].h=100;

    game->ledges[2].x=320;
    game->ledges[2].y=220;
    game->ledges[2].w=90;
    game->ledges[2].h=70;

    game->ledges[3].x=500;
    game->ledges[3].y=40;
    game->ledges[3].w=90;
    game->ledges[3].h=120;

    game->ledges[4].x=500;
    game->ledges[4].y=320;
    game->ledges[4].w=90;
    game->ledges[4].h=100;

    //images
    game->IHearYou=0;
    //if hiding spot can still be hidden
    game->spot[0]=0;
    game->spot[1]=0;
    game->spot[2]=0;
    game->spot[3]=0;
    game->spot[4]=0;

    //sound status
    game->knockS=0;
    game->laughVal=0;


    game->thunderTime=300;
    game->time = 0;
}

void process(GameState *game){
    if(game->pauseGame!=1){
        game->time++;
        if(game->tempSec==0 && game->textStatus==0){
            game->tempSec=game->time+600;
            game->hunt=0;
        }
    }
    if(game->thunderTime == game->time){
        Mix_PlayChannel(-1,game->thunder,0);
        game->thunderStatus=1;
        game->thunderTempTime=game->time+80;
        int ra=rand()%5;
        if(ra!=0){ra=ra*10*10;}else{ra=600;}
        game->thunderTime=game->time+ra;
    }

    if(game->man.wx!=0){
        if(game->time%6==0){
            if(game->man.animFrame == 0){
                game->man.animFrame = 1;
            } else {
                game->man.animFrame = 0;
            }
        }else{
            game->man.wx=0;
        }
    }
    if((game->killer.dy!=0)){
        if(game->time%8==0){
           if(game->killer.frame == 0){
                game->killer.frame = 1;
            }else{
                game->killer.frame = 0;
            }
        }
    }
    if(game->man.wy!=0){
        if(game->time%6==0){
            if(game->man.dy==1){
                if(game->man.animFrame == 2){
                    game->man.animFrame = 3;
                } else {
                    game->man.animFrame = 2;
                }
            }
            if(game->man.dy==2){
                if(game->man.animFrame == 4){
                    game->man.animFrame = 5;
                } else {
                    game->man.animFrame = 4;
                }
            }
        }else{
            game->man.wy=0;
        }
    }
    printf("===============\nCreated by: Mark Allen Cabutaje\nMembers:\nAshley Pimping\nCzarina Pielago\nKenneth Brondial\nTime: %d\nTempSec: %d\nHunt: %d\nPlayerPick: %d\nKillerPick: %d\n",
           game->time,game->tempSec,game->hunt,game->man.hidden,game->killer.pick);

    if(game->pauseGame==0 && game->textStatus==0 && game->man.hidden==0){
        if(game->mili!=0){
            game->mili--;
        }else{
            if(game->sec!=0){
                game->sec--;
                game->mili=59;
            }else{
                if(game->min!=0){
                    game->min--;
                    game->sec=60;
                    game->mili=59;
                    game->rmSpot=1;
                    game->hunt=1;
                    game->timerStatus=1;
                    game->tempSec=game->time+300;
                }else{
                    if(game->statusState==1){
                        game->IHearYou=0;
                        game->knockS=0;
                        game->statusState=3;
                        game->tempSec=game->time;
                    }
                }
            }
        }
    }


    if((!(game->sec-10<=0) && game->run==0) || game->rmSpot==1){
        if(game->signalTime==100 && game->signalStatus==0 && game->min>1){
            int ss=rand()%2;
            int ssss=rand()%5;
            game->signalTime=0;
            if(ss<=1){
                system("cls");
                game->signalNumTemp=ssss;
                game->signalStatus=1;
                while(ssss==game->signalNumTemp){
                    ssss=rand()%5;
                }
                switch(ssss){
                case 0:
                    game->snD.x=120;
                    game->snD.y=150;
                    break;
                case 1:
                    game->snD.x=550;
                    game->snD.y=200;
                    break;
                case 2:
                    game->snD.x=100;
                    game->snD.y=400;
                    break;
                case 3:
                    game->snD.x=335;
                    game->snD.y=300;
                    break;
                case 4:
                    game->snD.x=570;
                    game->snD.y=380;
                    break;
                }
            }
        }else{
            if(game->signalStatus==0){
                game->signalTime++;
            }
        }
        if(game->hunt==1 && game->run==0){
            //play knock
            if(game->tempSec >= game->time+360){
                Mix_PlayChannel(8, game->knock,0);
                game->knockS=1;
            }
            //IhearYou image
            if(game->tempSec-360 <= game->time && game->killer.pick<0){
                if(game->laughVal==0){
                    int hihi=rand()%2;
                    if(hihi==0){
                        Mix_PlayChannel(-1, game->cl1,0);
                    }else if(hihi==1){
                        Mix_PlayChannel(-1, game->cl2,0);
                    }else{
                        Mix_PlayChannel(-1, game->cl3,0);
                    }
                    game->laughVal=1;
                }
                game->IHearYou=1;
                game->knockS=0;
            }

            if(game->tempSec-198 == game->time && game->killer.pick<0){
                int r=rand()%5;
                if(r==0){r+=1;}     //random of how long the killer will walk before checking the spot
                int kPick=rand()%4;
                while(game->spot[kPick]==1){
                    kPick=rand()%4;
                }
                game->killer.pick=kPick+1;
                game->tempSec=game->tempSec+(r*10*2*2);
            }

            if(game->tempSec-100 <= game->time){
                game->laughVal=0;
                game->IHearYou=0;
                Mix_PlayChannel(10, game->doorOpen,0);
                if(game->man.hidden == 0){
                    game->run=1;
                }
            }

            if(game->tempSec-1 == game->time){
                //if killer pick the hiding spot of the player
                if(game->man.hidden==game->killer.pick){
                    SDL_Rect hideRect={50,25,630,400};
                    SDL_SetRenderDrawColor(game->renderer,10,10,10,255);
                    for(int i=1;i<5;i++){
                        SDL_RenderClear(game->renderer);
                        switch(game->man.hidden){
                        case 1:
                            SDL_RenderCopy(game->renderer,game->window[i],NULL,&hideRect);
                            break;
                        case 2:
                            SDL_RenderCopy(game->renderer,game->sofa[i],NULL,&hideRect);
                            break;
                        case 3:
                            SDL_RenderCopy(game->renderer,game->table[i],NULL,&hideRect);
                            break;
                        case 4:
                            SDL_RenderCopy(game->renderer,game->closet[i],NULL,&hideRect);
                            break;
                        case 5:
                            SDL_RenderCopy(game->renderer,game->bed[i],NULL,&hideRect);
                            break;
                        }
                        SDL_RenderPresent(game->renderer);
                        if(i!=4){SDL_Delay(100);}
                    }
                    Mix_PlayChannel(-1,game->slash,0);
                    game->statusState=2;
                    game->man.killed = 1;
                    SDL_RenderClear(game->renderer);
                    SDL_SetRenderDrawColor(game->renderer,10,10,10,255);
                    SDL_RenderPresent(game->renderer);
                    SDL_Delay(1000);
                }else{
                    Mix_PlayChannel(11, game->doorClose,0);
                    //if 1 min have passed a spot will be removed
                    if(game->rmSpot==1){
                        game->rmSpot=0;
                        game->spot[game->killer.pick-1]=1;
                    }
                    game->hunt=0;
                    game->IHearYou=0;
                    game->knockS=0;
                    game->timerStatus=0;
                    game->killer.pick=-1;
                    game->tempSec=game->time+600;
                }
            }
        }else{
            if(game->tempSec-1 <= game->time && game->sec >= 10){
                int status=rand()%5;
                if(status<=3){
                        game->hunt=1;
                        game->timerStatus=1;
                        game->tempSec=game->time+600;
                }else{
                    game->killer.pick=-1;
                    game->tempSec=game->time+200;
                }
            }
        }
    }else{
        if(game->textStatus==0 && game->statusState==1){
            game->hunt=0;
            game->IHearYou=0;
            game->timerStatus=0;
            game->knockS=0;
            game->killer.pick=-1;
            game->tempSec=game->time+100;
        }
    }

    if(game->signalTemp<51 && game->signalStatus==1){
        int sx = game->snD.x,
            sy = game->snD.y,
            sh = 20,
            sw = 60,
            mx = game->man.x,
            my = game->man.y,
            mh = 50,
            mw = 50;
        //if player is inside the circle
        if(sx+sw+30>=mx+mw && sx+sw-20<=mx+mw && sy+sh-20<=my+mh && sy+sh+10>=my+mh){
            game->signalTemp++;
        }else if(game->signalTemp>0){
            game->signalTemp--;
        }

        if(game->signalTemp == 50){
            if(game->hunt==0){
                if(game->sec >= 10){
                    game->sec -= 5;
                }
            }
            game->signalTemp=0;
            game->signalStatus=0;
        }
    }
}

void loadGame(GameState *game){
    SDL_Surface *surface=NULL;

    //Sounds
    game->bgMusic=Mix_LoadWAV("SoundEffects/bgm.wav");
    if(game->bgMusic != NULL){
        Mix_VolumeChunk(game->bgMusic,30);
    }

    game->rain=Mix_LoadWAV("SoundEffects/rain.wav");
    if(game->rain != NULL){
        Mix_VolumeChunk(game->rain,35);
    }

    game->thunder=Mix_LoadWAV("SoundEffects/thunder.wav");
    if(game->thunder != NULL){
        Mix_VolumeChunk(game->thunder,60);
    }

    game->doorOpen=Mix_LoadWAV("SoundEffects/doorOpen.wav");
    if(game->doorOpen != NULL){
        Mix_VolumeChunk(game->doorOpen,80);
    }

    game->doorClose=Mix_LoadWAV("SoundEffects/doorClose.wav");
    if(game->doorClose != NULL){
        Mix_VolumeChunk(game->doorClose,80);
    }

    game->slash=Mix_LoadWAV("SoundEffects/slash.wav");
    if(game->slash != NULL){
        Mix_VolumeChunk(game->slash,60);
    }

    game->knock=Mix_LoadWAV("SoundEffects/knock.wav");
    if(game->knock != NULL){
        Mix_VolumeChunk(game->knock,50);
    }

    game->jumpscare=Mix_LoadWAV("SoundEffects/jumpscare.wav");
    if(game->jumpscare != NULL){
        Mix_VolumeChunk(game->jumpscare,80);
    }

    game->cl1=Mix_LoadWAV("SoundEffects/cl1.wav");
    if(game->cl1 != NULL){
        Mix_VolumeChunk(game->cl1,30);
    }

    game->cl2=Mix_LoadWAV("SoundEffects/cl2.wav");
    if(game->cl2 != NULL){
        Mix_VolumeChunk(game->cl2,30);
    }

    game->cl3=Mix_LoadWAV("SoundEffects/cl3.wav");
    if(game->cl3 != NULL){
        Mix_VolumeChunk(game->cl3,30);
    }
    //Menu background image
    surface=IMG_Load("pic/m1.png");
    if(surface==NULL){
        printf("Cannot find IMG files\n\n");
        SDL_Quit();
        exit(1);
    }
    game->bgMenu[0]=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);

    surface=IMG_Load("pic/m2.png");
    if(surface==NULL){
        printf("Cannot find IMG files\n\n");
        SDL_Quit();
        exit(1);
    }
    game->bgMenu[1]=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);

    //instruction
    surface=IMG_Load("pic/instruction.png");
    if(surface==NULL){
        printf("Cannot find IMG files\n\n");
        SDL_Quit();
        exit(1);
    }
    game->instruction=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);


    //InGame background Image
    surface=IMG_Load("pic/bg.JPG");
    if(surface==NULL){
        printf("Cannot find IMG files\n\n");
        SDL_Quit();
        exit(1);
    }
    game->bg=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);

    //pause background
    surface=IMG_Load("pic/pause.png");
    if(surface==NULL){
        printf("Cannot find IMG files1.0\n\n");
        SDL_Quit();
        exit(1);
    }
    game->pauseImg=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);

    //IhearYou image
    surface=IMG_Load("pic/hear.png");
    if(surface==NULL){
        printf("Cannot find IMG files\n\n");
        SDL_Quit();
        exit(1);
    }
    game->hear=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);

    //Signal (yellow circle for -10 on time)
    surface=IMG_Load("pic/signal.png");
    if(surface==NULL){
        printf("Cannot find IMG files\n\n");
        SDL_Quit();
        exit(1);
    }
    game->signal=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);

    //game over
    surface=IMG_Load("pic/gameover.jpg");
    if(surface==NULL){
        printf("Cannot find IMG files\n\n");
        SDL_Quit();
        exit(1);
    }
    game->gameover=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);

    //win
    surface=IMG_Load("pic/win.JPG");
    if(surface==NULL){
        printf("Cannot find IMG filesW\n\n");
        SDL_Quit();
        exit(1);
    }
    game->win=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);

    //=================Load furniture image=================
    //window
    surface=IMG_Load("pic/hide1.png");
    if(surface==NULL){
        printf("Cannot find IMG files2\n\n");
        SDL_Quit();
        exit(1);
    }
    game->furniture[0]=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);

    surface=IMG_Load("pic/hide1Open.png");
    if(surface==NULL){
        printf("Cannot find IMG files2\n\n");
        SDL_Quit();
        exit(1);
    }
    game->furniture[1]=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);

    //sofa
    surface=IMG_Load("pic/hide2.png");
    if(surface==NULL){
        printf("Cannot find IMG files2\n\n");
        SDL_Quit();
        exit(1);
    }
    game->furniture[2]=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);

    surface=IMG_Load("pic/hide2Open.png");
    if(surface==NULL){
        printf("Cannot find IMG files2\n\n");
        SDL_Quit();
        exit(1);
    }
    game->furniture[3]=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);

    //table
    surface=IMG_Load("pic/hide3.png");
    if(surface==NULL){
        printf("Cannot find IMG files2\n\n");
        SDL_Quit();
        exit(1);
    }
    game->furniture[4]=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);

    surface=IMG_Load("pic/hide3Open.png");
    if(surface==NULL){
        printf("Cannot find IMG files2\n\n");
        SDL_Quit();
        exit(1);
    }
    game->furniture[5]=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);

    //closet
    surface=IMG_Load("pic/hide4.png");
    if(surface==NULL){
        printf("Cannot find IMG files2\n\n");
        SDL_Quit();
        exit(1);
    }
    game->furniture[6]=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);

    surface=IMG_Load("pic/hide4Open.png");
    if(surface==NULL){
        printf("Cannot find IMG files2\n\n");
        SDL_Quit();
        exit(1);
    }
    game->furniture[7]=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);

    //bed
    surface=IMG_Load("pic/hide5.png");
    if(surface==NULL){
        printf("Cannot find IMG files2\n\n");
        SDL_Quit();
        exit(1);
    }
    game->furniture[8]=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);

    surface=IMG_Load("pic/hide5Open.png");
    if(surface==NULL){
        printf("Cannot find IMG files2\n\n");
        SDL_Quit();
        exit(1);
    }
    game->furniture[9]=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);
    //======================================================

    //load player Frame0 and Frame1 side
    surface=IMG_Load("pic/s.png");
    if(surface==NULL){
        printf("Cannot find IMG files3\n\n");
        SDL_Quit();
        exit(1);
    }
    game->manFrames[0]=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);

    surface=IMG_Load("pic/sw.png");
    if(surface==NULL){
        printf("Cannot find IMG files4\n\n");
        SDL_Quit();
        exit(1);
    }
    game->manFrames[1]=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);

    //load image Frame2 and Frame3 back
    surface=IMG_Load("pic/b.png");
    if(surface==NULL){
        printf("Cannot find IMG files3\n\n");
        SDL_Quit();
        exit(1);
    }
    game->manFrames[2]=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);

    surface=IMG_Load("pic/bw.png");
    if(surface==NULL){
        printf("Cannot find IMG files4\n\n");
        SDL_Quit();
        exit(1);
    }
    game->manFrames[3]=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);

    //load image Frame4 and Frame5 back
    surface=IMG_Load("pic/f.png");
    if(surface==NULL){
        printf("Cannot find IMG files3\n\n");
        SDL_Quit();
        exit(1);
    }
    game->manFrames[4]=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);

    surface=IMG_Load("pic/fw.png");
    if(surface==NULL){
        printf("Cannot find IMG files4\n\n");
        SDL_Quit();
        exit(1);
    }
    game->manFrames[5]=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);
    //======================================================
    //killer
    surface=IMG_Load("pic/killer.png");
    if(surface==NULL){
        printf("Cannot find IMG files3\n\n");
        SDL_Quit();
        exit(1);
    }
    game->boo[0]=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);

    surface=IMG_Load("pic/killer2.png");
    if(surface==NULL){
        printf("Cannot find IMG files3\n\n");
        SDL_Quit();
        exit(1);
    }
    game->boo[1]=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);

    //======================================================

    surface=IMG_Load("pic/red.png");
    if(surface==NULL){
        printf("Cannot find IMG filesR\n\n");
        SDL_Quit();
        exit(1);
    }
    game->police[0]=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);

    surface=IMG_Load("pic/blue.png");
    if(surface==NULL){
        printf("Cannot find IMG filesB\n\n");
        SDL_Quit();
        exit(1);
    }
    game->police[1]=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);

    //thunder
    surface=IMG_Load("pic/thunder.png");
    if(surface==NULL){
        printf("Cannot find IMG filesT\n\n");
        SDL_Quit();
        exit(1);
    }
    game->lightning=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);

    //font
    game->font = TTF_OpenFont("fonts/Pixel Tactical.otf",48);
    if(!game->font){
        printf("Cannot load font: %s\n",TTF_GetError());
        SDL_Quit();
        exit(1);
    }

    //======================================================
    //Hiding
    //chase
    surface=IMG_Load("pic/@ch/1.JPG");
    if(surface==NULL){
        printf("Cannot find IMG filesC\n\n");
        SDL_Quit();
        exit(1);
    }
    game->chase[0]=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);

    surface=IMG_Load("pic/@ch/2.JPG");
    if(surface==NULL){
        printf("Cannot find IMG filesC\n\n");
        SDL_Quit();
        exit(1);
    }
    game->chase[1]=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);

    surface=IMG_Load("pic/@ch/3.JPG");
    if(surface==NULL){
        printf("Cannot find IMG filesC\n\n");
        SDL_Quit();
        exit(1);
    }
    game->chase[2]=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);

    surface=IMG_Load("pic/@ch/4.JPG");
    if(surface==NULL){
        printf("Cannot find IMG filesC\n\n");
        SDL_Quit();
        exit(1);
    }
    game->chase[3]=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);

    surface=IMG_Load("pic/@ch/5.JPG");
    if(surface==NULL){
        printf("Cannot find IMG filesC\n\n");
        SDL_Quit();
        exit(1);
    }
    game->chase[4]=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);


    //window
    surface=IMG_Load("pic/@w/1.png");
    if(surface==NULL){
        printf("Cannot find IMG files4\n\n");
        SDL_Quit();
        exit(1);
    }
    game->window[0]=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);

    surface=IMG_Load("pic/@w/2.png");
    if(surface==NULL){
        printf("Cannot find IMG files4\n\n");
        SDL_Quit();
        exit(1);
    }
    game->window[1]=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);

    surface=IMG_Load("pic/@w/3.png");
    if(surface==NULL){
        printf("Cannot find IMG files4\n\n");
        SDL_Quit();
        exit(1);
    }
    game->window[2]=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);

    surface=IMG_Load("pic/@w/4.png");
    if(surface==NULL){
        printf("Cannot find IMG files4\n\n");
        SDL_Quit();
        exit(1);
    }
    game->window[3]=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);

    surface=IMG_Load("pic/@w/5.png");
    if(surface==NULL){
        printf("Cannot find IMG files4\n\n");
        SDL_Quit();
        exit(1);
    }
    game->window[4]=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);

    //sofa
    surface=IMG_Load("pic/@co/1.png");
    if(surface==NULL){
        printf("Cannot find IMG filesS\n\n");
        SDL_Quit();
        exit(1);
    }
    game->sofa[0]=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);

    surface=IMG_Load("pic/@co/2.png");
    if(surface==NULL){
        printf("Cannot find IMG filesS\n\n");
        SDL_Quit();
        exit(1);
    }
    game->sofa[1]=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);

    surface=IMG_Load("pic/@co/3.png");
    if(surface==NULL){
        printf("Cannot find IMG filesS\n\n");
        SDL_Quit();
        exit(1);
    }
    game->sofa[2]=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);

    surface=IMG_Load("pic/@co/4.png");
    if(surface==NULL){
        printf("Cannot find IMG filesS\n\n");
        SDL_Quit();
        exit(1);
    }
    game->sofa[3]=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);

    surface=IMG_Load("pic/@co/5.png");
    if(surface==NULL){
        printf("Cannot find IMG filesS\n\n");
        SDL_Quit();
        exit(1);
    }
    game->sofa[4]=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);

    //table
    surface=IMG_Load("pic/@t/1.png");
    if(surface==NULL){
        printf("Cannot find IMG filesT\n\n");
        SDL_Quit();
        exit(1);
    }
    game->table[0]=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);

    surface=IMG_Load("pic/@t/2.png");
    if(surface==NULL){
        printf("Cannot find IMG filesT\n\n");
        SDL_Quit();
        exit(1);
    }
    game->table[1]=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);

    surface=IMG_Load("pic/@t/3.png");
    if(surface==NULL){
        printf("Cannot find IMG filesT\n\n");
        SDL_Quit();
        exit(1);
    }
    game->table[2]=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);

    surface=IMG_Load("pic/@t/4.png");
    if(surface==NULL){
        printf("Cannot find IMG filesT\n\n");
        SDL_Quit();
        exit(1);
    }
    game->table[3]=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);

    //closet
    surface=IMG_Load("pic/@c/1.png");
    if(surface==NULL){
        printf("Cannot find IMG filesCL\n\n");
        SDL_Quit();
        exit(1);
    }
    game->closet[0]=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);

    surface=IMG_Load("pic/@c/2.png");
    if(surface==NULL){
        printf("Cannot find IMG filesCL\n\n");
        SDL_Quit();
        exit(1);
    }
    game->closet[1]=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);

    surface=IMG_Load("pic/@c/3.png");
    if(surface==NULL){
        printf("Cannot find IMG filesCL\n\n");
        SDL_Quit();
        exit(1);
    }
    game->closet[2]=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);

    surface=IMG_Load("pic/@c/4.png");
    if(surface==NULL){
        printf("Cannot find IMG filesCL\n\n");
        SDL_Quit();
        exit(1);
    }
    game->closet[3]=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);

    surface=IMG_Load("pic/@c/5.png");
    if(surface==NULL){
        printf("Cannot find IMG filesCL\n\n");
        SDL_Quit();
        exit(1);
    }
    game->closet[4]=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);

    //bed
    surface=IMG_Load("pic/@t/1.png");
    if(surface==NULL){
        printf("Cannot find IMG filesB\n\n");
        SDL_Quit();
        exit(1);
    }
    game->table[0]=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);

    surface=IMG_Load("pic/@t/2.png");
    if(surface==NULL){
        printf("Cannot find IMG filesB\n\n");
        SDL_Quit();
        exit(1);
    }
    game->table[1]=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);

    surface=IMG_Load("pic/@t/3.png");
    if(surface==NULL){
        printf("Cannot find IMG filesB\n\n");
        SDL_Quit();
        exit(1);
    }
    game->table[2]=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);

    surface=IMG_Load("pic/@t/4.png");
    if(surface==NULL){
        printf("Cannot find IMG filesB\n\n");
        SDL_Quit();
        exit(1);
    }
    game->table[3]=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);

    //bed
    surface=IMG_Load("pic/@b/1.png");
    if(surface==NULL){
        printf("Cannot find IMG files4\n\n");
        SDL_Quit();
        exit(1);
    }
    game->bed[0]=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);

    surface=IMG_Load("pic/@b/2.png");
    if(surface==NULL){
        printf("Cannot find IMG files44\n\n");
        SDL_Quit();
        exit(1);
    }
    game->bed[1]=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);

    surface=IMG_Load("pic/@b/3.png");
    if(surface==NULL){
        printf("Cannot find IMG files44\n\n");
        SDL_Quit();
        exit(1);
    }
    game->bed[2]=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);

    surface=IMG_Load("pic/@b/4.png");
    if(surface==NULL){
        printf("Cannot find IMG files44\n\n");
        SDL_Quit();
        exit(1);
    }
    game->bed[3]=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);

    surface=IMG_Load("pic/@b/5.png");
    if(surface==NULL){
        printf("Cannot find IMG files44\n\n");
        SDL_Quit();
        exit(1);
    }

    game->bed[4]=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);
    surface=IMG_Load("pic/@t/5.png");
    if(surface==NULL){
        printf("Cannot find IMG files44\n\n");
        SDL_Quit();
        exit(1);
    }
    game->table[4]=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);



    surface=IMG_Load("pic/win.JPG");
    if(surface==NULL){
        printf("Cannot find IMG files4\n\n");
        SDL_Quit();
        exit(1);
    }
    game->win=SDL_CreateTextureFromSurface(game->renderer,surface);
    SDL_FreeSurface(surface);
}

void collisionDetect(GameState *game){
    //check for collision on every ledges
    float mw=50,mh=50;
    float mx=game->man.x,
          my=game->man.y;
    for(int i=0;i<5;i++){
        float bx=game->ledges[i].x,
              by=game->ledges[i].y,
              bw=game->ledges[i].w,
              bh=game->ledges[i].h;

        if(my+mh-10 > by && my<by+bh-30){
            //rubbing against right edge
            if(mx<bx+bw-10 && mx+mw>bx+bw-10){
                //correct x
                game->man.bump=i+1;
                if(i!=0){
                    game->man.x=bx+bw-10;
                    mx=bx+bw;
                    game->man.tempx=game->man.x;
                    game->man.tempy=game->man.y;

                }
                //rubbing against left edge
            } else if(mx+mw>bx && mx<bx){
                //correct x
                game->man.bump=i+1;
                if(i!=0){
                    game->man.x=bx-mw;
                    mx=bx-bw;
                    game->man.tempx=game->man.x;
                    game->man.tempy=game->man.y;
                }
            }
        }
        if(mx+mw>bx && mx<bx+bw-10){
            //bump top
            if(my<by+(bh-30) && my>by-10){
                //correct y
                game->man.bump=i+1;
                if(i!=0){
                    game->man.y=by+(bh-30);
                    game->man.tempx=game->man.x;
                    game->man.tempy=game->man.y;
                }

            }else if(my+mh>by-8 && my<by-8){
                //correct y
                game->man.bump=i+1;
                if(i!=0){
                    game->man.y=by-8-mh;
                    game->man.tempx=game->man.x;
                    game->man.tempy=game->man.y;
                }
            }
        }
    }

    if(game->man.hidden==0 && game->statusState==1){
        switch(game->man.bump){
            case 0:
                game->text=" ";
                game->textW=0;
                break;
            case 1:
                if(game->spot[0]==0){
                    game->text="Hide at the window?    [Press Spacebar]";
                }else{
                    game->text="I cant hide in here";
                }
                game->textW=450;
                break;

            case 2:
                if(game->spot[1]==0){
                    game->text="Hide inside the couch?      [ Press Spacebar ]";
                }else{
                    game->text="I cant hide in here";
                }
                game->textW=450;
                break;

            case 3:
                if(game->spot[2]==0){
                    game->text="Hide under the table?      [ Press Spacebar ]";
                }else{
                    game->text="I cant hide in here";
                }
                game->textW=450;
                break;

            case 4:
                if(game->spot[3]==0){
                    game->text="Hide inside the closet?     [ Press Spacebar ]";
                }else{
                    game->text="I cant hide in here";
                }
                game->textW=450;
                break;

            case 5:
                if(game->spot[4]==0){
                    game->text="Hide under the bed?      [ Press Spacebar ]";
                }else{
                    game->text="I cant hide in here";
                }
                game->textW=450;
                break;
            default:
                game->text=" ";
                game->textW=0;
            }
    }else{
        if(game->statusState!=3){
            game->text="Get out?";
            game->textW=150;
        }else{
            game->text="Congratulation You survived...    [ Press Esc ]";
            game->textW=550;
        }
    }

    if(game->man.bump!=0 && game->count<=20){
        if(game->man.tempx!=mx || game->man.tempy!=my){
            game->count++;
        }
    }else{
        game->man.bump=0;
        game->count=0;
    }
}

void inGameControl(GameState *game){
    //Mix_PlayChannel(-1, game->rain,-1);
    if((game->statusState==1 || game->statusState==3) && game->pauseGame!=1){
        const Uint8 *state = SDL_GetKeyboardState(NULL);
        if(game->textStatus==0){
            if(game->man.hidden==0){ //&& (game->min!=0 || game->sec!=0)
                if(state[SDL_SCANCODE_LEFT] && game->man.x>90){
                    game->man.x-=4;
                    game->man.wx=1;
                    game->man.dx=1;
                    game->man.dy=0;
                }else
                if(state[SDL_SCANCODE_RIGHT] && game->man.x<595){
                    game->man.x+=4;
                    game->man.wx=2;
                    game->man.dx=2;
                    game->man.dy=0;
                }else
                if(state[SDL_SCANCODE_UP] && game->man.y>100){
                    game->man.y-=4;
                    game->man.wy=1;
                    game->man.dy=1;
                }else
                if(state[SDL_SCANCODE_DOWN] && game->man.y<370){
                    game->man.y+=4;
                    game->man.wy=2;
                    game->man.dy=2;
                }
            }

            if(state[SDL_SCANCODE_SPACE] && game->statusState==1){
                if(game->man.bump!=0 && game->man.hidden==0 && game->spot[game->man.bump-1]==0){
                    game->man.hidden=game->man.bump;
                }else{
                    game->man.hidden=0;
                }
                SDL_Delay(500);
            }
            if(game->run!=0 && game->statusState!=3){
//                Mix_PlayChannel(10, game->doorOpen,0);
                int kx = game->killer.x,
                    ky = game->killer.y,
                    mx = game->man.x,
                    my = game->man.y;
                if(game->killer.status==0){
                    game->killer.x=330;
                    game->killer.y=400;
                    game->killer.status=1;
                }else{
                    if(game->killer.status==1 && (game->min!=0 || game->sec!=0)){
                        if(kx >= mx){
                            game->killer.x-=3;
                        }
                        if(kx <= mx){
                            game->killer.x+=3;
                        }
                        if(ky >= my){
                            game->killer.y-=3;
                            game->killer.dy=1;
                        }
                        if(ky <= my){
                            game->killer.y+=3;
                            game->killer.dy=1;
                        }
                    }
                }
                if(((kx+20>=mx && kx<=mx)||(kx-20<=mx && kx>=mx)) && ((ky+20>=my && ky<=my)||(ky-20<=my && ky>=my))){
                    Mix_PlayChannel(-1,game->jumpscare,0);
                    //chase animation
                    SDL_Rect hideRect={50,25,630,400};
                    SDL_SetRenderDrawColor(game->renderer,10,10,10,255);
                    for(int i=1;i<5;i++){
                        SDL_RenderClear(game->renderer);
                        SDL_RenderCopy(game->renderer,game->chase[i],NULL,&hideRect);
                        SDL_RenderPresent(game->renderer);
                        if(i!=4){SDL_Delay(100);}
                    }
                    Mix_PlayChannel(-1,game->slash,0);
                    game->statusState=2;
                    game->man.killed = 1;
                    SDL_RenderClear(game->renderer);
                    SDL_SetRenderDrawColor(game->renderer,10,10,10,255);
                    SDL_RenderPresent(game->renderer);
                    SDL_Delay(500);
                }
            }
        }
    }else{
        const Uint8 *state = SDL_GetKeyboardState(NULL);
        if(state[SDL_SCANCODE_UP]){
            if(game->button!=1){
                game->button--;
            }else{
                game->button=3;
            }
            SDL_Delay(200);
        }
        if(state[SDL_SCANCODE_DOWN]){
            if(game->button!=3){
                game->button++;
            }else{
                game->button=1;
            }
            SDL_Delay(200);
        }

        if(state[SDL_SCANCODE_SPACE]){
            switch(game->button){
                case 1:
                    game->pauseGame=0;
                    SDL_RenderClear(game->renderer);
                    game->button=1;
                    break;
                case 2:
                    game->pauseGame=0;
                    SDL_RenderClear(game->renderer);
                    game->statusState=2;
                    break;
                case 3:
                    game->pauseGame=0;
                    SDL_RenderClear(game->renderer);
                    game->statusState=0;
                    game->button=1;
                    break;
            }
            SDL_Delay(500);
        }
    }
}

int processEvents(SDL_Window *window, GameState *game){
    SDL_Event event;
    int done=0;
    while(SDL_PollEvent(&event)){
        switch(event.type){
            case SDL_WINDOWEVENT_CLOSE:
                if(window){
                    SDL_DestroyWindow(window);
                    window=NULL;
                }
                break;
            case SDL_KEYDOWN:
                switch(event.key.keysym.sym){
                    case SDLK_ESCAPE:
                        if(game->statusState==1){
                            if(game->pauseGame==0){
                                game->pauseGame=1;
                                game->button=1;
                            }else{
                                game->pauseGame=0;
                                SDL_RenderClear(game->renderer);
                                game->button=1;
                            }
                        }else if(game->statusState==0){
                            done=1;
                        }else if(game->statusState==2 || game->statusState==3 || game->statusState==4){
                            game->statusState=0;
                        }
                        break;
                }
                break;
            case SDL_QUIT://quit out of the game
                done=1;
                break;
        }
    }

    if(game->textStatus!=0 && game->statusState==1 && game->pauseGame!=1){
        if(game->tempT==0){
            game->temp=game->time;
        }
        if(game->temp==game->time && game->tempT==0){
            //set the background black
            SDL_SetRenderDrawColor(game->renderer,10,10,10,255);
            SDL_RenderClear(game->renderer);

            switch(game->textStatus){
            case 4:
                game->temp=game->time+50;
                game->text="I called the police";
                game->textW=200;
                break;
            case 3:
                game->temp=game->time+25;
                game->temp=game->time;
                game->text=" ";
                game->textW=10;
                break;
            case 2:
                game->temp=game->time+50;
                game->text="I need to hide before the killer finds me";
                game->textW=450;
                break;
            case 1:
                game->temp=game->time+50;
                game->text="Where do I hide now?";
                game->textW=450;
                break;
            }
            game->tempT=1;
        }
        SDL_Color white={255,255,255,255};
        SDL_Surface *tmp = TTF_RenderText_Blended(game->font,game->text,white);
        SDL_Texture *tmpTex = SDL_CreateTextureFromSurface(game->renderer, tmp);
        SDL_Rect textRect = {50,450, game->textW,35};
        SDL_RenderCopy(game->renderer, tmpTex, NULL, &textRect);
        SDL_FreeSurface(tmp);
        SDL_DestroyTexture(tmpTex);
        SDL_RenderPresent(game->renderer);
        if(game->temp==game->time){
            game->textStatus--;
            game->tempT=0;
        }
    }
    return done;
}

void doRenderer(SDL_Renderer *renderer, GameState *game){
    SDL_Color white={255,255,255,255};
    SDL_Color red={200,50,50,255};
    SDL_Color blue={50,50,150,255};
    if(game->statusState == 0){
        //set the background black
        SDL_SetRenderDrawColor(renderer,10,10,10,255);
        SDL_RenderClear(renderer);

        SDL_Rect menu={0,0,720,500};

        //bg menu animation
        if(game->bgStatus==1){
            SDL_RenderCopy(renderer,game->bgMenu[0],NULL,&menu);
         }else{
            SDL_RenderCopy(renderer,game->bgMenu[1],NULL,&menu);
        }
        if(game->bgTemptime==game->bgtime){
            if(game->bgStatus==1){
                game->bgStatus=0;
                game->bgtime+=3;
            }else{
                game->bgStatus=1;
                if(game->bgnum==1 || game->bgnum==2 || game->bgnum==4 || game->bgnum==5){
                    game->bgtime+=3;
                }else{
                    game->bgtime+=100;
                }
            }
            if(game->bgnum<6){game->bgnum++;}else{game->bgnum=0;}
        }else{
            if(game->bgTemptime<game->bgtime){
                    game->bgTemptime++;
            }
        }

        for(int i=0;i<3;i++){
            switch(i){
                case 0:
                    game->btext="Play";
                    break;
                case 1:
                    game->btext="Instruction";
                    break;
                case 2:
                    game->btext=" Exit ";
                    break;
            }
            if(game->button==i+1){
                SDL_Surface *bt = TTF_RenderText_Blended(game->font,game->btext,red);
                SDL_Texture *tmpbt = SDL_CreateTextureFromSurface(renderer, bt);
                SDL_Rect BtextRect = {310,(i*50)+180, 120,60};
                SDL_RenderCopy(renderer, tmpbt, NULL, &BtextRect);
                SDL_FreeSurface(bt);
                SDL_DestroyTexture(tmpbt);

            }else{
                SDL_Surface *bt = TTF_RenderText_Blended(game->font,game->btext,blue);
                SDL_Texture *tmpbt = SDL_CreateTextureFromSurface(renderer, bt);
                SDL_Rect BtextRect = {320,(i*50)+200, 100,40};
                SDL_RenderCopy(renderer, tmpbt, NULL, &BtextRect);
                SDL_FreeSurface(bt);
                SDL_DestroyTexture(tmpbt);
            }
        }
    }
    if((game->statusState == 1 || game->statusState==3) && game->textStatus == 0){
        //set the background black
        SDL_SetRenderDrawColor(renderer,10,10,10,255);
        SDL_RenderClear(renderer);

        if(game->man.hidden==0){
            //draw floor
            SDL_Rect bg={50,25,630,400};
            SDL_RenderCopy(renderer,game->bg,NULL,&bg);

            //signal yellow circle
            if(game->signalStatus==1){
                if(game->hunt!=0){
                    game->signalTemp=0;
                    game->signalStatus=0;
                }else{
                    SDL_Rect signalRect={game->snD.x,game->snD.y,60,20};
                    SDL_RenderCopy(renderer,game->signal,NULL,&signalRect);
                }
            }

            //draw furniture
            int s=0;
            for(int i=0;i<5;i++){
                SDL_Rect furnituresRect={game->ledges[i].x, game->ledges[i].y, game->ledges[i].w, game->ledges[i].h};
                if(game->spot[i]==0){
                    SDL_RenderCopy(renderer,game->furniture[s],NULL,&furnituresRect);
                }else{
                    SDL_RenderCopy(renderer,game->furniture[s+1],NULL,&furnituresRect);
                }
                s+=2;
            }


            //draw at man's position
            if(game->man.dy!=0){
                SDL_Rect rect={game->man.x,game->man.y,40,55};
                SDL_RenderCopy(renderer,game->manFrames[game->man.animFrame],NULL,&rect);
            }else if(game->man.dx!=0){
                SDL_Rect rect={game->man.x,game->man.y,35,55};
                SDL_RenderCopyEx(renderer,game->manFrames[game->man.animFrame],NULL,&rect,0,NULL,(game->man.dx==2));
            }

            //killer
            SDL_Rect rect={game->killer.x,game->killer.y,40,55};
            SDL_RenderCopy(renderer,game->boo[game->killer.frame],NULL,&rect);

            if(game->thunderStatus!=1){
                SDL_Rect hearRect={0,0,720,500};
                SDL_RenderCopy(renderer,game->lightning,NULL,&hearRect);
            }else{
                if(game->thunderTempTime-60<=game->time){
                    SDL_Rect hearRect={0,0,720,500};
                    SDL_RenderCopy(renderer,game->lightning,NULL,&hearRect);
                }else if(game->thunderTempTime==20){
                    SDL_Rect hearRect={0,0,720,500};
                    SDL_RenderCopy(renderer,game->lightning,NULL,&hearRect);
                    game->thunderTempTime=0;
                }
                if(game->thunderTempTime==0){
                    game->thunderStatus=0;
                }
            }

            if(game->IHearYou == 1){
                SDL_Rect hearRect={0,0,720,500};
                SDL_RenderCopy(renderer,game->hear,NULL,&hearRect);
            }
        }else{
            SDL_Rect hideRect={50,25,630,400};
            switch(game->man.hidden){
            case 1:
                SDL_RenderCopy(renderer,game->window[0],NULL,&hideRect);
                break;
            case 2:
                SDL_RenderCopy(renderer,game->sofa[0],NULL,&hideRect);
                break;
            case 3:
                SDL_RenderCopy(renderer,game->table[0],NULL,&hideRect);
                break;
            case 4:
                SDL_RenderCopy(renderer,game->closet[0],NULL,&hideRect);
                break;
            case 5:
                SDL_RenderCopy(renderer,game->bed[0],NULL,&hideRect);
                break;
            }
        }


        //Timer
        char m[128]="";
        sprintf(m,"0%d:%d",(int)game->min,(int)game->sec);
        if(game->timerStatus==1){
            SDL_Surface *gTimer = TTF_RenderText_Blended(game->font,m,red);
            SDL_Texture *tmpGTimer = SDL_CreateTextureFromSurface(renderer, gTimer);
            SDL_Rect textRect = {300,0,125,65};
            SDL_RenderCopy(renderer, tmpGTimer, NULL, &textRect);
            SDL_FreeSurface(gTimer);
            SDL_DestroyTexture(tmpGTimer);
        }else{
            SDL_Surface *gTimer = TTF_RenderText_Blended(game->font,m,white);
            SDL_Texture *tmpGTimer = SDL_CreateTextureFromSurface(renderer, gTimer);
            SDL_Rect textRect = {330,0,80,20};
            SDL_RenderCopy(renderer, tmpGTimer, NULL, &textRect);
            SDL_FreeSurface(gTimer);
            SDL_DestroyTexture(tmpGTimer);
        }

        if(game->textStatus==0){
            //Text
            SDL_Surface *tmp = TTF_RenderText_Blended(game->font,game->text,white);
            SDL_Texture *tmpTex = SDL_CreateTextureFromSurface(renderer, tmp);
            SDL_Rect textRect = {50,450, game->textW,25};
            SDL_RenderCopy(renderer, tmpTex, NULL, &textRect);
            SDL_FreeSurface(tmp);
            SDL_DestroyTexture(tmpTex);
        }
    }

    //if game is won
    if(game->statusState==3){
        game->textStatus=0;
        SDL_Rect powpow={50,25,630,400};
        if(game->tempSec==game->time){
            if(game->powpow!=0){
                game->powpow=0;
            }else{
                game->powpow=1;
            }
            game->tempSec=game->time+40;
        }
        SDL_RenderCopy(renderer,game->win,NULL,&powpow);
        if(game->powpow!=0){
            SDL_RenderCopy(renderer,game->police[0],NULL,&powpow);

        }else{
            SDL_RenderCopy(renderer,game->police[1],NULL,&powpow);
        }
    }

    if(game->pauseGame==1){
        SDL_Rect pauseImg={0,0,720,500};
        SDL_RenderCopy(renderer,game->pauseImg,NULL,&pauseImg);
        for(int i=0;i<3;i++){
            switch(i){
                case 0:
                    game->btext="Resume";
                    break;
                case 1:
                    game->btext="Restart";
                    break;
                case 2:
                    game->btext=" Exit ";
                    break;
            }
            if(game->button==i+1){
                SDL_Surface *bt = TTF_RenderText_Blended(game->font,game->btext,red);
                SDL_Texture *tmpbt = SDL_CreateTextureFromSurface(renderer, bt);
                SDL_Rect BtextRect = {310,(i*50)+180, 120,60};
                SDL_RenderCopy(renderer, tmpbt, NULL, &BtextRect);
                SDL_FreeSurface(bt);
                SDL_DestroyTexture(tmpbt);

            }else{
                SDL_Surface *bt = TTF_RenderText_Blended(game->font,game->btext,white);
                SDL_Texture *tmpbt = SDL_CreateTextureFromSurface(renderer, bt);
                SDL_Rect BtextRect = {320,(i*50)+200, 100,40};
                SDL_RenderCopy(renderer, tmpbt, NULL, &BtextRect);
                SDL_FreeSurface(bt);
                SDL_DestroyTexture(tmpbt);
            }
        }
    }
    SDL_RenderPresent(renderer);
}


int main(int argc, char *argv[]){
    GameState gameState;
    srand(time(NULL));
    SDL_Window *window=NULL;        //Declare a window
    SDL_Renderer *renderer=NULL;    //Declare a renderer

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);//Initialize SDL2

    window = SDL_CreateWindow("Hide&Seek",            //window title
                              SDL_WINDOWPOS_CENTERED, //initial x position
                              SDL_WINDOWPOS_CENTERED, //initial y position
                              720,                    //width
                              500,                    //hight
                              0);                     //flags

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    gameState.renderer = renderer;
    TTF_Init();

    Mix_OpenAudio(MIX_DEFAULT_FREQUENCY,MIX_DEFAULT_FORMAT,MIX_DEFAULT_CHANNELS,4096);
    //Load images and create rendering texture from them
    loadGame(&gameState);
    //reset(&gameState);
    reset(&gameState);
    //window is open: enter program loop(SDL_PollEvent)
    int done=0;
    system("cls");
    while(!done){
        if(gameState.statusState==0){ //menu
            done = processEvents(window, &gameState);
            const Uint8 *state = SDL_GetKeyboardState(NULL);

            if(gameState.musicChannel!=1){
                gameState.musicChannel=Mix_PlayChannel(1, gameState.bgMusic,-1);
                gameState.musicChannel=1;
                Mix_HaltChannel(2);
            }

            if(state[SDL_SCANCODE_UP]){
                if(gameState.button!=1){
                    gameState.button--;
                }else{
                    gameState.button=3;
                }
                SDL_Delay(300);
            }
            if(state[SDL_SCANCODE_DOWN]){
                if(gameState.button!=3){
                    gameState.button++;
                }else{
                    gameState.button=1;
                }
                SDL_Delay(300);
            }
            if(state[SDL_SCANCODE_SPACE]){
                switch(gameState.button){
                    case 1:
                        reset(&gameState);
                        Mix_HaltChannel(gameState.musicChannel);
                        Mix_PlayChannel(2, gameState.rain,-1);
                        gameState.statusState=2;
                        gameState.textStatus=0;//4
                        break;
                    case 2:
                        gameState.statusState=4;
                        break;
                    case 3:
                        done=1;
                        break;
                }
                SDL_Delay(500);
            }
            doRenderer(renderer, &gameState);//Render display
        }
        if(gameState.statusState==1 || gameState.statusState==3){//In game or //won
            Mix_HaltChannel(1);
            gameState.musicChannel=0;
            process(&gameState);
            collisionDetect(&gameState);
            inGameControl(&gameState);
            done = processEvents(window, &gameState);
            doRenderer(renderer, &gameState);//Render display
        }
        if(gameState.statusState==2){//game over
            gameState.thunderStatus=0;
            if(gameState.man.killed==1){
                SDL_Delay(500);
                done = processEvents(window, &gameState);
                SDL_Rect goRect={0,0,720,500};
                SDL_RenderCopy(gameState.renderer,gameState.gameover,NULL,&goRect);
                SDL_RenderPresent(gameState.renderer);
            }else{
                reset(&gameState);
                gameState.statusState=1;
                process(&gameState);
                collisionDetect(&gameState);
                inGameControl(&gameState);
                done = processEvents(window, &gameState);
                doRenderer(renderer, &gameState);//Render display
            }
        }
        if(gameState.statusState==4){//Instruction
            done = processEvents(window, &gameState);
            SDL_RenderClear(gameState.renderer);
            SDL_Rect hearRect={0,0,720,500};
            SDL_RenderCopy(gameState.renderer,gameState.instruction,NULL,&hearRect);
            SDL_RenderPresent(gameState.renderer);
            printf("%d",gameState.statusState);
            SDL_Delay(100);
        }
    }

    //shutdown game and unload all memory
    SDL_DestroyTexture(gameState.bg);
    SDL_DestroyTexture(gameState.pauseImg);
    SDL_DestroyTexture(gameState.boo[0]);
    SDL_DestroyTexture(gameState.boo[1]);
    SDL_DestroyTexture(gameState.manFrames[0]);
    SDL_DestroyTexture(gameState.manFrames[1]);
    SDL_DestroyTexture(gameState.furniture[0]);
    SDL_DestroyTexture(gameState.furniture[1]);
    SDL_DestroyTexture(gameState.furniture[2]);
    SDL_DestroyTexture(gameState.furniture[3]);
    SDL_DestroyTexture(gameState.furniture[4]);
    SDL_DestroyTexture(gameState.gameover);
    SDL_DestroyTexture(gameState.hear);
    SDL_DestroyTexture(gameState.label);
    SDL_DestroyTexture(gameState.buttonLabel);
    SDL_DestroyTexture(gameState.police[0]);
    SDL_DestroyTexture(gameState.police[1]);
    SDL_DestroyTexture(gameState.bgMenu[0]);
    SDL_DestroyTexture(gameState.bgMenu[1]);

    SDL_DestroyTexture(gameState.window[0]);
    SDL_DestroyTexture(gameState.window[1]);
    SDL_DestroyTexture(gameState.window[2]);
    SDL_DestroyTexture(gameState.window[3]);
    SDL_DestroyTexture(gameState.window[4]);
    SDL_DestroyTexture(gameState.window[5]);

    SDL_DestroyTexture(gameState.sofa[0]);
    SDL_DestroyTexture(gameState.sofa[1]);
    SDL_DestroyTexture(gameState.sofa[2]);
    SDL_DestroyTexture(gameState.sofa[3]);
    SDL_DestroyTexture(gameState.sofa[4]);
    SDL_DestroyTexture(gameState.sofa[5]);


    SDL_DestroyTexture(gameState.table[0]);
    SDL_DestroyTexture(gameState.table[1]);
    SDL_DestroyTexture(gameState.table[2]);
    SDL_DestroyTexture(gameState.table[3]);
    SDL_DestroyTexture(gameState.table[4]);
    SDL_DestroyTexture(gameState.table[5]);

    SDL_DestroyTexture(gameState.closet[0]);
    SDL_DestroyTexture(gameState.closet[1]);
    SDL_DestroyTexture(gameState.closet[2]);
    SDL_DestroyTexture(gameState.closet[3]);
    SDL_DestroyTexture(gameState.closet[4]);
    SDL_DestroyTexture(gameState.closet[5]);

    SDL_DestroyTexture(gameState.bed[0]);
    SDL_DestroyTexture(gameState.bed[1]);
    SDL_DestroyTexture(gameState.bed[2]);
    SDL_DestroyTexture(gameState.bed[3]);
    SDL_DestroyTexture(gameState.bed[4]);
    SDL_DestroyTexture(gameState.bed[5]);
    TTF_CloseFont(gameState.font);

    Mix_FreeChunk(gameState.bgMusic);
    Mix_FreeChunk(gameState.rain);
    Mix_FreeChunk(gameState.thunder);
    Mix_FreeChunk(gameState.doorOpen);
    Mix_FreeChunk(gameState.doorClose);
    Mix_FreeChunk(gameState.slash);
    Mix_FreeChunk(gameState.knock);
    Mix_FreeChunk(gameState.jumpscare);
    Mix_FreeChunk(gameState.cl1);
    Mix_FreeChunk(gameState.cl2);
    Mix_FreeChunk(gameState.cl3);
    //Close and destroy the window
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    //clean
    TTF_Quit();
    SDL_Quit();
    return 0;
}
