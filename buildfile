cxx.std = latest
using cxx

hxx{*}: extension = hpp
cxx{*}: extension = cpp

cxx.poptions =+ "-I$src_base"

./: exe{main}: cxx{main} hxx{gpp}
./: exe{kursawe}: cxx{kursawe} hxx{**}