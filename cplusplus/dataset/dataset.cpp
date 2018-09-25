#include "dataset.h"

#include <fstream>
#include <iostream>

#include <boost/algorithm/string.hpp>

#include "../utils/utils.h"

namespace {

int entity_type_to_int(const std::string& type) {
    if (type[0] == 's') {
        return 100;
    } else if (type[0] == 'h') {
        return 101;
    }
    return std::stoi(type);
}

} // anonymous namespace

DataSet::DataSet(const std::string& input_folder, size_t split_index): house_numbers_distribution(1000000) {
    // read names file
    std::ifstream names_reader(input_folder + "/names");
    std::string line;
    while (getline(names_reader, line)) {
        std::vector<std::string> splitted = split(line, '|');
        index_to_name[std::stoi(splitted[1])] = { entity_type_to_int(splitted[0]), splitted[2] };
    }

    // read transitions
    const std::string transitions_file = (split_index == static_cast<size_t>(-1)) ? input_folder + "/transitions"
                                                                                  : input_folder + "/split_" + std::to_string(split_index);
    std::cout << "reading dataset from file: " << transitions_file << std::endl;
    size_t counter = 0;
    std::ifstream transitions_reader(transitions_file);
    while (getline(transitions_reader, line)) {
        Node* node = &root;
        std::vector<std::string> splitted = split(line, ' ');
        for (const std::string& index_string : splitted) {
            size_t index = std::stoi(index_string);
            if (index_to_name[index].type == 101) {
                node->house_numbers.push_back(index);
            } else {
                node = node->add(index);
            }
        }
        if (++counter % 100000 == 0) {
            std::cout << "reading transitions: " << counter << std::endl;
        }
    }

    // create transition distributions for each node
    root.finalize();

    // save all tokens in a set for prefix search
    for (const auto& item : index_to_name) {
        tokens.insert(item.second.name);
    }
}

DataSet::Node* DataSet::Node::add(size_t index) {
    ++count;
    auto found = tranisition_name_index_to_transition_index.find(index);
    if (found != tranisition_name_index_to_transition_index.end()) {
        return &transitions[found->second];
    }
    tranisition_name_index_to_transition_index[index] = transitions.size();
    transitions.emplace_back();
    Node& added_node = transitions.back();
    added_node.count = 1;
    added_node.name_index = index;
    return &added_node;
}

void DataSet::Node::finalize() {
    std::vector<size_t> counters(transitions.size());
    for (size_t i = 0; i < transitions.size(); ++i) {
        counters[i] = transitions[i].count;
    }
    transition_distribution = std::discrete_distribution<size_t>(counters.begin(), counters.end());
    for (Node& node : transitions) {
        node.finalize();
    }
}

std::vector<const DataSet::Entity*> DataSet::get_random_item(std::mt19937& generator) const {
    std::vector<const Entity*> entities;
    const Node* node = &root;
    while (!node->transitions.empty()) {
        size_t transition_index = node->transition_distribution(generator);
        node = &node->transitions[transition_index];
        entities.push_back(&index_to_name.find(node->name_index)->second);
        if (!node->house_numbers.empty()) {
            size_t house_number_index = house_numbers_distribution(generator) % node->house_numbers.size();
            entities.push_back(&index_to_name.find(node->house_numbers[house_number_index])->second);
        }
    }
    return entities;
}

const std::unordered_map<size_t, DataSet::Entity>& DataSet::content() const {
    return index_to_name;
}

std::vector<std::string> DataSet::find_by_prefix(const std::string& prefix, size_t max_number) const {
    std::vector<std::string> results;
    for (auto found = tokens.lower_bound(prefix);
         found != tokens.end() && boost::starts_with(*found, prefix) && results.size() <= max_number;
         ++found) {
        results.push_back(*found);
    }
    return results;
}

