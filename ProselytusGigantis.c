#include <stdio.h>
#include<time.h>
#include <stdlib.h>
#include <windows.h>
#include <dos.h>
#include <dir.h>


#define DIM 35
#define N_ROBOT 1
#define MAX_LEN 128

int SEED, pos, robot, WDoR, x_c, y_c, WD1=N_ROBOT, R1=N_ROBOT, WD2=N_ROBOT, R2=N_ROBOT;
char mappaC[DIM][DIM], mappaMod[DIM][DIM];
int posWD[2][2], rWD=0, cWD=0, posR[2][2], rR=0, cR=0; //per sapere sempre le posizioni
int primo=0;

typedef struct datiWrDg{
    int voidShieldsWD;
    int strutturaWD;
    int movimento;
    int colpiGatling;
    int forzaGatling;
    int gitGatling;
    int colpiRaggio;
    int forzaRaggio;
    int gitRaggio;
}WarDog;
WarDog datiWD[1]={
    {
        4,
        6,
        4,
        6,
        1,
        4,
        1,
        2,
        6
    },
    {
        4,
        6,
        4,
        6,
        1,
        4,
        1,
        2,
        6
    }
};


typedef struct datiRdr{
    int voidShieldsR;
    int strutturaR;
    int movimento;
    int colpiRaggi;
    int forzaRaggi;
    int gitRaggi;
    int colpiMissili;
    int forzaMissili;
    int gitMissili;
}Raider;
Raider datiR[1]={
    {
        6,
        8,
        2,
        3,
        2,
        10,
        3,
        1,
        20
    },
    {
        6,
        8,
        2,
        3,
        2,
        10,
        3,
        1,
        20
    }
};

void splashScreen(FILE *fptr){
    char read_string[MAX_LEN];

    while(fgets(read_string,sizeof(read_string),fptr) != NULL)
        printf("%s",read_string);
}

void disegna(){

    char *filename_splash = "./Titolo.txt";

    FILE *fptr_splash = NULL;
    if((fptr_splash = fopen(filename_splash,"r")) == NULL)
    {
        fprintf(stderr,"errore durante l'apertura di %s, controlla se il file e' nella directory\n",filename_splash);
        getchar();
        return 1;
    }
    splashScreen(fptr_splash);
    fclose(fptr_splash);
    printf("\n\nPremi un qualunque tasto per continuare...");
    getchar();
}

void SetColor(int ForgC){
    WORD wColor;

    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;

                       //We use csbi for the wAttributes word.
    if(GetConsoleScreenBufferInfo(hStdOut, &csbi)){
                 //Mask out all but the background attribute, and add in the forgournd     color
        wColor = (csbi.wAttributes & 0xF0) + (ForgC & 0x0F);
        SetConsoleTextAttribute(hStdOut, wColor);
    }
    return;
}

static int hash[] = {208,34,231,213,32,248,233,56,161,78,24,140,71,48,140,254,245,255,247,247,40,
                     185,248,251,245,28,124,204,204,76,36,1,107,28,234,163,202,224,245,128,167,204,
                     9,92,217,54,239,174,173,102,193,189,190,121,100,108,167,44,43,77,180,204,8,81,
                     70,223,11,38,24,254,210,210,177,32,81,195,243,125,8,169,112,32,97,53,195,13,
                     203,9,47,104,125,117,114,124,165,203,181,235,193,206,70,180,174,0,167,181,41,
                     164,30,116,127,198,245,146,87,224,149,206,57,4,192,210,65,210,129,240,178,105,
                     228,108,245,148,140,40,35,195,38,58,65,207,215,253,65,85,208,76,62,3,237,55,89,
                     232,50,217,64,244,157,199,121,252,90,17,212,203,149,152,140,187,234,177,73,174,
                     193,100,192,143,97,53,145,135,19,103,13,90,135,151,199,91,239,247,33,39,145,
                     101,120,99,3,186,86,99,41,237,203,111,79,220,135,158,42,30,154,120,67,87,167,
                     135,176,183,191,253,115,184,21,233,58,129,233,142,39,128,211,118,137,139,255,
                     114,20,218,113,154,27,127,246,250,1,8,198,250,209,92,222,173,21,88,102,219};

int noise2(int x, int y){
    int tmp = hash[(y + SEED) % 256];
    return hash[(tmp + x) % 256];
}

float lin_inter(float x, float y, float s){
    return x + s * (y-x);
}

float smooth_inter(float x, float y, float s){
    return lin_inter(x, y, s * s * (3-2*s));
}

float noise2d(float x, float y){
    int x_int = x;
    int y_int = y;
    float x_frac = x - x_int;
    float y_frac = y - y_int;
    int s = noise2(x_int, y_int);
    int t = noise2(x_int+1, y_int);
    int u = noise2(x_int, y_int+1);
    int v = noise2(x_int+1, y_int+1);
    float low = smooth_inter(s, t, x_frac);
    float high = smooth_inter(u, v, x_frac);
    return smooth_inter(low, high, y_frac);
}

float perlin2d(float x, float y, float freq, int depth){
    float xa = x*freq;
    float ya = y*freq;
    float amp = 1.0;
    float fin = 0;
    float div = 0.0;

    int i;
    for(i=0; i<depth; i++)
    {
        div += 256 * amp;
        fin += noise2d(xa, ya) * amp;
        amp /= 2;
        xa *= 2;
        ya *= 2;
    }

    return fin/div;
}

void mappaGen(){
    int x, y, ancora=1;
    int mappaI[DIM][DIM];

    FILE *fptr;

    do{
        srand(time(0));
        SEED=rand();
        printf("seed:\t%d\n",SEED);

        for(y=0; y<DIM; y++){
            for(x=0; x<DIM; x++){
                mappaI[y][x]=(int)100*perlin2d(x, y, 0.1, 4);
            }
        }

        for(y=0; y<DIM; y++){
            for(x=0; x<DIM; x++){
                if(mappaI[y][x]<30)
                    mappaC[y][x]='~';
                if(mappaI[y][x]>=30&&mappaI[y][x]<60)
                    mappaC[y][x]='-';
                if(mappaI[y][x]>=60&&mappaI[y][x]<75)
                    mappaC[y][x]='+';
                if(mappaI[y][x]>=75&&mappaI[y][x]<90)
                    mappaC[y][x]='*';
                if(mappaI[y][x]>=90)
                    mappaC[y][x]='%';
                mappaMod[y][x]=mappaC[y][x];
            }
        }

        for(y=0; y<DIM; y++){
            for(x=0; x<DIM; x++){
                printf("%c ",mappaC[y][x]);
            }
            printf("\n");
        }

        printf("\n\n\nAncora: (1 == SI', qualunque altro intero = NO)\t");
        scanf("%d",&ancora);
    }while(ancora==1);

    fptr = fopen("mappaC.txt","w");

    for(y=0;y<DIM;y++){
        for(x=0;x<DIM;x++){
               fprintf(fptr,"%c",mappaC[y][x]);
        }
        fprintf(fptr,"\n");
    }
    fclose(fptr);
}

