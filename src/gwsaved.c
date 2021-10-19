/*
 * gwsaved.c   Bullfrog's GeneWars savegame and levels editor
 */
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
// TODO: non-standard - replace getch() and remove this include
#include <conio.h>

#include "bulcommn.h"
#include "lbfileio.h"
#include "keycodes.h"
#include "gwsaved_private.h"

const int SAVES_COUNT=8;
const int PLAYERS_COUNT=4;
const int CREATURE_TYPES_COUNT=25;
const int MAX_CREATURES_ALIVE=300;
// Data types

enum gameType {gtUnknown = 0x000,gtSave = 0x001,gtLevel = 0x002,gtUsrLvl = 0x003};

typedef struct {
        unsigned char unkn9;
        unsigned char lightness;//10h=standard,<10h=darker,>10h=brighter; max is about 40h
        unsigned char unkn1;
        unsigned char height;   //height of the left vertex
        unsigned char txt_type; //texture number, selected from B32 file (0..159)
        unsigned char txt_rot;  //texture rotation (+16=+90 degrees; maybe other flags too)
        unsigned char unkn2[6];
        unsigned char revealed;  //0=segment is not reveald, 63=segment is revealed
        unsigned char goop;     //amount of goop on this segment
       } SAVEDATA_MAPITEM;

typedef struct {
        unsigned char unkn01[16];
        unsigned char unkn10[8];
        unsigned char owner; //0..3 - player number; 4 - native
        unsigned char unkn15[7];
        unsigned char unkn20[16];
        unsigned char unkn30[16];
        unsigned char unkn40[16];
        unsigned char unkn50[10];
        unsigned char energy_max[2]; //max energy the creature can have
        unsigned char health_max[2]; //max health the creature can have
        unsigned char lifespan[2]; //lifespan; signed (max=32767).
        unsigned char energy[2];
        unsigned char health[2];
        unsigned char unkn60[4];
        unsigned char unkn65[4];
        unsigned char unkn69[4];
        unsigned char unkn70[2];
        unsigned char hyb_type;   //hybrid type; 4=mule
        unsigned char unkn75;
       } SAVEDATA_CREATURE;

typedef struct {
        char firstname[21];
        unsigned char character;    //visual character used to represent specialist
        unsigned char occupation;   //0-engineer,1-botanic,2-genetic,3-ranger
        unsigned char ability;
        unsigned char zero1;
        unsigned char healthmax;
        unsigned char healthlvl;
        unsigned char onorbit;      //0-not on orbit, 2-on orbit
        unsigned char wasinmission; //0-wasn't on this planet, 1-was on this planet
        unsigned char zero2[3];
        unsigned char status;       //0-on orbit, 1-in rocket, 3-on planet
        unsigned char zero4[3];
       } SAVEDATA_SPECIALYST;

typedef struct {
        //level is stored as big-endian number, to get value in percents
        //you must divide it by 2068 (1 byte and 3 bits)
        unsigned char level[4];
       } SAVEDATA_HYBRIDLEVEL;

typedef struct {
        unsigned char avaible;
        unsigned char zeros[3];
       } SAVEDATA_PLANT;

typedef struct {
        char val1;
        char val2;
        char val3;
        char val4;
        char val5;
        char val6;
        char val7;
        char unkn1[14];
        //First of this starts exacly at offset 40
        //(at 21 of PLAYERINFO structure)
        unsigned char specialysts_num;
        SAVEDATA_SPECIALYST specialysts[32];
        //First of this starts about 1193
        char unkn2[2];
        char val20;
        char unkn3[10];
        unsigned char goop[2];
        unsigned char unkn5[16];
        unsigned char unkn6[2];//number, usually divides itself by 100
        unsigned char unkn7[302];
        //This is at 1509(0x5e5) of PLAYERINFO structure
        unsigned char unkn_cntr;//increases all the time during gameplay
        unsigned char unkn8[52];
        //This is at 1563(0x61b) of PLAYERINFO structure
        SAVEDATA_HYBRIDLEVEL hybridlevel[25];
        SAVEDATA_PLANT plants[12];
        unsigned char hybridorder[25];//in new saves - 0, in mature ones 0..24
        unsigned char hybridblessed[25];//0 or 1
        //This is at 1760(0x6e0) of PLAYERINFO structure
        unsigned char hybridscount;
        unsigned char unkn11[6];
        unsigned char level_num;
        unsigned char unkn_flag1;
        char playername[16];
        unsigned char unkn95[12];
        //This is at 1797(0x705) of PLAYERINFO structure
        unsigned char newspiecesfound;//number of new spieces found on this level
        unsigned char unkn99[7];
       } SAVEDATA_PLAYERINFO;

typedef struct {
        char magic_id[4];
        char comment[16];
        //This starts exacly at offset 19
        SAVEDATA_PLAYERINFO players[4];
        char landname[12];
        //This starts about 7251
        unsigned char unkn3[6247];
        unsigned char tipshown[83];
        unsigned char unkn20[76];
        unsigned char timevar1[4];//This data dynamicly changes in time
        //This starts about 13661
        //map size is 128x128
        SAVEDATA_MAPITEM mapdata[128][128];
        //This starts about 243039
        SAVEDATA_CREATURE creaturedata[300];
        unsigned char objdata1[20996];
        //This starts about 298856(0x48F68)
        unsigned char timevar2[45];//This data dynamicly changes in time
        unsigned char objdata2[38097];
        //This starts about 336999(0x52467)
        unsigned char timevar3[26];//This data dynamicly changes in time
        unsigned char objdata10[25];
        //Viewed map coords at 337050(0x5249a)
        unsigned char mapcoordx[2];
        unsigned char mapcoordy[2];
        unsigned char objdata20[6904];
       } SAVEDATA;


void getSaveFileName(char *dest, int game_type,int num)
{
  switch (game_type)
  {
  case gtSave:
    sprintf (dest,".\\save\\save%d.sav",num);
      break;
  case gtLevel:
    sprintf (dest,".\\levels\\conq%3d.dat",num);
  case gtUsrLvl:
    sprintf (dest,".\\levels\\cust%3d.dat",num);
      break;
  }
}

char *getHybridName(int hynum)
{
    switch (hynum)
    {
    case  0:return "dino";
    case  1:return "crab";
    case  2:return "frog";
    case  3:return "bird";
    case  4:return "mule";
    case  5:return "dinocrab";
    case  6:return "dinofrog";
    case  7:return "dinobird";
    case  8:return "dinomule";
    case  9:return "crabofrog";
    case 10:return "crabobird";
    case 11:return "crabomule";
    case 12:return "frogobird";
    case 13:return "frogomule";
    case 14:return "birdomule";
    case 15:return "crabosaur";
    case 16:return "frogosaur";
    case 17:return "frogocrab";
    case 18:return "birdosaur";
    case 19:return "birdocrab";
    case 20:return "birdofrog";
    case 21:return "muleosaur";
    case 22:return "muleocrab";
    case 23:return "muleofrog";
    case 24:return "muleobird";
    default:return "unknown";
    }
}

char *getPlantName(int plnum)
{
    switch (plnum)
    {
    case  0:return "Terrinium Bulb";
    case  1:return "Phylax Root";
    case  2:return "Waxx Leaf";
    case  3:return "Arrid Bloom";
    case  4:return "Bralm";
    case  5:return "Dune Palm";
    case  6:return "Pelid Snowdrop";
    case  7:return "Kelpie";
    case  8:return "Hexilian Spikey";
    case  9:return "Zarnian Fungus";
    case 10:return "Venular Spiral";
    case 11:return "Cone";
    default:return "unknown";
    }
}

