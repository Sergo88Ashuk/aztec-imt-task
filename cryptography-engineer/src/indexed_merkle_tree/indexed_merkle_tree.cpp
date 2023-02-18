#include "indexed_merkle_tree.hpp"
#include <stdlib/merkle_tree/hash.hpp>

namespace plonk {
namespace stdlib {
namespace indexed_merkle_tree {

void IndexedMerkleTree::rehash(std::vector<barretenberg::fr>& hashes)
{
    // create vector of height str idxs, for depth = 3: { 0 , 8 , 12 }
    //                                                    + (1 << 3), + (1 << 2)
    //                                       depth = 4: { 0 , 16, 24 , 28 }
    std::vector<size_t> level_str_idxs{};
    level_str_idxs.push_back(0);
    for (size_t i = 1; i < depth_; i++) {
        size_t prev = level_str_idxs[i - 1];
        level_str_idxs.push_back(prev + (1 << (depth_ - (i - 1))));
    }

    // TODO: move into separate func (smth like 'hash_range' [vector of indexes])
    for (size_t i = total_size_; i < hashes.size(); i++) {
        size_t level_str_idx{};
        size_t prev_level_str_idx{};
        for (size_t l = 0; l < level_str_idxs.size(); l++) {
            auto lsi = level_str_idxs[l];
            if (i >= lsi) {
                level_str_idx = lsi;
                if (l > 0)
                    prev_level_str_idx = level_str_idxs[l - 1];
            }
        }
        auto in_level_idx = i - level_str_idx;
        auto ch_base_idx = prev_level_str_idx + in_level_idx * 2;
        auto rchld = hashes[ch_base_idx];
        auto lchld = hashes[ch_base_idx + 1];
        hashes[i] = compress_pair(rchld, lchld);
    }
}
void IndexedMerkleTree::init_hashes(std::vector<barretenberg::fr>& hashes)
{
    // calc hash of {0, 0, 0}
    auto zleaf_hash = leaf({ 0, 0, 0 }).hash();
    for (size_t i = 0; i < total_size_; i++) {
        hashes[i] = zleaf_hash;
    }

    // create vector of height str idxs, for depth = 3: { 0 , 8 , 12 }
    //                                                    + (1 << 3), + (1 << 2)
    //                                       depth = 4: { 0 , 16, 24 , 28 }
    std::vector<size_t> level_str_idxs{};
    level_str_idxs.push_back(0);
    for (size_t i = 1; i < depth_; i++) {
        size_t prev = level_str_idxs[i - 1];
        level_str_idxs.push_back(prev + (1 << (depth_ - (i - 1))));
    }

    // TODO: move into separate func (smth like 'hash_range' [vector of indexes])
    for (size_t i = total_size_; i < hashes.size(); i++) {
        size_t level_str_idx{};
        size_t prev_level_str_idx{};
        for (size_t l = 0; l < level_str_idxs.size(); l++) {
            auto lsi = level_str_idxs[l];
            if (i >= lsi) {
                level_str_idx = lsi;
                if (l > 0)
                    prev_level_str_idx = level_str_idxs[l - 1];
            }
        }
        auto in_level_idx = i - level_str_idx;
        auto ch_base_idx = prev_level_str_idx + in_level_idx * 2;
        auto rchld = hashes[ch_base_idx];
        auto lchld = hashes[ch_base_idx + 1];
        hashes[i] = compress_pair(rchld, lchld);
    }
}

void IndexedMerkleTree::calculate_root(const std::vector<barretenberg::fr>& hashes)
{
    size_t hs = hashes.size();
    root_ = compress_native(hashes[hs - 2], hashes[hs - 1]);
}

/**
 * Initialise an indexed merkle tree state with all the leaf values: H({0, 0, 0}).
 * Note that the leaf pre-image vector `leaves_` must be filled with {0, 0, 0} only at index 0.
 */
IndexedMerkleTree::IndexedMerkleTree(size_t depth)
    : depth_(depth)
{
    ASSERT(depth_ >= 1 && depth <= 32);
    total_size_ = 1UL << depth_;
    hashes_.resize(total_size_ * 2 - 2);

    // Exercise: Build the initial state of the entire tree.
    leaves_ = { { 0, 0, 0 } };
    // calc hashes_
    init_hashes(hashes_);
    calculate_root(hashes_);
}

/**
 * Fetches a hash-path from a given index in the tree.
 * Note that the size of the fr_hash_path vector should be equal to the depth of the tree.
 */
fr_hash_path IndexedMerkleTree::get_hash_path(size_t idx)
{
    // Exercise: fill the hash path for a given index.
    fr_hash_path path(depth_);
    // calculate_hashes
    rehash(hashes_);
    // calculate_root
    calculate_root(hashes_);
    // create vector of height str idxs, for depth = 3: { 0 , 8 , 12 }
    //                                                    + (1 << 3), + (1 << 2)
    //                                       depth = 4: { 0 , 16, 24 , 28 }
    std::vector<size_t> level_str_idxs{};
    level_str_idxs.push_back(0);
    for (size_t i = 1; i < depth_; i++) {
        size_t prev = level_str_idxs[i - 1];
        level_str_idxs.push_back(prev + (1 << (depth_ - (i - 1))));
    }

    auto h0 = idx % 2 ? hashes_[idx - 1] : hashes_[idx];
    auto h1 = idx % 2 ? hashes_[idx] : hashes_[idx + 1];
    path[0] = std::make_pair(h0, h1);

    for (size_t l = 1; l < depth_; l++) {
        size_t parent_idx = idx / 2;
        idx = level_str_idxs[1] + parent_idx;
        auto h0 = idx % 2 ? hashes_[idx - 1] : hashes_[idx];
        auto h1 = idx % 2 ? hashes_[idx] : hashes_[idx + 1];
        path[l] = std::make_pair(h0, h1);
    }

    return path;
}

/**
 * Update the node values (i.e. `hashes_`) given the leaf hash `value` and its index `index`.
 * Note that indexing in the tree starts from 0.
 * This function should return the updated root of the tree.
 */
fr IndexedMerkleTree::update_element_internal(size_t, fr const&)
{
    // Exercise: insert the leaf hash `value` at `index`.
    return 0;
}

/**
 * Insert a new `value` in a new leaf in the `leaves_` vector in the form: {value, nextIdx, nextVal}
 * You will need to compute `nextIdx, nextVal` according to the way indexed merkle trees work.
 * Further, you will need to update one old leaf pre-image on inserting a new leaf.
 * Lastly, insert the new leaf hash in the tree as well as update the existing leaf hash of the old leaf.
 */
fr IndexedMerkleTree::update_element(fr const& val)
{
    // Exercise: add a new leaf with value `value` to the tree.
    auto cur_idx = leaves_.size();
    if (cur_idx < total_size_) {
        index_t next_idx;
        fr next_value;
        size_t idx = 0;

        while (true) {
            if ((uint64_t)leaves_[idx].nextValue >= (uint64_t)val || leaves_[idx].nextValue == 0) {
                if ((uint64_t)leaves_[idx].nextValue == (uint64_t)val)
                    return 0;
                next_idx = leaves_[idx].nextIndex;
                next_value = leaves_[idx].nextValue;
                leaves_[idx].nextValue = val;
                leaves_[idx].nextIndex = cur_idx;
                break;
            }

            idx = static_cast<size_t>(leaves_[idx].nextIndex);
        }

        leaves_.push_back(leaf({ val, next_idx, next_value }));
        hashes_[cur_idx] = leaves_[cur_idx].hash();
        hashes_[idx] = leaves_[idx].hash();
    }

    return 0;
}

} // namespace indexed_merkle_tree
} // namespace stdlib
} // namespace plonk
