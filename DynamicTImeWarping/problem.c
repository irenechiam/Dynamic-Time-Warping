/*
    Written by Grady Fitzpatrick for 
    COMP20007 Assignment 2 2024 Semester 1
    
    Implementation for module which contains  
        Problem 1-related data structures and 
        functions.
    
    Sample solution implemented by Grady Fitzpatrick
*/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <float.h>
#include <math.h>
#include "problem.h"
#include "problemStruct.c"
#include "solutionStruct.c"

/* Number of words to allocate space for initially. */
#define INITIALWORDSALLOCATION 64

/* Denotes that the dimension has not yet been set. */
#define DIMENSION_UNSET (-1)

#define LDINFINITY (LDBL_MAX / 2.0L)

struct problem;
struct solution;

/* Sets up a solution for the given problem. */
struct solution *newSolution(struct problem *problem);

void readSequence(FILE *seqFile, int *seqLen, long double **seq);

void readSequence(FILE *seqFile, int *seqLen, long double **seq){
    char *seqText = NULL;
    /* Read in text. */
    size_t allocated = 0;
    /* Exit if we read no characters or an error caught. */
    int success = getdelim(&seqText, &allocated, '\0', seqFile);

    if(success == -1){
        /* Encountered an error. */
        perror("Encountered error reading dictionary file");
        exit(EXIT_FAILURE);
    } else {
        /* Assume file contains at least one character. */
        assert(success > 0);
    }

    /* Progress through string. */
    int progress = 0;
    /* Table string length. */
    int seqTextLength = strlen(seqText);

    int commaCount = 0;
    /* Count how many numbers are present. */
    for(int i = 0; i < seqTextLength; i++){
        if(seqText[i] == ','){
            commaCount++;
        }
    }
    long double *seqLocal = (long double *) malloc(sizeof(long double) * (commaCount + 1));
    assert(seqLocal);

    int seqAdded = 0;
    while(progress < seqTextLength){
        int nextProgress;
        /* Read each value into the sequence. */
        assert(sscanf(seqText + progress, "%Lf , %n", &seqLocal[seqAdded], &nextProgress) == 1);
        assert(nextProgress > 0);
        progress += nextProgress;
        seqAdded++;
    }
    assert(seqAdded == (commaCount + 1));
    *seq = seqLocal;
    *seqLen = seqAdded;
}

/* 
    Reads the given dict file into a list of words 
    and the given board file into a nxn board.
*/
struct problem *readProblemA(FILE *seqAFile, FILE *seqBFile){
    struct problem *p = (struct problem *) malloc(sizeof(struct problem));
    assert(p);

    int seqALength = 0;
    long double *seqA = NULL;
    readSequence(seqAFile, &seqALength, &seqA);
    int seqBLength = 0;
    long double *seqB = NULL;
    readSequence(seqBFile, &seqBLength, &seqB);

    /* The length of the first sequence. */
    p->seqALength = seqALength;
    /* The first sequence. */
    p->sequenceA = seqA;

    /* The length of the second sequence. */
    p->seqBLength = seqBLength;
    /* The second sequence. */
    p->sequenceB = seqB;

    /* For Part D & F only. */
    p->windowSize = -1;
    p->maximumPathLength = -1;

    p->part = PART_A;

    return p;
}

struct problem *readProblemD(FILE *seqAFile, FILE *seqBFile, int windowSize){
    /* Fill in Part A sections. */
    struct problem *p = readProblemA(seqAFile, seqBFile);

    p->part = PART_D;
    p->windowSize = windowSize;

    return p;
}

struct problem *readProblemF(FILE *seqAFile, FILE *seqBFile, 
    int maxPathLength){
    /* Interpretation of inputs is same as Part A. */
    struct problem *p = readProblemA(seqAFile, seqBFile);
    
    p->part = PART_F;
    p->maximumPathLength = maxPathLength;

    return p;
}

/*
    Outputs the given solution to the given file. If colourMode is 1, the
    sentence in the problem is coloured with the given solution colours.
*/
void outputProblem(struct problem *problem, struct solution *solution, 
    FILE *outfileName){
    assert(solution);
    fprintf(outfileName, "%.2Lf\n", solution->optimalValue);
    switch(problem->part){
        case PART_A:
            assert(solution->matrix);
            for(int i = 1; i <= problem->seqALength; i++){
                for(int j = 1; j <= problem->seqBLength; j++){
                    if(solution->matrix[i][j] == LDINFINITY){
                        fprintf(outfileName, "    ");
                    } else {
                        fprintf(outfileName, "%.2Lf", solution->matrix[i][j]);
                    }
                    if(j < (problem->seqBLength)){
                        /* Intercalate with spaces. */
                        fprintf(outfileName, " ");
                    }
                }
                fprintf(outfileName, "\n");
            }
            break;
        case PART_D:
        case PART_F:
            break;
    }
}

