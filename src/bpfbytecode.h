#pragma once

#include "bpffeature.h"
#include "bpfmap.h"
#include "bpfprogram.h"
#include "config.h"
#include "required_resources.h"
#include "types.h"

#include <bpf/libbpf.h>
#include <cereal/access.hpp>
#include <map>
#include <span>
#include <string>
#include <vector>

namespace bpftrace {

// Representation of the entire BPF bytecode generated by bpftrace.
// Created from ELF emitted by CodegenLLVM.
// Encapsulates libbpf's 'struct bpf_object' and contains BPF maps and programs.
class BpfBytecode {
public:
  BpfBytecode()
  {
  }
  BpfBytecode(std::span<const std::byte> elf);
  BpfBytecode(std::span<uint8_t> elf);
  BpfBytecode(std::span<char> elf);

  BpfBytecode(const BpfBytecode &) = delete;
  BpfBytecode &operator=(const BpfBytecode &) = delete;
  BpfBytecode(BpfBytecode &&) = default;
  BpfBytecode &operator=(BpfBytecode &&) = default;

  void update_global_vars(BPFtrace &bpftrace);
  void load_progs(const RequiredResources &resources,
                  const BTF &btf,
                  BPFfeature &feature,
                  const Config &config);

  const BpfProgram &getProgramForProbe(const Probe &probe) const;
  BpfProgram &getProgramForProbe(const Probe &probe);

  bool hasMap(MapType internal_type) const;
  bool hasMap(const StackType &stack_type) const;
  const BpfMap &getMap(const std::string &name) const;
  const BpfMap &getMap(MapType internal_type) const;
  const BpfMap &getMap(int map_id) const;
  void set_map_ids(RequiredResources &resources);

  const std::map<std::string, BpfMap> &maps() const;
  int countStackMaps() const;

private:
  void prepare_progs(const std::vector<Probe> &probes,
                     const BTF &btf,
                     BPFfeature &feature,
                     const Config &config);
  bool all_progs_loaded();

  // We need a custom deleter for bpf_object which will call bpf_object__close.
  // Note that it is not possible to run bpf_object__close in ~BpfBytecode
  // as the desctuctor may be called upon move assignment.
  struct bpf_object_deleter {
    void operator()(struct bpf_object *object)
    {
      bpf_object__close(object);
    }
  };
  std::unique_ptr<struct bpf_object, bpf_object_deleter> bpf_object_;

  std::map<std::string, BpfMap> maps_;
  std::map<int, BpfMap *> maps_by_id_;
  std::map<std::string, BpfProgram> programs_;
  std::unordered_map<std::string, struct bpf_map *>
      section_names_to_global_vars_map_;
};

} // namespace bpftrace
