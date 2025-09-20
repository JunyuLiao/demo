#include "util.h"
#include <iostream>
#include <string>

int show_to_user(point_set_t* P_raw, point_set_t* S, std::set<int> selected_dimensions, point_t* u){
    int m = selected_dimensions.size();
    int size = S->numberOfPoints;

    // Skip printing S details - not needed for user interaction

    printf("\nPlease choose the option you favor more:\n");
    printf("|%10s", "Options");
    for (int j = 0; j < m; ++j) {
        printf("|%10d", *next(selected_dimensions.begin(), j));
    }
    printf("|\n");
    printf("------------------------------------------------------------------------------\n");

    for (int j = 0; j < size; ++j) {
        printf("|%10s", ("Option" + std::to_string(j+1)).c_str());
        for (int k = 0; k < m; ++k) {
            printf("|%10.3f", P_raw->points[S->points[j]->id]->coord[*next(selected_dimensions.begin(), k)]);
        }
        printf("|\n");
    }
    printf("------------------------------------------------------------------------------\n");
    printf("Your choice (0 for not interested): ");
    fflush(stdout);  // Force output to be sent immediately
    
    // Fixed input handling - read once and validate
    int maxIdx = -1;
    scanf("%d", &maxIdx);
    
    // Validate and return appropriate value
    if (maxIdx == -99) {
        return -99;  // Stop signal
    } else if (maxIdx == 0) {
        return -1;   // Not interested
    } else if (maxIdx == 1 || maxIdx == 2) {
        return maxIdx - 1;  // Convert to 0-based index
    } else {
        return -1;   // Invalid input, treat as not interested
    }
}

point_set_t* generate_S(point_set_t* P, std::set<int> selected_dimensions, int size){
    //Restrict D to dimensions i*d_hat, i*d_hat+1, ..., i*d_hat+d_hat-1
    int n = P->numberOfPoints;
    point_set_t* D = alloc_point_set(n);
    for (int j=0;j<n;++j){
        D->points[j] = alloc_point(selected_dimensions.size());
        D->points[j]->id = P->points[j]->id;
        for (int p=0;p<selected_dimensions.size();++p){
            D->points[j]->coord[p] = P->points[j]->coord[*next(selected_dimensions.begin(), p)];
        }
    }
    point_set_t* S = alloc_point_set(size);
    // select randomly size points from D
    // create a random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(0, n-1);
    // select size points from D
    for (int j = 0; j < size; ++j) {
        int idx = dis(gen);
        // S->points[j] = D->points[idx];
        // allocate a new point
        S->points[j] = alloc_point(selected_dimensions.size());
        S->points[j]->id = D->points[idx]->id;
        for (int p=0;p<selected_dimensions.size();++p){
            S->points[j]->coord[p] = D->points[idx]->coord[p];
        }
    }
    release_point_set(D, true);
    return S;
}
