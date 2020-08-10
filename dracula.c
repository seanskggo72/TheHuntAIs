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
#define VERY_LOW_HEALTH 4

// Local "movement" related functions
void doFirstMove(DraculaView dv);
void makeRandomMove(DraculaView dv, PlaceId *validMoves, int *numValidMoves);
void rmMovesToHuners(DraculaView dv, PlaceId *validMoves, int *numValidMoves);
void goToCastleDrac(DraculaView dv, Map map, PlaceId *validMoves, 
	int *numValidMoves, bool *moveMade, bool highHealth);
//void checkIfHide(DraculaView dv, Map map, PlaceId *validMoves, 
    //int *numValidMoves);
void moveAwayFromClosestHunters(DraculaView dv, Map map, PlaceId *validMoves, 
    int *numValidMoves);
void SeaMoves(DraculaView dv, PlaceId *validMoves, int *numValidMoves, 
    Map map, bool *moveMade);
void goToCastleDrac(DraculaView dv, Map map, PlaceId *validMoves, 
	int *numValidMoves, bool *moveMade, bool highHealth);
void beenToCDRecentlyCheck(DraculaView dv, Map map, PlaceId *validMoves, 
	int *numValidMoves, bool *moveMade);
    
// Local utility functions
PlaceId *findPathBFS(Map map, PlaceId src, PlaceId dest, bool getPath, 
    int *hops, bool *canFree);
int PlayerToDrac(Map map, DraculaView dv, Player player, PlaceId DracLoc);
Player FindClosestPlayer(DraculaView dv, Map map, int *distance, int PlayerToDracDistance[4]);
void removeMove(DraculaView dv, PlaceId move, PlaceId *validMoves, 
	int *numValidMoves);
bool isValidMove(PlaceId move, PlaceId *validMoves, int *numValidMoves);
void McBubbleSort(int *a, int max);
bool isOnMainland(PlaceId Location);
bool isDracOnSea(DraculaView dv, Map map);
void removeSeaMoves(DraculaView dv, PlaceId *validMoves, int *numValidMoves);
void removeHide(DraculaView dv, PlaceId *validMoves, int *numValidMoves);
void removeClosestHunterReachable(DraculaView dv, PlaceId *validMoves, 
	int *numValidMoves, Player closest);
bool isHunterLocationOrAdjacent(DraculaView dv, PlaceId loc, Map map);
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
	Player closest = FindClosestPlayer(dv, map, &distanceToClose, temp);
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
	if (placeIsSea(DvGetPlayerLocation(dv, PLAYER_DRACULA))) {
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
		// go to MAINLAND/LAND AT LEASt
		// TO-Do
		
		// do stuff 
		removeSeaMoves(dv, validMoves, &numValidMoves);
		makeRandomMove(dv, validMoves, &numValidMoves);
		numSeaMovesMade =  (numValidMoves != 0) ? 0 : 4;
	}	
	
	// Don't go to locations occupied by hunters
	rmMovesToHuners(dv, validMoves, &numValidMoves); 
	 
	// Now calculate even better moves
	
	// If player is right next to drac, remove HIDES and DOUBLE_BACK_1s...
	if (distanceToClose < 3 ) removeHide(dv, validMoves, &numValidMoves);
	
	// if the distance to the cloest hunter is < 5, avoid going to places they 
	// can reach
	if (distanceToClose < 5)
		removeClosestHunterReachable(dv, validMoves, &numValidMoves, closest);
	
	// prevent drac going to ATHENS if possible...only if he's on mainland
	if (isOnMainland(DvGetPlayerLocation(dv, PLAYER_DRACULA))) {
		removeMove(dv, ATHENS, validMoves, &numValidMoves);
	}
	
	
	// drac does different moves if he has low health
	if (DvGetHealth(dv, PLAYER_DRACULA) <= LOW_HEALTH) {
		bool moveMade = false;
		// function was pretty good but should be double checked
	    SeaMoves(dv, validMoves, &numValidMoves, map, &moveMade);
		if (moveMade) return;
		else goToCastleDrac(dv, map, validMoves, &numValidMoves, &moveMade, false);
		if (moveMade) return;
		else moveAwayFromClosestHunters(dv, map, validMoves, &numValidMoves);
		return;
	} else if (DvGetHealth(dv, PLAYER_DRACULA) < 30) {
		bool moveMade = false;
		goToCastleDrac(dv, map, validMoves, &numValidMoves, &moveMade, true);
		if (moveMade) return;
		//removeMove(dv, CASTLE_DRACULA, validMoves, &numValidMoves);
	}
	
	
	
	moveAwayFromClosestHunters(dv, map, validMoves, &numValidMoves);
	
	free(validMoves);
	
}

