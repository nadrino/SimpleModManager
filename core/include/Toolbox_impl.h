//
// Created by Adrien BLANCHET on 21/06/2020.
//

#ifndef SIMPLEMODMANAGER_TOOLBOX_IMPL_H
#define SIMPLEMODMANAGER_TOOLBOX_IMPL_H

#include <numeric>
#include <vector>
#include <algorithm>
#include <functional>
#include <array>
#include <iostream>

template <typename T, typename Compare>
std::vector<size_t> Toolbox::sort_permutation(std::vector<T>& vec, Compare& compare ) {
  std::vector<size_t> p(vec.size());
  std::iota(p.begin(), p.end(), 0);
  std::sort(p.begin(), p.end(),
            [&](size_t i, size_t j){ return compare(vec[i], vec[j]); });
  return p;
}

template <typename T>
std::vector<T> Toolbox::apply_permutation(const std::vector<T>& vec, const std::vector<size_t>& p ) {
  std::vector<T> sorted_vec(vec.size());
  std::transform(p.begin(), p.end(), sorted_vec.begin(),
                 [&](std::size_t i){ return vec[i]; });
  return sorted_vec;
}

#endif //SIMPLEMODMANAGER_TOOLBOX_IMPL_H