int chiPerPrimo(){

    int d6;

    printf("Decidiamo il primo giocatore:\n");
    srand( time( NULL ));
    d6=1+rand() % 100 ;
    if(d6%2==0){
        printf("Giocatore 2 va per primo\n");
        getchar();
        primo=2;
        return 2;
    }
    if(d6%2!=0){
        printf("Giocatore 1 va per primo\n");
        getchar();
        primo=1;
        return 1;
    }


}

void posizionaRobot(int ym, int yM, int g){

    int x, y;

    printf("\nGiocatore %d, inserisci i tuoi robot\n",g);
    if(WDoR==0){//inserisci wardog
        do{
            printMappa();
            printf("\nWarDog disponibili %d\n",robot);
            printf("x:\t");
            scanf("%d",&x);
            x-=1;
            while(y<ym||y>yM){
                printf("y (tra %d e %d):\t",ym+1,yM+1);
                scanf("%d",&y);
                y-=1;
            }
            robot--;
            system("cls");
            mappaMod[y][x]='W';

            if(g==primo){
                posWD[0][0]=y;
                posWD[0][1]=x;
            }
            else{
                posWD[1][0]=y;//qualcosa non va
                posWD[1][1]=x;
            }

            y=-1;
        }while(robot>0);
    }
    else{//inserisci raider (se nuovi robot basta trasformare in if(WDoR==1,....)
        do{
            printMappa();
            printf("\nRaider disponibili %d\n",robot);
            printf("x:\t");
            scanf("%d",&x);
            x-=1;
            while(y<ym||y>yM){
                printf("y (tra %d e %d):\t",ym+1,yM+1);
                scanf("%d",&y);
                y-=1;
            }
            robot--;
            system("cls");
            mappaMod[y][x]='R';
            if(g==primo){
                posR[0][0]=y;
                posR[0][1]=x;
            }
            else{
                posR[1][0]=y;
                posR[1][1]=x;
            }
            y=-1;
        }while(robot>0);
    }
}

void printMappa(){

    int y, x;
    printf("   | 1| 2| 3| 4| 5| 6| 7| 8| 9|10| 1| 2| 3| 4| 5| 6| 7| 8| 9|20| 1| 2| 3| 4| 5| 6| 7| 8| 9|30| 1| 2| 3| 4| 5|\n");//coordinate x

    for(y=0;y<(2*DIM+38);y++){
        printf("_");
    }

    printf("\n");

    for(y=0; y<DIM; y++){
        printf("%2d|",y+1); //coordinate y
        for(x=0; x<DIM; x++){
            if(mappaMod[y][x]=='~'){
                SetColor(9);
            }
            if(mappaMod[y][x]=='-'){
                SetColor(2);
            }
            if(mappaMod[y][x]=='*'){
                SetColor(7);
            }
            if(mappaMod[y][x]=='+'){
                SetColor(8);
            }
            if(mappaMod[y][x]=='%'){
                SetColor(15);
            }
            if(mappaMod[y][x]=='W'){
                SetColor(12);
            }
            if(mappaMod[y][x]=='R'){
                SetColor(13);
            }
            printf("%3c",mappaMod[y][x]);
        }
        SetColor(15);
        printf("|\n");
    }

    for(y=0;y<(2*DIM+38);y++){
        printf("_");
    }

    printf("\n");
}

void cercaRobot(int g){

    int x, y;

    if(g==1&&WDoR==0){
        for(y=DIM-1;y>=0;y--){// -!-
            for(x=0;x<DIM;x++){
                if(mappaMod[y][x]=='W'){
                    x_c=x;
                    y_c=y;
                    //printf("%d||%d",x,y);
                }
            }
        }
    }
    if(g==1&&WDoR==1){
        for(y=DIM-1;y>=0;y--){// -!-
            for(x=0;x<DIM;x++){
                if(mappaMod[y][x]=='R'){
                    x_c=x;
                    y_c=y;
                }
            }
        }
    }
    if(g==2&&WDoR==0){
        for(y=0;y<DIM;y++){// -!-
            for(x=0;x<DIM;x++){
                if(mappaMod[y][x]=='W'){
                    x_c=x;
                    y_c=y;
                }
            }
        }
    }
    if(g==2&&WDoR==1){
        for(y=0;y<DIM;y++){// -!-
            for(x=0;x<DIM;x++){
                if(mappaMod[y][x]=='R'){
                    x_c=x;
                    y_c=y;
                }
            }
        }
    }

}

