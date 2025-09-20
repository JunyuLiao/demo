#include "maxUtility.h"
#include <iostream>
#include <string>

// get the index of the "current best" point
// P: the input car set
// C_idx: the indexes of the current candidate favorite car in P
// ext_vec: the set of extreme vecotr
int get_current_best_pt(point_set_t* P, vector<int>& C_idx, vector<point_t*>& ext_vec)
{
	int dim = P->points[0]->dim;

	// the set of extreme points of the candidate utility range R
	vector<point_t*> ext_pts;
	ext_pts = get_extreme_pts(ext_vec);

	// use the "mean" utility vector in R (other strategies could also be used)
	point_t* mean = alloc_point(dim);
	for(int i = 0; i < dim; i++)
	{
		mean->coord[i] = 0;
	}
	for(int i = 0; i < ext_pts.size(); i++)
	{
		for(int j = 0; j < dim; j++)
			mean->coord[j] += ext_pts[i]->coord[j];
	}
	for(int i = 0; i < dim; i++)
	{
		mean->coord[i] /= ext_pts.size();
	}

	// look for the maximum utility point w.r.t. the "mean" utility vector
	int best_pt_idx;
	double max = 0;
	for(int i = 0; i < C_idx.size(); i++)
	{
		point_t* pt = P->points[C_idx[i]];

		double v = dot_prod(pt, mean);
		if(v > max)
		{
			max = v;
			best_pt_idx =  C_idx[i];
		}
	}

	for(int i = 0; i < ext_pts.size(); i++)
		release_point(ext_pts[i]);
	return best_pt_idx;
}

// generate s cars for selection in a round
// P: the input car set
// C_idx: the indexes of the current candidate favorite car in P
// s: the number of cars for user selection
// current_best_idx: the current best car
// last_best: the best car in previous interaction
// frame: the frame for obtaining the set of neibouring vertices of the current best vertiex (used only if cmp_option = SIMPLEX)
// cmp_option: the car selection mode, which must be either SIMPLEX or RANDOM
vector<int> generate_S(point_set_t* P, vector<int>& C_idx, int s, int current_best_idx, int& last_best, vector<int>& frame, int cmp_option)
{
	// the set of s cars for selection
	vector<int> S;

	if(cmp_option == RANDOM) // RANDOM car selection mode
	{
		// randoming select at most s non-overlaping cars in the candidate set 
		while(S.size() < s && S.size() < C_idx.size())
		{
			int idx = rand() % C_idx.size();

			bool isNew = true;
			for(int i = 0; i < S.size(); i++)
			{
				if(S[i] == idx)
				{
					isNew = false;
					break;
				}
			}
			if(isNew)
				S.push_back(idx);
		}
	}
	else if(cmp_option == SIMPLEX) // SIMPLEX car selection mode
	{
		if(last_best != current_best_idx || frame.size() == 0) // the new frame is not computed before (avoid duplicate frame computation)
		{
			// create one ray for each car in P for computing the frame
			vector<point_t*> rays;
			int best_i = -1;
			for(int i = 0; i < P->numberOfPoints; i++)
			{
				if(i == current_best_idx)
				{
					best_i = i;
					continue;
				}

				point_t* best = P->points[current_best_idx];
				point_t* newRay = sub(P->points[i], best);
				rays.push_back(newRay);
			}

			// frame compuatation
			frameConeFastLP(rays, frame);
		
			// update the indexes lying after current_best_idx
			for(int i = 0; i < frame.size(); i++)
			{
				if(frame[i] >= current_best_idx)
					frame[i]++;

				//S[i] = C_idx[S[i]];
			}

			for(int i = 0; i < rays.size(); i++)
				release_point(rays[i]);
		}

		//printf("current_best: %d, frame:", P->points[current_best_idx]->id);
		//for(int i = 0; i < frame.size(); i++)
		//	printf("%d ", P->points[frame[i]]->id);
		//printf("\n");

		//S.push_back(best_i);

		for(int j = 0; j < C_idx.size(); j++)
		{
			if(C_idx[j] == current_best_idx) // it is possible that current_best_idx is no longer in the candidate set, then no need to compare again
			{
				S.push_back(j);
				break;
			}
		}

		// select at most s non-overlaping cars in the candidate set based on "neighboring vertices" obtained via frame compuation
		for(int i = 0; i < frame.size() && S.size() < s; i++)
		{
			for(int j = 0; j < C_idx.size() && S.size() < s; j++)
			{
				if(C_idx[j] == current_best_idx)
					continue;

				if(C_idx[j] == frame[i])
					S.push_back(j);
			}
		}

		// if less than s car are selected, fill in the remaing one
		if (S.size() < s && C_idx.size() > s)
		{
			for (int j = 0; j < C_idx.size(); j++)
			{
				bool noIn = true;
				for (int i = 0; i < S.size(); i++)
				{
					if (j == S[i])
						noIn = false;
				}
				if (noIn)
					S.push_back(j);

				if (S.size() == s)
					break;
			}
		}
	}
	else // for testing only. Do not use this!
	{
		vector<point_t*> rays;

		int best_i = -1;
		for(int i = 0; i < C_idx.size(); i++)
		{
			if(C_idx[i] == current_best_idx)
			{
				best_i = i;
				continue;
			}

			point_t* best = P->points[current_best_idx];

			point_t* newRay = sub(P->points[C_idx[i]], best);

			rays.push_back(newRay);
		}

		partialConeFastLP(rays, S, s - 1);
		if(S.size() > s - 1)
			S.resize(s - 1);
		for(int i = 0; i < S.size(); i++)
		{
			if(S[i] >= best_i)
				S[i]++;

			//S[i] = C_idx[S[i]];
		}
		S.push_back(best_i);


		for(int i = 0; i < rays.size(); i++)
			release_point(rays[i]);
	}
	return S;
}


