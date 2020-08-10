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

// Creates message when Drac location is found and displays its distance
static char *createMessage(int distance);
// Make random move without clashing of locations with other hunters
static void makeRandomMove(HunterView hv);
// Function for initial play
static void initialPlay(Player current);
// Predicts Drac's next destination if drac is encountered
static PlaceId predictDracDest(HunterView hv); 
// Helper function for makeRandomMove. Ensures no location is clashed  
static PlaceId alternateRoute(HunterView hv, PlaceId current);

//---------------------------------------------------------------//

///////////////////////////////////////////////////////////////////////////
//
// Version 1.0 - The hunters scout in predefined paths. If Dracula trail is 
// found, then move towards the most recent location. Else, randomly 
// wander about until the Dracula is found again.
//
// Version 2.0 - The hunters scout and if Dracula path is not found, then
// hunters move about randomly until found.
// If hunter's health falls critically low, then the hunter
// rests for that turn
//
// Version 3.0 - No two hunters can be in the same city.
//
// Version 3.1 - collective research decreased the performance drastically
// so the feature was removed
//
// Version 4.0 - undergoing major reconstruction. When drac is found, the 
// hunters use whereCanTheyGo function to randomly choose a hypothetical
// place the drac can go and make shtat place the target place. Also a lot of
// other functionalities were removed and added
//
// Version 5.0 - Final version. Although there are issues with hunters camping,
// the algorithm makes efficient use of Dracula's last location. After a period
// of time. the hunters realise the last Drac location is redundant and proceeds
// to find the new drac trail. This step also involved removing and adding
// a lot of new functionalities
//
//////////////////////////////////////////////////////////////////////////