void muoviRobot(int mov, int g){

    int x, y, direzione, diagDirezione, movRimanente, boost,movEff;

    movRimanente=mov;

    if(mov==4){//wardog
        do{
            printf("Ti rimangono %d caselle di movimento.\Specifica la direzione\n\t1-su\n\t2-giu'\n\t3-destra\n\t4-sinistra\n\t5-diagonale (non ancora)\nDirezione:\t",movRimanente);
            scanf("%d",&direzione);
            if(direzione==5){
                printf("\t1-diagonale destra su\n\t2-diagonale destra giu'\n\t3-diagonale sinistra giu'\n\t4-diagonale sinistra su\nDirezione:\t");
                scanf("%d",&diagDirezione);
            }
            else{
                printf("Di quanto:\t");
                scanf("%d",&movEff);
                if(movEff<=0){//se non vuole muoversi
                    return;
                }
                else{
                    movRimanente-=movEff;
                }

                if(g==primo){
                    y=posWD[0][0];
                    x=posWD[0][1];
                    printf("\t\ty:%d x:%d\n",y,x);
                }
                if(g!=primo){
                    y=posWD[1][0];
                    x=posWD[1][1];
                    printf("\t\ty:%d x:%d\n",y,x);
                    getchar();
                }


                if(direzione==1){
                    mappaMod[y-movEff][x]='W';
                    mappaMod[y][x]=mappaC[y][x];
                    y=x-movEff;
                }
                if(direzione==2){
                    mappaMod[y+movEff][x]='W';
                    mappaMod[y][x]=mappaC[y][x];
                    y=y+movEff;
                }
                if(direzione==3){
                    mappaMod[y][x+movEff]='W';
                    mappaMod[y][x]=mappaC[y][x];
                    x=x+movEff;
                }
                if(direzione==4){
                    mappaMod[y][x-movEff]='W';
                    mappaMod[y][x]=mappaC[y][x];
                    x=x-movEff;
                }

                if(g==primo){
                    posWD[0][0]=y;
                    posWD[0][1]=x;
                    printf("\t\tNuova y:%d Nuova x:%d\n",y,x);
                    getchar();
                }
                else{
                    posWD[1][0]=y;
                    posWD[1][1]=x;
                    printf("\t\tNuova y:%d Nuova x:%d\n\n",y,x);
                    getchar();
                }
                system("cls");
                printMappa();
                getchar();
            }
        }while(movRimanente>0);
        return;
    }
    if(mov==2){//raider
        do{
            printf("Ti rimangono %d caselle di movimento.\Specifica la direzione\n\t1-su\n\t2-giu'\n\t3-destra\n\t4-sinistra\n\t5-diagonale (no)\nDirezione:\t",movRimanente);
            scanf("%d",&direzione);
            if(direzione==5){
                printf("\t1-diagonale destra su\n\t2-diagonale destra giu'\n\t3-diagonale sinistra giu'\n\t4-diagonale sinistra su\nDirezione:\t");
                scanf("%d",&diagDirezione);
            }
            else{
                printf("Di quanto:\t");
                scanf("%d",&movEff);
                if(movEff<=0){//se non vuole muoversi
                    return;
                }
                else{
                    movRimanente-=movEff;
                }

                if(g==primo){
                    y=posR[0][0];
                    x=posR[0][1];
                    printf("\t\ty:%d x:%d\n",y,x);
                    getchar();
                }
                if(g!=primo){
                    y=posR[1][0];
                    x=posR[1][1];
                    printf("\t\ty:%d x:%d\n\n",y,x);
                    getchar();
                }

                if(direzione==1){
                    mappaMod[y-movEff][x]='R';
                    mappaMod[y][x]=mappaC[y][x];
                    y=y-movEff;
                }
                if(direzione==2){
                    mappaMod[y+movEff][x]='R';
                    mappaMod[y][x]=mappaC[y][x];
                    y=y+movEff;
                }
                if(direzione==3){
                    mappaMod[y][x+movEff]='R';
                    mappaMod[y][x]=mappaC[y][x];
                    x=x+movEff;
                }
                if(direzione==4){
                    mappaMod[y][x-movEff]='R';
                    mappaMod[y][x]=mappaC[y][x];
                    x=x-movEff;
                }

                if(g==primo){
                    posR[0][0]=y;
                    posR[0][1]=x;
                    printf("\t\tNuova y:%d Nuova x:%d\n",y,x);
                    getchar();
                }
                else{
                    posR[1][0]=y;
                    posR[1][1]=x;
                    printf("\t\tNuova y:%d Nuova x:%d\n",y,x);
                    getchar();
                }
                system("cls");
                printMappa();
                getchar();
            }
        }while(movRimanente>0);
        return;
    }

}

void faseMovimento(int mov, int g){

    int scelta;

    if(mov==4){
        printf("Giocatore %d, vuoi muovere i tuoi WarDog? 0-no, 1-si'\n\t",g);
        scanf("%d",&scelta);
        if(scelta==1){
            muoviRobot(mov,g);
        }
        else{
            return;
        }
    }
    if(mov==2){
        printf("Giocatore %d, vuoi muovere i tuoi Raider? 0-no, 1-si'\n\t",g);
        scanf("%d",&scelta);
        if(scelta==1){
            muoviRobot(mov,g);
        }
        else{
            return;
        }
    }

}

void faseSparo(int mov, int g){

    int scelta;

    if(mov==4){
        if(g==primo){
            printf("Giocatore %d, vuoi sparare con i tuoi WarDog? 0-no, 1-si'\n\t",g);
            scanf("%d",&scelta);
            if(scelta==1){
                sparaWD(mov,g);
            }
            else{
                return;
            }
        }
        else{
            printf("Giocatore %d, vuoi sparare con i tuoi WarDog? 0-no, 1-si'\n\t",g);
            scanf("%d",&scelta);
            if(scelta==1){
                sparaWD(mov,g);
            }
            else{
                return;
            }
        }
    }
    if(mov==2){
        if(g==primo){
            printf("Giocatore %d, vuoi sparare con i tuoi Raider? 0-no, 1-si'\n\t",g);
            scanf("%d",&scelta);
            if(scelta==1){
                sparaR(mov,g);
            }
            else{
                return;
            }
        }
        else{
            printf("Giocatore %d, vuoi sparare con i tuoi Raider? 0-no, 1-si'\n\t",g);
            scanf("%d",&scelta);
            if(scelta==1){
                sparaR(mov,g);
            }
            else{
                return;
            }
        }
    }

}

