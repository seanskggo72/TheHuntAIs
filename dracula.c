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
void rmMovesInEndOfDirectPlayerPath(DraculaView dv, Map map, 
	PlaceId *validMoves, int *numValidMoves);
void goToCastleDrac(DraculaView dv, Map map, PlaceId *validMoves, 
	int *numValidMoves, bool *moveMade);
//void checkIfHide(DraculaView dv, Map map, PlaceId *validMoves, 
    //int *numValidMoves);
void moveAwayFromClosestHunters(DraculaView dv, Map map, PlaceId *validMoves, 
    int *numValidMoves);
void SeaMoves(DraculaView dv, PlaceId *validMoves, int *numValidMoves, 
    Map map, bool *moveMade);
    
// Local utility functions
PlaceId *findPathBFS(Map map, PlaceId src, PlaceId dest, bool getPath, 
    int *hops, bool *canFree);
int PlayerToDrac(Map map, DraculaView dv, Player player, PlaceId DracLoc);
Player FindClosestPlayer(DraculaView dv, Map map, int *distance, int PlayerToDracDistance[4]);
void removeMove(PlaceId move, PlaceId *validMoves, int *numValidMoves);
bool isValidMove(PlaceId move, PlaceId *validMoves, int *numValidMoves);
void McBubbleSort(int *a, int max);
bool isOnMainland(PlaceId Location);
bool isDracOnSea(DraculaView dv, Map map);
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
	
	if (round % 13 == 0) {
		// remove all sea moves
		// TO-DO
		for (int i = 0; i < numValidMoves; i++) {
			if (placeIsSea(validMoves[i])) {
				removeMove(validMoves[i], validMoves, &numValidMoves);
			}
		} 
	}
	
	if (numSeaMovesMade >= 3) {
		// go to MAINLAND/LAND AT LEASt
		// TO-Do
	}	
	 
	// Calculate better moves
	Map map = DvGetMap(dv); 
	
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
	/*
	if (DvGetHealth(dv, PLAYER_DRACULA) <= 20) {
		bool moveMade = false;
		goToCastleDrac(dv, map, validMoves, &numValidMoves, &moveMade);
		if (moveMade) return;
		// else we need to register a new move
		else {
			makeRandomMove(dv, validMoves, &numValidMoves);
			return;
		}
	} */ 
	
	// if all hunters are nearby, hide.
	// NOT USING THIS RN - IN FACT MAY BE MADE
	//checkIfHide(dv, map, validMoves, &numValidMoves);
	
	// If the current bestPlay is in the shortest path of a player to drac,
	// remove it from validMoves and suggest another?
	
	// I think this function is now next to useless...it turns out it;s not 
	// BUT!! but in some scenarios
	rmMovesInEndOfDirectPlayerPath(dv, map, validMoves, &numValidMoves);
	
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
	registerBestPlay(play, "Mwahahahaha");
	return; 
}

