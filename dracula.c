/////////////////////////////////////////////////////////////////////////
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
#include <stdbool.h>

#include "dracula.h"
#include "DraculaView.h"
#include "Game.h"
#include "Places.h"
#include "Map.h"
#include "Queue.h"

#define HUNTER_NUM 4
#define LOW_HEALTH 12
#define MID_HEALTH 25
#define VERY_LOW_HEALTH 4
#define DOUBLE_BACK_NUM 5
#define ARBITRARY_LARGE_NUMBER 100000

// Local "movement" related functions
void doFirstMove(DraculaView dv);
void makeRandomMove(DraculaView dv, PlaceId *validMoves, int *numValidMoves);
void rmMovesToHunters(DraculaView dv, PlaceId *validMoves, int *numValidMoves);
void goToCastleDrac(DraculaView dv, Map map, PlaceId *validMoves, 
	int *numValidMoves, bool *moveMade, bool highHealth);
void moveAwayFromClosestHunters(DraculaView dv, Map map, PlaceId *validMoves, 
    int *numValidMoves);
void seaMoves(DraculaView dv, PlaceId *validMoves, int *numValidMoves, 
    Map map, bool *moveMade);
void goToCastleDrac(DraculaView dv, Map map, PlaceId *validMoves, 
	int *numValidMoves, bool *moveMade, bool highHealth);
void beenToCDRecentlyCheck(DraculaView dv, Map map, PlaceId *validMoves, 
	int *numValidMoves, bool *moveMade);
    
// Local utility functions
PlaceId *findPathBFS(Map map, PlaceId src, PlaceId dest, bool getPath, 
    int *hops, bool *canFree);
int playerToDrac(Map map, DraculaView dv, Player player, PlaceId DracLoc);
Player findClosestPlayer(DraculaView dv, Map map, int *distance, 
	int playerToDracDistance[4]);
void removeMove(DraculaView dv, PlaceId move, PlaceId *validMoves, 
	int *numValidMoves);
bool isValidMove(PlaceId move, PlaceId *validMoves, int *numValidMoves);
void bubbleSort(int *a, int max);
bool isOnMainland(PlaceId location);
bool isDracOnSea(DraculaView dv, Map map);
void removeSeaMoves(DraculaView dv, PlaceId *validMoves, int *numValidMoves);
void removeHide(DraculaView dv, PlaceId *validMoves, int *numValidMoves);
bool safeToGoCastleDrac(DraculaView dv, Map map, bool highHealth);
//

