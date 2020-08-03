////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// hunter.c: your "Fury of Dracula" hunter AI.
//
// 2014-07-01	v1.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2017-12-01	v1.1	Team Dracula <cs2521@cse.unsw.edu.au>
// 2018-12-31	v2.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2020-07-10	v3.0	Team Dracula <cs2521@cse.unsw.edu.au>
//
////////////////////////////////////////////////////////////////////////

#include "Game.h"
#include "hunter.h"
#include "HunterView.h"

////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// dracula.c: your "Fury of Dracula" Dracula AI
//
// 2014-07-01	v1.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2017-12-01	v1.1	Team Dracula <cs2521@cse.unsw.edu.au>
// 2018-12-31	v2.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2020-07-10	v3.0	Team Dracula <cs2521@cse.unsw.edu.au>
//
////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#include "hunter.h"
#include "HunterView.h"
#include "Game.h"
#include "Places.h"

typedef struct hunter {
	PlaceId *path;
	int pathcount;
} Hunter;

//---------------------- Local Functions ------------------------//

char *createMessage(int distance);
void makeRandomMove(HunterView hv);

//---------------------------------------------------------------//

// Aim of this algorithm is to corner the Dracula 
// Version 1 - The hunters scout in predefined paths. If Dracula trail is 
// found, then move towards the most recent location. Else, randomly 
// wander about until the Dracula is found again.

void decideHunterMove(HunterView hv) {
   
   makeRandomMove(hv);
   return;
   int round = HvGetRound(hv);
	Player current = HvGetPlayer(hv);

   // If Dracula or trail found, go towards that direction
   PlaceId DraculaLoc;
   int trail;
   int *trailpointer = &trail;
   DraculaLoc = HvGetLastKnownDraculaLocation(hv, trailpointer);
   if (DraculaLoc != NOWHERE) {
      int distance;
      int *pathLength = &distance;
      PlaceId *path = HvGetShortestPathTo(hv, current, DraculaLoc, pathLength);
      char *name = (char *)placeIdToAbbrev(path[0]);
      char *message = createMessage(distance);
      registerBestPlay(name, message);
   }

   // initial path of hunters
   if (round == 0) {
      switch(current) {
         case PLAYER_LORD_GODALMING:
         registerBestPlay("SR", "Bleh");     
         break;
      case PLAYER_DR_SEWARD:
         registerBestPlay("MR", "bLeh");
         break;
      case PLAYER_VAN_HELSING:
         registerBestPlay("NP", "blEh");  
         break;
      case PLAYER_MINA_HARKER:
         registerBestPlay("AT", "bleH");   
         break;
      case PLAYER_DRACULA:
         break;
      }
   } else {
      char *name;
      PlaceId *path;
      switch(current) {
         case PLAYER_LORD_GODALMING:
         if (HvGetPlayerLocation(hv, PLAYER_LORD_GODALMING) == LIVERPOOL)
            makeRandomMove(hv);
         path = HvGetShortestPathTo(hv, current, LIVERPOOL, NULL);
         name = (char *)placeIdToAbbrev(path[0]);
         registerBestPlay(name, "Bleh");     
         break;
      case PLAYER_DR_SEWARD:
         if (HvGetPlayerLocation(hv, PLAYER_DR_SEWARD) == AMSTERDAM)
            makeRandomMove(hv);
         path = HvGetShortestPathTo(hv, current, AMSTERDAM, NULL);
         name = (char *)placeIdToAbbrev(path[0]);
         registerBestPlay(name, "bLeh");
         break;
      case PLAYER_VAN_HELSING:
         if (HvGetPlayerLocation(hv, PLAYER_VAN_HELSING) == LEIPZIG) 
            makeRandomMove(hv);
         path = HvGetShortestPathTo(hv, current, LEIPZIG, NULL);
         name = (char *)placeIdToAbbrev(path[0]);
         registerBestPlay(name, "blEh");  
         break;
      case PLAYER_MINA_HARKER:
         if (HvGetPlayerLocation(hv, PLAYER_MINA_HARKER) == BUDAPEST) 
            makeRandomMove(hv);
         path = HvGetShortestPathTo(hv, current, BUDAPEST, NULL);
         name = (char *)placeIdToAbbrev(path[0]);
         registerBestPlay(name, "bleH");   
         break;
      case PLAYER_DRACULA:
         break;
      }
   }
}	

//---------------------- Local Functions ------------------------//

char *createMessage(int distance) {
   if (distance == 1) return "Distance of 1";
   else if (distance == 2) return "Distance of 2";
   else if (distance == 3) return "Distance of 3";
   else if (distance == 4) return "Distance of 4";
   else if (distance == 5) return "Distance of 5";
   else return "Nothing new";
}

void makeRandomMove(HunterView hv) {
	int numValidMoves;
   int *numReturnedLocs = &numValidMoves;
   PlaceId *validMoves = HvWhereCanIGo(hv, numReturnedLocs);
   unsigned int seed = (unsigned int)time(NULL);
	srand(seed);
   if (numValidMoves == 0) {
      printf("No legal moves for hunter\n");
      exit(EXIT_FAILURE);
   } else {
	   int randomIndex = rand() % numValidMoves;
      char *play = (char *)placeIdToAbbrev(validMoves[randomIndex]);
   }
   printf("%s\n", play);
	registerBestPlay(play, "YOLO");
   return;
}

//------------------------- Backup Functions ---------------------------//

/*

// Random move generator 
void makeRandomMove(HunterView hv, PlaceId *validMoves, int numValidMoves) 
{
	// Use time function to get random seed
	unsigned int seed = (unsigned int) time(NULL);
	srand(seed);
	int randomIndex = rand() % numValidMoves;
	char *play = (char *) placeIdToAbbrev(validMoves[randomIndex]);
	registerBestPlay(play, "YOLO");
	return; 
}

void randomMove(HunterView hv) {
   int places;
   int *numReturnedLocs = &places;
   PlaceId *options = HvWhereCanIGo(hv, numReturnedLocs);
   int random = rand();
   printf("%d\n", places);
   if (places == 0) {
      random = 0;
   } else {
      random = random % places;
   }
   printf("%d", random);
   char *name = (char *)placeIdToAbbrev(options[random]);
   registerBestPlay(name, "Random Move");
}

*/