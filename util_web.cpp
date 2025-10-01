#include "util.h"
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

static std::string join_ints(const std::vector<int>& values) {
    std::ostringstream oss;
    for (size_t i = 0; i < values.size(); ++i) {
        if (i > 0) oss << ",";
        oss << values[i];
    }
    return oss.str();
}

static std::string trim(const std::string& s) {
    size_t a = s.find_first_not_of(" \t\n\r");
    if (a == std::string::npos) return "";
    size_t b = s.find_last_not_of(" \t\n\r");
    return s.substr(a, b - a + 1);
}

static void append_interaction_record(const std::vector<int>& ids,
                                      const std::vector<int>& attributes,
                                      int answer) {
    const std::string filepath = "user_feedback.json";

    // Read existing file (if any)
    std::ifstream in(filepath);
    std::string content;
    if (in.good()) {
        std::ostringstream buffer;
        buffer << in.rdbuf();
        content = buffer.str();
    }
    in.close();

    std::string trimmed = trim(content);
    if (trimmed.empty()) trimmed = "[]";

    // Build one interaction item
    std::ostringstream item;
    item << "{\"id\":[" << join_ints(ids)
         << "],\"attribute\":[" << join_ints(attributes)
         << "],\"answer\":" << answer << "}";

    std::string out_doc;
    if (trimmed == "[]") {
        // If no skeleton present (should be created at start), create one
        out_doc = "[\n{\"interaction\":[" + item.str() + "]}\n]";
    } else {
        // Find the last top-level interaction array and append robustly
        size_t key_pos = trimmed.rfind("\"interaction\"");
        if (key_pos == std::string::npos) {
            // Fallback: add a new record with interaction
            size_t end_arr = trimmed.rfind(']');
            if (end_arr == std::string::npos) {
                out_doc = "[\n{\"interaction\":[" + item.str() + "]}\n]";
            } else {
                out_doc = trimmed.substr(0, end_arr) + ",\n{\"interaction\":[" + item.str() + "]}\n]";
            }
        } else {
            size_t open_br = trimmed.find('[', key_pos);
            if (open_br == std::string::npos) {
                // Insert fresh array before the closing brace of last object
                size_t end_arr = trimmed.rfind('}');
                if (end_arr == std::string::npos) {
                    out_doc = trimmed; // give up silently
                } else {
                    out_doc = trimmed.substr(0, end_arr) + ",\"interaction\":[" + item.str() + "]" + trimmed.substr(end_arr);
                }
            } else {
                // Find the matching closing bracket for the interaction array, accounting for nested arrays
                int depth = 1;
                size_t i = open_br + 1;
                for (; i < trimmed.size(); ++i) {
                    char ch = trimmed[i];
                    if (ch == '[') depth++;
                    else if (ch == ']') { depth--; if (depth == 0) break; }
                }
                if (i >= trimmed.size()) {
                    out_doc = trimmed; // malformed
                } else {
                    std::string inside = trimmed.substr(open_br + 1, i - open_br - 1);
                    std::string trimmed_inside = trim(inside);
                    std::string to_insert = trimmed_inside.empty() ? item.str() : ("," + item.str());
                    out_doc = trimmed.substr(0, i) + to_insert + trimmed.substr(i);
                }
            }
        }
    }

    std::ofstream out(filepath, std::ios::trunc);
    out << out_doc;
    out.close();
}

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
    
    // For web interface, we need to wait for user input
    int maxIdx = -1000000;

    // Accept any integer in [0..size] or -99 to stop
    while (true) {
        int rc = scanf("%d", &maxIdx);
        if (rc != 1) {
            // Clear invalid token
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF) {}
            printf("Invalid input. Enter 0..%d or -99 to stop: ", size);
            fflush(stdout);
            continue;
        }
        if (maxIdx == -99 || (maxIdx >= 0 && maxIdx <= size)) {
            break;
        }
        printf("Out of range. Enter 0..%d or -99 to stop: ", size);
        fflush(stdout);
    }

    // Prepare interaction record
    std::vector<int> shown_ids;
    for (int j = 0; j < size; ++j) {
        shown_ids.push_back(S->points[j]->id);
    }
    std::vector<int> attributes;
    for (int dim : selected_dimensions) attributes.push_back(dim);

    int chosen = (maxIdx == -99) ? -99 : (maxIdx - 1);

    // Append to user_feedback.json for any answered question (including Not Interested = -1),
    // only discard when the user stops (-99)
    if (chosen != -99) {
        append_interaction_record(shown_ids, attributes, chosen);
    }

    return chosen;
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