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
#include <stdbool.h>

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

// Version 2 - The huunters scout and if Dracula path is not found, then
// hunters move about randomly until found.
// If hunter's health falls critically low, then the hunter
// rests for that turn

// Version 3 (upcoming) - No two hunters can be in the same city.

void decideHunterMove(HunterView hv) {
   

   // Static boolean to see whether initial scouting is finished
   // This boolean is a one time use variable and so once changed, it stays
   // changed for the remainder of the game
   static bool scoutFinished = false;
   static PlaceId latestFound = NOWHERE;
   static bool DracPlaceReached = false;
   int round = HvGetRound(hv);
	Player current = HvGetPlayer(hv);
   int health = HvGetHealth(hv, current);
   PlaceId DraculaLoc;
   int trail;
   int *trailpointer = &trail;
   DraculaLoc = HvGetLastKnownDraculaLocation(hv, trailpointer);
   PlaceId place = HvGetPlayerLocation(hv, current);
   
   // Just in case no valid move is made, make a random move as backup move
   if (round != 0)
      makeRandomMove(hv);

   // If Drac trail found,
   if (DraculaLoc != NOWHERE) {

      // If player health is critically low, rest for the turn
      if (health <= 3) {
         char *name = (char *)placeIdToAbbrev(place);
         registerBestPlay(name, "Resting");
         latestFound = DraculaLoc;
         return;
      }

      // If the new Drac trail is different from past trail, make the new 
      // location the target
      if (latestFound != DraculaLoc) 
         DracPlaceReached = false;
      
      // If player reaches the last known location and no more trail is found,
      // Move randomly
      if (DracPlaceReached) {
         makeRandomMove(hv);
         latestFound = DraculaLoc;
         return;
      }

      // TODO: Reject new move if two hunters in the same city. Use messages

      // Else, go towards the last known Drac location
      int distance;
      int *pathLength = &distance;
      PlaceId *path = HvGetShortestPathTo(hv, current, DraculaLoc, pathLength);
      char *name = (char *)placeIdToAbbrev(path[0]);
      char *message = createMessage(distance);
      registerBestPlay(name, message);
      latestFound = DraculaLoc;
      return;
   }

   // TODO: If two hunters in same city, change to new move

   // If hunter health is critically low
   if (health <= 3) {
      PlaceId place = HvGetPlayerLocation(hv, current);
      char *name = (char *)placeIdToAbbrev(place);
      registerBestPlay(name, "Resting");
      return;
   }

   // If scouting finished, and Dracula trail is NOT found, move randomly 
   if (scoutFinished) {
      makeRandomMove(hv);
      return;
   }

   // initial path of hunters
   if (round == 0) {
      switch(current) {
         case PLAYER_LORD_GODALMING:
         registerBestPlay("SR", "First move");     
         return;
      case PLAYER_DR_SEWARD:
         registerBestPlay("MR", "First move");
         return;
      case PLAYER_VAN_HELSING:
         registerBestPlay("NP", "First move");  
         return;
      case PLAYER_MINA_HARKER:
         registerBestPlay("AT", "First move");   
         return;
      case PLAYER_DRACULA:
         return;
      }
   } else { 
      char *name;
      PlaceId *path;
      int count = 0;
      int *pathLength = &count;
      switch(current) {
         case PLAYER_LORD_GODALMING:
         if (HvGetPlayerLocation(hv, PLAYER_LORD_GODALMING) == LIVERPOOL){
            makeRandomMove(hv);
            scoutFinished = true;
            return;
         }
         path = HvGetShortestPathTo(hv, current, LIVERPOOL, pathLength);
         name = (char *)placeIdToAbbrev(path[0]);
         registerBestPlay(name, "Scouting");     
         return;
      case PLAYER_DR_SEWARD:
         if (HvGetPlayerLocation(hv, PLAYER_DR_SEWARD) == AMSTERDAM) {
            makeRandomMove(hv);
            scoutFinished = true;
            return;
         }
         path = HvGetShortestPathTo(hv, current, AMSTERDAM, pathLength);
         name = (char *)placeIdToAbbrev(path[0]);
         registerBestPlay(name, "Scouting");
         return;
      case PLAYER_VAN_HELSING:
         if (HvGetPlayerLocation(hv, PLAYER_VAN_HELSING) == LEIPZIG) {
            makeRandomMove(hv);
            scoutFinished = true;
            return;
         }
         path = HvGetShortestPathTo(hv, current, LEIPZIG, pathLength);
         name = (char *)placeIdToAbbrev(path[0]);
         registerBestPlay(name, "Scouting");  
         return;
      case PLAYER_MINA_HARKER:
         if (HvGetPlayerLocation(hv, PLAYER_MINA_HARKER) == BUDAPEST) {
            makeRandomMove(hv);
            scoutFinished = true;
            return;
         }
         path = HvGetShortestPathTo(hv, current, BUDAPEST, pathLength);
         name = (char *)placeIdToAbbrev(path[0]);
         registerBestPlay(name, "Scouting");   
         return;
      case PLAYER_DRACULA:
         return;
      }
      return;
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

// Updated: Make random move without clashing of location with other hunters
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
	   registerBestPlay(play, "Random move");
   }
}

//------------------------- Backup Functions ---------------------------//

/*

*/