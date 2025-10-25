#include "highdim.h"
#include <algorithm>
#include <iterator>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <chrono>


highdim_output* interactive_highdim(point_set_t* P_raw, point_set_t* skyline, int size, int d_bar, int d_hat, int d_hat_2, point_t* u, int K, int s, double epsilon, int maxRound, double& Qcount, double& Csize, int cmp_option, int stop_option, int prune_option, int dom_option, int& num_questions){

    bool keep_answer = true;
    int n = skyline->numberOfPoints;
    int d = skyline->points[0]->dim;
    // record the time for phase 1 and 2
    double time_12 = 0.0;
    auto start_time_12 = std::chrono::high_resolution_clock::now();
    
    // Initialize question mapping to record all questions asked
    // question_mapping qm;
    
    // Helper lambdas to append a simple phase record to user_feedback.json
    auto trim = [](const std::string& s) {
        size_t a = s.find_first_not_of(" \t\n\r");
        if (a == std::string::npos) return std::string("");
        size_t b = s.find_last_not_of(" \t\n\r");
        return s.substr(a, b - a + 1);
    };
    auto append_phase_record = [&](int phase){
        const char* dir = std::getenv("DATA_DIR");
        std::string base = (dir && *dir) ? std::string(dir) : std::string("/data");
        if (!base.empty() && base.back() == '/') base.pop_back();
        const char* sid = std::getenv("SESSION_ID");
        std::string sessionId = sid ? std::string(sid) : std::string("default");
        std::string filepath = base + "/sessions/" + sessionId + ".json";
        std::ifstream in(filepath);
        std::string content; if (in.good()) { std::ostringstream b; b << in.rdbuf(); content = b.str(); }
        in.close();
        if (content.empty()) content = "{}";
        std::string obj = content;
        size_t key = obj.find("\"phase\"");
        if (key == std::string::npos) {
            bool needs_comma = obj.find(':') != std::string::npos;
            std::ostringstream ins; ins << (needs_comma ? "," : "") << "\"phase\":" << phase;
            obj.insert(obj.size()-1, ins.str());
        } else {
            size_t colon = obj.find(':', key);
            if (colon != std::string::npos) {
                size_t val_begin = colon + 1; while (val_begin < obj.size() && obj[val_begin] == ' ') ++val_begin;
                size_t val_end = val_begin; while (val_end < obj.size() && obj[val_end] != ',' && obj[val_end] != '}') ++val_end;
                std::ostringstream val; val << " " << phase;
                obj.replace(val_begin, val_end - val_begin, val.str());
            }
        }
        std::ofstream o(filepath, std::ios::trunc); o << obj; o.close();
    };

    int stop_phase = 0; // 0 means not stopped early; 1/2 indicate stopping phase

    // phase 1: narrow down the dimensions
    printf("Phase 1: Initializing dimension selection...\n");
	// store the dimensions if the user is interested in at least one in the set
	std::set<int> selected_dimensions;
    for (int i = 0; i < d; ++i) selected_dimensions.insert(i); // initial all dimensions
    // keep a copy of the whole set of dimensions
    std::set<int> selected_dimensions_init = selected_dimensions;

	for (int i=0;i<d/d_hat;++i){
        // first check if the user is willing to answer questions
        if (num_questions <= 0 && keep_answer){
            break; // no more questions left
        }
        std::set<int> selected_dimensions_i;
        for (int j = 0; j < d_hat; ++j) {
            selected_dimensions_i.insert(i*d_hat+j);
        }
        // generate S
        point_set_t* S = generate_S(skyline, selected_dimensions_i, size);
        // interaction
        int maxIdx = show_to_user(P_raw, S, selected_dimensions_i, u);
        if (maxIdx == -99) {
            keep_answer = false;
            stop_phase = 1;
            release_point_set(S, true);
            selected_dimensions_i.clear();  
            break;
        }
        else {num_questions -= 1;}
        int id = maxIdx != -1 ? S->points[maxIdx]->id : -1;
        
		if (id==-1){
			for (int j=0;j<selected_dimensions_i.size();++j) selected_dimensions.erase(*next(selected_dimensions_i.begin(), j));
		}
        release_point_set(S, true);
        selected_dimensions_i.clear();  
	}
	// phase 2: narrow down the at most d_hat*d' dimensions further
    printf("Phase 2: Refining dimension selection...\n");
	// apply the Generalized_binary-splitting_algorithm
	// store the dimensions the user is interested in
	set<int> final_dimensions;
	int d_left = selected_dimensions.size();
    // for debugging purpose
    if (d_left == 0){
        printf("error_1: d_left is 0\n");
        exit(1);
    }
    
    // If no dimensions were removed in Phase 1, we need to handle this case
    // by randomly selecting some dimensions to remove to make the algorithm work
    if (d_left == d) {
        printf("No dimensions removed in Phase 1, randomly selecting dimensions to remove...\n");
        // Remove d_hat dimensions randomly to make the algorithm work
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dis(0, d-1);
        
        for (int i = 0; i < d_hat && selected_dimensions.size() > 0; ++i) {
            auto it = selected_dimensions.begin();
            std::advance(it, dis(gen) % selected_dimensions.size());
            selected_dimensions.erase(it);
        }
        d_left = selected_dimensions.size();
    }
    int d_target = d_bar;
	while (d_left > 0 && num_questions > 0 && final_dimensions.size() < d_bar && keep_answer){
        d_left = selected_dimensions.size();
		if (d_left <= 2*d_target-2){
			for (int i=0;i<d_left;++i){
                // extract the first dimension in selected_dimensions
                // for debugging purpose
                if (selected_dimensions.size() == 0){
                    printf("error_2: selected_dimensions is empty\n");
                    exit(1);
                }
                // construct the selected_dimensions set
                std::set<int> selected_dimensions_i;
                selected_dimensions_i.insert(*selected_dimensions.begin());
                while (selected_dimensions_i.size() < d_hat){
                    // choose a random dimension from selected_dimensions_init, but not in selected_dimensions or final_dimensions
                    std::random_device rd;
                    std::mt19937 gen(rd());
                    std::uniform_int_distribution<int> dis(0, selected_dimensions_init.size()-1);
                    int idx = dis(gen);
                    int dim = *next(selected_dimensions_init.begin(), idx);
                    if (selected_dimensions.find(dim) == selected_dimensions.end() && final_dimensions.find(dim) == final_dimensions.end()){
                        selected_dimensions_i.insert(dim);
                    }
                }
                // generate S
                point_set_t* S = generate_S(skyline, selected_dimensions_i, size);
                // interaction
                int maxIdx = show_to_user(P_raw, S, selected_dimensions_i, u);
                if (maxIdx == -99) {
                    keep_answer = false;
                    stop_phase = 2;
                    release_point_set(S, true);
                    selected_dimensions_i.clear();  
                    break;
                }
                else {num_questions -= 1;}
                int id = maxIdx != -1 ? S->points[maxIdx]->id : -1;
				if (id == -1){
					selected_dimensions.erase(selected_dimensions.begin());
                    d_left -= 1;
				}
				else {
					final_dimensions.insert(*selected_dimensions.begin());
					selected_dimensions.erase(selected_dimensions.begin());
                    d_left -= 1;
                    d_target -= 1;

				}
                release_point_set(S, true);
                selected_dimensions_i.clear();
			}
			// break; // unnecessary
		}
		else {
			int l = d_left - d_target + 1;
            int alpha = floor(log2(double(l)/d_target));
            int group_size = pow(2, alpha);
            // for debugging purpose
            if (group_size == 0){
                printf("error_3: group_size is 0\n");
                exit(1);
            }
            if (group_size > d_left){
                printf("error_4: group_size is greater than d_left\n");
                exit(1);
            }
            int id;
            if (group_size == 1){
                std::set<int> selected_dimensions_i;
                selected_dimensions_i.insert(*selected_dimensions.begin());
                while (selected_dimensions_i.size() < d_hat){
                    // choose a random dimension from selected_dimensions_init, but not in selected_dimensions or final_dimensions
                    std::random_device rd;
                    std::mt19937 gen(rd());
                    std::uniform_int_distribution<int> dis(0, selected_dimensions_init.size()-1);
                    int idx = dis(gen);
                    int dim = *next(selected_dimensions_init.begin(), idx);
                    if (selected_dimensions.find(dim) == selected_dimensions.end() && final_dimensions.find(dim) == final_dimensions.end()){
                        selected_dimensions_i.insert(dim);
                    }
                }
                point_set_t* S = generate_S(skyline, selected_dimensions_i, size);
                int maxIdx = show_to_user(P_raw, S, selected_dimensions_i, u);
                id = maxIdx != -1 ? S->points[maxIdx]->id : -1;
                if (maxIdx == -99) {
                    keep_answer = false;
                    stop_phase = 2;
                    release_point_set(S, true);
                    selected_dimensions_i.clear();  
                    break;
                }
                else {num_questions -= 1;}
                release_point_set(S, true);
                selected_dimensions_i.clear();
            }
            else {
                // select the first "group_size" dimensions
                std::set<int> selected_dimensions_i;
                for (int j = 0; j < group_size; ++j) {
                    selected_dimensions_i.insert(*next(selected_dimensions.begin(), j));
                }
                while (selected_dimensions_i.size() < d_hat){
                    // choose a random dimension from selected_dimensions_init, but not in selected_dimensions or final_dimensions
                    std::random_device rd;
                    std::mt19937 gen(rd());
                    std::uniform_int_distribution<int> dis(0, selected_dimensions_init.size()-1);
                    int idx = dis(gen);
                    int dim = *next(selected_dimensions_init.begin(), idx);
                    if (selected_dimensions.find(dim) == selected_dimensions.end() && final_dimensions.find(dim) == final_dimensions.end()){
                        selected_dimensions_i.insert(dim);
                    }
                }
                point_set_t* S = generate_S(skyline, selected_dimensions_i, size);
                int maxIdx = show_to_user(P_raw, S, selected_dimensions_i, u);
                id = maxIdx != -1 ? S->points[maxIdx]->id : -1;
                if (maxIdx == -99) {
                    keep_answer = false;
                    release_point_set(S, true);
                    selected_dimensions_i.clear();  
                    break;
                }
                else {num_questions -= 1;}
        
                release_point_set(S, true);
                selected_dimensions_i.clear();
            }
			if (id == -1){
				for (int j=0;j<group_size;++j){
					selected_dimensions.erase(selected_dimensions.begin());
				}
				d_left -= group_size;
				continue;
			}
			else {
                // check if the user is willing to answer more questions
                if (num_questions <= 0 && keep_answer) break;
                if (group_size == 1){
                    final_dimensions.insert(*selected_dimensions.begin());
                    selected_dimensions.erase(selected_dimensions.begin());
                    d_left -= 1;
                    d_target -= 1;
                }
                else{
                    // do a binary search to find one dimension
                    int left = 0;
                    int right = group_size-1;
                    while (left < right && num_questions > 0 && keep_answer){
                        int mid = left + (right - left) / 2;
                        if (mid == left){
                            if (right == left){
                                // one dimension has been found
                                final_dimensions.insert(*next(selected_dimensions.begin(), mid));
                                selected_dimensions.erase(next(selected_dimensions.begin(), mid));
                                d_left -= 1;
                                d_target -= 1;
                                break;
                            }
                            else {
                                // check whether the dimension is in the left half or the right half
                                std::set<int> selected_dimensions_i;
                                selected_dimensions_i.insert(*next(selected_dimensions.begin(), left));
                                while (selected_dimensions_i.size() < d_hat){
                                    // choose a random dimension from selected_dimensions_init, but not in selected_dimensions or final_dimensions
                                    std::random_device rd;
                                    std::mt19937 gen(rd());
                                    std::uniform_int_distribution<int> dis(0, selected_dimensions_init.size()-1);
                                    int idx = dis(gen);
                                    int dim = *next(selected_dimensions_init.begin(), idx);
                                    if (selected_dimensions.find(dim) == selected_dimensions.end() && final_dimensions.find(dim) == final_dimensions.end()){
                                        selected_dimensions_i.insert(dim);
                                    }
                                    // If no dimensions were removed in Phase 1, selected_dimensions == selected_dimensions_init
                                    // In this case, we need to add dimensions from selected_dimensions instead
                                    else if (d_left == d && selected_dimensions.find(dim) != selected_dimensions.end() && final_dimensions.find(dim) == final_dimensions.end()){
                                        selected_dimensions_i.insert(dim);
                                    }
                                }
                                point_set_t* S = generate_S(skyline, selected_dimensions_i, size);
                                int maxIdx = show_to_user(P_raw, S, selected_dimensions_i, u);
                                id = maxIdx != -1 ? S->points[maxIdx]->id : -1;
                                if (maxIdx == -99) {
                                    keep_answer = false;
                                    stop_phase = 2;
                                    release_point_set(S, true);
                                    selected_dimensions_i.clear();  
                                    break;
                                }
                                else {num_questions -= 1;}
                                release_point_set(S, true);
                                selected_dimensions_i.clear();
                                if (id != -1){ // the dimension is in the left half
                                    final_dimensions.insert(*next(selected_dimensions.begin(), left));
                                    selected_dimensions.erase(next(selected_dimensions.begin(), left));
                                    d_left -= 1;
                                    d_target -= 1;
                                }
                                else { // the dimension is in the right half
                                    final_dimensions.insert(*next(selected_dimensions.begin(), right));
                                    selected_dimensions.erase(next(selected_dimensions.begin(), right));
                                    d_left -= 1;
                                    d_target -= 1;
                                }
                                break;
                            }
                        }

                        std::set<int> selected_dimensions_i;
                        for (int j = 0; j < mid-left+1; ++j) {
                            selected_dimensions_i.insert(*next(selected_dimensions.begin(), left+j));
                        }
                        while (selected_dimensions_i.size() < d_hat){
                            // choose a random dimension from selected_dimensions_init, but not in selected_dimensions or final_dimensions
                            std::random_device rd;
                            std::mt19937 gen(rd());
                            std::uniform_int_distribution<int> dis(0, selected_dimensions_init.size()-1);
                            int idx = dis(gen);
                            int dim = *next(selected_dimensions_init.begin(), idx);
                            if (selected_dimensions.find(dim) == selected_dimensions.end() && final_dimensions.find(dim) == final_dimensions.end()){
                                selected_dimensions_i.insert(dim);
                            }
                            // If no dimensions were removed in Phase 1, selected_dimensions == selected_dimensions_init
                            // In this case, we need to add dimensions from selected_dimensions instead
                            else if (d_left == d && selected_dimensions.find(dim) != selected_dimensions.end() && final_dimensions.find(dim) == final_dimensions.end()){
                                selected_dimensions_i.insert(dim);
                            }
                        }
                        point_set_t* S = generate_S(skyline, selected_dimensions_i, size);
                        int maxIdx = show_to_user(P_raw, S, selected_dimensions_i, u);
                        id = maxIdx != -1 ? S->points[maxIdx]->id : -1;
                        if (maxIdx == -99) {
                            keep_answer = false;
                            stop_phase = 2;
                            release_point_set(S, true);
                            selected_dimensions_i.clear();  
                            break;
                        }
                        else {num_questions -= 1;}
                        
                        release_point_set(S, true);
                        selected_dimensions_i.clear();
                        
                        if (id == -1){
                            // the dimension is in the right half, remove all dimensions in the left half
                            for (int j=0; j < mid-left+1; ++j){
                                selected_dimensions.erase(next(selected_dimensions.begin(), left));
                                d_left -= 1;
                            }
                            // left = mid+1;
                            right = right - mid + left - 1; // adjust the right pointer (since we are now operating a set)
                        }
                        else {
                            // the dimension is in the left half, however cannot remove dimensions in the right half
                            // for debugging purpose
                            if (right == mid){
                                printf("error_5: right == mid\n");
                                exit(1);
                            }
                            right = mid;
                        }
                    }
                }
			}
		}
	}
    auto end_time_12 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration_12 = end_time_12 - start_time_12;
    time_12 = duration_12.count();
	// phase 3: find the optimal tuple or the optimal subset
    printf("Phase 3: Starting interactive questioning...\n");
	// take the union of the final_dimensions and the selected_dimensions
	std::set<int> set_final_dimensions;
    if (selected_dimensions.size() == 0 || final_dimensions.size() == d_bar){
        set_final_dimensions = final_dimensions;
    }
    else {
        std::set_union(final_dimensions.begin(), final_dimensions.end(),
            selected_dimensions.begin(), selected_dimensions.end(),
            std::inserter(set_final_dimensions, set_final_dimensions.begin()));
    }
    // Guard: if the user stops at the first Phase 2 question, we may have no
    // confirmed final dimensions yet. Ensure we keep at least some dimensions
    // so Phase 3 can proceed without zero-dimension projections.
    if (set_final_dimensions.empty()) {
        // Prefer whatever is still in selected_dimensions
        if (!selected_dimensions.empty()) {
            int take = std::min(d_bar, static_cast<int>(selected_dimensions.size()));
            for (int i = 0; i < take; ++i) {
                set_final_dimensions.insert(*std::next(selected_dimensions.begin(), i));
            }
        } else {
            // Fallback: take the first d_bar raw dimensions
            int take = std::min(d_bar, d);
            for (int i = 0; i < take; ++i) {
                set_final_dimensions.insert(i);
            }
        }
    }
	// construct the final subset
	point_set_t* S_output = nullptr;

	int final_d = set_final_dimensions.size();
    printf("number of final dimensions: %d\n", final_d);
	point_set_t* D_prime = alloc_point_set(n);
	for (int j=0;j<n;++j){
		D_prime->points[j] = alloc_point(final_d);
		D_prime->points[j]->id = skyline->points[j]->id;
		for (int p=0;p<final_d;++p){
			D_prime->points[j]->coord[p] = skyline->points[j]->coord[*next(set_final_dimensions.begin(), p)];
		}
	}

    // take the skyline of the newly constructed dataset D_prime
    point_set_t* skyline_D_prime = skyline_point(D_prime);


    // printf("number of points in skyline_D: %d\n", skyline_D_prime->numberOfPoints);
    // construct the final u
    point_t* u_final = alloc_point(final_d);
    for (int j = 0; j < final_d; ++j) {
        u_final->coord[j] = u->coord[*next(set_final_dimensions.begin(), j)];
    }

    double time_3 = 0.0;
    auto start_time_3 = std::chrono::high_resolution_clock::now();
	if (num_questions > 0 && keep_answer){
		// apply the interactive code to select the optimal tuple
        // printf("Applying the interactive algorithm to select the optimal tuple\n");
        S_output = alloc_point_set(1);
        // if (skyline_D_prime->points[0]->dim == final_d) printf("dimension: %d\n", final_d);
        
        // Use max_utility_with_questions instead of max_utility to incorporate pre-recorded questions
        point_t* opt_p = max_utility(P_raw, set_final_dimensions, skyline_D_prime, u_final, s, epsilon, num_questions, Qcount, Csize, cmp_option, stop_option, prune_option, dom_option);
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
        S_output->points[0] = matched_point;
        num_questions -= Qcount;
    }
	else {
        // if (final_d >= K){
        //     // exit
        //     printf("error: final_d >= K\n");
        //     exit(1);
        // }
        // printf("Applying the attribute subset method to output a regret minimizing subset\n");
        if (final_d <= d_hat_2){
            point_set_t* S = sphereWSImpLP(skyline_D_prime, K);
            S_output = alloc_point_set(S->numberOfPoints);
            for (int j = 0; j < S->numberOfPoints; ++j){
                // Find the point in skyline with the corresponding id
                int target_id = S->points[j]->id;
                point_t* matched_point = nullptr;
                for (int i = 0; i < skyline->numberOfPoints; ++i) {
                    if (skyline->points[i]->id == target_id) {
                        matched_point = skyline->points[i];
                        break;
                    }
                }
                if (matched_point == nullptr) {
                    printf("Error: Could not find point in skyline with id %d\n", target_id);
                    exit(1);
                }
                S_output->points[j] = matched_point;
            }
        }
        else {
            S_output = attribute_subset(skyline, S_output, final_d, d_hat_2, K, set_final_dimensions);
        }
	}

    auto end_time_3 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration_3 = end_time_3 - start_time_3;
    time_3 = duration_3.count();
    // If the user stopped early, record the phase; otherwise phase 3
    if (!keep_answer && stop_phase >= 1) {
        append_phase_record(stop_phase);
    } else {
        append_phase_record(3);
    }

    //also return the information about the dimensions chosen, as stored in set_final_dimensions
    highdim_output* output = new highdim_output;
    output->S = S_output;
    output->final_dimensions = set_final_dimensions;
    output->time_12 = time_12;
    output->time_3 = time_3;
    // release the memory
    release_point_set(skyline_D_prime, false);
    release_point_set(D_prime, true);
    release_point(u_final);
    return output;
}