char *getSpecialistOccupation(int ocnum)
{
    switch (ocnum)
    {
    case 0:return "engineer";
    case 1:return "botanic";
    case 2:return "genetic";
    case 3:return "ranger";
    default:return "unknown";
    }
}

int getUsrUValue(int minVal,int maxVal,int *newval)
{
         (*newval)=min(-1,minVal-1);
         printf("Enter new value (%d..%d):",minVal,maxVal);
         char buf[255];
         scanf("%s",buf);
         sscanf(buf,"%i",newval);
         if (((*newval)>=minVal) && ((*newval)<=maxVal))
         {
            printf("\nValue %d accepted.\n",*newval);
            return 1;
         }
         else
            printf("\nValue %d out of range, ignored.\n",*newval);
         return 0;
}

int getUsrFValue(float minVal,float maxVal,float *newval)
{
         (*newval)=min(-1.0,minVal-1.0);
         printf("Enter new value (%f..%f):",minVal,maxVal);
         char buf[255];
         scanf("%s",buf);
         sscanf(buf,"%f",newval);
         if (((*newval)>=minVal) && ((*newval)<=maxVal))
         {
            printf("\nValue %f saved.\n",*newval);
            return 1;
         }
         else
            printf("\nValue %f out of range, ignored.\n",*newval);
         return 0;
}

int getUsr8bitUValueTo(int minVal,int maxVal,unsigned char *dest_ptr)
{
    int newval;
    if (!getUsrUValue(minVal,maxVal,&newval))
        return 0;
    (*dest_ptr)=(unsigned char)(newval&255);
}

int getUsr16bitUValueTo(int minVal,int maxVal,unsigned char *dest_ptr)
{
    int newval;
    if (!getUsrUValue(minVal,maxVal,&newval))
        return 0;
    write_int16_le_buf(dest_ptr,newval&65535);
}

int saveStructureToDefFile(char *fname,char *src_ptr,unsigned long length)
{
         FILE *expfp=fopen(fname,"wb");
         if (expfp==NULL)
         {
            printf("\nOperation skipped - cannot create \"%s\"\n",fname);
            return 1;
         }
         fwrite (src_ptr, length, 1, expfp);
         fclose(expfp);
         printf("\nStructure saved as \"%s\".\n",fname);
         return 0;
}

int saveStructureToFile(char *samplename,char *src_ptr,unsigned long length)
{
         printf("\nEnter destination file name, ie \"%s\":",samplename);
         char newname[255];
         int i;
         for (i=0;i<255;i++)
             newname[i]=0;
         scanf("%s",newname);
         if ((strlen(newname)<1)||(newname[0]<'!')) strcpy(newname,samplename);
         return saveStructureToDefFile(newname,src_ptr,length);
}

int loadStructureFromDefFile(char *fname,char *dest_ptr,unsigned long length)
{
         FILE *impfp=fopen(fname,"rb");
         if (impfp==NULL)
         {
            printf("\nOperation skipped - cannot open \"%s\"\n",fname);
            return 1;
         }
         fread (dest_ptr, length, 1, impfp);
         fclose(impfp);
         printf("\nStructure loaded from \"%s\".\n",fname);
         return 0;
}

int loadStructureFromFile(char *samplename,char *dest_ptr,unsigned long length)
{
         printf("\nEnter source file name, ie \"%s\":",samplename);
         char newname[255];
         int i;
         for (i=0;i<255;i++)
             newname[i]=0;
         scanf("%s",newname);
         if ((strlen(newname)<1)||(newname[0]<'!')) strcpy(newname,samplename);
         return loadStructureFromDefFile(newname,dest_ptr,length);
}

void getSavesComments(char *dest_names,char *exist_table)
//Gets text comments inside saves, returns number of saves
{
    int num;
    for (num=0;num<SAVES_COUNT;num++)
    {
        FILE *savefp;
        char savefname[255];
        getSaveFileName(savefname,gtSave,num);
        savefp = fopen (savefname, "rb");
        if (!savefp)
        {
            exist_table[num]=0;
        }
        else
        {
            exist_table[num]=1;
            fseek (savefp, 4, SEEK_SET);
            fread (&(dest_names[17*num]), 16, 1, savefp);
            dest_names[17*num+16]=0;
            fclose(savefp);
        }
    }
}

int enableCreatureBuilding(SAVEDATA_PLAYERINFO *player,unsigned char creature_spiece)
// Creature that is avaible should has avaible flag set and should be in build order.
// This function adds creature to build order and rebuilds order table,
//  to make sure that there are no errors. It also sets hybridscount.
// Warning: hybridscount shouldn't be increased anywhere outside this function!
//Returns 1 on success, 0 on failure.
{
    //Old order table
    unsigned char *order=player->hybridorder;
    int order_end=min(player->hybridscount,CREATURE_TYPES_COUNT);
    //New order table
    unsigned char *neworder=malloc(CREATURE_TYPES_COUNT);
    int neworder_end=0;
    if (!neworder) return 0;
    //Clearing new table
    int crnum;
    for (crnum=0;crnum<CREATURE_TYPES_COUNT;crnum++)
        neworder[crnum]=0;
    //Rebuilding table
    for (crnum=0;crnum<order_end;crnum++)
    {
        unsigned char spiece=min(order[crnum],CREATURE_TYPES_COUNT-1);
        int already_in_table=0;
        int idx;
        for (idx=0;idx<neworder_end;idx++)
        {
            if (neworder[idx]==spiece)
                already_in_table=1;
        }
        if (!already_in_table)
        {
            neworder[neworder_end]=spiece;
            neworder_end++;
        }
    }
    //Adding new spiece to table
    unsigned char spiece=min(creature_spiece,CREATURE_TYPES_COUNT-1);
    {
        int already_in_table=0;
        int idx;
        for (idx=0;idx<neworder_end;idx++)
        {
            if (neworder[idx]==spiece)
                already_in_table=1;
        }
        if (!already_in_table)
        {
            neworder[neworder_end]=spiece;
            neworder_end++;
        }
    }
    //saving table
    player->hybridscount=(unsigned char)neworder_end;
    memcpy(order,neworder,CREATURE_TYPES_COUNT);
    free(neworder);
    return 1;
}

int disableCreatureBuilding(SAVEDATA_PLAYERINFO *player,unsigned char creature_spiece)
// Creature that is avaible should has avaible flag set and should be in build order.
// This function removes creature from build order and rebuilds order table,
//  to make sure that there are no errors. It also sets hybridscount.
// Warning: hybridscount shouldn't be decreased anywhere outside this function!
//Returns 1 on success, 0 on failure.
{
    //Old order table
    unsigned char *order=player->hybridorder;
    int order_end=min(player->hybridscount,CREATURE_TYPES_COUNT);
    //New order table
    unsigned char *neworder=malloc(CREATURE_TYPES_COUNT);
    int neworder_end=0;
    if (!neworder) return 0;
    //Clearing new table
    int crnum;
    for (crnum=0;crnum<CREATURE_TYPES_COUNT;crnum++)
        neworder[crnum]=0;
    //Rebuilding table without a spiece to remove
    unsigned char spiece_rm=min(creature_spiece,CREATURE_TYPES_COUNT-1);
    for (crnum=0;crnum<order_end;crnum++)
    {
        unsigned char spiece=min(order[crnum],CREATURE_TYPES_COUNT-1);
        int already_in_table=0;
        int idx;
        for (idx=0;idx<neworder_end;idx++)
        {
            if (neworder[idx]==spiece)
                already_in_table=1;
        }
        if ((!already_in_table)&&(spiece!=spiece_rm))
        {
            neworder[neworder_end]=spiece;
            neworder_end++;
        }
        
    }
    //saving table
    player->hybridscount=(unsigned char)neworder_end;
    memcpy(order,neworder,CREATURE_TYPES_COUNT);
    free(neworder);
    return 1;
}