// generate the options for user selection and update the extreme vecotrs based on the user feedback
// wPrt: record user's feedback
// P_car: the set of candidate cars with seqential ids
// skyline_proc_P: the skyline set of normalized cars
// C_idx: the indexes of the current candidate favorite car in skyline_proc_P
// ext_vec: the set of extreme vecotr
// current_best_idx: the current best car
// last_best: the best car in previous interaction
// frame: the frame for obtaining the set of neibouring vertices of the current best vertiex (used only if cmp_option = SIMPLEX)
// cmp_option: the car selection mode, which must be either SIMPLEX or RANDOM
void update_ext_vec(point_set_t* P_raw, std::set<int> set_final_dimensions, point_set_t* P, vector<int>& C_idx, point_t* u, int s, vector<point_t*>& ext_vec, int& current_best_idx, int& last_best, vector<int>& frame, int cmp_option, bool &keep_answer)
{
	// generate s cars for selection in a round
	vector<int> S = generate_S(P, C_idx, s, current_best_idx, last_best, frame, cmp_option);

	int m = set_final_dimensions.size();
    // double max = 0;
    // int max_i = 0;

    printf("\nPlease choose the option you favor more:\n");
    printf("dimensions: \n");
    for (int j = 0; j < m; ++j) {
        printf("|%10d", *next(set_final_dimensions.begin(), j));
    }
    printf("|\n");
    printf("-----------------------------------------------------------------\n");

    for (int j = 0; j < S.size(); ++j) {
        for (int k = 0; k < m; ++k) {
            printf("|%10.3f", P_raw->points[P->points[C_idx[S[j]]]->id]->coord[*next(set_final_dimensions.begin(), k)]);
        }
        printf("|\n");
    }
    printf("-----------------------------------------------------------------\n");
    printf("Your choice (0 for not interested): ");
    fflush(stdout);  // Force output to be sent immediately

	int max_i = -1;
    
    // // Use web-compatible input method
    // std::string input;
    // if (std::getline(std::cin, input)) {
    //     try {
    //         max_i = std::stoi(input);
    //     } catch (const std::exception& e) {
    //         printf("Invalid input, defaulting to 0\n");
    //         max_i = 0;
    //     }
    // } else {
    //     printf("No input received, defaulting to 0\n");
    //     max_i = 0;
    // }
	while (max_i != 0 && max_i != 1 && max_i != 2 && max_i != -99){
        scanf("%d", &max_i);
    }
	if (max_i == -99) {
		keep_answer = false;
		return;
	}
	if (max_i != 0) max_i = max_i-1;

	// get the better car among those from the user
	last_best = current_best_idx;
	current_best_idx = C_idx[S[max_i]];
	//if(current_best_idx == S[max_i])
		

	// for each non-favorite car, create a new extreme vecotr
	for(int i = 0; i < S.size(); i++)
	{
		if(max_i == i)
			continue;

		point_t* tmp = sub(P->points[ C_idx[S[i]] ], P->points[ C_idx[S[max_i]] ]);
		C_idx[S[i]] = -1;

		point_t* new_ext_vec = scale(1 / calc_len(tmp), tmp);
		
		release_point(tmp);
		ext_vec.push_back(new_ext_vec);
	}

	// directly remove the non-favorite car from the candidate set
	vector<int> newC_idx;
	for(int i = 0; i < C_idx.size(); i++)
	{
		if(C_idx[i] >= 0)
			newC_idx.push_back(C_idx[i]);
	}
	C_idx = newC_idx;
}

