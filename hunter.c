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

#include "hunter.h"
#include "HunterView.h"
#include "Game.h"
#include "Places.h"

// Local Static Function Declarations
void doFirstMove(HunterView hv);
void makeRandomMove(HunterView hv, PlaceId *validMoves, int numValidMoves);

void decideHunterMove(HunterView hv)
{
	if (HvGetRound(hv) == 0) {
		doFirstMove(hv);
		return;
	}

	int numValidMoves = 0;
	PlaceId *validMoves = HvWhereCanIGo(hv, &numValidMoves);
	
	// Get a random first move
	makeRandomMove(hv, validMoves, numValidMoves);

	free(validMoves);
	
	return;
	// Other better moves below 
}

void doFirstMove(HunterView hv) 
{
	Player hunter = HvGetPlayer(hv);
	switch(hunter) {
		case PLAYER_LORD_GODALMING:	
			registerBestPlay("SR", "YOLO");
			break;
		case PLAYER_DR_SEWARD:
			registerBestPlay("SZ", "YOLO");
			break;
		case PLAYER_VAN_HELSING:
			registerBestPlay("MI", "YOLO");
			break;
		case PLAYER_MINA_HARKER:
			registerBestPlay("LE", "YOLO");
			break;
		case PLAYER_DRACULA:
			fprintf(stderr, "ERROR: PLAYER IS NOT A HUNTER\n");
			exit(EXIT_FAILURE);
			break;
	}
	return;
	
}

void makeRandomMove(HunterView hv, PlaceId *validMoves, int numValidMoves) 
{
	// Use time function to get random seed
	unsigned int seed = (unsigned int) time(NULL);
	srand(seed);
	int randomIndex = rand() % numValidMoves;
	char *play = (char *) placeIdToAbbrev(validMoves[randomIndex]);
	registerBestPlay(play, "YOLO");
	return; 
}
