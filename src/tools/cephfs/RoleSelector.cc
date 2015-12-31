
#include "RoleSelector.h"

int MDSRoleSelector::parse_rank(
    const FSMap &fsmap,
    std::string const &str)
{
  if (str == "all" || str == "*") {
    for (auto rank : fsmap.get_filesystem(ns)->in) {
      roles.push_back(mds_role_t(ns, rank));
    }

    return 0;
  } else {
    std::string rank_err;
    mds_rank_t rank = strict_strtol(str.c_str(), 10, &rank_err);
    if (!rank_err.empty()) {
      return -EINVAL;
    }
    if (fsmap.get_filesystem(ns)->is_dne(rank)) {
      return -ENOENT;
    }
    roles.push_back(mds_role_t(ns, rank));
    return 0;
  }
}

int MDSRoleSelector::parse(const FSMap &fsmap, std::string const &str)
{
  auto colon_pos = str.find(":");
  if (colon_pos == std::string::npos) {
    // An unqualified rank.  Only valid if there is only one
    // namespace.
    if (fsmap.get_filesystems().size() == 1) {
      ns = fsmap.get_filesystems().begin()->first;
      return parse_rank(fsmap, str);
    } else {
      return -EINVAL;
    }
  } else if (colon_pos == 0 || colon_pos == str.size() - 1) {
    return -EINVAL;
  } else {
    const std::string ns_str = str.substr(0, colon_pos);
    const std::string rank_str = str.substr(colon_pos + 1);
    std::shared_ptr<Filesystem> fs_ptr;
    int r = fsmap.parse_filesystem(ns_str, &fs_ptr);
    if (r != 0) {
      return r;
    }
    ns = fs_ptr->ns;
    return parse_rank(fsmap, rank_str);
  }
}
