#include <iostream>
#include <vector>
#include <algorithm>
#include <bitset>
#include <utility>

int pickPerson(const std::vector<std::bitset<1200>>& people, const std::vector<int>& remaining, const std::bitset<1200>& covered, const std::bitset<1200>& mask) {

    int best_index = -1;
    int max_diff = 0;

    auto uncovered_mask = ~covered & mask;

    for (int idx : remaining) {
        int diff = (people[idx] & uncovered_mask).count();

        if (diff > max_diff) {
            max_diff = diff;
            best_index = idx;
        }
    }

    return best_index;
}

std::pair<int, std::vector<int>> approximate(const std::vector<std::bitset<1200>>& people, const std::bitset<1200>& mask) {

    std::bitset<1200> covered;
    std::vector<int> agents;
    int team_size = 0;

    // track which indices are still available
    std::vector<int> remaining(people.size());
    for (int i = 0; i < (int)people.size(); ++i) {
        remaining[i] = i;
    }

    while (covered != mask) {
        int best_index = pickPerson(people, remaining, covered, mask);
        if (best_index == -1) break;

        agents.push_back(best_index);
        covered |= people[best_index];
        team_size++;

        // remove chosen person from remaining
        remaining.erase(
            std::remove(remaining.begin(), remaining.end(), best_index),
            remaining.end()
        );
    }

    return {team_size, agents};
}

int main() {
    int n, k;
    std::cin >> n >> k;

    int num_skills, skill;

    std::vector<std::bitset<1200>> people(n);

    for (int i = 0; i < n; ++i) {
        std::cin >> num_skills;

        for (int j = 0; j < num_skills; ++j) {
            std::cin >> skill;
            people[i].set(skill);
        }
    }

    std::bitset<1200> mask;
    mask.set();

    auto [team_size, agents] = approximate(people, mask);

    std::cout << team_size << std::endl;
    for (int idx : agents) {
        std::cout << idx << " ";
    }
    std::cout << std::endl;

    return 0;
}
