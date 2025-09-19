#include "other/data_utility.h"
#include "other/operation.h"
#include "other/sphere.h"
#include "other/GeoGreedy.h"
#include "other/DMM.h"
#include "other/lp.h"
#include <iostream>
#include "stdlib.h"
#include "stdio.h"
#include "time.h"
#include <cmath>
#include <chrono>
#include <random>
#include <set>
#include <vector>
#include <algorithm>
#include "other/data_struct.h"
#include "other/read_write.h"
#include "other/maxUtility.h"
#include "attribute_subset.h"
#include "other/medianhull.h"
#include <ctime>
using namespace std;

int show_to_user(point_set_t* P_raw, point_set_t* S, std::set<int> selected_dimensions, point_t* u);

point_set_t* generate_S(point_set_t* P, std::set<int> selected_dimensions, int size);