/*
    Frees the given solution and all memory allocated for it.
*/
void freeSolution(struct solution *solution, struct problem *problem){
    if(solution){
        if(solution->matrix){
            for(int i = 0; i < problem->seqALength; i++){
                free(solution->matrix[i]);
            }
            free(solution->matrix);
        }
        free(solution);
    }
}

/*
    Frees the given problem and all memory allocated for it.
*/
void freeProblem(struct problem *problem){
    if(problem){
        if(problem->sequenceA){
            free(problem->sequenceA);
        }
        if(problem->sequenceB){
            free(problem->sequenceB);
        }
        free(problem);
    }
}

/* Sets up a solution for the given problem */
struct solution *newSolution(struct problem *problem){
    struct solution *s = (struct solution *) malloc(sizeof(struct solution));
    assert(s);
    if(problem->part == PART_F){
        s->matrix = NULL;
    } else {
        s->matrix = (long double **) malloc(sizeof(long double *) * 
            (problem->seqALength + 1));
        assert(s->matrix);
        for(int i = 0; i < (problem->seqALength + 1); i++){
            s->matrix[i] = (long double *) malloc(sizeof(long double) * 
                (problem->seqBLength + 1));
            assert(s->matrix[i]);
            for(int j = 0; j < (problem->seqBLength + 1); j++){
                s->matrix[i][j] = DBL_MAX;
            }
        }
        s->matrix[0][0] = 0;
    }
    s->optimalValue = -1;
    return s;
}

/*
    Solves the given problem according to Part A's definition
    and places the solution output into a returned solution value.
*/
struct solution *solveProblemA(struct problem *p){
    struct solution *s = newSolution(p);
    /* Fill in: Part A */
    for(int i = 1; i <= (p->seqALength); i++) {
        for(int j = 1; j <= (p->seqBLength); j++) {
            long double cost = fabsl((p->sequenceA)[i-1] - (p->sequenceB)[j-1]);
            (s->matrix)[i][j] = cost + min((s->matrix)[i-1][j], (s->matrix)[i][j-1], (s->matrix)[i-1][j-1]);
        }
    }
    s->optimalValue = s->matrix[p->seqALength][p->seqBLength];
    return s;
}

struct solution *solveProblemD(struct problem *p){
    struct solution *s = newSolution(p);
    /* Fill in: Part D */
    for(int i = 1; i <= (p->seqALength); i++) {
        for(int j = 1; j <= (p->seqBLength); j++) {
            if(j >=(i-p->windowSize) && j <= (i+p->windowSize)) {
                long double cost = fabsl((p->sequenceA)[i-1] - (p->sequenceB)[j-1]);
                (s->matrix)[i][j] = cost + min((s->matrix)[i-1][j], (s->matrix)[i][j-1], (s->matrix)[i-1][j-1]);
            }
        }
    }
    s->optimalValue = s->matrix[p->seqALength][p->seqBLength];
    return s;
}

struct solution *solveProblemF(struct problem *p){
    struct solution *s = newSolution(p);
    /* Fill in: Part F */

    /* Initializing the 3D matrix */
    long double*** costs = (long double***)malloc((p->maximumPathLength + 1) * sizeof(long double**));
    for(int i = 0; i < p->maximumPathLength + 1; i++) {
        costs[i] = (long double**)malloc((p->seqALength + 1) * sizeof(long double*));
        for(int j = 0; j < p->seqALength + 1; j++) {
            costs[i][j] = (long double*)malloc((p->seqBLength + 1) * sizeof(long double));
            for(int k = 0; k < p->seqBLength + 1; k++) {
                costs[i][j][k] = DBL_MAX;
            }
        }
    }
    costs[0][0][0] = 0.0;

    /* Calculating the alignment costs */
    for(int i = 1; i <= p->maximumPathLength; i++) {
        for(int j = 1; j <= p->seqALength && j <= i; j++) {
            for(int k = 1; k <= p->seqBLength && k <= i; k++) {
                long double alignment_cost = fabsl((p->sequenceA)[j-1] - (p->sequenceB)[k-1]);
                costs[i][j][k] = alignment_cost + min(costs[i-1][j-1][k], costs[i-1][j][k-1], costs[i-1][j-1][k-1]);
            }
        }
    }

    /* Getting the optimal value */
    long double optimalV = DBL_MAX;
    for (int i = 1; i <= p->maximumPathLength; i++) {
        if(costs[i][p->seqALength][p->seqBLength] < optimalV) {
            optimalV = costs[i][p->seqALength][p->seqBLength];
        }
    }
    s->optimalValue = optimalV;
    // Free the 3D matrix
    freeMatrix(costs, p->maximumPathLength, p->seqALength);
    return s;
}

long double min(long double a, long double b, long double c) {
    if(a <= b && a <= c) {
        return a;
    } else if(b <= a && b <= c) {
        return b;
    } else {
        return c;
    }
}

void freeMatrix(long double*** matrix, int row, int col) {
    for(int i = 0; i < row; i++) {
        for(int j = 0; j < col; j++) {
            free(matrix[i][j]);
        }
        free(matrix[i]);
    }
    free(matrix);
}
