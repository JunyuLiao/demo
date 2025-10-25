#include "other/data_utility.h"
#include "other/operation.h"
#include "other/sphere.h"
#include "other/GeoGreedy.h"
#include "other/DMM.h"
#include "other/lp.h"
#include "highdim.h"

#include <iostream>
using namespace std;
#include "stdlib.h"
#include "stdio.h"
#include "time.h"
#include <cmath>
#include <chrono>
#include <random>
#include <set>
#include <vector>
#include <algorithm>
#include <string>
#include <sstream>

// Real interactive version for web interface
int main(int argc, char *argv[]){
    // Disable stdio buffering to ensure immediate flush in containerized envs
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    // Default parameters
    char *input = (char*)"car.txt";
    int size = 2;
    int d_prime = 3;
    int d_hat = 7;
    int d_hat_2 = 6;
    int K = 10;
    int d_bar = 5;
    int num_questions = 100;
    int num_quest_init = num_questions;
    
    // Algorithm parameters
    int s = 2;
    double epsilon = 0.0;
    int maxRound = 1000;
    double Qcount, Csize;
    int prune_option = RTREE;
    int dom_option = HYPER_PLANE;
    int stop_option = EXACT_BOUND;
    int cmp_option = RANDOM;
    
    // Allow dataset selection via command line
    if (argc > 1) {
        input = argv[1];
    }
    
    printf("=== High-Dimensional Regret Minimization Algorithm (Real) ===\n");
    printf("Loading dataset: %s\n", input);
    
    // Debug: Check if file exists
    FILE* test_fp = fopen(input, "r");
    if (test_fp == NULL) {
        printf("ERROR: Cannot open file %s\n", input);
        printf("Current working directory: ");
        system("pwd");
        printf("Files in current directory: ");
        system("ls -la");
    // Datasets directory may not exist; only list current directory
        exit(1);
    } else {
        printf("File %s opened successfully\n", input);
        fclose(test_fp);
    }
    
    point_set_t* P_raw = read_points(input);
    if (!P_raw) {
        printf("Error: Could not load dataset %s\n", input);
        return 1;
    }
    
    point_set_t* P = read_points(input);
    int n = P->numberOfPoints;
    linear_normalize(P);
    int d = P->points[0]->dim;
    
    printf("Dataset loaded: %d points, %d dimensions\n", n, d);
    
    point_set_t* skyline = skyline_point(P);
    printf("Skyline computed: %d points\n", skyline->numberOfPoints);
    
    // Set n to be the number of skyline points
    n = skyline->numberOfPoints;

    // Generate utility vector
    point_t* u = alloc_point(d);
    for (int i = 0; i < d; i++) u->coord[i] = 0;

    // Randomly select d_prime dimensions to have non-zero values
    std::vector<int> indices(d);
    for (int i = 0; i < d; i++) indices[i] = i;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(indices.begin(), indices.end(), gen);

    // Generate random values
    double sum = 0;
    for (int i = 0; i < d_prime; i++) {
        u->coord[indices[i]] = ((double)rand()) / RAND_MAX;
        sum += u->coord[indices[i]];
    }

    // normalize the utility vector
    for (int i = 0; i < d; i++) {
        u->coord[i] /= sum;
    }

    // Find the ground truth maximum utility point
    int maxIdx = 0;
    double maxValue = 0;
    for(int i = 0; i < skyline->numberOfPoints; i++)
    {
        double value = dot_prod(u, skyline->points[i]);
        if(value > maxValue)
        {
            maxValue = value;
            maxIdx = i;
        }
    }
    
    printf("Ground truth maximum utility point: ID %d\n", skyline->points[maxIdx]->id);
    printf("Starting interactive algorithm...\n");
    printf("Parameters: d_prime=%d, d_hat=%d, d_hat_2=%d, K=%d, s=%d\n", 
           d_prime, d_hat, d_hat_2, K, s);
    printf("Initializing algorithm phases...\n");
    printf("========================================\n");

    // Run the real interactive algorithm
    highdim_output* h = interactive_highdim(P_raw, skyline, size, d_bar, d_hat, d_hat_2, u, K, s, epsilon, maxRound, Qcount, Csize, cmp_option, stop_option, prune_option, dom_option, num_questions);
    
    double time_12 = h->time_12;
    double time_3 = h->time_3;
    
    point_set_t* S = h->S;
    std::set<int> final_dimensions = h->final_dimensions;

    // Print results
    if (S->numberOfPoints == 1){
        printf("\nPhase 3A completed\n");
    }
    else{
        printf("\nPhase 3B completed\n");
    }
    
    printf("\n=== FINAL RECOMMENDATION ===\n");
    for (int i = 0; i < S->numberOfPoints; i++) {
        printf("Option %d: ", S->points[i]->id);
        for (int j = 0; j < final_dimensions.size(); j++) {
            // find the id to use P_raw
            int id = S->points[i]->id;
            for (int k = 0; k < P_raw->numberOfPoints; k++) {
                if (P_raw->points[k]->id == id) {
                    id = k;
                    break;
                }
            }
            printf("dim %d: %.4f ", *next(final_dimensions.begin(), j), P_raw->points[id]->coord[*next(final_dimensions.begin(), j)]);
        }
        printf("\n");
    }

    // printf("\nQuestions asked: %d\n", num_quest_init-num_questions);
    // printf("========================================\n");

    // Cleanup
    release_point_set(skyline, false);
    release_point(u);
    release_point_set(h->S, false);
    release_point_set(P, true);
    release_point_set(P_raw, true);
    delete h;
    
    return 0;
}