int editSpecialysts(SAVEDATA_PLAYERINFO *player)
{
    int special_num=player->specialysts_num;
    printf("\n\nSpecialysts for selected player:\n");
    int spnum;
    for (spnum=0;spnum<special_num;spnum++)
    {
        SAVEDATA_SPECIALYST *specialyst=&(player->specialysts[spnum]);
        int sp_abilit=specialyst->ability;
        int sp_health=specialyst->healthlvl;
        int sp_healmax=specialyst->healthmax;
        int sp_occup=specialyst->occupation;
        printf("%3d. \"%s\", %s, abil.%d, health %d/%d\n",spnum+1,&(specialyst->firstname),getSpecialistOccupation(sp_occup),sp_abilit,sp_health,sp_healmax);
    }
    printf("%3d. Upgrade Ability and Health 2x\n",92);
    printf("%3d. Degrade Ability and Health by half\n",93);
    printf("%3d. Change all specialysts Ability\n",95);
    printf("%3d. Change all specialysts Max. Health\n",96);
    printf("%3d. Import all specialysts from BIN\n",98);
    printf("%3d. Export all specialysts to BIN\n",99);
    printf(" NaN - %s\n","Back (NaN = not a number)");
    printf("\nSelect specialyst number:");
    int spec_num=-1;
    {
         char buf[255];
         scanf("%s",buf);
         sscanf(buf,"%i",&spec_num);
    }

    if (spec_num==92)
    {
         printf("\nUpgrading all specialysts...\n");
         for (spnum=0;spnum<special_num;spnum++)
         {
             SAVEDATA_SPECIALYST *specialyst=&(player->specialysts[spnum]);
              int sp_abilit=specialyst->ability;
              int sp_health=specialyst->healthlvl;
              int sp_healmax=specialyst->healthmax;
             specialyst->ability=min(sp_abilit*2,100);
             specialyst->healthlvl=min(sp_health*2,100);
             specialyst->healthmax=min(sp_healmax*2,100);
             printf("Values doubled for \"%s\".\n",&(specialyst->firstname));
         }
         return 0;
    }
    if (spec_num==93)
    {
         printf("\nDegrading all specialysts...\n");
         for (spnum=0;spnum<special_num;spnum++)
         {
             SAVEDATA_SPECIALYST *specialyst=&(player->specialysts[spnum]);
              int sp_abilit=specialyst->ability;
              int sp_health=specialyst->healthlvl;
              int sp_healmax=specialyst->healthmax;
             specialyst->ability=max(sp_abilit/2,4);
             specialyst->healthlvl=max(sp_health/2,2);
             specialyst->healthmax=max(sp_healmax/2,2);
             printf("Values halved for \"%s\".\n",&(specialyst->firstname));
         }
         return 0;
    }

    if (spec_num==95)
    {
         int newval;
         printf("hint: valid abilities are 4..100. Usually unupgraded specialyst\n");
         printf("      has about 20. Monoliths can upgrade ability up to 90.\n");
         if ( !getUsrUValue(0,255,&newval) )
             return 1;
         printf("\n");
         for (spnum=0;spnum<special_num;spnum++)
         {
             SAVEDATA_SPECIALYST *specialyst=&(player->specialysts[spnum]);
             specialyst->ability=newval;
             printf("Value %d saved for \"%s\".\n",newval,&(specialyst->firstname));
         }
         return 0;
    }
    if (spec_num==96)
    {
         printf("hint: valid health value is 2..100. Unupgraded specialyst\n");
         printf("      has about 15. Monoliths can upgrade it up to 90.\n");
         int newval;
         if ( !getUsrUValue(0,255,&newval) )
             return 1;
         printf("\n");
         for (spnum=0;spnum<special_num;spnum++)
         {
             SAVEDATA_SPECIALYST *specialyst=&(player->specialysts[spnum]);
             specialyst->healthmax=newval;
             if (newval<specialyst->healthlvl)
                 specialyst->healthlvl=newval;
             printf("Value %d saved for \"%s\".\n",newval,&(specialyst->firstname));
         }
         return 0;
    }
    if (spec_num==98)
    {
        int result=loadStructureFromFile("specfile.bin",(char *)&(player->specialysts), sizeof(player->specialysts));
        if (result==0)
        {
            special_num=0;
            while ((player->specialysts[special_num].firstname[0]!=0)&&(special_num<32))
                 special_num++;
            player->specialysts_num=special_num;
        }
        return result;
    }
    if (spec_num==99)
    {
        int result=saveStructureToFile("specfile.bin",(char *)&(player->specialysts), sizeof(player->specialysts));
        return result;
    }
    if ((spec_num<1)||(spec_num>special_num))
    {
        printf("\nEdit skipped - specialyst number not valid.\n");
        return 1;
    }
    SAVEDATA_SPECIALYST *specialyst=&(player->specialysts[spec_num-1]);
    int sp_char=specialyst->character;
    int sp_occup=specialyst->occupation;
    int sp_abilit=specialyst->ability;
    int sp_health=specialyst->healthlvl;
    int sp_healmax=specialyst->healthmax;
    printf("\n\nSpecialyst properties:\n");
    printf("1. Name \"%s\"\n",&(specialyst->firstname));
    printf("2. Race character %d\n",sp_char);
    printf("3. Occupation %d (%s)\n",sp_occup,getSpecialistOccupation(sp_occup));
    printf("4. Ability %d\n",sp_abilit);
    printf("5. Health %d\n",sp_health);
    printf("6. Max. Health %d\n",sp_healmax);
    printf(" NaN - %s\n","Back (NaN = not a number)");
    printf("\nSelect property:");
    int prop_num=-1;
    {
         char buf[255];
         scanf("%s",buf);
         sscanf(buf,"%i",&prop_num);
    }
    switch(prop_num)
    {
    case 1:
         {
         printf("Enter new name(max. 19 chars):");
         char newname[255];
         int i;
         for (i=0;i<255;i++)
             newname[i]=0;
         scanf("%s",newname);
         strncpy(specialyst->firstname,newname,20);
         printf("\nValue \"%s\" written.\n",specialyst->firstname);
         };break;
    case 2:
         {
           printf("hint: valid races of characters are 0..3\n");
           getUsr8bitUValueTo(0,255,&(specialyst->character));
         };break;
    case 3:
         {
           printf("hint: valid occupations are 0..3\n");
           getUsr8bitUValueTo(0,255,&(specialyst->occupation));
         };break;
    case 4:
         {
           printf("hint: valid abilities are 4..100. Usually unupgraded specialyst\n");
           printf("      has about 20. Monoliths can upgrade ability up to 90.\n");
           getUsr8bitUValueTo(0,255,&(specialyst->ability));
         };break;
    case 5:
         {
           printf("hint: valid health value is 2..100. Unupgraded specialyst\n");
           printf("      has about 15. Monoliths can upgrade it up to 90.\n");
           int newval;
           if ( getUsrUValue(0,255,&newval) )
           {
              specialyst->healthlvl=newval;
              if (newval>specialyst->healthmax)
                  specialyst->healthmax=newval;
           }
         };break;
    case 6:
         {
           printf("hint: valid health value is 2..100. Unupgraded specialyst\n");
           printf("      has about 15. Monoliths can upgrade it up to 90.\n");
           int newval;
           if ( getUsrUValue(0,255,&newval) )
           {
              specialyst->healthmax=newval;
              if (newval<specialyst->healthlvl)
                  specialyst->healthlvl=newval;
           }
         };break;
    default:
        printf("\nEdit skipped - property number not valid.\n");
        return 1;
    }
    return 0;
}