void decideDraculaMove(DraculaView dv)
{
	int round = DvGetRound(dv);
	static int numSeaMovesMade = 0; 
	if (round == 0) {
		doFirstMove(dv);
		return;
	}
	
	int numValidMoves = 0;
	PlaceId *validMoves = DvGetValidMoves(dv, &numValidMoves);
	
	// Random first move with slight error checking
	if (numValidMoves == 0) {
		registerBestPlay("TP", "Mwahahahaha");
		return;
	} else {
		makeRandomMove(dv, validMoves, &numValidMoves);
	}
	
	Map map = DvGetMap(dv); 
	int distanceToClose = 0;
	int temp[4];
	findClosestPlayer(dv, map, &distanceToClose, temp);
	PlaceId dracLoc = DvGetPlayerLocation(dv, PLAYER_DRACULA);
	
	if (round % 13 == 0 && distanceToClose > 3 && isOnMainland(dracLoc)) {
		// remove all sea moves
		removeSeaMoves(dv, validMoves, &numValidMoves);
		makeRandomMove(dv, validMoves, &numValidMoves);
	}
	
	// don't hide if players are close
	if (distanceToClose >= 2 && distanceToClose <= 3) {
		removeMove(dv, HIDE, validMoves, &numValidMoves);
		makeRandomMove(dv, validMoves, &numValidMoves);
	}	
	
	// if drac's loc is a sea loc, increment numSeaMovesMade
	if (placeIsSea(dracLoc)) {
		numSeaMovesMade++;
	}
	
	
	// If drac has been to CD recently, we must be careful about what moves 
	// we make...
	if (!(DvGetHealth(dv, PLAYER_DRACULA) <= LOW_HEALTH)) {
		bool moveMade = false;
		beenToCDRecentlyCheck(dv, map, validMoves, &numValidMoves, &moveMade);
		if (moveMade) return;
	}
	
	if (numSeaMovesMade >= 3) {
		removeSeaMoves(dv, validMoves, &numValidMoves);
		makeRandomMove(dv, validMoves, &numValidMoves);
		numSeaMovesMade =  (numValidMoves != 0) ? 0 : 4;
	}	
	
	// Don't go to locations occupied by hunters
	rmMovesToHunters(dv, validMoves, &numValidMoves); 
	 
	// Now calculate even better moves
	
	// If player is right next to drac, remove HIDES and DOUBLE_BACK_1s...
	if (distanceToClose < 3 ) removeHide(dv, validMoves, &numValidMoves);
	
	// prevent drac going to ATHENS if possible...only if he's on mainland
	if (isOnMainland(DvGetPlayerLocation(dv, PLAYER_DRACULA))) {
		removeMove(dv, ATHENS, validMoves, &numValidMoves);
	}
	
	
	// drac does different moves if he has low health
	if (DvGetHealth(dv, PLAYER_DRACULA) <= LOW_HEALTH) {
		bool moveMade = false;
		// check if drac is not on mainland and get him back
		// if he is, don't let him leave...
	    seaMoves(dv, validMoves, &numValidMoves, map, &moveMade);
		if (moveMade) return;
		else goToCastleDrac(dv, map, validMoves, &numValidMoves, &moveMade, 
			false);
		if (moveMade) return;
		else moveAwayFromClosestHunters(dv, map, validMoves, &numValidMoves);
		return;
	} else if (DvGetHealth(dv, PLAYER_DRACULA) < 30) {
		bool moveMade = false;
		goToCastleDrac(dv, map, validMoves, &numValidMoves, &moveMade, true);
		if (moveMade) return;
	} else {
		removeMove(dv, CASTLE_DRACULA, validMoves, &numValidMoves);
	}
	
	moveAwayFromClosestHunters(dv, map, validMoves, &numValidMoves);
	
	free(validMoves);
}

// Functions that handle AI "movement"

// makes drac do his initial move
void doFirstMove(DraculaView dv) 
{
	registerBestPlay("MR", "Don't try and catch me");
	return;
}

// make a random move from validMoves
void makeRandomMove(DraculaView dv, PlaceId *validMoves, int *numValidMoves) 
{
	// take care of mod 0 case
	if (*numValidMoves == 0) {
		return;
	}
	
	// Use time function to get random seed
	unsigned int seed = (unsigned int) time(NULL);
	srand(seed);
	int randomIndex = rand() % *numValidMoves;
	char *play = (char *) placeIdToAbbrev(validMoves[randomIndex]);
	registerBestPlay(play, "Mwahahahaha");
	return; 
}

// remove all moves that would place in a city occupied by the hunters
void rmMovesToHunters(DraculaView dv, PlaceId *validMoves, int *numValidMoves) 
{
	PlaceId helsingLoc = DvGetPlayerLocation(dv, PLAYER_VAN_HELSING);
	PlaceId sewardLoc = DvGetPlayerLocation(dv, PLAYER_DR_SEWARD);
	PlaceId godalmingLoc = DvGetPlayerLocation(dv, PLAYER_LORD_GODALMING);
	PlaceId harkerLoc = DvGetPlayerLocation(dv, PLAYER_MINA_HARKER);
	
	removeMove(dv, helsingLoc, validMoves, numValidMoves);
	removeMove(dv, sewardLoc, validMoves, numValidMoves);
	removeMove(dv, godalmingLoc, validMoves, numValidMoves);
	removeMove(dv, harkerLoc, validMoves, numValidMoves);
	
	makeRandomMove(dv, validMoves, numValidMoves);
	
	// remove all the moves to locations hunters can reach too!
	for (int i = 0; i < HUNTER_NUM; i++) {
		int numLocs = 0;
		PlaceId *hunterReachable = DvWhereCanTheyGo(dv, i, &numLocs);
		for (int i = 0; i < *numValidMoves; i++)
			for (int j = 0; j < numLocs; j++)
				if (validMoves[i] == hunterReachable[j])
					removeMove(dv, validMoves[i], validMoves, numValidMoves);
				else if (isDoubleBack(validMoves[i])) {
					PlaceId doubleBack = resolveDoubleBack(dv, validMoves[i]);
					if (doubleBack == hunterReachable[j])
						removeMove(dv, doubleBack, validMoves, numValidMoves);
				}
	}
	
	makeRandomMove(dv, validMoves, numValidMoves);
}

