#ifndef _KERNEL_ARCHIVE_HPP_
#define _KERNEL_ARCHIVE_HPP_

#include <fstream>

#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>


namespace frovedis {
namespace kernel {

template <typename Model, typename Archive>
static void save_model(const Model& model, const std::string& path, std::ios_base::openmode mode) {
  std::ofstream f;
  f.open(path, mode);
  Archive archive(f);
  archive << model;
}

template <typename Model, typename Archive>
static Model load_model(const std::string& path, std::ios_base::openmode mode) {
  Model model;
  std::ifstream f;
  f.open(path, mode);
  Archive archive(f);
  archive >> model;
  return model;
}

template <typename Model>
void save_model(const Model& model, const std::string& path, const bool binary) {
  if (binary) {
    save_model<Model, cereal::BinaryOutputArchive>(model, path, std::ios::out | std::ios::binary);
  } else {
    save_model<Model, cereal::JSONOutputArchive>(model, path, std::ios::out);
  }
}

template <typename Model>
Model load_model(const std::string& path, const bool binary) {
  if (binary) {
    return load_model<Model, cereal::BinaryInputArchive>(path, std::ios::in | std::ios::binary);
  } else {
    return load_model<Model, cereal::JSONInputArchive>(path, std::ios::in);
  }
}

}  // namespace kernel
}  // namespace frovedis

#endif  // _KERNEL_ARCHIVE_HPP_