int editLiveHybrids(SAVEDATA *save,unsigned int player_num)
{
    printf("\n\nAlive hybrids for player %u:\n",player_num);
    unsigned int creat_alive[CREATURE_TYPES_COUNT];
    unsigned int creat_dead=0;
    {
      unsigned int i;
      for (i=0;i<CREATURE_TYPES_COUNT;i++) creat_alive[i]=0;
      int hyb_num;
      for (hyb_num=0;hyb_num<MAX_CREATURES_ALIVE;hyb_num++)
      {
        unsigned int lifespan=read_int16_le_buf((unsigned char *)&(save->creaturedata[hyb_num].lifespan));
        unsigned char owner=save->creaturedata[hyb_num].owner;
        if (owner==player_num)
        {
          if (lifespan>0)
          {
              unsigned char hyb_type=save->creaturedata[hyb_num].hyb_type;
              creat_alive[hyb_type%CREATURE_TYPES_COUNT]++;
          } else
          {
              creat_dead++;
          }
        }
      }
    }
    char has_creature=0;
    unsigned int i;
    for (i=0;i<CREATURE_TYPES_COUNT;i++)
    {
      if (creat_alive[i]>0)
        {
        printf("%10s: %u creatures owned\n",getHybridName(i),creat_alive[i]);
        has_creature=1;
        }
    }
    if (creat_dead>0)
    {
        printf("     dead creatures owned %u\n",creat_dead);
        has_creature=1;
    }
    if (!has_creature)
        printf("     no creatures owned\n");

//    printf("%3d. Change owner of some creatures\n",90);
//    printf("%3d. Improve all alive player creatures level\n",91);
//    printf("%3d. Decrease all alive player creatures level\n",92);
    printf("%3d. Heal all player creatures\n",93);
    printf("%3d. Decrease all player creatures health\n",94);
    printf("%3d. Improve all player creatures lifespan\n",95);
    printf("%3d. Decrease all player creatures lifespan\n",96);
//    printf("%3d. Resurrect all player dead creatures\n",97);
    printf("%3d. Import all alive creatures from BIN\n",98);
    printf("%3d. Export all alive creatures to BIN\n",99);
    printf(" NaN - %s\n","Back (NaN = not a number)");
    printf("\nSelect hybrid number:");
    int hyb_num=-1;
    {
         char buf[255];
         scanf("%s",buf);
         sscanf(buf,"%i",&hyb_num);
    }

    if (hyb_num==93)
    {
         printf("\nImproving creatures health and energy...\n");
          unsigned int hyb_improved=0;
          int hnum;
          for (hnum=0;hnum<MAX_CREATURES_ALIVE;hnum++)
          {
            unsigned int lifespan=read_int16_le_buf((unsigned char *)&(save->creaturedata[hnum].lifespan));
            unsigned char owner=save->creaturedata[hnum].owner;
            if (owner==player_num)
            {
              if (lifespan>0)
              {
                  unsigned int energy=read_int16_le_buf((unsigned char *)&(save->creaturedata[hnum].energy_max));
                  unsigned int health=read_int16_le_buf((unsigned char *)&(save->creaturedata[hnum].health_max));
                  if (energy<150)
                      energy=2*energy;
                  else
                  if (energy<300)
                      energy=3*energy/2;
                  else
                      energy=4*energy/3;
                  if (health<150)
                      health=2*health;
                  else
                  if (energy<300)
                      health=3*health/2;
                  else
                      health=4*health/3;
                  if ((energy>32767)||(energy<=0)) energy=32767;
                  if ((health>32767)||(health<=0)) health=32767;
                  write_int16_le_buf((unsigned char *)&(save->creaturedata[hnum].energy_max),energy);
                  write_int16_le_buf((unsigned char *)&(save->creaturedata[hnum].energy),energy);
                  write_int16_le_buf((unsigned char *)&(save->creaturedata[hnum].health_max),health);
                  write_int16_le_buf((unsigned char *)&(save->creaturedata[hnum].health),health);
                  hyb_improved++;
              }
            }
          }
         printf("\nImproved %u creatures\n",hyb_improved);
         return 0;
    }
    if (hyb_num==94)
    {
         printf("\nDecreasing creatures health and energy...\n");
          unsigned int hyb_improved=0;
          int hnum;
          for (hnum=0;hnum<MAX_CREATURES_ALIVE;hnum++)
          {
            unsigned int lifespan=read_int16_le_buf((unsigned char *)&(save->creaturedata[hnum].lifespan));
            unsigned char owner=save->creaturedata[hnum].owner;
            if (owner==player_num)
            {
              if (lifespan>0)
              {
                  //max parameters
                  unsigned int energy_max=read_int16_le_buf((unsigned char *)&(save->creaturedata[hnum].energy_max));
                  unsigned int health_max=read_int16_le_buf((unsigned char *)&(save->creaturedata[hnum].health_max));
                  energy_max=2*energy_max/3;
                  health_max=2*health_max/3;
                  if ((energy_max>32767)||(energy_max<=0)) energy_max=1;
                  if ((health_max>32767)||(health_max<=0)) health_max=1;
                  write_int16_le_buf((unsigned char *)&(save->creaturedata[hnum].energy_max),energy_max);
                  write_int16_le_buf((unsigned char *)&(save->creaturedata[hnum].health_max),health_max);
                  //current parameters
                  unsigned int energy=read_int16_le_buf((unsigned char *)&(save->creaturedata[hnum].energy));
                  unsigned int health=read_int16_le_buf((unsigned char *)&(save->creaturedata[hnum].health));
                  energy=2*energy/3;
                  health=2*health/3;
                  if ((energy>energy_max)||(energy<=0)) energy=1;
                  if ((health>health_max)||(health<=0)) health=1;
                  write_int16_le_buf((unsigned char *)&(save->creaturedata[hnum].energy),energy);
                  write_int16_le_buf((unsigned char *)&(save->creaturedata[hnum].health),health);
                  hyb_improved++;
              }
            }
          }
         printf("\nInjuried %u creatures\n",hyb_improved);
         return 0;
    }
    if (hyb_num==95)
    {
         printf("\nRegenerating creatures lifespan...\n");
          unsigned int hyb_improved=0;
          int hnum;
          for (hnum=0;hnum<MAX_CREATURES_ALIVE;hnum++)
          {
            unsigned int lifespan=read_int16_le_buf((unsigned char *)&(save->creaturedata[hnum].lifespan));
            unsigned char owner=save->creaturedata[hnum].owner;
            if (owner==player_num)
            {
              if (lifespan>0)
              {
                  if (lifespan<100) lifespan=100;
                  if (lifespan<4096)
                      lifespan*=2;
                  else
                      lifespan=3*lifespan/2;
                  if ((lifespan>32767)||(lifespan<=0)) lifespan=32767;
                  write_int16_le_buf((unsigned char *)&(save->creaturedata[hnum].lifespan),lifespan);
                  hyb_improved++;
              }
            }
          }
         printf("\nLonger life made for %u creatures\n",hyb_improved);
         return 0;
    }
    if (hyb_num==96)
    {
         printf("\nDecreasing creatures lifespan...\n");
          unsigned int hyb_improved=0;
          int hnum;
          for (hnum=0;hnum<MAX_CREATURES_ALIVE;hnum++)
          {
            unsigned int lifespan=read_int16_le_buf((unsigned char *)&(save->creaturedata[hnum].lifespan));
            unsigned char owner=save->creaturedata[hnum].owner;
            if (owner==player_num)
            {
              if (lifespan>0)
              {
                  if (lifespan>30)
                      lifespan=lifespan/2;
                  else
                      lifespan=2*lifespan/3;
                  if ((lifespan>32767)||(lifespan<=0)) lifespan=2;
                  write_int16_le_buf((unsigned char *)&(save->creaturedata[hnum].lifespan),lifespan);
                  hyb_improved++;
              }
            }
          }
         printf("\nLife is shorter for %u creatures\n",hyb_improved);
         return 0;
    }
    if (hyb_num==97)
    {
         //FINISH!!!!!!!
         printf("\nUnfinished, sorry\n");
         return 0;
    }
    if (hyb_num==98)
    {
        int result=loadStructureFromFile("hyblive.bin",(char *)&(save->creaturedata),
                  sizeof(save->creaturedata));
        return result;
    }
    if (hyb_num==99)
    {
        int result=saveStructureToFile("hyblive.bin",(char *)&(save->creaturedata),
                  sizeof(save->creaturedata));
        return result;
    }
    //Here we should edit a single creature!!!!!
    return 0;
}