// handle the behaviour of drac related to the sea 
// when he is at low health
void seaMoves(DraculaView dv, PlaceId *validMoves, int *numValidMoves, 
    Map map, bool *moveMade)
{   
    int dracHealth = DvGetHealth(dv, PLAYER_DRACULA);
    PlaceId dracLoc = DvGetPlayerLocation(dv, PLAYER_DRACULA);
    // if health is <= VERY_LOW_HEALTH and on great britain, remain on 
    // land no matter what
    if (dracHealth <= VERY_LOW_HEALTH && isOnMainland(dracLoc) == false) {
        int i = 0;
        for (; i < *numValidMoves; i++) {
            if (placeIdToType(validMoves[i]) == SEA) {
                removeMove(dv, validMoves[i], validMoves, numValidMoves);
            } else {
            	// We must consider DOUBLE_BACK moves!!!
            	if (!isDoubleBack(validMoves[i])) continue;
            	else {
            		// if the double back move is a sea move, remove it!
            		PlaceId move = resolveDoubleBack(dv, validMoves[i]);
            		if (placeIsSea(move)) {
            			removeMove(dv, move, validMoves, numValidMoves);
            		}
            	}
            }
        }
        
    } else if (dracHealth <= LOW_HEALTH && 
    	isOnMainland(dracLoc) == false) {
        
        // if health is <LOW_HEALTH and on great britain, take shortest route 
        // to mainland
        if (dracLoc == MANCHESTER) {
            moveAwayFromClosestHunters(dv, map, validMoves, numValidMoves);
            *moveMade = true;
        } else if (dracLoc == LIVERPOOL) {
        	removeMove(dv, IRISH_SEA, validMoves, numValidMoves);
        } else {
            int i = 0;
            for (; i < *numValidMoves; i++) {
                if (placeIdToType(validMoves[i]) == SEA) {
			        char *play = (char *) placeIdToAbbrev(validMoves[i]);
			        registerBestPlay(play, "OFF TO THE SEVEN SEAS");
			        *moveMade = true;
                }
            }            
        }
    } else if (dracHealth <= LOW_HEALTH && isDracOnSea(dv, map) == true) {
        // GET TO MAINLAND ASAP, remove all Great Britain moves
        
        // if drac is at the Irsih sea force him to go to Atlantic Ocean
		if (dracLoc == IRISH_SEA) {
			if (isValidMove(ATLANTIC_OCEAN, validMoves, numValidMoves))
				registerBestPlay("AO", "A tricky little situation");
			*moveMade = true;
			return;
		}
		// else remove non-mainland moves
        int i = 0;
        for (; i < *numValidMoves; i++) {
            if (isOnMainland(validMoves[i]) == false) {
		        removeMove(dv, validMoves[i], validMoves, numValidMoves);
            }
        }         
    } else if (dracHealth <= LOW_HEALTH && isOnMainland(dracLoc) == true) {
        // if health is <= LOW_HEALTH and on mainland do not go to sea
        int i = 0;
        for (; i < *numValidMoves; i++) {
            if (placeIdToType(validMoves[i]) == SEA) {
                removeMove(dv, validMoves[i], validMoves, numValidMoves);
            } else {
            	// We must consider DOUBLE_BACK moves!!!
            	if (!isDoubleBack(validMoves[i])) continue;
            	else {
            		// if the double back move is a sea move, remove it!
            		PlaceId move = resolveDoubleBack(dv, validMoves[i]);
            		if (placeIsSea(move)) {
            			removeMove(dv, move, validMoves, numValidMoves);
            		}
            	}
            }
        }        
    }
    return;
}

