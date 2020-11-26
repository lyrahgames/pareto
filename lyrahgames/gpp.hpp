#pragma once
#include <cstdio>
#include <iostream>
#include <stdexcept>

namespace lyrahgames {

class gpp {
 public:
  static constexpr char gnuplot_cmd[] = "gnuplot -persist";

  gpp() {
    if (!(pipe_ = popen(gnuplot_cmd, "w")))
      throw std::runtime_error("Could not open Gnuplot pipeline!");
  }
  gpp(gpp&& plot) : pipe_{plot.pipe_} { plot.pipe_ = nullptr; }
  gpp& operator=(gpp&& plot) {
    std::swap(pipe_, plot.pipe_);
    return *this;
  }
  gpp(const gpp&) = delete;
  gpp& operator=(const gpp&) = delete;
  virtual ~gpp() {
    if (pipe_) pclose(pipe_);
  }

  friend gpp& operator<<(gpp& plot, const char* cmd) {
    fputs(cmd, plot.pipe_);
    fflush(plot.pipe_);
    return plot;
  }

 private:
  FILE* pipe_{nullptr};
};

}  // namespace lyrahgames