void sparaWD(int mov, int g){

    int x, y, xb, yb, sceltaG, sceltaC, colpiG, colpiC, chi, dist1, dist2, trueWD, trueR, i,j,decisione,f;
    chi=g-1;
    colpiG=datiWD[chi].colpiGatling;
    colpiC=datiWD[chi].colpiRaggio;
    f=datiWD[chi].forzaRaggio;

    if(g==primo){//controllo se wardog a gittata gatling, quindi tutti
        if(posWD[0][0]==posWD[1][0]){
            if(posWD[0][1]<posWD[1][1]){
                dist1=posWD[1][1]-posWD[0][1];
                if(dist1>datiWD[chi].gitGatling){
                    trueWD=0;
                }
                else{
                    trueWD=1;
                }
            }
            else{
                dist1=posWD[0][1]-posWD[1][1];
                if(dist1>datiWD[chi].gitGatling){
                    trueWD=0;
                }
                else{
                    trueWD=1;
                }
            }
        }
        if(posWD[0][1]==posWD[1][1]){
            if(posWD[0][0]<posWD[1][0]){
                dist1=posWD[1][0]-posWD[0][0];
                if(dist1>datiWD[chi].gitGatling){
                    trueWD=0;
                }
                else{
                    trueWD=1;
                }
            }
            else{
                dist1=posWD[0][0]-posWD[1][0];
                if(dist1>datiWD[chi].gitGatling){
                    trueWD=0;
                }
                else{
                    trueWD=1;
                }
            }
        }
        if(posWD[0][1]!=posWD[1][1]&&posWD[0][0]!=posWD[1][0]){
            //quadrato 1
            for(i=posWD[0][0]-datiWD[chi].gitGatling;i<posWD[0][0];i++){
                for(j=posWD[0][1]-datiWD[chi].gitGatling;j<posWD[0][1];j++){
                    if(i!=posWD[0][0]&&j!=posWD[0][1]&&mappaMod[i][j]=='W'){
                        trueWD=1;
                    }
                }
            }
            //quadrato 2
            for(i=posWD[0][0]-datiWD[chi].gitGatling;i<posWD[0][0];i++){
                for(j=posWD[0][1];j<posWD[0][1]+datiWD[chi].gitGatling;j++){
                    if(i!=posWD[0][0]&&j!=posWD[0][1]&&mappaMod[i][j]=='W'){
                        trueWD=1;
                    }
                }
            }
            //quadrato 3
            for(i=posWD[0][0];i<posWD[0][0]+datiWD[chi].gitGatling;i++){
                for(j=posWD[0][1]-datiWD[chi].gitGatling;j<posWD[0][1];j++){
                    if(i!=posWD[0][0]&&j!=posWD[0][1]&&mappaMod[i][j]=='W'){
                        trueWD=1;
                    }
                }
            }
            //quadrato 4
            for(i=posWD[0][0]-datiWD[chi].gitGatling;i<posWD[0][0];i++){
                for(j=posWD[0][1];j<posWD[0][1]+datiWD[chi].gitGatling;j++){
                    if(i!=posWD[0][0]&&j!=posWD[0][1]&&mappaMod[i][j]=='W'){
                        trueWD=1;
                    }
                }
            }
        }
        //controllo se raider a gittata raggi
        if(posWD[0][0]==posR[1][0]){
            if(posWD[0][1]<posR[1][1]){
                dist1=posR[1][1]-posWD[0][1];
                if(dist1>datiWD[chi].gitGatling){
                    trueR=0;
                }
                else{
                    trueR=1;
                }
            }
            else{
                dist1=posWD[0][1]-posR[1][1];
                if(dist1>datiWD[chi].gitGatling){
                    trueR=0;
                }
                else{
                    trueR=1;
                }
            }
        }
        if(posWD[0][1]==posR[1][1]){
            if(posWD[0][0]<posR[1][0]){
                dist1=posR[1][0]-posWD[0][0];
                if(dist1>datiWD[chi].gitGatling){
                    trueR=0;
                }
                else{
                    trueR=1;
                }
            }
            else{
                dist1=posWD[0][0]-posR[1][0];
                if(dist1>datiWD[chi].gitGatling){
                    trueR=0;
                }
                else{
                    trueR=1;
                }
            }
        }
        if(posWD[0][1]!=posR[1][1]&&posWD[0][0]!=posR[1][0]){
            //quadrato 1
            for(i=posWD[0][0]-datiWD[chi].gitGatling;i<posWD[0][0];i++){
                for(j=posWD[0][1]-datiWD[chi].gitGatling;j<posWD[0][1];j++){
                    if(i!=posR[0][0]&&j!=posR[0][1]&&mappaMod[i][j]=='R'){
                        trueR=1;
                    }
                }
            }
            //quadrato 2
            for(i=posWD[0][0]-datiWD[chi].gitGatling;i<posWD[0][0];i++){
                for(j=posWD[0][1];j<posWD[0][1]+datiWD[chi].gitGatling;j++){
                    if(i!=posR[0][0]&&j!=posR[0][1]&&mappaMod[i][j]=='R'){
                        trueR=1;
                    }
                }
            }
            //quadrato 3
            for(i=posWD[0][0];i<posWD[0][0]+datiWD[chi].gitGatling;i++){
                for(j=posWD[0][1]-datiWD[chi].gitGatling;j<posWD[0][1];j++){
                    if(i!=posR[0][0]&&j!=posR[0][1]&&mappaMod[i][j]=='R'){
                        trueR=1;
                    }
                }
            }
            //quadrato 4
            for(i=posWD[0][0]-datiWD[chi].gitGatling;i<posWD[0][0];i++){
                for(j=posWD[0][1];j<posWD[0][1]+datiWD[chi].gitGatling;j++){
                    if(i!=posR[0][0]&&j!=posR[0][1]&&mappaMod[i][j]=='R'){
                        trueR=1;
                    }
                }
            }
        }
    }
    else{//controllo se wardog a gittata gatling, quindi tutti
        if(posWD[1][0]==posWD[0][0]){
            if(posWD[1][1]<posWD[0][1]){
                dist1=posWD[0][1]-posWD[1][1];
                if(dist1>datiWD[chi].gitGatling){
                    trueWD=0;
                }
                else{
                    trueWD=1;
                }
            }
            else{
                dist1=posWD[1][1]-posWD[0][1];
                if(dist1>datiWD[chi].gitGatling){
                    trueWD=0;
                }
                else{
                    trueWD=1;
                }
            }
        }
        if(posWD[1][1]==posWD[0][1]){
            if(posWD[1][0]<posWD[0][0]){
                dist1=posWD[0][0]-posWD[1][0];
                if(dist1>datiWD[chi].gitGatling){
                    trueWD=0;
                }
                else{
                    trueWD=1;
                }
            }
            else{
                dist1=posWD[1][0]-posWD[0][0];
                if(dist1>datiWD[chi].gitGatling){
                    trueWD=0;
                }
                else{
                    trueWD=1;
                }
            }
        }
        if(posWD[0][1]!=posWD[1][1]&&posWD[0][0]!=posWD[1][0]){
            //quadrato 1
            for(i=posWD[1][0]-datiWD[chi].gitGatling;i<posWD[1][0];i++){
                for(j=posWD[1][1]-datiWD[chi].gitGatling;j<posWD[1][1];j++){
                    if(i!=posWD[1][0]&&j!=posWD[1][1]&&mappaMod[i][j]=='W'){
                        trueWD=1;
                    }
                }
            }
            //quadrato 2
            for(i=posWD[1][0]-datiWD[chi].gitGatling;i<posWD[1][0];i++){
                for(j=posWD[1][1];j<posWD[1][1]+datiWD[chi].gitGatling;j++){
                    if(i!=posWD[1][0]&&j!=posWD[1][1]&&mappaMod[i][j]=='W'){
                        trueWD=1;
                    }
                }
            }
            //quadrato 3
            for(i=posWD[1][0];i<posWD[1][0]+datiWD[chi].gitGatling;i++){
                for(j=posWD[1][1]-datiWD[chi].gitGatling;j<posWD[1][1];j++){
                    if(i!=posWD[1][0]&&j!=posWD[1][1]&&mappaMod[i][j]=='W'){
                        trueWD=1;
                    }
                }
            }
            //quadrato 4
            for(i=posWD[1][0]-datiWD[chi].gitGatling;i<posWD[1][0];i++){
                for(j=posWD[1][1];j<posWD[1][1]+datiWD[chi].gitGatling;j++){
                    if(i!=posWD[1][0]&&j!=posWD[1][1]&&mappaMod[i][j]=='W'){
                        trueWD=1;
                    }
                }
            }
        }
        //controllo se raider a gittata
        if(posWD[1][0]==posR[0][1]){
            if(posWD[1][1]<posR[0][1]){
                dist1=posR[0][1]-posWD[1][1];
                if(dist1>datiWD[chi].gitGatling){
                    trueR=0;
                }
                else{
                    trueR=1;
                }
            }
            else{
                dist1=posWD[1][1]-posR[0][1];
                if(dist1>datiWD[chi].gitGatling){
                    trueR=0;
                }
                else{
                    trueR=1;
                }
            }
        }
        if(posWD[1][1]==posR[0][1]){
            if(posWD[1][0]<posR[0][0]){
                dist1=posR[0][0]-posWD[1][0];
                if(dist1>datiWD[chi].gitGatling){
                    trueR=0;
                }
                else{
                    trueR=1;
                }
            }
            else{
                dist1=posWD[1][0]-posR[0][0];
                if(dist1>datiWD[chi].gitGatling){
                    trueR=0;
                }
                else{
                    trueR=1;
                }
            }
        }
        if(posWD[0][1]!=posR[1][1]&&posWD[0][0]!=posR[1][0]){
            //quadrato 1
            for(i=posWD[1][0]-datiWD[chi].gitGatling;i<posWD[1][0];i++){
                for(j=posWD[1][1]-datiWD[chi].gitGatling;j<posWD[1][1];j++){
                    if(i!=posR[1][0]&&j!=posR[1][1]&&mappaMod[i][j]=='R'){
                        trueR=1;
                    }
                }
            }
            //quadrato 2
            for(i=posWD[1][0]-datiWD[chi].gitGatling;i<posWD[1][0];i++){
                for(j=posWD[1][1];j<posWD[1][1]+datiWD[chi].gitGatling;j++){
                    if(i!=posR[1][0]&&j!=posR[1][1]&&mappaMod[i][j]=='R'){
                        trueR=1;
                    }
                }
            }
            //quadrato 3
            for(i=posWD[1][0];i<posWD[1][0]+datiWD[chi].gitGatling;i++){
                for(j=posWD[1][1]-datiWD[chi].gitGatling;j<posWD[1][1];j++){
                    if(i!=posR[1][0]&&j!=posR[1][1]&&mappaMod[i][j]=='R'){
                        trueR=1;
                    }
                }
            }
            //quadrato 4
            for(i=posWD[1][0]-datiWD[chi].gitGatling;i<posWD[1][0];i++){
                for(j=posWD[1][1];j<posWD[1][1]+datiWD[chi].gitGatling;j++){
                    if(i!=posR[1][0]&&j!=posR[1][1]&&mappaMod[i][j]=='R'){
                        trueR=1;
                    }
                }
            }
        }
    }

    if(trueWD){
        decisione=1;
        printf("WarDog nemico a gittata\n");
    }
    if(trueR){
        decisione=2;
        printf("Raider nemico a gittata\n");
    }
    if(trueWD&&trueR){
        printf("A chi vuoi sparare?\n\t1-WarDog\n\t2-Raider\n");
        scanf("%d",&decisione);
    }
    printf("Giocatore %d, vuoi sparare con la gatling?\n\t0-no\n\t1-si'\n%d-%d\n",g,chi+1,chi-1);
    scanf("%d",&sceltaG);
    printf("Giocatore %d, i vuoi sparare con il cannone?\n\t0-no\n\t1-si'\n%d-%d\n'",g,chi+1,chi-1);
    scanf("%d",&sceltaC);
    if(sceltaG==1){
        if(g==primo&&primo==1){
            if(decisione==1){
                if(datiWD[chi+1].voidShieldsWD>=0)
                    datiWD[chi+1].voidShieldsWD-colpiG;
                else
                    datiWD[chi+1].strutturaWD-colpiG;
            }
            else{
                if(datiR[chi+1].voidShieldsR>=0)
                    datiR[chi+1].voidShieldsR-colpiG;
                else
                    datiR[chi+1].strutturaR-colpiG;
            }
        }
        if(g==primo&&primo==2){
            if(decisione==1){
                if(datiWD[chi].voidShieldsWD>=0)
                    datiWD[chi].voidShieldsWD-colpiG;
                else
                    datiWD[chi].strutturaWD-colpiG;
            }
            else{
                if(datiR[chi].voidShieldsR>=0)
                    datiR[chi].voidShieldsR-colpiG;
                else
                    datiR[chi].strutturaR-colpiG;
            }
        }
        if(g!=primo&&primo==1){
            if(decisione==1){
                if(datiWD[chi-1].voidShieldsWD>=0)
                    datiWD[chi-1].voidShieldsWD-colpiG;
                else
                    datiWD[chi-1].strutturaWD-colpiG;
            }
            else{
                if(datiR[chi-1].voidShieldsR>=0)
                    datiR[chi-1].voidShieldsR-colpiG;
                else
                    datiR[chi-1].strutturaR-colpiG;
            }
        }
        if(g!=primo&&primo==2){
            if(decisione==1){
                if(datiWD[chi].voidShieldsWD>=0)
                    datiWD[chi].voidShieldsWD-colpiG;
                else
                    datiWD[chi].strutturaWD-colpiG;
            }
            else{
                if(datiR[chi].voidShieldsR>=0)
                    datiR[chi].voidShieldsR-colpiG;
                else
                    datiR[chi].strutturaR-colpiG;
            }
        }
    }
    if(sceltaC==1){
        if(g==primo&&primo==1){
            if(decisione==1){
                if(datiWD[chi+1].voidShieldsWD>=0)
                    datiWD[chi+1].voidShieldsWD-colpiC*f;
                else
                    datiWD[chi+1].strutturaWD-colpiC*f;
            }
            else{
                if(datiR[chi+1].voidShieldsR>=0)
                    datiR[chi+1].voidShieldsR-colpiC*f;
                else
                    datiR[chi+1].strutturaR-colpiC*f;
            }
        }
        if(g==primo&&primo==2){
            if(decisione==1){
                if(datiWD[chi].voidShieldsWD>=0)
                    datiWD[chi].voidShieldsWD-colpiC*f;
                else
                    datiWD[chi].strutturaWD-colpiC*f;
            }
            else{
                if(datiR[chi].voidShieldsR>=0)
                    datiR[chi].voidShieldsR-colpiC*f;
                else
                    datiR[chi].strutturaR-colpiC*f;
            }
        }
        if(g!=primo&&primo==1){
            if(decisione==1){
                if(datiWD[chi-1].voidShieldsWD>=0)
                    datiWD[chi-1].voidShieldsWD-colpiC*f;
                else
                    datiWD[chi-1].strutturaWD-colpiC*f;
            }
            else{
                if(datiR[chi-1].voidShieldsR>=0)
                    datiR[chi-1].voidShieldsR-colpiC*f;
                else
                    datiR[chi-1].strutturaR-colpiC*f;
            }
        }
        if(g!=primo&&primo==2){
            if(decisione==1){
                if(datiWD[chi].voidShieldsWD>=0)
                    datiWD[chi].voidShieldsWD-colpiC*f;
                else
                    datiWD[chi].strutturaWD-colpiC*f;
            }
            else{
                if(datiR[chi].voidShieldsR>=0)
                    datiR[chi].voidShieldsR-colpiC*f;
                else
                    datiR[chi].strutturaR-colpiC*f;
            }
        }
    }
}

