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
    std::bitset<200> uncovered;
    std::bitset<200> covered;
    int team_size;
    int tree_depth;
    std::bitset<200> skills_mask;
};

int pickPerson(const Solution& state) {
    int best_index = -1;
    int max_diff = 0;

    // only care about uncovered skills
    auto uncovered_mask = ~state.covered & state.skills_mask;

    for (size_t i = 0; i < state.undecided.size(); ++i) {
        int diff = (state.undecided[i] & uncovered_mask).count();

        // pick person that adds most new skills
        if (diff > max_diff) {
            max_diff = diff;
            best_index = i;
        }
    }
    return best_index;
}

void simplify(Solution& state) {
    bool changed = true;

    // keep simplifying until nothing changes
    while (changed) {
        changed = false;

        // remove subsets
        for (size_t i = 0; i < state.undecided.size(); ++i) {
            for (size_t j = i + 1; j < state.undecided.size();) {
                auto a = state.undecided[i] & ~state.covered;
                auto b = state.undecided[j] & ~state.covered;

                if ((a & b) == a) {
                    // a is useless (subset of b)
                    std::swap(state.undecided[i], state.undecided.back());
                    state.undecided.pop_back();
                    changed = true;
                    break;
                } else if ((a & b) == b) {
                    // b is useless (subset of a)
                    std::swap(state.undecided[j], state.undecided.back());
                    state.undecided.pop_back();
                    changed = true;
                } else {
                    ++j;
                }
            }
        }

        int skill_count[200] = {0};

        // count how many people can cover each skill
        for (auto& person : state.undecided) {
            auto valid = person & ~state.covered;

            for (int i = valid._Find_first(); i < 200; i = valid._Find_next(i)) {
                skill_count[i]++;
            }
        }

        std::bitset<200> unique_bits = 0;

        // find skills that only one person can cover
        for (int b = 0; b < 200; ++b) {
            if (skill_count[b] == 1) {
                unique_bits.set(b);
            }
        }

        // force include people with unique skills
        for (size_t i = 0; i < state.undecided.size();) {
            auto p = state.undecided[i];
            auto valid = p & ~state.covered;

            if ((valid & unique_bits).any()) {
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

        // remove people that add nothing new
        for (size_t i = 0; i < state.undecided.size();) {
            auto p = state.undecided[i];
            auto valid = p & ~state.covered;

            if (!valid.any()) {
                std::swap(state.undecided[i], state.undecided.back());
                state.undecided.pop_back();
                changed = true;
            } else {
                ++i;
            }
        }
    }
}

void solve(Solution& state, Solution& best_state) {

    // found full solution
    if (state.covered == state.skills_mask) {
        if (state.team_size < best_state.team_size) {
            best_state = state;
        }
        return;
    }

    // check if even possible to finish
    std::bitset<200> possible = 0;
    for (auto& p : state.undecided) {
        possible |= p;
    }
    possible &= ~state.covered;

    if ((state.uncovered & ~possible).any()) return;

    // how many bits we still need to cover
    int bits_remaining = state.uncovered.count();

    int max_diff = 0;
    for (auto& person : state.undecided) {
        int diff = (person & state.uncovered).count();
        if (diff > max_diff) {
            max_diff = diff;
        }
    }

    // nobody can help anymore
    if (max_diff == 0) return;

    // ceil division - using the ceil function broke because of integer division
    int bound = (bits_remaining + max_diff - 1) / max_diff;

    // prune if can't beat best state
    if (state.team_size + bound >= best_state.team_size) {
        return;
    }

    int idx = pickPerson(state);
    if (idx == -1) return;

    // save full state - needed because simplify is hard to reverse 
    Solution original = state;

    // pick next person
    std::swap(state.undecided[idx], state.undecided.back());
    auto next_person = state.undecided.back();
    state.undecided.pop_back();

    // include branch
        state = original;

        // remove chosen person
        std::swap(state.undecided[idx], state.undecided.back());
        state.undecided.pop_back();

        state.included.push_back(next_person);
        state.covered |= next_person;
        state.uncovered &= ~next_person & state.skills_mask;
        state.team_size++;

        simplify(state);
        solve(state, best_state);

    // exclude branch
        state = original;

        // remove chosen person (don't allow it anymore)
        std::swap(state.undecided[idx], state.undecided.back());
        state.undecided.pop_back();

        simplify(state);
        solve(state, best_state);

    // restore state
    state = original;
}

void approximate(Solution& state) {

    // greedy: keep picking best person
    while (state.covered != state.skills_mask) {
        int best_index = pickPerson(state);
        if (best_index == -1) break;

        std::swap(state.undecided[best_index], state.undecided.back());
        auto val = state.undecided.back();
        state.undecided.pop_back();

        state.included.push_back(val);
        state.covered |= val;
        state.uncovered &= (~val & state.skills_mask);
        state.team_size++;
    }
}

int main() {
    int n, k;
    std::cin >> n >> k;

    std::unordered_map<std::string, std::bitset<200>> skillset;
    std::string s;
    std::bitset<200> skills_mask = 0;

    // map each skill to a bit
    for (int i = 0; i < k; ++i) {
        std::cin >> s;
        std::bitset<200> mask;
        mask.set(i);

        skillset[s] = mask;
        skills_mask.set(i);
    }

    int num;
    std::vector<std::bitset<200>> people(n, 0);

    // build people skill masks
    for (int i = 0; i < n; ++i) {
        std::cin >> num;
        for (int j = 0; j < num; ++j) {
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

    // initial reductions
    simplify(initial_state);

    // greedy starting solution
    Solution best_guess = initial_state;
    approximate(best_guess);

    Solution best_state = best_guess;

    // branch and bound
    solve(initial_state, best_state);

    std::cout << best_state.team_size << std::endl;

    return 0;
}