// the main interactive algorithm
// P: the input dataset (assumed skyline)
// u: the unkonwn utility vector
// s: the question size
// epsilon: the required regret ratio
// maxRound: the maximum number of rounds of interacitons
// Qcount: the number of question asked
// Csize: the size the candidate set when finished
// cmp_option: the car selection mode, which must be either SIMPLEX or RANDOM
// stop_option: the stopping condition, which must be NO_BOUND or EXACT_BOUND or APRROX_BOUND
// prune_option: the skyline algorithm, which must be either SQL or RTREE
// dom_option: the domination checking mode, which must be either HYPER_PLANE or CONICAL_HULL
point_t* max_utility(point_set_t* P_raw, std::set<int> set_final_dimensions, point_set_t* P, point_t* u, int s,  double epsilon, int maxRound, double &Qcount, double &Csize,  int cmp_option, int stop_option, int prune_option, int dom_option){
	
	bool keep_answer = true;
	int dim = P->points[0]->dim;

	// the indexes of the candidate set
	// initially, it is all the skyline cars
	vector<int> C_idx;
	for(int i = 0; i < P->numberOfPoints; i++)
		C_idx.push_back(i);

	double time;

	// the initial exteme vector sets V = {−ei | i ∈ [1, d], ei [i] = 1 and ei [j] = 0 if i , j}.
	vector<point_t*> ext_vec;
	for (int i = 0; i < dim; i++)
	{
		point_t* e = alloc_point(dim);
		for (int j = 0; j < dim; j++)
		{
			if (i == j)
				e->coord[j] = -1;
			else
				e->coord[j] = 0;
		}
		ext_vec.push_back(e);
	}

	int current_best_idx = -1;
	int last_best = -1;
	vector<int> frame;

	// get the index of the "current best" point
	//if(cmp_option != RANDOM)
	current_best_idx = get_current_best_pt(P, C_idx, ext_vec);
	
	// if not skyline
	//sql_pruning(P, C_idx, ext_vec);

	// Qcount - the number of querstions asked
	// Csize - the size of the current candidate set

	Qcount = 0;
	double rr = 1;

	// interactively reduce the candidate set and shrink the candidate utility range
	while (C_idx.size()> 1 && (rr > epsilon  && !isZero(rr - epsilon)) && Qcount <  maxRound && keep_answer)  // while none of the stopping conditiong is true
	{
		Qcount++;
		sort(C_idx.begin(), C_idx.end()); // prevent select two different points after different skyline algorithms
		
		// generate the options for user selection and update the extreme vecotrs based on the user feedback
		update_ext_vec(P_raw, set_final_dimensions, P, C_idx, u, s, ext_vec, current_best_idx, last_best, frame, cmp_option, keep_answer);

		if(C_idx.size()==1 ) // || global_best_idx == current_best_idx
			break;

		//update candidate set
		if(prune_option == SQL)
			sql_pruning(P, C_idx, ext_vec, rr, stop_option, dom_option);
		else
			rtree_pruning(P, C_idx, ext_vec, rr, stop_option, dom_option);
	}

	// get the final result 
	point_t* result = P->points[get_current_best_pt(P, C_idx, ext_vec)];
	Csize = C_idx.size();

	for (int i = 0; i < ext_vec.size(); i++)
		release_point(ext_vec[i]);

	return result;
}

// // construct extreme vectors from question mappings
// void construct_ext_vec_from_questions(point_set_t* P, const question_mapping& qm, point_t* u, vector<point_t*>& ext_vec, int full_dim, const std::map<int, int>& dim_mapping, point_set_t* D_prime)
// {
//     for (const auto& question : qm.questions) {
//         const std::set<int>& original_dimensions = question.first;
//         const std::vector<int>& tuple_indices = question.second;
        
//         // Check if the set contains more than one key dimension (non-zero weight in u)
//         int key_dim_count = 0;
//         for (int original_dim : original_dimensions) {
//             // Map to reduced dimension and check if it's a key dimension
//             if (dim_mapping.find(original_dim) != dim_mapping.end()) {
//                 int reduced_dim = dim_mapping.at(original_dim);
//                 if (reduced_dim < full_dim && u->coord[reduced_dim] > 0) {
//                     key_dim_count++;
//                 }
//             }
//         }
        