int editHybrids(SAVEDATA_PLAYERINFO *player)
{
    printf("\n\nHybrids for selected player:\n");
    int hyb_num;
    for (hyb_num=0;hyb_num<CREATURE_TYPES_COUNT;hyb_num++)
    {
        unsigned long hybr_lvl=read_int32_le_buf((unsigned char *)&(player->hybridlevel[hyb_num]));
        int hybr_avaible=(hybr_lvl!=0);
        float hybr_lvl_percent=((float)hybr_lvl)/2048;
        int hybr_blessed=(unsigned char)(player->hybridblessed[hyb_num]);
        printf("%3d. %10s, avaible %d, level %7.3f, blessed %2d\n",hyb_num+1,getHybridName(hyb_num),hybr_avaible,hybr_lvl_percent,hybr_blessed);
    }
    printf("%3d. Set all avaible creatures level\n",95);
    printf("%3d. Mark all avaible creatures as blessed\n",96);
    printf("%3d. Clear all Ethereal blessings\n",97);
    printf("%3d. Import all hybrids and plants from BIN\n",98);
    printf("%3d. Export all hybrids and plants to BIN\n",99);
    printf(" NaN - %s\n","Back (NaN = not a number)");
    printf("\nSelect hybrid number:");
    hyb_num=-1;
    {
         char buf[255];
         scanf("%s",buf);
         sscanf(buf,"%i",&hyb_num);
    }
    if (hyb_num==95)
    {
         float newval_f;
         printf("\nEditing research level for %s\n","all avaible creatures");
         if ( !getUsrFValue(0,1048576,&newval_f) )
             return 1;
         printf("\n");
         unsigned long newval=newval_f*2048;
         int hnum;
         for (hnum=0;hnum<CREATURE_TYPES_COUNT;hnum++)
         {
             unsigned long old_level=read_int32_le_buf((unsigned char *)&(player->hybridlevel[hnum]));
             if (old_level != 0)
             {
                write_int32_le_buf((unsigned char *)&(player->hybridlevel[hnum]),newval);
                 printf("Value %7.3f set for %s.\n",newval_f,getHybridName(hnum));
             }
         }
         return 0;
    }
    if (hyb_num==96)
    {
         printf("\nMarking creatures as blessed...\n");
         int hnum;
         for (hnum=0;hnum<CREATURE_TYPES_COUNT;hnum++)
         {
             unsigned long hybrid_level=read_int32_le_buf((unsigned char *)&(player->hybridlevel[hnum]));
             if (hybrid_level>0)
             {
                 player->hybridblessed[hnum]=1;
                 printf("Blessings set for %s.\n",getHybridName(hnum));
             }
         }
         return 0;
    }
    if (hyb_num==97)
    {
         printf("\nClearing Ethereal blessings...\n");
         int hnum;
         for (hnum=0;hnum<CREATURE_TYPES_COUNT;hnum++)
         {
             player->hybridblessed[hnum]=0;
             printf("Blessings cleared for %s.\n",getHybridName(hnum));
         }
         return 0;
    }
    if (hyb_num==98)
    {
        int result=loadStructureFromFile("hybfile.bin",(char *)&(player->hybridlevel),
                  sizeof(player->hybridlevel)+sizeof(player->plants)+sizeof(player->hybridorder)+sizeof(player->hybridblessed));
        return result;
    }
    if (hyb_num==99)
    {
        int result=saveStructureToFile("hybfile.bin",(char *)&(player->hybridlevel),
                  sizeof(player->hybridlevel)+sizeof(player->plants)+sizeof(player->hybridorder)+sizeof(player->hybridblessed));
        return result;
    }
    //We must make 1=>0, 2=>1 ect.
    hyb_num-=1;
    if ((hyb_num<0)||(hyb_num>=CREATURE_TYPES_COUNT))
    {
        printf("\nEdit skipped - hybryd number not valid.\n");
        return 1;
    }

    float newval_f;
    printf("\nEditing research level for %s\n",getHybridName(hyb_num));
    printf("hint: enter 0 if you wish this hybrid to be unavaible\n");
    if ( !getUsrFValue(0,1048576,&newval_f) )
        return 1;
    printf("\n");
    unsigned long newval=newval_f*2048;
    if (newval==0)
        {
        disableCreatureBuilding(player,hyb_num);
        write_int32_le_buf((unsigned char *)&(player->hybridlevel[hyb_num]),0);
        player->hybridblessed[hyb_num]=0;
        printf("Hybrid %s set as unavaible to build.\n",getHybridName(hyb_num));
        return 0;
        }
    enableCreatureBuilding(player,hyb_num);
    write_int32_le_buf((unsigned char *)&(player->hybridlevel[hyb_num]),newval);
    printf("Research level %7.3f set for %s.\n",newval_f,getHybridName(hyb_num));
    //Editing blessed flag
    printf("\nEditing hybrid blessed by ethereals flag for %s\n",getHybridName(hyb_num));
    printf("hint: type 0 for standard, 1 for blessed by ethereals\n");
    getUsr8bitUValueTo(0,255,(unsigned char *)&(player->hybridblessed[hyb_num]));
    return 0;
}

int editPlants(SAVEDATA_PLAYERINFO *player)
{
    int plants_num=12;
    printf("\n\nPlants for selected player:\n");
    int plnt_num;
    for (plnt_num=0;plnt_num<plants_num;plnt_num++)
    {
        SAVEDATA_PLANT *plant=&(player->plants[plnt_num]);
        int plant_avaibl=(unsigned char)(plant->avaible);
        printf("%3d. %15s, avaible %2d\n",plnt_num+1,getPlantName(plnt_num),plant_avaibl);
    }
    printf("%3d. Import all plants from BIN\n",98);
    printf("%3d. Export all plants to BIN\n",99);
    printf(" NaN - %s\n","Back (NaN = not a number)");
    printf("\nSelect plant number:");
    plnt_num=-1;
    {
         char buf[255];
         scanf("%s",buf);
         sscanf(buf,"%i",&plnt_num);
    }
    if (plnt_num==98)
    {
        int result=loadStructureFromFile("plnfile.bin",(char *)&(player->plants), sizeof(player->plants));
        return result;
    }
    if (plnt_num==99)
    {
        int result=saveStructureToFile("plnfile.bin",(char *)&(player->plants), sizeof(player->plants));
        return result;
    }
    //We must make 1=>0, 2=>1 ect.
    plnt_num-=1;
    if ((plnt_num<0)||(plnt_num>=plants_num))
    {
        printf("\nEdit skipped - plant number not valid.\n");
        return 1;
    }
    SAVEDATA_PLANT *plant=&(player->plants[plnt_num]);
    //Editing avaible flag
    printf("\nEditing plant avaibility for %s\n",getPlantName(plnt_num));
    printf("hint: type 0 for unavaible, 1 for avaible (can be planted by botanist)\n");
    getUsr8bitUValueTo(0,255,(unsigned char *)&(plant->avaible));
}

