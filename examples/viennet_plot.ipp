// Store the data into a file for plotting.
fstream file{"viennet.dat", ios::out};
for (size_t i = 0; i < pareto_front.sample_count(); ++i) {
  for (auto x : pareto_front.parameters(i))
    file << setw(20) << x << ' ';
  for (auto y : pareto_front.objectives(i))
    file << setw(20) << y << ' ';
  file << '\n';
}
file << flush;

// Plot the data.
gnuplot::pipe plot{};
plot << "set term qt 1\n"
        "set title \"Viennet Parameters\"\n"
        "plot 'viennet.dat' u 1:2 w p lt rgb '#ff3333' pt 13\n"
     << "set term qt 2\n"
        "set title \"Viennet Objectives\"\n"
        "splot 'viennet.dat' u 3:4:5 w p lt rgb '#ff3333' pt 13\n";