//         if (key_dim_count > 1 && tuple_indices.size() > 1) {
//             // Get the selected point (first in the vector) - map from original to reduced index
//             int original_selected_idx = tuple_indices[0];
//             if (D_prime->points[original_selected_idx] == NULL) {
//                 continue; // Skip if the point is not in the reduced dataset
//             }
//             int selected_idx = original_selected_idx;
//             point_t* p = P->points[selected_idx];
            
//             // For each non-selected point, create an extreme vector
//             for (size_t i = 1; i < tuple_indices.size(); i++) {
//                 int original_q_idx = tuple_indices[i];
//                 if (D_prime->points[original_q_idx] == NULL) {
//                     continue; // Skip if the point is not in the reduced dataset
//                 }
//                 int q_idx = original_q_idx;
//                 point_t* q = P->points[q_idx];
                
//                 // Create the difference vector p - q
//                 point_t* diff = alloc_point(full_dim);
//                 for (int j = 0; j < full_dim; j++) {
//                     diff->coord[j] = 0; // Initialize to 0
//                 }
                
//                 // Fill in the dimensions that were shown in the question
//                 for (int original_dim : original_dimensions) {
//                     if (dim_mapping.find(original_dim) != dim_mapping.end()) {
//                         int reduced_dim = dim_mapping.at(original_dim);
//                         if (reduced_dim < full_dim) {
//                             diff->coord[reduced_dim] = q->coord[reduced_dim] - p->coord[reduced_dim];
//                         }
//                     }
//                 }
                
//                 // Normalize the vector
//                 double len = calc_len(diff);
//                 if (len > 0) {
//                     point_t* new_ext_vec = scale(1.0 / len, diff);
//                     ext_vec.push_back(new_ext_vec);
//                 }
                
//                 release_point(diff);
//             }
//         }
//     }
// }

// // the main interactive algorithm with pre-recorded questions
// point_t* max_utility_with_questions(point_set_t* P_raw, std::set<int> set_final_dimensions, point_set_t* P, point_t* u, int s, double epsilon, int maxRound, double &Qcount, double &Csize, int cmp_option, int stop_option, int prune_option, int dom_option, const question_mapping& qm, const std::map<int, int>& dim_mapping, point_set_t* D_prime)
// {
//     int dim = P->points[0]->dim;

//     // the indexes of the candidate set
//     // initially, it is all the skyline cars
//     vector<int> C_idx;
//     for(int i = 0; i < P->numberOfPoints; i++)
//         C_idx.push_back(i);

//     double time;

//     // the initial exteme vector sets V = {−ei | i ∈ [1, d], ei [i] = 1 and ei [j] = 0 if i , j}.
//     vector<point_t*> ext_vec;
//     for (int i = 0; i < dim; i++)
//     {
//         point_t* e = alloc_point(dim);
//         for (int j = 0; j < dim; j++)
//         {
//             if (i == j)
//                 e->coord[j] = -1;
//             else
//                 e->coord[j] = 0;
//         }
//         ext_vec.push_back(e);
//     }

//     // Construct extreme vectors from pre-recorded questions using dimension mapping
//     construct_ext_vec_from_questions(D_prime, qm, u, ext_vec, dim, dim_mapping, D_prime);

//     int current_best_idx = -1;
//     int last_best = -1;
//     vector<int> frame;

//     // get the index of the "current best" point
//     current_best_idx = get_current_best_pt(P, C_idx, ext_vec);
    
//     // Qcount - the number of querstions asked
//     // Csize - the size of the current candidate set

//     Qcount = 0;
//     double rr = 1;

//     // interactively reduce the candidate set and shrink the candidate utility range
//     while (C_idx.size()> 1 && (rr > epsilon  && !isZero(rr - epsilon)) && Qcount <  maxRound)  // while none of the stopping conditiong is true
//     {
//         Qcount++;
//         sort(C_idx.begin(), C_idx.end()); // prevent select two different points after different skyline algorithms
        
//         // generate the options for user selection and update the extreme vecotrs based on the user feedback
//         update_ext_vec(P_raw, set_final_dimensions, P, C_idx, u, s, ext_vec, current_best_idx, last_best, frame, cmp_option);

//         if(C_idx.size()==1 ) // || global_best_idx == current_best_idx
//             break;

//         //update candidate set
//         if(prune_option == SQL)
//             sql_pruning(P, C_idx, ext_vec, rr, stop_option, dom_option);
//         else
//             rtree_pruning(P, C_idx, ext_vec, rr, stop_option, dom_option);
//     }

//     // get the final result 
//     point_t* result = P->points[get_current_best_pt(P, C_idx, ext_vec)];
//     Csize = C_idx.size();

//     for (int i = 0; i < ext_vec.size(); i++)
//         release_point(ext_vec[i]);

//     return result;
// }