// take the fastest route to castle drac if possible...
void goToCastleDrac(DraculaView dv, Map map, PlaceId *validMoves, 
	int *numValidMoves, bool *moveMade, bool highHealth) {
	if (safeToGoCastleDrac(dv, map, highHealth) == false) return;
	int distance = 0;
	PlaceId *path2;
	bool canFree2 = false;
	PlaceId dracLoc = DvGetPlayerLocation(dv, PLAYER_DRACULA);
	if (placeIsReal(dracLoc)) {
		path2 = findPathBFS(map, dracLoc, CASTLE_DRACULA, false, &distance, 
			&canFree2);
	}
	if (canFree2) free (path2);
    for (int i = 0; i < *numValidMoves; i++) {
    	int hops = 0;
        bool canFree = false;
        PlaceId *path;
        if (placeIsReal(validMoves[i])) {
       		path = findPathBFS(map, validMoves[i], CASTLE_DRACULA, false, &hops, 
       			&canFree);
   		} 
   		
        if (hops > distance && placeIsReal(validMoves[i])) {
			char *play = (char *) placeIdToAbbrev(validMoves[i]);
			registerBestPlay(play, "Gotta go fast");
			distance = hops;
        }
        if (canFree) free(path);
    }
    
	return;
}

// checks if Drac can move towards CD
bool safeToGoCastleDrac(DraculaView dv, Map map, bool highHealth)
{
    int distance = ARBITRARY_LARGE_NUMBER;
    int hunterLoc[4] = {0};
    for (int i = 0; i < HUNTER_NUM; i++) {
    	hunterLoc[i] = DvGetPlayerLocation(dv, i);
    }
    
    
   	// get distances to all hunters
   	for (int i = 0; i < HUNTER_NUM; i++) {
   		int tempDistance = 0;
   		bool canFree = false;
   		findPathBFS(map, hunterLoc[i], CASTLE_DRACULA, false, 
    		&tempDistance, &canFree);
    	if (tempDistance < distance) distance = tempDistance;
   	}
   
   
   
    if (highHealth) return (distance > 7) ? true : false;
    else return (distance > 5) ? true : false;
}

// If drac has been to CD recently and his health is high, don't go back!
// Otherwise 
void beenToCDRecentlyCheck(DraculaView dv, Map map, PlaceId *validMoves, 
	int *numValidMoves, bool *moveMade)
{
	PlaceId dracLoc = DvGetPlayerLocation(dv, PLAYER_DRACULA);
	
	if (dracLoc == CASTLE_DRACULA && DvGetHealth(dv, PLAYER_DRACULA) <= 
		MID_HEALTH) {
		
		if (safeToGoCastleDrac(dv, map, true)) {
			if (isValidMove(DOUBLE_BACK_1, validMoves, numValidMoves)) {
				registerBestPlay("D1", "I am restored!");
				*moveMade = true;
				return;
			} else if (isValidMove(HIDE, validMoves, numValidMoves)) {
				registerBestPlay("HI", "I am restored!");
				*moveMade = true;
				return;
			}
		}
	} else if (dracLoc == CASTLE_DRACULA) {
		removeMove(dv, DOUBLE_BACK_1, validMoves, numValidMoves);
		removeMove(dv, HIDE, validMoves, numValidMoves);
	} else {
		removeMove(dv, CASTLE_DRACULA, validMoves, numValidMoves);
		// remove doubleBacks
		for (int i = 0; i < DOUBLE_BACK_NUM; i++)
			removeMove(dv, (DOUBLE_BACK_1 + i), validMoves, numValidMoves);
	}
	
	makeRandomMove(dv, validMoves, numValidMoves);
	return;
}


// Utility functions