int editGoop(SAVEDATA_PLAYERINFO *player)
{
    printf("warning: this modifies only \"starting money\",\n");
    printf("         not the money player earned in the game.\n");
    getUsr16bitUValueTo(0,65535,(unsigned char *)&(player->goop));
}

int editLevelNumber(SAVEDATA_PLAYERINFO *player)
{
    printf("hint: Increasing the level number gives you ability to build more\n");
    printf("      structures, but if you don't want to skip some levels after the\n");
    printf("      current level, you should restore level number brfore mission end.\n");
    getUsr8bitUValueTo(0,255,&(player->level_num));
}

int editPlayer(SAVEDATA *save,int player_num)
{
  while (1)
  {
    SAVEDATA_PLAYERINFO *player=&(save->players[player_num]);
    int money_val=read_int16_le_buf((unsigned char *)&(player->goop));
    int special_num=player->specialysts_num;
    int level_num=player->level_num;
    printf("\n\nModifications for player %d:\n",player_num+1);
    printf("%3d - Goop value: %d\n",1,money_val);
    printf("%3d - Specialysts: %d\n",2,special_num);
    printf("%3d - Purebreeds and Hybrids availability\n",3);
    printf("%3d - Edit alive Purebreeds and Hybrids\n",4);
    printf("%3d - Plant spieces\n",5);
    printf("%3d - Level number (0 is first level): %d\n",6,level_num);
    printf("%3d - Import all player data from BIN\n",8);
    printf("%3d - Export all player data to BIN\n",9);
    printf("%3s - Return\n","Esc");
    printf("\nSelect what to edit:");
    int funcnum;
    char usr_answer;
    do
    {
        usr_answer=getch();
        if ((usr_answer<'1')||(usr_answer>'9'))
            funcnum=-1;
        else
            funcnum=usr_answer-'0';
    }
    while ((usr_answer!=VK_ESCAPE)&&((funcnum<0)));
    if (usr_answer==VK_ESCAPE)
    {
        printf("\n\nExiting - have a nice play!\n");
        return 0;
    }
    printf("%d\n",funcnum);
    switch (funcnum)
    {
    case 1:editGoop(player);break;
    case 2:editSpecialysts(player);break;
    case 3:editHybrids(player);break;
    case 4:editLiveHybrids(save,player_num);break;
    case 5:editPlants(player);break;
    case 6:editLevelNumber(player);break;
    //Import/export
    case 8:
      {
         int result=loadStructureFromFile("playfile.bin",(char *)player, sizeof(*player));
         if (result!=0) return result;
         break;
      }
    case 9:
      {
         int result=saveStructureToFile("playfile.bin",(char *)player, sizeof(*player));
         if (result!=0) return result;
         break;
      }
    default:
         printf("\nUnknown function. Select one of known functions!\n");
         break;
    }
  }
}

int editGlobalGameParams(SAVEDATA *save)
{
  while (1)
  {
    printf("\n\nGlobal game parameters modifications:\n");
    printf("%3d - Reset all game tips\n",1);
    printf("%3s - Exit\n","Esc");
    printf("\nSelect what to edit:");
    int funcnum;
    char usr_answer;
    do
    {
        usr_answer=getch();
        if ((usr_answer<'1')||(usr_answer>'9'))
            funcnum=-1;
        else
            funcnum=usr_answer-'0';
    }
    while ((usr_answer!=VK_ESCAPE)&&((funcnum<0)));
    if (usr_answer==VK_ESCAPE)
    {
        printf("\n\nExiting - have a nice play!\n");
        return 0;
    }
    printf("%d\n",funcnum);
    switch (funcnum)
    {
    case 1:
      {
         int newval;
         printf("hint: Type 0 to enable all tips again, 1 to disable all tips.\n");
         printf("      Tips are shown once when you firs click on a item.\n");
         printf("      To see tips again you can click \"?\" icon or enable them here.\n");
         if ( !getUsrUValue(0,255,&newval) )
             return 1;
         printf("\n");
         int tnum;
         int tips_num=sizeof(save->tipshown);
         for (tnum=0;tnum<tips_num;tnum++)
         {
             save->tipshown[tnum]=(unsigned char)tnum;
         }
         printf("Value %d saved for all %d tips.\n",newval,tips_num);
      };break;
    default:
         printf("\nUnknown function. Select one of known functions!\n");
         break;
    }
  }
}

float countTotalUndergroundGoop(SAVEDATA *save)
{
    unsigned int i,k;
    float total_goop=0.0;
    for (i=0;i<128;i++)
    for (k=0;k<128;k++)
    {
          unsigned int goop=save->mapdata[i][k].goop;
          if (goop>1)
              total_goop+=goop;
    }
    return total_goop;
}

int editGlobalGrowRevealedMap(SAVEDATA *save,char shrink)
{
    if (shrink)
      printf("\n\nHiding some of map\n");
    else
      printf("\n\nRevealing more of map\n");
    unsigned long seg_revealed=0;
    unsigned int i,k;
    for (i=0;i<128;i++)
    for (k=0;k<128;k++)
    {
          unsigned int revealed=save->mapdata[i][k].revealed;
          if (shrink)
            revealed=(revealed>>1)&63;
          else
            revealed=((revealed<<1)|1)&63;
          save->mapdata[i][k].revealed=revealed;
          if (revealed>=63) seg_revealed++;
          if ( (!shrink) && (revealed>=7) )
          {
            unsigned int i_prev=(i+128-1)%128;
            unsigned int i_next=(i+1)%128;
            unsigned int k_prev=(k+128-1)%128;
            unsigned int k_next=(k+1)%128;
            {
              unsigned int adj_rev=save->mapdata[i][k_prev].revealed;
              if (adj_rev<1) save->mapdata[i][k_prev].revealed=1;
            }
            {
              unsigned int adj_rev=save->mapdata[i][k_next].revealed;
              if (adj_rev<1) save->mapdata[i][k_next].revealed=1;
            }
            {
              unsigned int adj_rev=save->mapdata[i_prev][k].revealed;
              if (adj_rev<1) save->mapdata[i_prev][k].revealed=1;
            }
            {
              unsigned int adj_rev=save->mapdata[i_next][k].revealed;
              if (adj_rev<1) save->mapdata[i_next][k].revealed=1;
            }
          }
    }
    printf("Completely revealed map segments: %lu of %lu\n",seg_revealed,(unsigned long)(128*128));
    return 0;
}