void rmMovesInEndOfDirectPlayerPath(DraculaView dv, Map map, 
	PlaceId *validMoves, int *numValidMoves) 
{
	PlaceId helsingLoc = DvGetPlayerLocation(dv, PLAYER_VAN_HELSING);
	PlaceId sewardLoc = DvGetPlayerLocation(dv, PLAYER_DR_SEWARD);
	PlaceId godalmingLoc = DvGetPlayerLocation(dv, PLAYER_LORD_GODALMING);
	PlaceId harkerLoc = DvGetPlayerLocation(dv, PLAYER_MINA_HARKER);
	PlaceId dracLoc = DvGetPlayerLocation(dv, PLAYER_DRACULA);
	
	// path to hesling 
	int helsingPathLength = 0;
	bool canFreeHelsingPath = false;
	PlaceId *helsingPath = findPathBFS(map, dracLoc, helsingLoc, true, 
		&helsingPathLength, &canFreeHelsingPath);
	
	// path to seward 
	int sewardPathLength = 0;
	bool canFreeSewardPath = false;
	PlaceId *sewardPath = findPathBFS(map, dracLoc, sewardLoc, true, 
		&sewardPathLength, &canFreeSewardPath);
		
	// path to godalming
	int godalmingPathLength = 0;
	bool canFreeGodalmingPath = false;
	PlaceId *godalmingPath = findPathBFS(map, dracLoc, godalmingLoc, true, 
		&godalmingPathLength, &canFreeGodalmingPath);
		
	// path to harker
	int harkerPathLength = 0;
	bool canFreeHarkerPath = false;
	PlaceId *harkerPath = findPathBFS(map, dracLoc, harkerLoc, true, 
	&harkerPathLength, &canFreeHarkerPath);
		
	// we only consider removing moves that lie in the hunter's path 
	// if they are close (i.e. within <= 5 cities)
	
	// these if statements call internal funcs
	// note that paths are given in reverse order...
	// so accessing them at index one gives the move in the path that
	// is adjacent to drac's current loc
	if (helsingPathLength <= 5 && helsingPathLength >= 2)  {
		removeMove(helsingPath[1], validMoves, 
			numValidMoves);
	}
	
	if (sewardPathLength <= 5 && sewardPathLength >= 2) {
		removeMove(sewardPath[1], validMoves, numValidMoves);
	}
	
	if (godalmingPathLength <= 5 && godalmingPathLength >= 2) {
		removeMove(godalmingPath[1], validMoves, 
			numValidMoves);
	}
	
	if (harkerPathLength <= 5 && harkerPathLength >= 2) {
		removeMove(harkerPath[1], validMoves, numValidMoves);
	}
	
	makeRandomMove(dv, validMoves, numValidMoves);
	if (canFreeHelsingPath) free(helsingPath);
	if (canFreeHarkerPath) free(harkerPath);
	if (canFreeSewardPath) free(sewardPath);
	if (canFreeGodalmingPath) free(godalmingPath);
}

// take the fastest route to castle drac if possible...
void goToCastleDrac(DraculaView dv, Map map, PlaceId *validMoves, 
	int *numValidMoves, bool *moveMade) {
	
	PlaceId dracLoc = DvGetPlayerLocation(dv, PLAYER_DRACULA);
	int pathLength = 0;
	bool canFree = false;
	PlaceId *path = findPathBFS(map, dracLoc, CASTLE_DRACULA, true, &pathLength,
		&canFree);
	// special cases...(probs not necessary, more for error checking)...
	if (pathLength == 0) return;
	// i actually don't think this can ever occur lol, but it's here in case...
	if (pathLength == 1) {
		int i = 0;
		for (; i < *numValidMoves; i++) {
			if (validMoves[i] == path[0]) {
				char *play = (char *) placeIdToAbbrev(validMoves[i]);
				registerBestPlay(play, "Mwahahahaha");
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
				registerBestPlay(play, "Mwahahahaha");
				*moveMade = true;
				break;
			}
		}
	}

	if (canFree) free(path);
	return;
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
    PlaceId *Path = findPathBFS(map, playerLoc, DracLoc, getPath, 
        &hops, &canFree);
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
	
	// Get hunter locations
	for (int i = 0; i < HUNTER_NUM; i++) 
		hunterLocs[i] = DvGetPlayerLocation(dv, i);
			
	McBubbleSort(hunterToDracDistance, HUNTER_NUM);
	// outer loop checks closest hunter, then closest 2 hunters, etv...
	for (int j = 0; j < HUNTER_NUM; j++) {
		int i = 0;
		
		// this is the sum of distance from dracula to each hunter being considered
		int dracToHunterTotal = 0;
		for(int counter = 0; counter < j; counter++) dracToHunterTotal += hunterToDracDistance[counter];
		
		int validMoveToHunterDist = 0;
		// this loop checks all of drac's valid moves
		for (; i < *numValidMoves; i++) {
		    bool canFree = false;
		    PlaceId *path;
		    // inner loop compares validMoves to the locs of hunters
		    for (int counter = 0; counter < j; counter++) {
		    	int tempDistance = 0;
				if (placeIsReal(validMoves[i])) {
			   		path = findPathBFS(map, validMoves[i], hunterLocs[j], false, &tempDistance, &canFree);
		   		}
		   		validMoveToHunterDist += tempDistance;
	   		}
	   		
		    if (validMoveToHunterDist > dracToHunterTotal) {
				char *play = (char *) placeIdToAbbrev(validMoves[i]);
				registerBestPlay(play, "Just try and catch me");
				dracToHunterTotal = validMoveToHunterDist;
		    }
		    if (canFree) free(path);
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
