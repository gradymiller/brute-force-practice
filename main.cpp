#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <cstdint>
#include <bitset>


struct Solution {
	std::vector<std::bitset<200>> undecided;
	std::vector<std::bitset<200>> included;
	std::vector<std::bitset<200>> excluded;
	std::bitset<200> uncovered;
	std::bitset<200> covered;
	int team_size;
	int tree_depth;
	std::bitset<200> skills_mask;
};

int pickPerson(const Solution& state) {
	int best_index = -1;
	int max_diff = 0;
	for (size_t i=0; i<state.undecided.size(); ++i) {
		// num unique skills
		int diff = ((state.undecided[i] & ~state.covered) & state.skills_mask).count();

		if (diff > max_diff) {
			max_diff = diff;
			best_index = i;
		}
	}
	return best_index;
}

Solution simplify(Solution state) {
    bool changed = true;

    while (changed) {
        changed = false;

		// Remove subsets
        for (size_t i = 0; i < state.undecided.size(); ++i) {
            for (size_t j = i + 1; j < state.undecided.size();) {
                std::bitset<200> a = state.undecided[i] & ~state.covered;
                std::bitset<200> b = state.undecided[j] & ~state.covered;

                if ((a & b) == a) {
					// remove if a is subset of b
                    std::swap(state.undecided[i], state.undecided.back());
                    state.undecided.pop_back();
                    changed = true;
                    break;
                } else if ((a & b) == b) {
					// remove if b is a subset of a
                    std::swap(state.undecided[j], state.undecided.back());
                    state.undecided.pop_back();
                    changed = true;
                } else {
                    ++j;
                }
            }
        }

        int skill_count[200] = {0};

        // find what skills each remaining person has
        for (std::bitset<200> person : state.undecided) {
			std::bitset<200> valid = person & ~state.covered;
            for (int b = 0; b < 200; ++b) {
                if (valid.test(b)) {
                    skill_count[b]++;
                }
            }
        }

		// get the unique ones
        std::bitset<200> unique_bits = 0;
        for (int b = 0; b < 64; ++b) {
            if (skill_count[b] == 1) {
                unique_bits.set(b);
            }
        }

        // include the people who are forced because of unique bits
        for (size_t i = 0; i < state.undecided.size();) {
            std::bitset<200> p = state.undecided[i];
		
            if ((p & unique_bits).any()) {
                state.included.push_back(p);
                state.covered |= p;
                state.uncovered &= (~p & state.skills_mask);
                state.team_size++;

                std::swap(state.undecided[i], state.undecided.back());
                state.undecided.pop_back();
                changed = true;
            } else {
                ++i;
            }
        }
    }

    return state;
}

Solution solve(Solution state, Solution& best_state) {
	// // bounding. Improvement: if two less than current best, can end if one person can't get us solution
	// if (state.team_size == best_state.team_size - 1) {
	// 	return best_state;
	// }

	// bounding. Improvement: if two less than current best, can end if one person can't get us solution
	if (state.team_size == best_state.team_size - 2) {
		if ((state.skills_mask & ~(state.covered | state.undecided[pickPerson(state)])).any()) {
		return best_state;
		}
	}

	if (state.covered  == state.skills_mask) {
		// needed? We already checked if geq
		if (state.team_size < best_state.team_size) {
			best_state = state;
		}
		return state;
	}
	// potentially can ensure we never reach this by starting in the right place? have to check.
	if (state.undecided.empty()) {
		return best_state;
	}
	// do we always want to choose based on skills diff? I guess yeah? Idk.
	int next_index = pickPerson(state);
	// nobody adds a skill
	if (next_index == -1) {
		return best_state;
	}
	// swaps selected person to back and pops it
	std::swap(state.undecided[next_index], state.undecided.back());
	std::bitset<200> next_person = state.undecided.back();
	state.undecided.pop_back();
	
	// include and exclude state and then recursion time
	Solution include_state = state;
	include_state.included.push_back(next_person);
	include_state.covered |= next_person; // add their skills to covered
	include_state.uncovered &= ~next_person & state.skills_mask; // update uncovered (needed? When do we check/use this? Add optim for this?)
	include_state.team_size += 1;
	include_state = simplify(include_state);
	Solution include_solution = solve(include_state, best_state);

	Solution exclude_state = state;
	exclude_state.excluded.push_back(next_person);
	exclude_state = simplify(exclude_state);
	Solution exclude_solution = solve(exclude_state, best_state);

	// use max? seems like it could be covered by the checks maybe? idk.
	Solution better;
	if (include_solution.team_size <= exclude_solution.team_size) {
		better = include_solution;
	} else {
		better = exclude_solution;
	}	

	if (better.team_size < best_state.team_size) {
		best_state = better;
	}
	return better;
}

// seems fine i'll deal with this later.
Solution approximate(Solution state) {
	while (state.covered != state.skills_mask) {
		int best_index = pickPerson(state);
		if (best_index == -1) {
			break;
		}
		std::swap(state.undecided[best_index], state.undecided.back());
		std::bitset<200> val = state.undecided.back();
		state.undecided.pop_back();
		state.included.push_back(val);
		state.covered |= val;
		state.uncovered &= ~val;
		state.team_size += 1;
	}
	return state;
}

int main() {

	int n, k;
	std::cin >> n >> k;

	std::unordered_map<std::string, std::bitset<200>> skillset;
	std::string s;
	std::bitset<200> skills_mask = 0;

	for (int i=0; i<k; ++i) {
		std::cin >> s;
		std::bitset<200> mask;
		mask.set(i);

		skillset[s] = mask;
		skills_mask.set(i);
	}

	int num;
	std::vector<std::bitset<200>> people(n, 0);

	for (int i=0; i<n; ++i) {
		std::cin >> num;
		for (int j=0; j<num; ++j) {
			std::cin >> s;
			people[i] |= skillset[s];
		}
	}
	
	Solution initial_state;
	initial_state.undecided = people;
	initial_state.uncovered = skills_mask;
	initial_state.covered = 0;
	initial_state.team_size = 0;
	initial_state.tree_depth = 0;
	initial_state.skills_mask = skills_mask;

//	Solution best_guess = approximate(initial_state);
//	for (size_t i=0; i<best_guess.included.size(); ++i) {
//		std::bitset<64> bits(best_guess.included[i]);
//		std::cout << bits << std::endl;
//	}
	initial_state = simplify(initial_state);
	Solution best_guess = approximate(initial_state);
//	for (size_t i=0; i<best_guess.included.size(); ++i) {
//		std::bitset<64> bits(best_guess.included[i]);
//		std::cout << bits << std::endl;
//	}

	Solution solution_state = solve(initial_state, best_guess);

	std::cout << solution_state.team_size << std::endl;
//	for (size_t i=0; i<solution_state.included.size(); ++i) {
//		std::bitset<64> bits(solution_state.included[i]);
//		std::cout << bits << std::endl;
//	}



	// Read inputs -- COMPLETE
	// Run Simplifications - init state = Simplify(state_state) -- COMPLETE
	// Run approximation - start with most skills, then add person with most -- COMPLETE
	// new skills - this can be the pickPerson function -- COMPLETE
	// Solve(state_state) -> We can run Simlify in the branching -- COMPLETE

	// Simplifications:
	// S1: if someone is a subset of someone else, automatically exclude the
	// SUBSET (we don't need people who are tied either)
	// S2: if someone has a unique skill, auto include 
	// LOOP OVER THE SIMPLIFICATIONS USING WHILE LOOP
	// S3: if everyone has a skill then remove that skill
	// S4: there are more simplifications that will really help
	
	return 0;
}
