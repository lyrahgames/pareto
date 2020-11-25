#pragma once
#include <cstdio>
#include <iostream>
#include <stdexcept>

namespace gpp {

class pipe {
 public:
  static constexpr char gnuplot_cmd[] = "gnuplot -persist";

  pipe() {
    if (!(pipe_ = popen(gnuplot_cmd, "w")))
      throw std::runtime_error("Could not open Gnuplot pipeline!");
  }
  pipe(pipe&& gpp) : pipe_{gpp.pipe_} { gpp.pipe_ = nullptr; }
  pipe& operator=(pipe&& gpp) {
    std::swap(pipe_, gpp.pipe_);
    return *this;
  }
  pipe(const pipe&) = delete;
  pipe& operator=(const pipe&) = delete;
  virtual ~pipe() {
    if (pipe_) pclose(pipe_);
  }

  friend pipe& operator<<(pipe& gpp, const char* cmd) {
    fputs(cmd, gpp.pipe_);
    fflush(gpp.pipe_);
    return gpp;
  }

 private:
  FILE* pipe_{nullptr};
};

}  // namespace gpp