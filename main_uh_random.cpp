#include "other/data_utility.h"
#include "other/operation.h"
#include "other/GeoGreedy.h"
#include "other/DMM.h"
#include "other/lp.h"
#include "highdim.h"
#include "other/data_struct.h"
#include "other/read_write.h"
#include "other/maxUtility.h"
#include "other/medianhull.h"
#include "util.h"

#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <cmath>
#include <chrono>
#include <random>
#include <set>
#include <vector>
#include <algorithm>
#include <string>
#include <sstream>

using namespace std;


int main(int argc, char *argv[]){
    // Default parameters
    char *input = (char*)"car.txt";
    
    // Algorithm parameters
    int s = 2;
    double epsilon = 0.0;
    int maxRound = 1000;
    double Qcount, Csize;
    int prune_option = RTREE;
    int dom_option = HYPER_PLANE;
    int stop_option = EXACT_BOUND;
    int cmp_option = RANDOM;
    int num_questions = 100;
    
    // Allow dataset selection via command line
    if (argc > 1) {
        input = argv[1];
    }
    
    printf("=== UH-Random Algorithm ===\n");
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

    std::set<int> set_final_dimensions;
    // wait for user to input the final dimensions
    printf("Please input the number of final dimensions: ");
    int final_d;
    scanf("%d", &final_d);
    for (int i = 0; i < final_d; i++) {
        int dim;
        scanf("%d", &dim);
        set_final_dimensions.insert(dim);
    }

    printf("number of final dimensions: %d\n", final_d);
	point_set_t* D_prime = alloc_point_set(n);
	for (int j=0;j<n;++j){
		D_prime->points[j] = alloc_point(final_d);
		D_prime->points[j]->id = skyline->points[j]->id;
		for (int p=0;p<final_d;++p){
			D_prime->points[j]->coord[p] = skyline->points[j]->coord[*next(set_final_dimensions.begin(), p)];
		}
	}

    point_set_t* skyline_D_prime = skyline_point(D_prime);

    // Use max_utility_with_questions instead of max_utility to incorporate pre-recorded questions
    point_t* opt_p = max_utility(P_raw, set_final_dimensions, skyline_D_prime, u, s, epsilon, num_questions, Qcount, Csize, cmp_option, stop_option, prune_option, dom_option);
    // Find the point in skyline that matches the id of opt_p
    point_t* matched_point = nullptr;
    for (int i = 0; i < n; ++i) {
        if (skyline->points[i]->id == opt_p->id) {
            matched_point = skyline->points[i];
            break;
        }
    }
    if (matched_point == nullptr) {
        printf("Error: Could not find point in skyline with id %d\n", opt_p->id);
        exit(1);
    }
    
    printf("\n=== FINAL RECOMMENDATION ===\n");
    printf("Option %d: ", matched_point->id);
    for (int j = 0; j < final_d; j++) {
        // find the id to use P_raw
        int id = matched_point->id;
        for (int k = 0; k < P_raw->numberOfPoints; k++) {
            if (P_raw->points[k]->id == id) {
                id = k;
                break;
            }
        }
        printf("dim %d: %.4f ", *next(set_final_dimensions.begin(), j), P_raw->points[id]->coord[*next(set_final_dimensions.begin(), j)]);
    }
    printf("\n");

    // printf("\nQuestions asked: %d\n", num_quest_init-num_questions);
    // printf("========================================\n");

    // Cleanup
    release_point_set(skyline, false);
    release_point(u);
    release_point_set(P, true);
    release_point_set(P_raw, true);
    release_point_set(D_prime, true);
    release_point_set(skyline_D_prime, false);
    
    return 0;
}