// returns a path from src to dest in reverse order (i.e. starting with dest).
PlaceId *findPathBFS(Map map, PlaceId src, PlaceId dest, bool getPath, 
    int *hops, bool *canFree)
{
    PlaceId visited[NUM_REAL_PLACES];
    for (int i = 0; i < MAX_REAL_PLACE; i++) {
        visited[i] = -1;
    }
    bool found = false;
    visited[src] = src;
    Queue q = QueueNew();
    QueueEnqueue(q, src);
    while (found == false && QueueIsEmpty(q) == false) {
        PlaceId v = QueueDequeue(q);
        if (v == dest) {
            found = true;
        } else {
            ConnList edges = MapGetConnections(map, v);
            while (edges != NULL) {
                if (visited[edges->p] == -1) {
                	visited[edges->p] = v;
                	QueueEnqueue(q, edges->p);
                }
                edges = edges->next;
            }
        }
    }
    QueueDrop(q);
    if (found == true) {
        int total = 0;
        PlaceId current = dest;
        while (current != src) {
            current = visited[current];
            total++;
        }
        // take care of last case..
        total++;
        
        *hops = total;
        if (getPath == true) {
            PlaceId *Path = malloc(sizeof(PlaceId)*(total));
            int j = total - 1;   
            current = dest;
            while (j >= 0) {
                Path[j] = current;
                current = visited[current]; 
                j--;
            }
            *canFree = true;
            return Path;
        } else {
            *canFree = false;
            return NULL;
        }   
    } else {
        *canFree = false;
        return NULL;
    }
}

// find and return the closest player 
// also takes in an array that can be used to get distances from dracLoc
// to all players
Player findClosestPlayer(DraculaView dv, Map map, int *distance, 
	int playerToDracDistance[4])
{   
    PlaceId DracLoc = DvGetPlayerLocation(dv, PLAYER_DRACULA);
    playerToDracDistance[0] = playerToDrac(map, dv, PLAYER_LORD_GODALMING, 
    	DracLoc);
    playerToDracDistance[1] = playerToDrac(map, dv, PLAYER_DR_SEWARD, DracLoc);
    playerToDracDistance[2] = playerToDrac(map, dv, PLAYER_VAN_HELSING, 
    	DracLoc);
    playerToDracDistance[3] = playerToDrac(map, dv, PLAYER_MINA_HARKER, 
    	DracLoc);
    int minimum = playerToDracDistance[0];
    Player closest;
    for (int i = 0; i < HUNTER_NUM; i++) {
        if (playerToDracDistance[i] < minimum) {
            minimum = playerToDracDistance[i];
            *distance = minimum;
            closest = i;    
        }
    }
    
    return closest;
}

// helper function for findClosestPlayer
// gets distance from each player to drac
int playerToDrac(Map map, DraculaView dv, Player player, PlaceId DracLoc)
{
    int hops = -1;
    bool canFree = false;
    bool getPath = false;
    PlaceId playerLoc = DvGetPlayerLocation(dv, player);
    PlaceId *Path = findPathBFS(map, playerLoc, DracLoc, getPath, 
        &hops, &canFree);
    if (canFree == true) {
        free(Path);
    }
    return hops;
}

// remove a move from the validMoves array
void removeMove(DraculaView dv, PlaceId move, PlaceId *validMoves, 
	int *numValidMoves) 
{
	bool foundMove = false;
	int i = 0;
	for (; i < *numValidMoves; i++) {
		if (validMoves[i] == move) {
			foundMove = true;
			break;
		} else if (isDoubleBack(validMoves[i])) {
			PlaceId doubleBack = resolveDoubleBack(dv, validMoves[i]);
    		if (doubleBack == move) {
    			foundMove = true;
    			break;
    		}
		}
	}
	
	if (foundMove) {
		for (; i < (*numValidMoves - 1); i++) {
			validMoves[i] = validMoves[i + 1];
		}
		*numValidMoves -= 1;
	}
	
	return; 
}

// checks if a given move is valid 
bool isValidMove(PlaceId move, PlaceId *validMoves, int *numValidMoves) 
{
	int i = 0;
	for (; i < *numValidMoves; i++) {
		if (validMoves[i] == move) {
			return true;
		}
	}
    return false;
}

