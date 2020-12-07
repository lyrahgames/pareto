./: tests/ manifest doc{*.md} legal{COPYING}

./: lib{lyrahgames-pareto}: lyrahgames/hxx{**}
{
  cxx.export.poptions = "-I$out_root" "-I$src_root"
}
hxx{**}: install.subdirs = true

tests/: install = false

./: exe{main}: cxx{main} hxx{gpp}
./: exe{kursawe_nsga2}: cxx{kursawe_nsga2} hxx{**}
./: exe{nsga2}: cxx{nsga2} hxx{**}
./: exe{kursawe_naive}: cxx{kursawe_naive} hxx{**}
./: exe{viennet_naive}: cxx{viennet_naive} hxx{**}
./: exe{surface_naive}: cxx{surface_naive} hxx{**}