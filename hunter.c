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

static char *createMessage(int distance);
static void makeRandomMove(HunterView hv);
static void initialPlay(Player current);
static bool defaultPlayerMove(HunterView hv, Player current, PlaceId place);

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
   
   // Static variables to check certain conditions are met
   static bool DracPlaceReached = false;
   static PlaceId latestFound = NOWHERE;

   // Player target locations
   static PlaceId godalming = LIVERPOOL;
   static PlaceId seward = AMSTERDAM;
   static PlaceId vanHelsing = LEIPZIG;
   static PlaceId harker = BUDAPEST;

   // Player scouting status
   static bool GscoutFinished = false;
   static bool SscoutFinished = false;
   static bool VscoutFinished = false;
   static bool HscoutFinished = false;
   
   // Other important variables initialised
	Player current = HvGetPlayer(hv);
   int round = HvGetRound(hv), trail = 0, health = HvGetHealth(hv, current);
   PlaceId place = HvGetPlayerLocation(hv, current);;
   PlaceId DraculaLoc = HvGetLastKnownDraculaLocation(hv, &trail);

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
      
      // TODO: Reject new move if two hunters in the same city. Use messages

      // Else, go towards the last known Drac location
      int distance;
      int *pathLength = &distance;
      PlaceId *path = HvGetShortestPathTo(hv, current, DraculaLoc, pathLength);
      if (distance == 0) DracPlaceReached = true;
      // If player reaches the last known location and no more trail is found,
      // Move randomly
      if (DracPlaceReached) {
         makeRandomMove(hv);
         latestFound = DraculaLoc;
         return;
      } else {
         char *name = (char *)placeIdToAbbrev(path[0]);
         char *message = createMessage(distance);
         registerBestPlay(name, message);
         latestFound = DraculaLoc;
         return;
      }
   }

   // TODO: If two hunters in same city, change to new move

   // TODO: If  immature vampire exists, make the closest player 
   // go after the immature vamp
   PlaceId vampLoc = HvGetVampireLocation(hv);
   if (vampLoc != NOWHERE && vampLoc != CITY_UNKNOWN) {
      int length = 0;
      HvGetShortestPathTo(hv, current, vampLoc, &length);
      if (length < 6) {
         if (current == PLAYER_LORD_GODALMING) 
            godalming = vampLoc;
         else if (current == PLAYER_DR_SEWARD)
            seward = vampLoc;
         else if (current == PLAYER_VAN_HELSING)
            vanHelsing = vampLoc;
         else if (current == PLAYER_MINA_HARKER) 
            harker = vampLoc;
      }
   }

   // If hunter health is critically low
   if (health <= 3) {
      PlaceId place = HvGetPlayerLocation(hv, current);
      char *name = (char *)placeIdToAbbrev(place);
      registerBestPlay(name, "Resting");
      return;
   }

   // If scouting finished, and Dracula trail is NOT found, move randomly 
   if (current == PLAYER_LORD_GODALMING) {
      if (GscoutFinished) {
         makeRandomMove(hv);
         return;
      }
   } else if (current == PLAYER_DR_SEWARD) {
      if (SscoutFinished) {
         makeRandomMove(hv);
         return;
      }
   }  else if (current == PLAYER_VAN_HELSING) {
      if (VscoutFinished) {
         makeRandomMove(hv);
         return;
      }
   } else if (current == PLAYER_MINA_HARKER) {
      if (HscoutFinished) {
         makeRandomMove(hv);
         return;
      }
   }
   
   // initial path of hunters
   if (round == 0) {
      initialPlay(current);
      return;
   } else { 
      switch(current) {
         case PLAYER_LORD_GODALMING:
            GscoutFinished = defaultPlayerMove(hv, current, godalming);
         case PLAYER_DR_SEWARD:
            SscoutFinished = defaultPlayerMove(hv, current, seward);
         case PLAYER_VAN_HELSING:
            VscoutFinished = defaultPlayerMove(hv, current, vanHelsing);
         case PLAYER_MINA_HARKER:
            HscoutFinished = defaultPlayerMove(hv, current, harker);
         case PLAYER_DRACULA: break;
      } 
      return;
   }
}	

//---------------------- Local Functions ------------------------//

static char *createMessage(int distance) {
   if (distance == 0) return "I'm at the target";
   else if (distance == 1) return "Distance of 1";
   else if (distance == 2) return "Distance of 2";
   else if (distance == 3) return "Distance of 3";
   else if (distance == 4) return "Distance of 4";
   else if (distance == 5) return "Distance of 5";
   else return "Target far away";
}

// Updated: Make random move without clashing of location with other hunters
static void makeRandomMove(HunterView hv) {
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

static void initialPlay(Player current) {
   switch(current) {
      case PLAYER_LORD_GODALMING:
         registerBestPlay("SR", "First move");     
      case PLAYER_DR_SEWARD:
         registerBestPlay("MR", "First move");
      case PLAYER_VAN_HELSING:
         registerBestPlay("NP", "First move");  
      case PLAYER_MINA_HARKER:
         registerBestPlay("AT", "First move");   
      case PLAYER_DRACULA: break;
   }
}

static bool defaultPlayerMove(HunterView hv, Player current, PlaceId place) {
   int pathLength = 0;
   if (HvGetPlayerLocation(hv, PLAYER_LORD_GODALMING) == place) {
      makeRandomMove(hv);
      return true;
   }
   PlaceId *path = HvGetShortestPathTo(hv, current, place, &pathLength);
   char *name = (char *)placeIdToAbbrev(path[0]);
   registerBestPlay(name, "Scouting/heading in direction");     
   return false;
}

//------------------------- Backup Functions ---------------------------//

/*

*/