// drac should maximise his distance from the two closest hunters!
void moveAwayFromClosestHunters(DraculaView dv, Map map, PlaceId *validMoves,
	int *numValidMoves)
{
	int hunterToDracDistance[4];
	int hunterLocs[4];
	char *bestPlay = NULL;
	
	// Get hunter locations
	for (int i = 0; i < (HUNTER_NUM - 2); i++) 
		hunterLocs[i] = DvGetPlayerLocation(dv, i);
			
	bubbleSort(hunterToDracDistance, HUNTER_NUM);
	// outer loop checks closest hunter, then closest 2 hunters,
	for (int j = 1; j <= (HUNTER_NUM - 1); j++) {
		int i = 0;
		
		// this is the sum of distance from dracula to each hunter 
		// being considered
		int dracToHunterTotal = 0;
		for(int counter = 0; counter < j; counter++) 
			dracToHunterTotal += hunterToDracDistance[counter];
		
		// this loop checks all of drac's valid moves
		for (; i < *numValidMoves; i++) {
			int validMoveToHunterDist = 0;
		    bool canFree = false;
		    PlaceId *path;
		    // inner loop compares validMoves[i] to the locs of hunters
		    for (int counter = 0; counter < j; counter++) {
		    	int tempDistance = 0;
				if (placeIsReal(validMoves[i])) {
			   		path = findPathBFS(map, validMoves[i], hunterLocs[counter], 
			   			false, &tempDistance, &canFree);
		   		}
		   		validMoveToHunterDist += tempDistance;
	   		}
	   		
		    if (validMoveToHunterDist > dracToHunterTotal) {
				bestPlay = (char *) placeIdToAbbrev(validMoves[i]);
				dracToHunterTotal = validMoveToHunterDist;
		    }
		    if (canFree) free(path);
		}
		// we've been overwriting bestPlay with the best play until now
		// so this IS the best play..
		if (bestPlay != NULL) {
			registerBestPlay(bestPlay, "Just try and catch me");
		}
	}
}

// sorts an array of size max
void bubbleSort(int *a, int max) 
{
	int i, j, nswaps;
	for (i = 0; i < max; i++) {
		nswaps = 0;
		for (j = max; j > i; j--) {
			if (a[j] < a[j - 1]) {
				//swap
				int temp = a[j];
				a[j] = a[j - 1];
				a[j - 1] = temp;
				nswaps++;
			}
		}
		if (nswaps == 0) break;
	} 
}

// checks if a Loc is on the "maindland"
bool isOnMainland(PlaceId location)
{
    if (location == LONDON || location == PLYMOUTH || 
        location == SWANSEA || location == LIVERPOOL || 
        location == MANCHESTER || location == EDINBURGH || 
        location == DUBLIN || location == GALWAY) {
        return false;
    } else {
        return true;
    }
}

// checks if dracula is at sea
bool isDracOnSea(DraculaView dv, Map map)
{
    PlaceId dracLoc = DvGetPlayerLocation(dv, PLAYER_DRACULA);
    if (placeIdToType(dracLoc) == SEA) {
        return true;
    } else {
        return false;
    }
}

// removes all sea moves from validMoves
void removeSeaMoves(DraculaView dv, PlaceId *validMoves, int *numValidMoves) 
{
	for (int i = 0; i < *numValidMoves; i++) {
		// remove Sea Moves
		if (placeIsSea(validMoves[i])) {
			removeMove(dv, validMoves[i], validMoves, numValidMoves);
		} else if (isDoubleBack(validMoves[i])) {
    		// if the double back move is a sea move, remove it!
    		PlaceId move = resolveDoubleBack(dv, validMoves[i]);
    		if (placeIsSea(move)) {
    			removeMove(dv, move, validMoves, numValidMoves);
    		}
    	}
	} 
}

// remove HIDE and DOUBLE_BACK_1 moves from validMoves array
void removeHide(DraculaView dv, PlaceId *validMoves, int *numValidMoves) 
{
	for (int i = 0; i < *numValidMoves; i++) {
		if (validMoves[i] == HIDE || validMoves[i] == DOUBLE_BACK_1)
			removeMove(dv, validMoves[i], validMoves, numValidMoves);
	}	
}	
