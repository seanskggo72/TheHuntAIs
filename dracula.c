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
	int *numValidMoves, bool *moveMade);
//void checkIfHide(DraculaView dv, Map map, PlaceId *validMoves, 
    //int *numValidMoves);
void moveAwayFromClosestHunters(DraculaView dv, Map map, PlaceId *validMoves, 
    int *numValidMoves);
void SeaMoves(DraculaView dv, PlaceId *validMoves, int *numValidMoves, 
    Map map, bool *moveMade);
void beenToCDRecentlyCheck(DraculaView dv, PlaceId *validMoves, int *numValidMoves);
    
// Local utility functions
PlaceId *findPathBFS(Map map, PlaceId src, PlaceId dest, bool getPath, 
    int *hops, bool *canFree, bool avoidHunter, DraculaView dv);
int PlayerToDrac(Map map, DraculaView dv, Player player, PlaceId DracLoc);
Player FindClosestPlayer(DraculaView dv, Map map, int *distance, int PlayerToDracDistance[4], PlaceId Loc);
void removeMove(PlaceId move, PlaceId *validMoves, int *numValidMoves);
bool isValidMove(PlaceId move, PlaceId *validMoves, int *numValidMoves);
void McBubbleSort(int *a, int max);
bool isOnMainland(PlaceId Location);
bool isDracOnSea(DraculaView dv, Map map);
void removeSeaMoves(DraculaView dv, PlaceId *validMoves, int *numValidMoves);
bool isHunterLocationOrAdjacent(DraculaView dv, PlaceId loc, Map map);
bool safeToGoCastleDrac(DraculaView dv, Map map);
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
	// Now calculate even better moves
	Map map = DvGetMap(dv); 
	int distance;
	int PlayerToDracDistance[4];
	FindClosestPlayer(dv, map, &distance, PlayerToDracDistance, 
	    DvGetPlayerLocation(dv, PLAYER_DRACULA));
	
	if (distance > 3) {
		// remove all sea moves
		removeSeaMoves(dv, validMoves, &numValidMoves);
		makeRandomMove(dv, validMoves, &numValidMoves);
	}
	
	// if drac's loc is a sea loc, increment numSeaMovesMade
	if (placeIsSea(DvGetPlayerLocation(dv, PLAYER_DRACULA))) {
		numSeaMovesMade++;
	}
	
	
	// If drac has been to CD recently, we must be careful about what moves 
	// we make...
	beenToCDRecentlyCheck(dv, validMoves, &numValidMoves);
	
	
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
	 
	
	// Make sea move - CHANGE TO BE A LOW HEALTH FUNC...
	if (DvGetHealth(dv, PLAYER_DRACULA) <= LOW_HEALTH) {
		bool moveMade = false;
		// function was pretty good but should be double checked
	    SeaMoves(dv, validMoves, &numValidMoves, map, &moveMade);
		if (moveMade) return;
		else makeRandomMove(dv, validMoves, &numValidMoves);
	}
	
	// If drac's health is low, get to CASTLE_DRACULA!!!
	// maybe return early, maybe continue? maybe this logic should 
	// even come after removeMovesInDirectPlayerPath??
	
	
	/// THIS IS OLD...IDK IF WE NEED TO KEEP IT OR IMPROVE...
	// will definitely be blended with the above IF STATEMENT tho...
	
	if (DvGetHealth(dv, PLAYER_DRACULA) <= 20) {
		bool moveMade = false;
		goToCastleDrac(dv, map, validMoves, &numValidMoves, &moveMade);
		if (moveMade) return;
		// else we need to register a new move
		else {
			makeRandomMove(dv, validMoves, &numValidMoves);
		}
	} 
	
	// function needs checking and some reevaluation
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
	registerBestPlay(play, "THIS IS A RANDOM PLAY");
	return; 
}