void sparaR(int mov, int g){

    int x, y, xb, yb, sceltaM, sceltaC, colpiM, colpiC, chi, dist1, dist2, trueWD, trueR, i,j,decisione,f;
    chi=g-1;
    colpiM=datiR[chi].colpiMissili;
    colpiC=datiR[chi].colpiRaggi;
    f=datiR[chi].forzaRaggi;

    if(g==primo){//controllo se wardog a gittata raggi, quindi tutti
        if(posR[0][0]==posWD[1][0]){
            if(posR[0][1]<posWD[1][1]){
                dist1=posWD[1][1]-posR[0][1];
                if(dist1>datiR[chi].gitRaggi){
                    trueWD=0;
                }
                else{
                    trueWD=1;
                }
            }
            else{
                dist1=posR[0][1]-posWD[1][1];
                if(dist1>datiR[chi].gitRaggi){
                    trueWD=0;
                }
                else{
                    trueWD=1;
                }
            }
        }
        if(posR[0][1]==posWD[1][1]){
            if(posR[0][0]<posWD[1][0]){
                dist1=posWD[1][0]-posR[0][0];
                if(dist1>datiR[chi].gitRaggi){
                    trueWD=0;
                }
                else{
                    trueWD=1;
                }
            }
            else{
                dist1=posR[0][0]-posWD[1][0];
                if(dist1>datiR[chi].gitRaggi){
                    trueWD=0;
                }
                else{
                    trueWD=1;
                }
            }
        }
        if(posR[0][1]!=posWD[1][1]&&posR[0][0]!=posWD[1][0]){
            //quadrato 1
            for(i=posR[0][0]-datiR[chi].gitRaggi;i<posR[0][0];i++){
                for(j=posR[0][1]-datiR[chi].gitRaggi;j<posR[0][1];j++){
                    if(i!=posWD[0][0]&&j!=posWD[0][1]&&mappaMod[i][j]=='W'){
                        trueWD=1;
                    }
                }
            }
            //quadrato 2
            for(i=posR[0][0]-datiR[chi].gitRaggi;i<posR[0][0];i++){
                for(j=posR[0][1];j<posR[0][1]+datiR[chi].gitRaggi;j++){
                    if(i!=posWD[0][0]&&j!=posWD[0][1]&&mappaMod[i][j]=='W'){
                        trueWD=1;
                    }
                }
            }
            //quadrato 3
            for(i=posR[0][0];i<posR[0][0]+datiR[chi].gitRaggi;i++){
                for(j=posR[0][1]-datiR[chi].gitRaggi;j<posR[0][1];j++){
                    if(i!=posWD[0][0]&&j!=posWD[0][1]&&mappaMod[i][j]=='W'){
                        trueWD=1;
                    }
                }
            }
            //quadrato 4
            for(i=posR[0][0]-datiR[chi].gitRaggi;i<posR[0][0];i++){
                for(j=posR[0][1];j<posR[0][1]+datiWD[chi].gitGatling;j++){
                    if(i!=posWD[0][0]&&j!=posWD[0][1]&&mappaMod[i][j]=='W'){
                        trueWD=1;
                    }
                }
            }
        }
        //controllo se raider a gittata
        if(posR[0][0]==posR[1][0]){
            if(posR[0][1]<posR[1][1]){
                dist1=posR[1][1]-posR[0][1];
                if(dist1>datiR[chi].gitRaggi){
                    trueR=0;
                }
                else{
                    trueR=1;
                }
            }
            else{
                dist1=posR[0][1]-posR[1][1];
                if(dist1>datiR[chi].gitRaggi){
                    trueR=0;
                }
                else{
                    trueR=1;
                }
            }
        }
        if(posR[0][1]==posR[1][1]){
            if(posR[0][0]<posR[1][0]){
                dist1=posR[1][0]-posR[0][0];
                if(dist1>datiR[chi].gitRaggi){
                    trueR=0;
                }
                else{
                    trueR=1;
                }
            }
            else{
                dist1=posR[0][0]-posR[1][0];
                if(dist1>datiR[chi].gitRaggi){
                    trueR=0;
                }
                else{
                    trueR=1;
                }
            }
        }
        if(posR[0][1]!=posR[1][1]&&posR[0][0]!=posR[1][0]){
            //quadrato 1
            for(i=posR[0][0]-datiR[chi].gitRaggi;i<posR[0][0];i++){
                for(j=posR[0][1]-datiWD[chi].gitGatling;j<posR[0][1];j++){
                    if(i!=posR[0][0]&&j!=posR[0][1]&&mappaMod[i][j]=='R'){
                        trueR=1;
                    }
                }
            }
            //quadrato 2
            for(i=posR[0][0]-datiR[chi].gitRaggi;i<posR[0][0];i++){
                for(j=posR[0][1];j<posR[0][1]+datiWD[chi].gitGatling;j++){
                    if(i!=posR[0][0]&&j!=posR[0][1]&&mappaMod[i][j]=='R'){
                        trueR=1;
                    }
                }
            }
            //quadrato 3
            for(i=posR[0][0];i<posR[0][0]+datiWD[chi].gitGatling;i++){
                for(j=posR[0][1]-datiWD[chi].gitGatling;j<posR[0][1];j++){
                    if(i!=posR[0][0]&&j!=posR[0][1]&&mappaMod[i][j]=='R'){
                        trueR=1;
                    }
                }
            }
            //quadrato 4
            for(i=posR[0][0]-datiR[chi].gitRaggi;i<posR[0][0];i++){
                for(j=posR[0][1];j<posR[0][1]+datiR[chi].gitRaggi;j++){
                    if(i!=posR[0][0]&&j!=posR[0][1]&&mappaMod[i][j]=='R'){
                        trueR=1;
                    }
                }
            }
        }
    }
    else{//controllo se wardog a gittata raggi, quindi tutti
        if(posR[1][0]==posWD[0][0]){
            if(posR[1][1]<posWD[0][1]){
                dist1=posWD[0][1]-posR[1][1];
                if(dist1>datiR[chi].gitRaggi){
                    trueWD=0;
                }
                else{
                    trueWD=1;
                }
            }
            else{
                dist1=posR[1][1]-posWD[0][1];
                if(dist1>datiR[chi].gitRaggi){
                    trueWD=0;
                }
                else{
                    trueWD=1;
                }
            }
        }
        if(posR[1][1]==posWD[0][1]){
            if(posR[1][0]<posWD[0][0]){
                dist1=posWD[0][0]-posR[1][0];
                if(dist1>datiR[chi].gitRaggi){
                    trueWD=0;
                }
                else{
                    trueWD=1;
                }
            }
            else{
                dist1=posR[1][0]-posWD[0][0];
                if(dist1>datiR[chi].gitRaggi){
                    trueWD=0;
                }
                else{
                    trueWD=1;
                }
            }
        }
        if(posWD[0][1]!=posR[1][1]&&posWD[0][0]!=posR[1][0]){
            //quadrato 1
            for(i=posR[1][0]-datiR[chi].gitRaggi;i<posR[1][0];i++){
                for(j=posR[1][1]-datiR[chi].gitRaggi;j<posR[1][1];j++){
                    if(i!=posWD[1][0]&&j!=posWD[1][1]&&mappaMod[i][j]=='W'){
                        trueWD=1;
                    }
                }
            }
            //quadrato 2
            for(i=posR[1][0]-datiR[chi].gitRaggi;i<posR[1][0];i++){
                for(j=posR[1][1];j<posR[1][1]+datiR[chi].gitRaggi;j++){
                    if(i!=posWD[1][0]&&j!=posWD[1][1]&&mappaMod[i][j]=='W'){
                        trueWD=1;
                    }
                }
            }
            //quadrato 3
            for(i=posR[1][0];i<posR[1][0]+datiR[chi].gitRaggi;i++){
                for(j=posR[1][1]-datiR[chi].gitRaggi;j<posR[1][1];j++){
                    if(i!=posWD[1][0]&&j!=posWD[1][1]&&mappaMod[i][j]=='W'){
                        trueWD=1;
                    }
                }
            }
            //quadrato 4
            for(i=posR[1][0]-datiR[chi].gitRaggi;i<posR[1][0];i++){
                for(j=posR[1][1];j<posR[1][1]+datiR[chi].gitRaggi;j++){
                    if(i!=posWD[1][0]&&j!=posWD[1][1]&&mappaMod[i][j]=='W'){
                        trueWD=1;
                    }
                }
            }
        }
        //controllo se raider a gittata
        if(posR[1][0]==posR[0][1]){
            if(posR[1][1]<posR[0][1]){
                dist1=posR[0][1]-posR[1][1];
                if(dist1>datiR[chi].gitRaggi){
                    trueR=0;
                }
                else{
                    trueR=1;
                }
            }
            else{
                dist1=posR[1][1]-posR[0][1];
                if(dist1>datiR[chi].gitRaggi){
                    trueR=0;
                }
                else{
                    trueR=1;
                }
            }
        }
        if(posR[1][1]==posR[0][1]){
            if(posR[1][0]<posR[0][0]){
                dist1=posR[0][0]-posR[1][0];
                if(dist1>datiR[chi].gitRaggi){
                    trueR=0;
                }
                else{
                    trueR=1;
                }
            }
            else{
                dist1=posR[1][0]-posR[0][0];
                if(dist1>datiR[chi].gitRaggi){
                    trueR=0;
                }
                else{
                    trueR=1;
                }
            }
        }
        if(posR[0][1]!=posR[1][1]&&posR[0][0]!=posR[1][0]){
            //quadrato 1
            for(i=posR[1][0]-datiR[chi].gitRaggi;i<posR[1][0];i++){
                for(j=posR[1][1]-datiR[chi].gitRaggi;j<posR[1][1];j++){
                    if(i!=posR[1][0]&&j!=posR[1][1]&&mappaMod[i][j]=='R'){
                        trueR=1;
                    }
                }
            }
            //quadrato 2
            for(i=posR[1][0]-datiR[chi].gitRaggi;i<posR[1][0];i++){
                for(j=posR[1][1];j<posR[1][1]+datiR[chi].gitRaggi;j++){
                    if(i!=posR[1][0]&&j!=posR[1][1]&&mappaMod[i][j]=='R'){
                        trueR=1;
                    }
                }
            }
            //quadrato 3
            for(i=posR[1][0];i<posR[1][0]+datiR[chi].gitRaggi;i++){
                for(j=posR[1][1]-datiR[chi].gitRaggi;j<posR[1][1];j++){
                    if(i!=posR[1][0]&&j!=posR[1][1]&&mappaMod[i][j]=='R'){
                        trueR=1;
                    }
                }
            }
            //quadrato 4
            for(i=posR[1][0]-datiR[chi].gitRaggi;i<posR[1][0];i++){
                for(j=posR[1][1];j<posR[1][1]+datiR[chi].gitRaggi;j++){
                    if(i!=posR[1][0]&&j!=posR[1][1]&&mappaMod[i][j]=='R'){
                        trueR=1;
                    }
                }
            }
        }
    }

    if(trueWD){
        decisione=1;
        printf("WarDog nemico a gittata\n");
    }
    if(trueR){
        decisione=2;
        printf("Raider nemico a gittata\n");
    }
    if(trueWD&&trueR){
        printf("A chi vuoi sparare?\n\t1-WarDog\n\t2-Raider\n");
        scanf("%d",&decisione);
    }
    printf("Giocatore %d, vuoi sparare con i missili?\n\t0-no\n\t1-si'",g);
    scanf("%d",&sceltaM);
    printf("Giocatore %d, i vuoi sparare con il cannone?\n\t0-no\n\tsi'",g);
    scanf("%d",&sceltaC);
    /*if(sceltaG==1){
        if(g==primo){
            if(decisione==1){
                if(datiWD[chi+1].voidShieldsWD>=0)
                    datiWD[chi+1].voidShieldsWD-colpiM;
                else
                    datiWD[chi+1].strutturaWD-colpiM;
            }
            else{
                if(datiR[chi+1].voidShieldsR>=0)
                    datiR[chi+1].voidShieldsR-colpiM;
                else
                    datiR[chi+1].strutturaR-colpiM;
            }
        }
        else{
            if(decisione==1){
                if(datiWD[chi-1].voidShieldsWD>=0)
                    datiWD[chi-1].voidShieldsWD-colpiM;
                else
                    datiWD[chi-1].strutturaWD-colpiM;
            }
            else{
                if(datiR[chi-1].voidShieldsR>=0)
                    datiR[chi-1].voidShieldsR-colpiM;
                else
                    datiR[chi-1].strutturaR-colpiM;
            }
        }
    }
    if(sceltaC==1){
        if(g==primo){
            if(decisione==1){
                if(datiWD[chi+1].voidShieldsWD>=0)
                    datiWD[chi+1].voidShieldsWD-colpiC*f;
                else
                    datiWD[chi+1].strutturaWD-colpiC*f;
            }
            else{
                if(datiR[chi+1].voidShieldsR>=0)
                    datiR[chi+1].voidShieldsR-colpiC*f;
                else
                    datiR[chi+1].strutturaR-colpiC*f;
            }
        }
        else{
            if(decisione==1){
                if(datiWD[chi-1].voidShieldsWD>=0)
                    datiWD[chi-1].voidShieldsWD-colpiC*f;
                else
                    datiWD[chi-1].strutturaWD-colpiC*f;
            }
            else{
                if(datiR[chi-1].voidShieldsR>=0)
                    datiR[chi-1].voidShieldsR-colpiC*f;
                else
                    datiR[chi-1].strutturaR-colpiC*f;
            }
        }
    }*/
    if(sceltaM==1){
        if(g==primo&&primo==1){
            if(decisione==1){
                if(datiWD[chi+1].voidShieldsWD>=0)
                    datiWD[chi+1].voidShieldsWD-colpiM;
                else
                    datiWD[chi+1].strutturaWD-colpiM;
            }
            else{
                if(datiR[chi+1].voidShieldsR>=0)
                    datiR[chi+1].voidShieldsR-colpiM;
                else
                    datiR[chi+1].strutturaR-colpiM;
            }
        }
        if(g==primo&&primo==2){
            if(decisione==1){
                if(datiWD[chi].voidShieldsWD>=0)
                    datiWD[chi].voidShieldsWD-colpiM;
                else
                    datiWD[chi].strutturaWD-colpiM;
            }
            else{
                if(datiR[chi].voidShieldsR>=0)
                    datiR[chi].voidShieldsR-colpiM;
                else
                    datiR[chi].strutturaR-colpiM;
            }
        }
        if(g!=primo&&primo==1){
            if(decisione==1){
                if(datiWD[chi-1].voidShieldsWD>=0)
                    datiWD[chi-1].voidShieldsWD-colpiM;
                else
                    datiWD[chi-1].strutturaWD-colpiM;
            }
            else{
                if(datiR[chi-1].voidShieldsR>=0)
                    datiR[chi-1].voidShieldsR-colpiM;
                else
                    datiR[chi-1].strutturaR-colpiM;
            }
        }
        if(g!=primo&&primo==2){
            if(decisione==1){
                if(datiWD[chi].voidShieldsWD>=0)
                    datiWD[chi].voidShieldsWD-colpiM;
                else
                    datiWD[chi].strutturaWD-colpiM;
            }
            else{
                if(datiR[chi].voidShieldsR>=0)
                    datiR[chi].voidShieldsR-colpiM;
                else
                    datiR[chi].strutturaR-colpiM;
            }
        }
    }
    if(sceltaC==1){
        if(g==primo&&primo==1){
            if(decisione==1){
                if(datiWD[chi+1].voidShieldsWD>=0)
                    datiWD[chi+1].voidShieldsWD-colpiC*f;
                else
                    datiWD[chi+1].strutturaWD-colpiC*f;
            }
            else{
                if(datiR[chi+1].voidShieldsR>=0)
                    datiR[chi+1].voidShieldsR-colpiC*f;
                else
                    datiR[chi+1].strutturaR-colpiC*f;
            }
        }
        if(g==primo&&primo==2){
            if(decisione==1){
                if(datiWD[chi].voidShieldsWD>=0)
                    datiWD[chi].voidShieldsWD-colpiC*f;
                else
                    datiWD[chi].strutturaWD-colpiC*f;
            }
            else{
                if(datiR[chi].voidShieldsR>=0)
                    datiR[chi].voidShieldsR-colpiC*f;
                else
                    datiR[chi].strutturaR-colpiC*f;
            }
        }
        if(g!=primo&&primo==1){
            if(decisione==1){
                if(datiWD[chi-1].voidShieldsWD>=0)
                    datiWD[chi-1].voidShieldsWD-colpiC*f;
                else
                    datiWD[chi-1].strutturaWD-colpiC*f;
            }
            else{
                if(datiR[chi-1].voidShieldsR>=0)
                    datiR[chi-1].voidShieldsR-colpiC*f;
                else
                    datiR[chi-1].strutturaR-colpiC*f;
            }
        }
        if(g!=primo&&primo==2){
            if(decisione==1){
                if(datiWD[chi].voidShieldsWD>=0)
                    datiWD[chi].voidShieldsWD-colpiC*f;
                else
                    datiWD[chi].strutturaWD-colpiC*f;
            }
            else{
                if(datiR[chi].voidShieldsR>=0)
                    datiR[chi].voidShieldsR-colpiC*f;
                else
                    datiR[chi].strutturaR-colpiC*f;
            }
        }
    }
}