// Functions that handle AI "movement"

void doFirstMove(DraculaView dv) 
{
	registerBestPlay("BR", "Don't try and catch me");
	return;
	
}

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

void rmMovesToHuners(DraculaView dv, PlaceId *validMoves, int *numValidMoves) 
{
	PlaceId helsingLoc = DvGetPlayerLocation(dv, PLAYER_VAN_HELSING);
	PlaceId sewardLoc = DvGetPlayerLocation(dv, PLAYER_DR_SEWARD);
	PlaceId godalmingLoc = DvGetPlayerLocation(dv, PLAYER_LORD_GODALMING);
	PlaceId harkerLoc = DvGetPlayerLocation(dv, PLAYER_MINA_HARKER);
	
	removeMove(dv, helsingLoc, validMoves, numValidMoves);
	removeMove(dv, sewardLoc, validMoves, numValidMoves);
	removeMove(dv, godalmingLoc, validMoves, numValidMoves);
	removeMove(dv, harkerLoc, validMoves, numValidMoves);
	
	for (int i = 0; i < *numValidMoves; i++) {
		if (isDoubleBack(validMoves[i])) {
    		// if the double back move is a sea move, remove it!
    		PlaceId move = resolveDoubleBack(dv, validMoves[i]);
    		if (move == helsingLoc || move == sewardLoc || 
    			move == godalmingLoc || move == harkerLoc) {
    			removeMove(dv, move, validMoves, numValidMoves);
    		}
		}
	}
	
	makeRandomMove(dv, validMoves, numValidMoves);
	
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

void SeaMoves(DraculaView dv, PlaceId *validMoves, int *numValidMoves, 
    Map map, bool *moveMade)
{
    //int numValidLocs;
    //PlaceId *validLocs = DvWhereCanIGo(dv, &numValidLocs);
    
    int DracHealth = DvGetHealth(dv, PLAYER_DRACULA);
    PlaceId DracLocation = DvGetPlayerLocation(dv, PLAYER_DRACULA);
    // if health is <= VERY_LOW_HEALTH and on great britain, remain on 
    // land no matter what
    if (DracHealth <= VERY_LOW_HEALTH && isOnMainland(DracLocation) == false) {
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
        
    } else if (DracHealth <= LOW_HEALTH && isOnMainland(DracLocation) == false) {
        // if health is <LOW_HEALTH and on great britain, take shortest route 
        // to mainland
        if (DracLocation == MANCHESTER) {
            moveAwayFromClosestHunters(dv, map, validMoves, numValidMoves);
            *moveMade = true;
        } else {
            int i = 0;
            for (; i < *numValidMoves; i++) {
                if (placeIdToType(validMoves[i]) == SEA) {
                // THIS DOES NOT CONSIDER LIVERPOOL/IRISH SEA!!!
			        char *play = (char *) placeIdToAbbrev(validMoves[i]);
			        registerBestPlay(play, "OFF TO THE SEVEN SEAS");
			        *moveMade = true;
                }
            }            
        }
    } else if (DracHealth <= LOW_HEALTH && isDracOnSea(dv, map) == true) {
        // GET TO MAINLAND ASAP, remove all Great Britain moves
        int i = 0;
        for (; i < *numValidMoves; i++) {
        	// Irish Sea check is needed so that drac doesn't get stranded...see the game map
        	// IN FACT the Irish Sea/LIVERPOOl case needs to be run through VERY carefully....
        	// it's an ODD outlier of a case...
        	
     		// I think this whole little section needs to be re-examined!!!!
            if (isOnMainland(validMoves[i]) == false && validMoves[i] != IRISH_SEA) {
		        removeMove(dv, validMoves[i], validMoves, numValidMoves);
            }
        }         
    } else if (DracHealth <= LOW_HEALTH && isOnMainland(DracLocation) == true) {
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
		path2 = findPathBFS(map, dracLoc, CASTLE_DRACULA, false, &distance, &canFree2);
	}
	if (canFree2) free (path2);
    for (int i = 0; i < *numValidMoves; i++) {
    	int hops = 0;
        bool canFree = false;
        PlaceId *path;
        if (placeIsReal(validMoves[i])) {
       		path = findPathBFS(map, validMoves[i], CASTLE_DRACULA, false, &hops, &canFree);
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

bool safeToGoCastleDrac(DraculaView dv, Map map, bool highHealth)
{
    int distance = 100000;
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

void beenToCDRecentlyCheck(DraculaView dv, Map map, PlaceId *validMoves, 
	int *numValidMoves, bool *moveMade)
{
	PlaceId dracLoc = DvGetPlayerLocation(dv, PLAYER_DRACULA);
	
	if (dracLoc == CASTLE_DRACULA && DvGetHealth(dv, PLAYER_DRACULA) <= 25) {
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
		int trailLength = 0;
		PlaceId *trail = DvWhereHaveIBeen(dv, &trailLength);
		for (int i = 0; i < trailLength; i++) {
			if (trail[i] == CASTLE_DRACULA) {
				removeMove(dv, (DOUBLE_BACK_1 + i), validMoves, numValidMoves);
				if (i < 4) removeMove(dv, (HIDE), validMoves, numValidMoves);
			}
		}
		removeMove(dv, CASTLE_DRACULA, validMoves, numValidMoves);
	}
	
	makeRandomMove(dv, validMoves, numValidMoves);
	return;
}


// Utility functions

// returns a path from src to dest in reverse order (i.e. starting with dest).
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


Player FindClosestPlayer(DraculaView dv, Map map, int *distance, int PlayerToDracDistance[4])
{   
    PlaceId DracLoc = DvGetPlayerLocation(dv, PLAYER_DRACULA);
    PlayerToDracDistance[0] = PlayerToDrac(map, dv, PLAYER_LORD_GODALMING, DracLoc);
    PlayerToDracDistance[1] = PlayerToDrac(map, dv, PLAYER_DR_SEWARD, DracLoc);
    PlayerToDracDistance[2] = PlayerToDrac(map, dv, PLAYER_VAN_HELSING, DracLoc);
    PlayerToDracDistance[3] = PlayerToDrac(map, dv, PLAYER_MINA_HARKER, DracLoc);
    int minimum = 10000000;
    Player closest;
    for (int i = 0; i < HUNTER_NUM; i++) {
        if (PlayerToDracDistance[i] < minimum) {
            minimum = PlayerToDracDistance[i];
            *distance = minimum;
            closest = i;    
        }
    }
    
    return closest;
}

int PlayerToDrac(Map map, DraculaView dv, Player player, PlaceId DracLoc)
{
    int hops = -1;
    bool canFree = false;
    bool getPath = false;
    PlaceId playerLoc = DvGetPlayerLocation(dv, player);
    //if (!placeIsReal(DracLoc)) return 100000;
    PlaceId *Path = findPathBFS(map, playerLoc, DracLoc, getPath, 
        &hops, &canFree);
    if (canFree == true) {
        free(Path);
    }
    return hops;
}

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
/*
void checkIfHide(DraculaView dv, Map map, PlaceId *validMoves, 
    int *numValidMoves)
{
    if (isValidMove(HIDE, validMoves, numValidMoves) == true) {
        int distance = 0;
        // this variable doesn't do anything else in this func but must be passed in
        int playerDistance[4];
        FindClosestPlayer(dv, map, &distance, playerDistance);
        if (distance == 1) {
            registerBestPlay("HI", "BLEHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH");
        }
    } else {
        return;
    }
}*/

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

void moveAwayFromClosestHunters(DraculaView dv, Map map, PlaceId *validMoves, int *numValidMoves)
{
	int hunterToDracDistance[4];
	//Player closest = FindClosestPlayer(dv, map, &distance, hunterToDracDistance);
	int hunterLocs[4];
	char *bestPlay = NULL;
	
	// Get hunter locations
	for (int i = 0; i < (HUNTER_NUM - 2); i++) 
		hunterLocs[i] = DvGetPlayerLocation(dv, i);
			
	McBubbleSort(hunterToDracDistance, HUNTER_NUM);
	// outer loop checks closest hunter, then closest 2 hunters, etv...
	for (int j = 1; j <= (HUNTER_NUM - 1); j++) {
		int i = 0;
		
		// this is the sum of distance from dracula to each hunter being considered
		int dracToHunterTotal = 0;
		for(int counter = 0; counter < j; counter++) dracToHunterTotal += hunterToDracDistance[counter];
		
		// this loop checks all of drac's valid moves
		for (; i < *numValidMoves; i++) {
			int validMoveToHunterDist = 0;
		    bool canFree = false;
		    PlaceId *path;
		    // inner loop compares validMoves[i] to the locs of hunters
		    for (int counter = 0; counter < j; counter++) {
		    	int tempDistance = 0;
				if (placeIsReal(validMoves[i])) {
			   		path = findPathBFS(map, validMoves[i], hunterLocs[counter], false, &tempDistance, &canFree);
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

void McBubbleSort(int *a, int max) 
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


bool isOnMainland(PlaceId Location)
{
    if (Location == LONDON || Location == PLYMOUTH || 
        Location == SWANSEA || Location == LIVERPOOL || 
        Location == MANCHESTER || Location == EDINBURGH || 
        Location == DUBLIN || Location == GALWAY) {
        return false;
    } else {
        return true;
    }
}
bool isDracOnSea(DraculaView dv, Map map)
{
    PlaceId DracLocation = DvGetPlayerLocation(dv, PLAYER_DRACULA);
    if (placeIdToType(DracLocation) == SEA) {
        return true;
    } else {
        return false;
    }
}

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

void removeHide(DraculaView dv, PlaceId *validMoves, int *numValidMoves) 
{
	for (int i = 0; i < *numValidMoves; i++) {
		if (validMoves[i] == HIDE || validMoves[i] == DOUBLE_BACK_1)
			removeMove(dv, validMoves[i], validMoves, numValidMoves);
	}	
}

void removeClosestHunterReachable(DraculaView dv, PlaceId *validMoves, 
	int *numValidMoves, Player closest) 
{
	int numLocs = 0;
	PlaceId *closestReachable = DvWhereCanTheyGo(dv, closest, &numLocs);
	for (int i = 0; i < *numValidMoves; i++) {
		for (int j = 0; j < numLocs; j++) {
			if (validMoves[i] == closestReachable[j])
				removeMove(dv, validMoves[i], validMoves, numValidMoves);
			else if (isDoubleBack(validMoves[i])) {
				PlaceId doubleBack = resolveDoubleBack(dv, validMoves[i]);
				if (doubleBack == closestReachable[j]) {
					removeMove(dv, doubleBack, validMoves, numValidMoves);
				}
			}
		}
	}
}		

bool isHunterLocationOrAdjacent(DraculaView dv, PlaceId loc, Map map)
{
    int i = 0;
    for (; i < 4; i++) {
        PlaceId Loc = DvGetPlayerLocation(dv, i);
        if (loc == Loc) return true;
        ConnList edges = MapGetConnections(map, Loc);
        while (edges != NULL) {
            if (edges->p == Loc) return true;
            else edges = edges->next;
        }
    }
    return false;
}	
