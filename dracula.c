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
#include <stdbool.h>

#include "dracula.h"
#include "DraculaView.h"
#include "Game.h"
#include "Places.h"
#include "Map.h"
#include "Queue.h"

#define HUNTER_NUM 4
// Local "movement" related functions
void doFirstMove(DraculaView dv);
void makeRandomMove(DraculaView dv, PlaceId *validMoves, int *numValidMoves);
void rmMovesInEndOfDirectPlayerPath(DraculaView dv, Map map, 
	PlaceId *validMoves, int *numValidMoves);
void goToCastleDrac(DraculaView dv, Map map, PlaceId *validMoves, 
	int *numValidMoves, bool *moveMade);
	
// Local utility functions
PlaceId *findPathBFS(Map map, PlaceId src, PlaceId dest, bool getPath, 
    int *hops, bool *canFree);
int PlayerToDrac(Map map, DraculaView dv, Player player, PlaceId DracLoc);
Player FindClosestPlayer(DraculaView dv, Map map);
void removeMove(PlaceId move, PlaceId *validMoves, int *numValidMoves);
//

void decideDraculaMove(DraculaView dv)
{
	if (DvGetRound(dv) == 0) {
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
	
	 
	// Calculate better moves
	Map map = DvGetMap(dv); 
	
	// If drac's health is low, get to CASTLE_DRACULA!!!
	// maybe return early, maybe continue? maybe this logic should 
	// even come after removeMovesInDirectPlayerPath??
	
	if (DvGetHealth(dv, PLAYER_DRACULA) <= 20) {
		bool moveMade = false;
		goToCastleDrac(dv, map, validMoves, &numValidMoves, &moveMade);
		if (moveMade) return;
	}
	
	// If the current bestPlay is in the shortest path of a player to drac,
	// remove it from validMoves and suggest another?
	
	rmMovesInEndOfDirectPlayerPath(dv, map, validMoves, &numValidMoves);
	makeRandomMove(dv, validMoves, &numValidMoves);
	
	// if all hunters are nearby, hide?
	//checkIfHide();
	
	free(validMoves);
	
	return;
}

// Functions that handle AI "movement"

void doFirstMove(DraculaView dv) 
{
	registerBestPlay("BR", "Mwahahahaha");
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
Player FindClosestPlayer(DraculaView dv, Map map)
{   
    PlaceId DracLoc = DvGetPlayerLocation(dv, PLAYER_DRACULA);
    int PlayerToDracLoc[4];
    PlayerToDracLoc[0] = PlayerToDrac(map, dv, PLAYER_LORD_GODALMING, DracLoc);
    PlayerToDracLoc[1] = PlayerToDrac(map, dv, PLAYER_DR_SEWARD, DracLoc);
    PlayerToDracLoc[2] = PlayerToDrac(map, dv, PLAYER_VAN_HELSING, DracLoc);
    PlayerToDracLoc[3] = PlayerToDrac(map, dv, PLAYER_MINA_HARKER, DracLoc);
    int minimum = 10000000;
    Player closest;
    for (int i = 0; i < HUNTER_NUM; i++) {
        if (PlayerToDracLoc[i] < minimum) {
            minimum = PlayerToDracLoc[i];
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