void main(){

    int turno=1, boostMovimento=2, chiPrima,y,x, temp, scelta, round=0;

    disegna();

    mappaGen();//generazione mappa
    system("cls");

    chiPrima=chiPerPrimo();
    if(chiPrima==1){//se 1 prima

        //giocatore 1
        robot=N_ROBOT;
        WDoR=0;//inserimento wardog
        posizionaRobot(28,34,1);
        robot=N_ROBOT;
        WDoR=1;//inserimento wardog
        posizionaRobot(28,34,1);

        //giocatore 2
        robot=N_ROBOT;
        WDoR=0;//inserimento wardog
        posizionaRobot(0,6,2);
        robot=N_ROBOT;
        WDoR=1;//inserimento wardog
        posizionaRobot(0,6,2);
    }
    else{//se 2 prima

        //giocatore 2
        robot=N_ROBOT;
        WDoR=0;//inserimento wardog
        posizionaRobot(0,6,2);
        robot=N_ROBOT;
        WDoR=1;//inserimento wardog
        posizionaRobot(0,6,2);

        //giocatore 1
        robot=N_ROBOT;
        WDoR=0;//inserimento wardog
        posizionaRobot(28,34,1);
        robot=N_ROBOT;
        WDoR=1;//inserimento wardog
        posizionaRobot(28,34,1);
    }
    while(turno<=1){//dovrebbe durare 5 turni divisi in due round ciascuno
        printMappa();
        //fase di movimento
        /*if(chiPrima==1){//se 1 prima
            faseMovimento(datiWD[0].movimento,chiPrima);
            faseMovimento(datiR[0].movimento,chiPrima);
            faseMovimento(datiWD[1].movimento,2);
            faseMovimento(datiR[1].movimento,2);
        }
        else{//se 2 prima
            faseMovimento(datiWD[0].movimento,chiPrima);
            faseMovimento(datiR[0].movimento,chiPrima);
            faseMovimento(datiWD[1].movimento,1);
            faseMovimento(datiR[1].movimento,1);
        }*/
        //fase di sparo
        if(chiPrima==1){//se 1 prima
            faseSparo(datiWD[0].movimento,chiPrima);
        }
        else{//se 2 prima
            faseSparo(datiWD[0].movimento,chiPrima);
        }
        turno++;
    }

}
