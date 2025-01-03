#pragma once
#include <stdlib/merkle_tree/hash_path.hpp>
#include "leaf.hpp"

namespace plonk {
namespace stdlib {
namespace indexed_merkle_tree {

using namespace barretenberg;
using namespace plonk::stdlib::merkle_tree;

/**
 * An IndexedMerkleTree is structured just like a usual merkle tree:
 *
 *                                       hashes_
 *    +------------------------------------------------------------------------------+
 *    |  0 -> h_{0,0}  h_{0,1}  h_{0,2}  h_{0,3}  h_{0,4}  h_{0,5}  h_{0,6}  h_{0,7} |
 *  i |                                                                              |
 *  n |  8 -> h_{1,0}  h_{1,1}  h_{1,2}  h_{1,3}                                     |
 *  d |                                                                              |
 *  e | 12 -> h_{2,0}  h_{2,1}                                                       |
 *  x |                                                                              |
 *    | 14 -> h_{3,0}                                                                |
 *    +------------------------------------------------------------------------------+
 *
 * Here, depth_ = 3 and {h_{0,j}}_{i=0..7} are leaf values.
 * Also, root_ = h_{3,0} and total_size_ = (2 * 8 - 2) = 14.
 * Lastly, h_{i,j} = hash( h_{i-1,2j}, h_{i-1,2j+1} ) where i > 1.
 *
 * 1. Initial state:
 *
 *                                        #
 *
 *                        #                               #
 *
 *                #               #               #               #
 *
 *            #       #       #       #        #       #       #       #
 *
 *  index     0       1       2       3        4       5       6       7
 *
 *  val       0       0       0       0        0       0       0       0
 *  nextIdx   0       0       0       0        0       0       0       0
 *  nextVal   0       0       0       0        0       0       0       0
 *
 * 2. Add new leaf with value 30
 *
 *  val       0       30      0       0        0       0       0       0
 *  nextIdx   1       0       0       0        0       0       0       0
 *  nextVal   30      0       0       0        0       0       0       0
 *
 * 3. Add new leaf with value 10
 *
 *  val       0       30      10      0        0       0       0       0
 *  nextIdx   2       0       1       0        0       0       0       0
 *  nextVal   10      0       30      0        0       0       0       0
 *
 * 4. Add new leaf with value 20
 *
 *  val       0       30      10      20       0       0       0       0
 *  nextIdx   2       0       3       1        0       0       0       0
 *  nextVal   10      0       20      30       0       0       0       0
 *
 * 5. Add new leaf with value 50
 *
 *  val       0       30      10      20       50      0       0       0
 *  nextIdx   2       4       3       1        0       0       0       0
 *  nextVal   10      50      20      30       0       0       0       0
 */
class IndexedMerkleTree {
  public:
    IndexedMerkleTree(size_t depth);

    fr_hash_path get_hash_path(size_t index);

    fr update_element_internal(size_t index, fr const& value);

    fr update_element(fr const& value);

    fr root() const { return root_; }

    const std::vector<barretenberg::fr>& get_hashes() { return hashes_; }
    const std::vector<leaf>& get_leaves() { return leaves_; }

  private:
    // helper functions
    void init_hashes();
    void calculate_root();
    void build_hashes_from_leaves();

  private:
    // The depth or height of the tree
    size_t depth_;

    // The total number of leaves in the tree
    size_t total_size_;

    // The root of the merkle tree
    barretenberg::fr root_;

    // Vector of pre-images of leaf values of the form {val, nextIdx, nextIdx}
    // Size = total_size_
    std::vector<leaf> leaves_;

    // Vector that stores all the leaf hashes as well as intermediate node values
    // Size: total_size_ + (total_size_ / 2) + (total_size_ / 4) + ... + 2 = 2 * total_size_ - 2
    std::vector<barretenberg::fr> hashes_;

    // Vector of starting indexes for tree levels
    std::vector<size_t> level_str_idxs_;

    // Hash statuses for performance optimizations
    enum class node_status : uint8_t {
        DIRTY = 0, // hash of this node outdated because of the change in leaf
        CLEAN = 1, // hash of this node is actual no need to rehash
    };
    std::vector<node_status> hstats_;
};

} // namespace indexed_merkle_tree
} // namespace stdlib
} // namespace plonk