int editGlobalMultiplyUndergroundGoop(SAVEDATA *save,float mul_var,char allow_grow)
{
    printf("\n\nMultiplying all underground resources by %f\n",mul_var);
    unsigned int i,k;
    float total_goop=0.0;
    for (i=0;i<128;i++)
      for (k=0;k<128;k++)
      {
          unsigned int prev_goop=save->mapdata[i][k].goop;
          float new_goop=(float)prev_goop*mul_var;
          if (new_goop>255.0) new_goop=255.0;
          //Other algorithms when we're allowing growth of goop to adjacent squares
          if (allow_grow)
          {
            if ( (prev_goop<1.01) || (new_goop<1.0) ) new_goop=1.0;
            if ((new_goop>1.01)&&(new_goop<2.0))
            {
              if (mul_var>1.0)
                new_goop=2.0;
              else
                new_goop=1.0;
            }
            if ( (new_goop>96.0) && (allow_grow) )
            {
                unsigned int i_prev;
                unsigned int i_next;
                unsigned int k_prev;
                unsigned int k_next;
                i_prev=(i+128-1)%128;
                i_next=(i+1)%128;
                k_prev=(k+128-1)%128;
                k_next=(k+1)%128;
                {
                  unsigned int adj_goop=save->mapdata[i][k_prev].goop;
                  if (adj_goop<2) save->mapdata[i][k_prev].goop=2;
                }
                {
                  unsigned int adj_goop=save->mapdata[i][k_next].goop;
                  if (adj_goop<2) save->mapdata[i][k_next].goop=2;
                }
                {
                  unsigned int adj_goop=save->mapdata[i_prev][k].goop;
                  if (adj_goop<2) save->mapdata[i_prev][k].goop=2;
                }
                {
                  unsigned int adj_goop=save->mapdata[i_next][k].goop;
                  if (adj_goop<2) save->mapdata[i_next][k].goop=2;
                }
            }
          } else
          // If growing is not allowed
          {
            if ( (prev_goop<1.01) || (new_goop<1.0) ) new_goop=1.0;
            if ( (new_goop>1.01) && (new_goop<2.0) )
            {
              if (mul_var>1.0)
                new_goop=1.0;
              else
                new_goop=2.0;
            }
          }
          unsigned int new_goop_i=floor(new_goop+0.49);
          save->mapdata[i][k].goop=(unsigned char)new_goop_i;
          if (new_goop_i>1)
              total_goop+=new_goop_i;
      }
    printf("\nTotal amount of underground goop after multiplication: %.0f\n",total_goop);
    return 0;
}

int editGlobalLevelParams(SAVEDATA *save)
{
  while (1)
  {
    float under_goop=countTotalUndergroundGoop(save);
    printf("\n\nGlobal level parameters modifications:\n");
    printf("%3d - Extend underground GOOP resources (currently %.0f)\n",1,under_goop);
    printf("%3d - Decrease underound GOOP resources (halves the amount)\n",2);
    printf("%3d - Reveal more map to user\n",3);
    printf("%3d - Hide some of map\n",4);
    printf("%3d - Modify native alive creatures on this level\n",5);
    printf("Esc - %s\n","Exit");
    printf("\nSelect what to edit:");
    int funcnum;
    char usr_answer;
    do
    {
        usr_answer=getch();
        if ((usr_answer<'1')||(usr_answer>'9'))
            funcnum=-1;
        else
            funcnum=usr_answer-'0';
    }
    while ((usr_answer!=VK_ESCAPE)&&((funcnum<0)));
    if (usr_answer==VK_ESCAPE)
    {
        printf("\n\nExiting - have a nice play!\n");
        return 0;
    }
    printf("%d\n",funcnum);
    switch (funcnum)
    {
    case 1:
         editGlobalMultiplyUndergroundGoop(save,1.6,1);
        break;
    case 2:
         editGlobalMultiplyUndergroundGoop(save,0.6,0);
        break;
    case 3:
         editGlobalGrowRevealedMap(save,0);
        break;
    case 4:
         editGlobalGrowRevealedMap(save,1);
        break;
    case 5:
         editLiveHybrids(save,4);
         break;
    default:
         printf("\nUnknown function. Select one of known functions!\n");
         break;
    }
  }
}

int editSavegame(char *savefname,char is_bare_map)
// Edits a savegame with given file name.
// is_bare_map informs if we're editing game level, not a savegame
{
  if (is_bare_map)
    printf("Reading level...");
  else
    printf("Reading savegame...");
  FILE *savefp;
  savefp = fopen (savefname, "rb");
    if (!savefp)
    {
        printf("\nError: Cannot open savegame/level file \"%s\"!\n",savefname);
        return 1;
    }
    SAVEDATA *save_data=malloc(sizeof(SAVEDATA));
    fread (save_data, sizeof(SAVEDATA), 1, savefp);
    printf("done.\n");
  fclose(savefp);
  while (1)
  {
/*    int mapx=read_int16_le_buf((unsigned char *)&(save_data->mapcoordx));
    int mapy=read_int16_le_buf((unsigned char *)&(save_data->mapcoordy));
    printf("User view map position %dx%d\n",mapx,mapy);*/
    printf("\nSELECT SAVE/LEVEL GAME OPERATION:\n");
    printf("Edit players:\n");
    unsigned int plnum;
    for (plnum=0;plnum<PLAYERS_COUNT;plnum++)
    {
        SAVEDATA_PLAYERINFO *player=&(save_data->players[plnum]);
        int money_val=read_int16_le_buf((unsigned char *)&(save_data->players[plnum].goop));
        int special_num=player->specialysts_num;
        printf("%3d - \"%s\", Goop: %d, Specs: %d\n",plnum+1,&(player->playername),money_val,special_num);
/*        printf("  Specialysts: ");
        int spnum;
        for (spnum=0;spnum<special_num;spnum++)
        {
            SAVEDATA_SPECIALYST *specialyst=&(player->specialysts[spnum]);
            printf("%s",&(specialyst->firstname));
            if ((((spnum+1)%7) == 0))
                printf(",\n%15s"," ");
            else
            if (spnum<special_num-1)
                printf(", ");
        }
        printf(".\n");*/
    }
    printf("%3d - Planet parameters for \"%s\"\n",5,&(save_data->landname));
    printf("%3d - Global parameters for this whole game\n",6);
    printf("%3d - Import all save data from set of BIN files\n",7);
    printf("%3d - Export all save data to set of BIN files\n",8);
    printf("%3d - Save changes into savegame/level file\n",9);
    printf("%s - Exit\n","Esc");
    printf("\nEnter function number:");
    char usr_answer;
    do
    {
        usr_answer=getch();
    }
    while ((usr_answer!=VK_ESCAPE)&&(usr_answer<'1')&&(usr_answer>'9'));
    printf("%c\n",usr_answer);
    int result=0;
    if (usr_answer==VK_ESCAPE)
    {
        printf("\n\nExiting - have a nice play!\n");
        result=1;
    }
    switch (usr_answer)
    {
    case '1':
    case '2':
    case '3':
    case '4':
       result=editPlayer(save_data,usr_answer-'1');
       break;
    case '5':
       result=editGlobalLevelParams(save_data);
       break;
    case '6':
       result=editGlobalGameParams(save_data);
       break;
    //Import/export
    case '7':
      {
         char defaultname[255];
         unsigned int plnum;
         result=0;
        printf("Reading structures...\n");
         for (plnum=0;plnum<PLAYERS_COUNT;plnum++)
         {
           sprintf (defaultname,".\\sav%02dplr.dat",plnum+1);
           result|=loadStructureFromDefFile(defaultname,(char *)&(save_data->players[plnum]), sizeof(SAVEDATA_PLAYERINFO));
         }
         sprintf (defaultname,".\\sav05un3.dat");
         result|=loadStructureFromDefFile(defaultname,(char *)&(save_data->unkn3), sizeof(save_data->unkn3));
         sprintf (defaultname,".\\sav06tip.dat");
         result|=loadStructureFromDefFile(defaultname,(char *)&(save_data->tipshown), sizeof(save_data->tipshown));
         sprintf (defaultname,".\\sav07u20.dat");
         result|=loadStructureFromDefFile(defaultname,(char *)&(save_data->unkn20), sizeof(save_data->unkn20));
         sprintf (defaultname,".\\sav08tv1.dat");
         result|=loadStructureFromDefFile(defaultname,(char *)&(save_data->timevar1), sizeof(save_data->timevar1));
         sprintf (defaultname,".\\sav09map.dat");
         result|=loadStructureFromDefFile(defaultname,(char *)&(save_data->mapdata), sizeof(save_data->mapdata));
         sprintf (defaultname,".\\sav10crt.dat");
         result|=loadStructureFromDefFile(defaultname,(char *)&(save_data->creaturedata), sizeof(save_data->creaturedata));
         sprintf (defaultname,".\\sav11tv2.dat");
         result|=loadStructureFromDefFile(defaultname,(char *)&(save_data->timevar2), sizeof(save_data->timevar2));
         sprintf (defaultname,".\\sav12ob2.dat");
         result|=loadStructureFromDefFile(defaultname,(char *)&(save_data->objdata2), sizeof(save_data->objdata2));
         sprintf (defaultname,".\\sav11tv3.dat");
         result|=loadStructureFromDefFile(defaultname,(char *)&(save_data->timevar3), sizeof(save_data->timevar3));
         sprintf (defaultname,".\\sav13o10.dat");
         result|=loadStructureFromDefFile(defaultname,(char *)&(save_data->objdata10), sizeof(save_data->objdata10));
         sprintf (defaultname,".\\sav14o20.dat");
         result|=loadStructureFromDefFile(defaultname,(char *)&(save_data->objdata20), sizeof(save_data->objdata20));
      };break;
    case '8':
      {
         char defaultname[255];
         unsigned int plnum;
         result=0;
        printf("Writing structures...\n");
         for (plnum=0;plnum<PLAYERS_COUNT;plnum++)
         {
           sprintf (defaultname,".\\sav%02dplr.dat",plnum+1);
           result|=saveStructureToDefFile(defaultname,(char *)&(save_data->players[plnum]), sizeof(SAVEDATA_PLAYERINFO));
         }
         sprintf (defaultname,".\\sav05un3.dat");
         result|=saveStructureToDefFile(defaultname,(char *)&(save_data->unkn3), sizeof(save_data->unkn3));
         sprintf (defaultname,".\\sav06tip.dat");
         result|=saveStructureToDefFile(defaultname,(char *)&(save_data->tipshown), sizeof(save_data->tipshown));
         sprintf (defaultname,".\\sav07u20.dat");
         result|=saveStructureToDefFile(defaultname,(char *)&(save_data->unkn20), sizeof(save_data->unkn20));
         sprintf (defaultname,".\\sav08tv1.dat");
         result|=saveStructureToDefFile(defaultname,(char *)&(save_data->timevar1), sizeof(save_data->timevar1));
         sprintf (defaultname,".\\sav09map.dat");
         result|=saveStructureToDefFile(defaultname,(char *)&(save_data->mapdata), sizeof(save_data->mapdata));
         sprintf (defaultname,".\\sav10crt.dat");
         result|=saveStructureToDefFile(defaultname,(char *)&(save_data->creaturedata), sizeof(save_data->creaturedata));
         sprintf (defaultname,".\\sav11ob1.dat");
         result|=saveStructureToDefFile(defaultname,(char *)&(save_data->objdata1), sizeof(save_data->objdata1));
         sprintf (defaultname,".\\sav12tv2.dat");
         result|=saveStructureToDefFile(defaultname,(char *)&(save_data->timevar2), sizeof(save_data->timevar2));
         sprintf (defaultname,".\\sav13ob2.dat");
         result|=saveStructureToDefFile(defaultname,(char *)&(save_data->objdata2), sizeof(save_data->objdata2));
         sprintf (defaultname,".\\sav14tv3.dat");
         result|=saveStructureToDefFile(defaultname,(char *)&(save_data->timevar3), sizeof(save_data->timevar3));
         sprintf (defaultname,".\\sav15o10.dat");
         result|=saveStructureToDefFile(defaultname,(char *)&(save_data->objdata10), sizeof(save_data->objdata10));
         sprintf (defaultname,".\\sav16o20.dat");
         result|=saveStructureToDefFile(defaultname,(char *)&(save_data->objdata20), sizeof(save_data->objdata20));
      };break;
    case '9':
        if (is_bare_map)
          printf("Writing level...");
        else
          printf("Writing savegame...");
        savefp = fopen (savefname, "wb");
        fwrite (save_data, sizeof(SAVEDATA), 1, savefp);
        printf("done.\n");
        fclose(savefp);
        printf("File has been rewritten.\n");
        result=0;
        break;
    default:
         //printf("\nUnknown function. Select one of known functions!\n");
         break;
    }
    if (result!=0)
    {
      free(save_data);
      return result;
    }
  }
}