void rmMovesToHuners(DraculaView dv, PlaceId *validMoves, int *numValidMoves) 
{
	PlaceId helsingLoc = DvGetPlayerLocation(dv, PLAYER_VAN_HELSING);
	PlaceId sewardLoc = DvGetPlayerLocation(dv, PLAYER_DR_SEWARD);
	PlaceId godalmingLoc = DvGetPlayerLocation(dv, PLAYER_LORD_GODALMING);
	PlaceId harkerLoc = DvGetPlayerLocation(dv, PLAYER_MINA_HARKER);
	
	removeMove(helsingLoc, validMoves, numValidMoves);
	removeMove(sewardLoc, validMoves, numValidMoves);
	removeMove(godalmingLoc, validMoves, numValidMoves);
	removeMove(harkerLoc, validMoves, numValidMoves);
	
	makeRandomMove(dv, validMoves, numValidMoves);
}

// take the fastest route to castle drac if possible...
void goToCastleDrac(DraculaView dv, Map map, PlaceId *validMoves, 
	int *numValidMoves, bool *moveMade) {
	if (safeToGoCastleDrac(dv, map) == true) return;
	PlaceId dracLoc = DvGetPlayerLocation(dv, PLAYER_DRACULA);
	int pathLength = 0;
	bool canFree = false;
	PlaceId *path = findPathBFS(map, dracLoc, CASTLE_DRACULA, true, &pathLength,
		&canFree, true, dv);
	// special cases...(probs not necessary, more for error checking)...
	if (pathLength == 0) return;
	// i actually don't think this can ever occur lol, but it's here in case...
	if (pathLength == 1) {
		int i = 0;
		for (; i < *numValidMoves; i++) {
			if (validMoves[i] == path[0]) {
				char *play = (char *) placeIdToAbbrev(validMoves[i]);
				registerBestPlay(play, "DOCTOR!!!!!!!!!!!!!!!!!!!!!!!!!");
				*moveMade = true;
				break;
			}
		}
	}
	
	// pathLength >= 2... is all other cases
	if (pathLength >= 2) {
		int i = 0;
		for (; i < *numValidMoves; i++) {
			if (validMoves[i] == path[1]) {
				char *play = (char *) placeIdToAbbrev(validMoves[i]);
				registerBestPlay(play, "DOCTOR!!!!!!!!!!!!!!!!!!!!!!!!!");
				*moveMade = true;
				break;
			}
		}
	}

	if (canFree) free(path);
	return;
}

