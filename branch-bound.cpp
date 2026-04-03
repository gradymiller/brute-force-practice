#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <bit>
#include <cstdint>
#include <bitset>


struct Solution {
	std::vector<uint64_t> undecided;
	std::vector<uint64_t> included;
	std::vector<uint64_t> excluded;
	uint64_t uncovered;
	uint64_t covered;
	int team_size;
	int tree_depth;
	uint64_t skills_mask;
};

int pickPerson(const Solution& state) {
	int best_index = -1;
	int max_diff = 0;
	for (size_t i=0; i<state.undecided.size(); ++i) {
		int diff = std::popcount((state.undecided[i] & ~state.covered) & state.skills_mask);

		if (diff > max_diff) {
			max_diff = diff;
			best_index = i;
		}
	}
	return best_index;
}

Solution simplify(Solution state) {
	bool changes = true;
	while (changes) {
		changes = false;
		std::sort(state.undecided.begin(), state.undecided.end(),
			[](uint64_t a, uint64_t b) {
				return std::popcount(a) > std::popcount(b);
			});

		for (size_t i = 0; i < state.undecided.size(); ++i) {
			for (size_t j = i + 1; j < state.undecided.size(); ) {
				if ((state.undecided[j] & state.undecided[i]) == state.undecided[j]) {
					std::swap(state.undecided[j], state.undecided.back());
					state.undecided.pop_back();
					changes = true;
				} else {
					++j;
				}
			}
		}

		uint64_t once = 0;
		uint64_t twice = 0;

		for (uint64_t v : state.undecided) {
			twice |= once & v;
			once ^= v;
		}

		uint64_t unique_bits = once & ~twice;
		unique_bits &= state.skills_mask;

		for (size_t i = 0; i < state.undecided.size(); ) {
			uint64_t val = state.undecided[i];

			if (val & unique_bits) {
				state.included.push_back(val);
				state.covered |= val;
				state.uncovered &= ~val & state.skills_mask;
				state.team_size += 1;

				std::swap(state.undecided[i], state.undecided.back());
				state.undecided.pop_back();
				changes = true;

			} else {
				++i;
			}
		}
	}
    return state;
}

Solution solve(Solution state, Solution& best_state) {
	if (state.team_size >= best_state.team_size) {
		return best_state;
	}

	if (state.covered  == state.skills_mask) {
		if (state.team_size < best_state.team_size) {
			best_state = state;
		}
		return state;
	}

	if (state.undecided.empty()) {
		return best_state;
	}

	int next_index = pickPerson(state);
	if (next_index == -1) {
		return best_state;
	}
	std::swap(state.undecided[next_index], state.undecided.back());
	uint64_t next_person = state.undecided.back();
	state.undecided.pop_back();
	
	Solution include_state = state;
	include_state.included.push_back(next_person);
	include_state.covered |= next_person;
	include_state.uncovered &= ~next_person & state.skills_mask;
	include_state.team_size += 1;
	include_state = simplify(include_state);
	Solution include_solution = solve(include_state, best_state);

	Solution exclude_state = state;
	exclude_state.excluded.push_back(next_person);
	exclude_state = simplify(exclude_state);
	Solution exclude_solution = solve(exclude_state, best_state);

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

Solution approximate(Solution state) {
	while (state.covered != state.skills_mask) {
		int best_index = pickPerson(state);
		if (best_index == -1) {
			break;
		}
		std::swap(state.undecided[best_index], state.undecided.back());
		uint64_t val = state.undecided.back();
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

	std::unordered_map<std::string, uint64_t> skillset;
	std::string s;
	uint64_t skills_mask = 0;

	for (int i=0; i<k; ++i) {
		std::cin >> s;
		skillset[s] = (1ULL << i);
		skills_mask |= skillset[s];
	}

	int num;
	std::vector<uint64_t> people(n, 0);

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

	initial_state = simplify(initial_state);
	Solution best_guess = approximate(initial_state);
	for (size_t i=0; i<best_guess.included.size(); ++i) {
		std::bitset<64> bits(best_guess.included[i]);
		std::cout << bits << std::endl;
	}

	Solution solution_state = solve(initial_state, best_guess);

	std::cout << solution_state.team_size << std::endl;
	for (size_t i=0; i<solution_state.included.size(); ++i) {
		std::bitset<64> bits(solution_state.included[i]);
		std::cout << bits << std::endl;
	}



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