int main(int argc, char *argv[])
{
    printf("\nBullfrog Engine Savegame editor for GeneWars %s",VER_STRING);
    printf("\n    written by Tomasz Lis, 2000-2008");
    printf("\n-------------------------------\n");
    printf("Listing savegames...");
    char *saved_names;
    saved_names=malloc(17*SAVES_COUNT);
    char *save_exist_table;
    save_exist_table=malloc(SAVES_COUNT+2);
    getSavesComments(saved_names,save_exist_table);
    save_exist_table[SAVES_COUNT+0]=1;
    save_exist_table[SAVES_COUNT+1]=0;
    printf("done.\n");
    printf("Saved games list:\n");
    int num;
    for (num=0;num<SAVES_COUNT;num++)
    {
        if (save_exist_table[num])
            printf("    %d - \"%s\"\n",num+1,&(saved_names[17*num]));
    }
    printf("    %c - %s\n",'M',"Edit campaing levels");
    printf("    %c - %s\n",'U',"Edit custom levels");
    printf("    %c - %s\n",'C',"Cheats");
    printf("  Esc - %s\n","Exit");
    printf("Enter savegame number:");
    int save_num;
    char usr_answer;
    do
    {
        usr_answer=toupper(getch());
        if ((usr_answer<'1')||(usr_answer>'9'))
            save_num=-1;
        else
            save_num=usr_answer-'1';
    }
    while ((usr_answer!=VK_ESCAPE)&&(usr_answer!='C')&&(usr_answer!='M')&&(usr_answer!='U')&&
          ((save_num<0)||(save_exist_table[save_num]==0)));
    if (usr_answer==VK_ESCAPE)
    {
        printf("\n\nExiting - have a nice play!\n");
        return 0;
    }
    if (usr_answer=='C')
    {
        printf("\n\nCHEAT:\n");
        printf("During the game, type quickly \"SALMONAXE\".\n");
        printf("    This enables cheat mode. You should hear \"Cheat, would you?\".\n");
        printf("When you're in cheat mode, you can press:\n");
        printf("  W - wins the level imediatelly\n");
        printf("  L - Brings monolith at mouse cursor\n      (use them to upgrade specialists/hybrids)\n");
        printf("  C - Gives you access to all purebreds, hybrids and plants\n");
        printf("  T - Makes all structures transparent\n");
        printf("  D - Shows memory statistics\n");
        printf("  U - Shows color palette\n");
        printf("  S - Increases level number. Gives you access to more structures,\n");
        printf("      but skips levels after the current level.\n");
        printf("  F5 - Seeds plant (Duranium bulb) at cursor\n");
        printf("  F6 - Throws grenade at cursor\n");
        printf("  F7 - Shoot at creature\n");
        printf("  F10 - Adds money\n");
        printf("  Alt+V - Shows program version\n");
        printf("  Alt+F9 - Brings Ethereals\n");
        printf("  Alt+F12 - Speeds up time\n");
        printf("  Shift+Z - Reveal map\n");
        printf("Press any key to exit...");
        char x=getch();
        if (x==0) getch();
        printf("\n");
        return 0;
    }
    if (usr_answer=='M')
    {
        printf("\n\nSorry, unfinished.\n");
        printf("Press any key to exit...");
        char x=getch();
        if (x==0) getch();
        printf("\n");
        return 0;
    }
    if (usr_answer=='U')
    {
        printf("\n\nSorry, unfinished.\n");
        printf("Press any key to exit...");
        char x=getch();
        if (x==0) getch();
        printf("\n");
        return 0;
    }
    printf("%d\n\n",save_num);
    char savefname[255];
    getSaveFileName(savefname,gtSave,save_num);
    return editSavegame(savefname,0);
}

