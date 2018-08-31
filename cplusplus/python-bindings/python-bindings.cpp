#include "python-bindings.h"

#include <cassert>
#include <memory>
#include <sstream>

#include <boost/filesystem.hpp>
#include <boost/make_unique.hpp>

#include "../dataset/compressor.h"
#include "../dataset/dataset.h"
#include "../dataset-generator/contaminator.h"
#include "../dataset-generator/random-batch-generator.h"
#include "../utils/utils.h"

extern "C" {

boost::filesystem::path               DATASET_FOLDER;
std::unique_ptr<Compressor>           COMPRESSOR;
std::unique_ptr<Contaminator>         CONTAMINATOR;
std::unique_ptr<DataSet>              DATASET;
std::unique_ptr<RandomBatchGenerator> BATCH_GENERATOR;

void set_dataset_folder(const char* input_folder) {
    assert(DATASET_FOLDER.empty());
    DATASET_FOLDER = input_folder;
    assert(boost::filesystem::exists(DATASET_FOLDER));
}

void create_dataset() {
    assert(!DATASET_FOLDER.empty());
    assert(!DATASET);
    DATASET = boost::make_unique<DataSet>(DATASET_FOLDER);
}

void create_contaminator(const char* ngrams_file, double mistake_probability) {
    assert(!CONTAMINATOR);
    CONTAMINATOR = boost::make_unique<Contaminator>(ngrams_file, mistake_probability);
}

void create_compressor(size_t message_size) {
    assert(DATASET);
    assert(!COMPRESSOR);
    COMPRESSOR = boost::make_unique<Compressor>(*DATASET, message_size);
}

void decompress(const char* token, char* output) {
    assert(COMPRESSOR);
    const std::vector<std::string>& decompressed = COMPRESSOR->decompress(token);
    std::stringstream stream;
    for (const std::string& decompresed_token : decompressed) {
        stream << decompresed_token << "|";
    }
    const std::string concatenated = stream.str();
    if (!concatenated.empty()) {
        std::memcpy(output, concatenated.c_str(), concatenated.length() - 1);
    }
}

void find_by_prefix(const char* prefix, size_t max_size, char* output) {
    assert(COMPRESSOR);
    std::vector<std::string> decompressed = COMPRESSOR->find_by_prefix(prefix, max_size);
    std::stringstream stream;
    for (const std::string& decompresed_token : decompressed) {
        stream << decompresed_token << "|";
    }
    const std::string concatenated = stream.str();
    if (!concatenated.empty()) {
        std::memcpy(output, concatenated.c_str(), concatenated.length() - 1);
    }
}

void create_random_batch_generator() {
    assert(DATASET);
    assert(!BATCH_GENERATOR);
    BATCH_GENERATOR = boost::make_unique<RandomBatchGenerator>(*DATASET, *CONTAMINATOR, *COMPRESSOR);
}

void generate_random_batch(int32_t* clean_batch, int32_t* contaminated_batch, size_t batch_size) {
    assert(BATCH_GENERATOR);
    BATCH_GENERATOR->generate_random_batch(clean_batch, contaminated_batch, batch_size);
}

size_t levenstein(const char* first, const char* second, size_t message_size) {
    return levenstein_distance(first, second, message_size);
}

} // extern "C"
