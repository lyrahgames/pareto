cxx.std = latest
using cxx

hxx{*}: extension = hpp
cxx{*}: extension = cpp

cxx.poptions =+ "-I$src_base"

./: exe{main}: cxx{main} hxx{gpp}
./: exe{kursawe_nsga2}: cxx{kursawe_nsga2} hxx{**}
./: exe{kursawe_naive}: cxx{kursawe_naive} hxx{**}