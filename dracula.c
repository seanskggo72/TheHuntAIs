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

#include "dracula.h"
#include "DraculaView.h"
#include "Game.h"
#include "Places.h"

// Local Static Function Declarations
void doFirstMove(DraculaView dv);
void makeRandomMove(DraculaView dv, PlaceId *validMoves, int numValidMoves);

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