void decideHunterMove(HunterView hv) {
   
   // Important game state variables initialised
	Player current = HvGetPlayer(hv);
   int round = HvGetRound(hv), trail = 0, health = HvGetHealth(hv, current);
   PlaceId place = HvGetPlayerLocation(hv, current);
   PlaceId DraculaLoc = HvGetLastKnownDraculaLocation(hv, &trail);

   // If round is 0, make default first move
   if (round == 0) {
      initialPlay(current);
      return;
   }
   
   // Default move. Return anywhere without modifying move to play this move
   if (round != 0) makeRandomMove(hv);

   // *************** Special Case - make resting move *************** // 

   // If player health is critically low, rest for the turn
   if (health <= 3) {
      char *name = (char *)placeIdToAbbrev(place);
      registerBestPlay(name, "Resting");
      return;
   }

   // **************************************************************** //
   
   // -------------- Target acquired - move accordingly ------------------ //

   // If immature vampire exists go after it and dislodge it
   PlaceId vampLoc = HvGetVampireLocation(hv);
   if (vampLoc != NOWHERE && vampLoc != CITY_UNKNOWN) {
      int length = 0;
      PlaceId *path = HvGetShortestPathTo(hv, current, vampLoc, &length);
      if (length > 0 && length < 6) {
         char *name = (char *)placeIdToAbbrev(path[0]);
         registerBestPlay(name, "Come here immature VAMP!!!");
         return;
      }
   }

   // If Drac trail is found and is not outdated, go towards it
   Round dracTrailRound = round - trail;
   if (DraculaLoc != NOWHERE && dracTrailRound <= 8) {
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
      // If already at the last known place, make random move to predicted 
      // drac place
      PlaceId *path = HvGetShortestPathTo(hv, current, predictedPlace, &length);
      if (length == 0) return;
      else {
         char *name = (char *)placeIdToAbbrev(path[0]);
         char *message = createMessage(length);
         registerBestPlay(name, message);
         return;
      }
   }

   // ----------------- Preliminary Scouting and moving ----------------- //

   // If every 6th and 7th round, the hunters have not found Dracula, perform 
   // collective research
   int researchRound = round % 7;
   if (round != 0) {
      if (researchRound == 0 || researchRound == 1) {
         char *name = (char *)placeIdToAbbrev(place);
         registerBestPlay(name, "Researching...");
         return;
      }
   }
   // Else, go along with the random move
   return;
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

//------------------------- Discarded Ideas ---------------------------//

/*
   // Latest location of dracula
   static PlaceId latestFound = NOWHERE;
   static PlaceId vampLoc = NOWHERE;
   static bool vampLocTaken = false;

   // Player target locations
   static PlaceId godalming = LIVERPOOL;
   static PlaceId seward = AMSTERDAM;
   static PlaceId vanHelsing = LEIPZIG;
   static PlaceId harker = BUDAPEST;

   // Player scouting status
   static bool GplaceReached = false;
   static bool SplaceReached = false;
   static bool VplaceReached = false;
   static bool HplaceReached = false;

// -------------- Defining Player Target Locations ------------------ //

   // If the latest dracula location is different from the previous, update
   // all hunter targets to a predicted location
   if (latestFound != DraculaLoc) {
      PlaceId predictedPlace = predictDracDest(hv, latestFound);   
      if (current == PLAYER_LORD_GODALMING) {
         if (predictedPlace == NOWHERE) GplaceReached = true;
         else {
            godalming = predictedPlace;
            GplaceReached = false;
         }
      } else if (current == PLAYER_DR_SEWARD) {
         if (predictedPlace == NOWHERE) SplaceReached = true;
         else {
            seward = predictedPlace;
            SplaceReached = false;
         }
      } else if (current == PLAYER_VAN_HELSING) {
         if (predictedPlace == NOWHERE) VplaceReached = true;
         else {
            vanHelsing = predictedPlace;
            VplaceReached = false;
         }
      } else if (current == PLAYER_MINA_HARKER) {
         if (predictedPlace == NOWHERE) HplaceReached = true;
         else {
            harker = predictedPlace;
            HplaceReached = false;
         }
      }
      // Update the latest found location to new dracula location
      latestFound = DraculaLoc;
   }

   // If  immature vampire exists, make the closest player 
   // go after the immature vamp
   vampLoc = HvGetVampireLocation(hv);
   if (vampLoc == NOWHERE) vampLocTaken = false;
   if (vampLoc != NOWHERE && vampLoc != CITY_UNKNOWN && round % 13 == 0) {
      int length = 0;
      HvGetShortestPathTo(hv, current, vampLoc, &length);
      if (length > 0 && length < 6) {
         if (current == PLAYER_LORD_GODALMING && !vampLocTaken) {
            vampLocTaken = true;
            godalming = vampLoc;
            GplaceReached = false;
         }
         else if (current == PLAYER_DR_SEWARD && !vampLocTaken) {
            vampLocTaken = true;
            seward = vampLoc;
            SplaceReached = false;
         }
         else if (current == PLAYER_VAN_HELSING && !vampLocTaken) {
            vampLocTaken = true;
            vanHelsing = vampLoc;
            VplaceReached = false;
         }
         else if (current == PLAYER_MINA_HARKER && !vampLocTaken)  {
            vampLocTaken = true;
            harker = vampLoc;
            HplaceReached = false;
         }
      }
   }

   // If players have reached their targets, make random move
      if (current == PLAYER_LORD_GODALMING) {
         if (GplaceReached) return;
      } else if (current == PLAYER_DR_SEWARD) {
         if (SplaceReached) return;
      } else if (current == PLAYER_VAN_HELSING) {
         if (VplaceReached) return;
      } else if (current == PLAYER_MINA_HARKER) {
         if (HplaceReached) return;
   }

   int length = 0;
   PlaceId *path = HvGetShortestPathTo(hv, current, currentTarget, &length);
   // If player reaches the last known location and no more trail is found,
   // Move randomly
   if (length == 0) {
      if (current == PLAYER_LORD_GODALMING)
         GplaceReached = true;
      else if (current == PLAYER_DR_SEWARD)
         SplaceReached = true;
      else if (current == PLAYER_VAN_HELSING)
         VplaceReached = true;
      else if (current == PLAYER_MINA_HARKER)
         HplaceReached = true;
      return;
   } else {
      char *name = (char *)placeIdToAbbrev(path[0]);
      char *message = createMessage(length);
      registerBestPlay(name, message);
      return;
   }

   // ----------------- Preliminary Scouting and moving ----------------- //

   // If hunter health is critically low
   if (health <= 3) {
      PlaceId place = HvGetPlayerLocation(hv, current);
      char *name = (char *)placeIdToAbbrev(place);
      registerBestPlay(name, "Resting");
      return;
   }

   // If scouting finished, and Dracula trail is NOT found, move randomly 
   if (current == PLAYER_LORD_GODALMING) {
      if (GplaceReached) return;
   } else if (current == PLAYER_DR_SEWARD) {
      if (SplaceReached) return;
   }  else if (current == PLAYER_VAN_HELSING) {
      if (VplaceReached) return;
   } else if (current == PLAYER_MINA_HARKER) {
      if (HplaceReached) return;
   }
   
   // initial path of hunters
   if (round == 0) {
      initialPlay(current);
      return;
   } else { 
      switch(current) {
         case PLAYER_LORD_GODALMING:
            GplaceReached = defaultPlayerMove(hv, current, godalming);
         case PLAYER_DR_SEWARD:
            SplaceReached = defaultPlayerMove(hv, current, seward);
         case PLAYER_VAN_HELSING:
            VplaceReached = defaultPlayerMove(hv, current, vanHelsing);
         case PLAYER_MINA_HARKER:
            HplaceReached = defaultPlayerMove(hv, current, harker);
         case PLAYER_DRACULA: break;
      } 
      return;
   }

   // Version 2 - If  immature vampire exists, make the closest player 
   // go after the immature vamp
   vampLoc = HvGetVampireLocation(hv);
   if (vampLoc == NOWHERE) vampLocTaken = false;
   else if (vampLoc != CITY_UNKNOWN && round % 13 == 0 && !vampLocTaken) {
      int length = 0;
      HvGetShortestPathTo(hv, current, vampLoc, &length);
      if (length > 0 && length < 6) {
         if (current == PLAYER_LORD_GODALMING && !vampLocTaken) {
            vampLocTaken = true;
            godalming = vampLoc;
            GplaceReached = false;
         }
         else if (current == PLAYER_DR_SEWARD && !vampLocTaken) {
            vampLocTaken = true;
            seward = vampLoc;
            SplaceReached = false;
         }
         else if (current == PLAYER_VAN_HELSING && !vampLocTaken) {
            vampLocTaken = true;
            vanHelsing = vampLoc;
            VplaceReached = false;
         }
         else if (current == PLAYER_MINA_HARKER && !vampLocTaken)  {
            vampLocTaken = true;
            harker = vampLoc;
            HplaceReached = false;
         }
      }
   }

   // initial path of hunters Version 2
   if (round == 0) {
      initialPlay(current);
      return;
   } else { 
      switch(current) {
         case PLAYER_LORD_GODALMING:
            GplaceReached = defaultPlayerMove(hv, current, godalming);
            break;
         case PLAYER_DR_SEWARD:
            SplaceReached = defaultPlayerMove(hv, current, seward);
            break;
         case PLAYER_VAN_HELSING:
            VplaceReached = defaultPlayerMove(hv, current, vanHelsing);\
            break;
         case PLAYER_MINA_HARKER:
            HplaceReached = defaultPlayerMove(hv, current, harker);
            break;
         case PLAYER_DRACULA: break;
      } 
      return;
   }

   static bool defaultPlayerMove(HunterView hv, Player current, PlaceId place) {
      int pathLength = 0;
      if (HvGetPlayerLocation(hv, current) == place) {
         makeRandomMove(hv);
         return true;
      }
      PlaceId *path = HvGetShortestPathTo(hv, current, place, &pathLength);
      char *name = (char *)placeIdToAbbrev(path[0]);
      registerBestPlay(name, "Scouting/heading in direction");     
      return false;
   }

*/
