// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <fs/PathLexer.h>

#include <List.h>
#include <dev/Console.h>

namespace valkyrie::kernel {

String PathLexer::normalize(String pathname) {
  bool is_absolute = pathname.front() == '/';
  List<String> components;
  String ret;

  deduplicate_delimiters(pathname);

  for (const auto& component : pathname.split('/')) {
    if (component == ".") {
      continue;
    } else if (component == "..") {
      components.pop_back();
    } else {
      components.push_back(move(component));
    }
  }

  ret = String("/").join(components);
  
  if (is_absolute) {
    ret = String("/") + ret;
  }

  return ret;
}

void PathLexer::deduplicate_delimiters(String& path) {
  const size_t len = path.size();
  size_t slow = 1;
  size_t fast = 1;

  while (fast < len) [[likely]] {
    if (path[fast] == '/' &&
        path[fast - 1] == path[fast]) {
      fast++;
    } else {
      path[slow++] = path[fast++];
    }
  }
  path[slow] = 0;
}

}  // namespace valkyrie::kernel
