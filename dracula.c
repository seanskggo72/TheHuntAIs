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
// Local Static Function Declarations
void doFirstMove(DraculaView dv);
void makeRandomMove(DraculaView dv, PlaceId *validMoves, int numValidMoves);
PlaceId *findPathBFS(Map map, PlaceId src, PlaceId dest, bool getPath, 
    int *hops, bool *canFree);
int PlayerToDrac(Map map, DraculaView dv, Player player, PlaceId DracLoc);
Player FindClosestPlayer(DraculaView dv, Map map);


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
	} else {

		printf("Hello\n");
		makeRandomMove(dv, validMoves, numValidMoves);
	}
	
	free(validMoves);
	
	return;
	// Other better moves below 
}

void doFirstMove(DraculaView dv) 
{
	registerBestPlay("VI", "Mwahahahaha");
	return;
	
}

void makeRandomMove(DraculaView dv, PlaceId *validMoves, int numValidMoves) 
{
	// Use time function to get random seed
	unsigned int seed = (unsigned int) time(NULL);
	srand(seed);
	int randomIndex = rand() % numValidMoves;
	char *play = (char *) placeIdToAbbrev(validMoves[randomIndex]);
	registerBestPlay(play, "Mwahahahaha");
	return; 
}

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
            while (edges != NULL && visited[edges->p] == -1) {
                visited[edges->p] = v;
                QueueEnqueue(q, edges->p);
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
        *hops = total;
        if (getPath == true) {
            PlaceId *Path = malloc(sizeof(PlaceId)*(total + 1));
            int j = total;   
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
