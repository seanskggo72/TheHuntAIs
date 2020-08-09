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

//---------------------- Local Functions ------------------------//

static char *createMessage(int distance);
static void makeRandomMove(HunterView hv);
static void initialPlay(Player current);
// static void defaultPlayerMove(HunterView hv, Player current, PlaceId place);
static PlaceId predictDracDest(HunterView hv);   
static PlaceId alternateRoute(HunterView hv, PlaceId current);

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

// Version 4 collective research decreased the performance drastically
// so the feature was removed

// Version 5 - undergoing major reconstruction. When drac is found, the 
// hunters use whereCanTheyGo function to randomly choose a hypotheical
// plcae the drac can go and make shtat place the target place

void decideHunterMove(HunterView hv) {
   
   // Other important variables initialised
	Player current = HvGetPlayer(hv);
   int round = HvGetRound(hv), trail = 0, health = HvGetHealth(hv, current);
   PlaceId place = HvGetPlayerLocation(hv, current);
   PlaceId DraculaLoc = HvGetLastKnownDraculaLocation(hv, &trail);

   // Default move. Return anywhere without modifying move to play this move
   if (round != 0) makeRandomMove(hv);

   
   // If Drac trail found and is not outdated,
   int dracTrailRound = round - trail;
   if (DraculaLoc != NOWHERE && dracTrailRound <= 8) {
      
   // -------------- Defining Player Target Locations ------------------ //

   // If the latest dracula location is different from the previous, update
   // all hunter targets to a predicted location

      int length = 0;
      PlaceId predictedPlace = predictDracDest(hv); 
      if (predictedPlace == NOWHERE) {
         PlaceId *path = HvGetShortestPathTo(hv, current, DraculaLoc, &length);
         if (length == 0) return;
         else {
            char *name = (char *)placeIdToAbbrev(path[0]);
            char *message = createMessage(length);
            registerBestPlay(name, message);
            return;
         }
      }
      PlaceId *path = HvGetShortestPathTo(hv, current, predictedPlace, &length);
      if (length == 0) return;
      else {
         char *name = (char *)placeIdToAbbrev(path[0]);
         char *message = createMessage(length);
         registerBestPlay(name, message);
         return;
      }
   }

   // *************** Special Case - make resting move *************** // 

   // If player health is critically low, rest for the turn
   if (health <= 3) {
      char *name = (char *)placeIdToAbbrev(place);
      registerBestPlay(name, "Resting");
      return;
   }

   // **************************************************************** // 

   // // If  immature vampire exists, make the closest player 
   // // maybe use function boolean
   // // go after the immature vamp
   // vampLoc = HvGetVampireLocation(hv);
   // if (vampLoc != NOWHERE && vampLoc != CITY_UNKNOWN) {
   //    int length = 0;
   //    HvGetShortestPathTo(hv, current, vampLoc, &length);
   //    if (length > 0 && length < 6) {
   //       char *name = (char *)placeIdToAbbrev(path[0]);
   //       char *message = createMessage(length);
   //       registerBestPlay(name, message);
   //       return;
   //    }
   // }

   // ------------------ Making moves accordingly ------------------ //
   
   // int length = 0;
   // PlaceId *path = HvGetShortestPathTo(hv, current, currentTarget, &length);
   // // If player reaches the last known location and no more trail is found,
   // // Move randomly
   // if (length == 0) {
   //    if (current == PLAYER_LORD_GODALMING)
   //       GplaceReached = true;
   //    else if (current == PLAYER_DR_SEWARD)
   //       SplaceReached = true;
   //    else if (current == PLAYER_VAN_HELSING)
   //       VplaceReached = true;
   //    else if (current == PLAYER_MINA_HARKER)
   //       HplaceReached = true;
   //    return;
   // } else {
   //    char *name = (char *)placeIdToAbbrev(path[0]);
   //    char *message = createMessage(length);
   //    registerBestPlay(name, message);
   //    return;
   // }
   //}

// ----------------- Preliminary Scouting and moving ----------------- //

// If hunter health is critically low

   if (health <= 3) {
      PlaceId place = HvGetPlayerLocation(hv, current);
      char *name = (char *)placeIdToAbbrev(place);
      registerBestPlay(name, "Resting");
      return;
   }

   // If after round 6, the hunters have not found Dracula, use collective
   // research
   if (round != 0 && round % 7 == 0) {
      char *name = (char *)placeIdToAbbrev(place);
      registerBestPlay(name, "Researching...");
      return;
   }

   // initial path of hunters
   if (round == 0) {
      initialPlay(current);
      return;
   // Make random scouting moves
   } else {
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
   int numReturnedLocs = 0;
   PlaceId *validMoves = HvWhereCanIGo(hv, &numReturnedLocs);
   unsigned int seed = (unsigned int)time(NULL);
	srand(seed);
   if (numReturnedLocs == 0) {
      printf("No legal moves for hunter\n");
      exit(EXIT_FAILURE);
   } else {
	   PlaceId random = rand() % numReturnedLocs;
      random = validMoves[random];
      random = alternateRoute(hv, random);
      char *play = (char *)placeIdToAbbrev(random);
	   registerBestPlay(play, "Random move");
   }
}

static void initialPlay(Player current) {
   switch(current) {
      case PLAYER_LORD_GODALMING:
         registerBestPlay("SR", "First move"); 
         break;    
      case PLAYER_DR_SEWARD:
         registerBestPlay("MR", "First move");
         break;
      case PLAYER_VAN_HELSING:
         registerBestPlay("NP", "First move"); 
         break; 
      case PLAYER_MINA_HARKER:
         registerBestPlay("AT", "First move");   
         break;
      case PLAYER_DRACULA: break;
   }
}

// static void defaultPlayerMove(HunterView hv, Player current, PlaceId place) {
//    int pathLength = 0;
//    PlaceId *path = HvGetShortestPathTo(hv, current, place, &pathLength);
//    if (pathLength == 0) {
//       makeRandomMove(hv);
//       return;
//    }
//    char *name = (char *)placeIdToAbbrev(path[0]);
//    registerBestPlay(name, "Scouting");     
//    return;
// }

static PlaceId predictDracDest(HunterView hv) {
   int numOptions;
   PlaceId *options = HvWhereCanTheyGo(hv, PLAYER_DRACULA, &numOptions);
   unsigned int seed = (unsigned int)time(NULL);
	srand(seed);
   if (numOptions == 0) return NOWHERE;
   else {
	   int randomIndex = rand() % numOptions;
      return options[randomIndex];
   }
}

static PlaceId alternateRoute(HunterView hv, PlaceId current) {
   // Dev note: the final array size is supposed to be 3 less thatn numPLAces
   // but because random omovement can affect the possible locations
   // So made it big enough to fit
   PlaceId array[3];
   Player currentPlayer = HvGetPlayer(hv);
   int i = 0, numPlaces;
   bool conflict = false;

   // check if the original current play makes conflict with other players
   for (int j = 0; j < 4; j++) {
      if (j == currentPlayer) continue;
      if (HvGetPlayerLocation(hv, j) == current) conflict = true;
   }
   if (!conflict) return current;
   
   // Else, look for another route
   for (int j = 0; j < 4; j++) {
      if (j == currentPlayer) continue;
      array[i] = HvGetPlayerLocation(hv, j);
      i++;
   }
   i = 0;
   PlaceId *possibilities = HvWhereCanIGo(hv, &numPlaces);
   // supposed to be 3 less but in this case, leave it as is and use
   // PLace is real to determine if locaiton is accessible. Remember to 
   // get rid of the same location
   PlaceId final[numPlaces - 1];
   if (numPlaces <= 3) return current;
   else {
      for (int k = 0; k < numPlaces - 1; k++) {
         conflict = false;
         for (int j = 0; j < 3; j++) {
            if (possibilities[k] == array[j]) conflict = true;
         } 
         if (!conflict && possibilities[k] != current) {
            final[i] = possibilities[k];
            i++;
         }
      }
   }
   if (placeIsReal(final[0])) return final[0];
   else return current;
}

//------------------------- Backup Functions ---------------------------//

/*

*/