bool safeToGoCastleDrac(DraculaView dv, Map map)
{
    int distance;
    int PlayerToDracDistance[4];
    FindClosestPlayer(dv, map, &distance, PlayerToDracDistance, CASTLE_DRACULA);
    if (distance >= 3) {
        return true;
    } else {
        return false;
    }
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
                removeMove(validMoves[i], validMoves, numValidMoves);
            } else {
            	// We must consider DOUBLE_BACK moves!!!
            	if (!isDoubleBack(validMoves[i])) return;
            	else {
            		// if the double back move is a sea move, remove it!
            		PlaceId move = resolveDoubleBack(dv, validMoves[i]);
            		if (placeIsSea(move)) {
            			removeMove(move, validMoves, numValidMoves);
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
        } else if (DracLocation == LIVERPOOL) {
        	removeMove(IRISH_SEA, validMoves, numValidMoves);
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
        
       	// deal with the case where drac is in the IRISH_SEA 
        if (DracLocation == IRISH_SEA) {
    		if (isValidMove(ATLANTIC_OCEAN, validMoves, numValidMoves))
    			registerBestPlay("AO", "A tricky little situation");
    		*moveMade = true;
    		return;
        }
        int i = 0;
        for (; i < *numValidMoves; i++) {
        	
     		// I think this whole little section needs to be re-examined!!!!
            if (isOnMainland(validMoves[i]) == false) {
		        removeMove(validMoves[i], validMoves, numValidMoves);
            }
        }         
    } else if (DracHealth <= LOW_HEALTH && isOnMainland(DracLocation) == true) {
        // if health is <= LOW_HEALTH and on mainland do not go to sea
        int i = 0;
        for (; i < *numValidMoves; i++) {
            if (placeIdToType(validMoves[i]) == SEA) {
                removeMove(validMoves[i], validMoves, numValidMoves);
            } else {
            	// We must consider DOUBLE_BACK moves!!!
            	if (!isDoubleBack(validMoves[i])) return;
            	else {
            		// if the double back move is a sea move, remove it!
            		PlaceId move = resolveDoubleBack(dv, validMoves[i]);
            		if (placeIsSea(move)) {
            			removeMove(move, validMoves, numValidMoves);
            		}
            	}
            }
        }        
    }
    return;
}

void beenToCDRecentlyCheck(DraculaView dv, PlaceId *validMoves, int *numValidMoves)
{
	if (DvGetPlayerLocation(dv, PLAYER_DRACULA) == CASTLE_DRACULA) {
		removeMove(DOUBLE_BACK_1, validMoves, numValidMoves);
		removeMove(HIDE, validMoves, numValidMoves);
	} else {
		int trailLength = 0;
		PlaceId *trail = DvWhereHaveIBeen(dv, &trailLength);
		for (int i = 0; i < trailLength; i++) {
			if (trail[i] == CASTLE_DRACULA) {
				removeMove((DOUBLE_BACK_1 + i), validMoves, numValidMoves);
				if (i < 4) removeMove((HIDE), validMoves, numValidMoves);
			}
		}
	}
	
	makeRandomMove(dv, validMoves, numValidMoves);
	return;
}


// Utility functions

// returns a path from src to dest in reverse order (i.e. starting with dest).
PlaceId *findPathBFS(Map map, PlaceId src, PlaceId dest, bool getPath, 
    int *hops, bool *canFree, bool avoidHunter, DraculaView dv)
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
            if (avoidHunter == true) {
                ConnList edges = MapGetConnections(map, v);
                while (edges != NULL) {
                    if (visited[edges->p] == -1 && 
                        isHunterLocationOrAdjacent(dv, edges->p, map) == false) {
                    	visited[edges->p] = v;
                    	QueueEnqueue(q, edges->p);
                    }
                    edges = edges->next;
                }            
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

Player FindClosestPlayer(DraculaView dv, Map map, int *distance, int PlayerToDracDistance[4], PlaceId Loc)
{   
    PlayerToDracDistance[0] = PlayerToDrac(map, dv, PLAYER_LORD_GODALMING, Loc);
    PlayerToDracDistance[1] = PlayerToDrac(map, dv, PLAYER_DR_SEWARD, Loc);
    PlayerToDracDistance[2] = PlayerToDrac(map, dv, PLAYER_VAN_HELSING, Loc);
    PlayerToDracDistance[3] = PlayerToDrac(map, dv, PLAYER_MINA_HARKER, Loc);
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
    bool avoidHunter = false;
    PlaceId playerLoc = DvGetPlayerLocation(dv, player);
    PlaceId *Path = findPathBFS(map, playerLoc, DracLoc, getPath, 
        &hops, &canFree, avoidHunter, dv);
    if (canFree == true) {
        free(Path);
    }
    return hops;
}

void removeMove(PlaceId move, PlaceId *validMoves, int *numValidMoves) 
{
	bool foundMove = false;
	int i = 0;
	for (; i < *numValidMoves; i++) {
		if (validMoves[i] == move) {
			foundMove = true;
			break;
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
	for (int i = 0; i < HUNTER_NUM; i++) 
		hunterLocs[i] = DvGetPlayerLocation(dv, i);
			
	McBubbleSort(hunterToDracDistance, HUNTER_NUM);
	// outer loop checks closest hunter, then closest 2 hunters, etv...
	for (int j = 1; j <= (HUNTER_NUM -2); j++) {
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
			   		path = findPathBFS(map, validMoves[i], hunterLocs[counter], false, &tempDistance, &canFree, false, dv);
		   		} else if (isDoubleBack(validMoves[i])) {
		   			path = findPathBFS(map, resolveDoubleBack(dv, validMoves[i]), hunterLocs[counter], false, &tempDistance, &canFree, false, dv);
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

void removeSeaMoves(DraculaView dv, PlaceId *validMoves, int *numValidMoves) {
	for (int i = 0; i < *numValidMoves; i++) {
		// remove Sea Moves
		if (placeIsSea(validMoves[i])) {
			removeMove(validMoves[i], validMoves, numValidMoves);
		} else {
    		// if the double back move is a sea move, remove it!
    		PlaceId move = resolveDoubleBack(dv, validMoves[i]);
    		if (placeIsSea(move)) {
    			removeMove(move, validMoves, numValidMoves);
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
