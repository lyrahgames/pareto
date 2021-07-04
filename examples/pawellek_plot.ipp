// Store the data into a file for plotting.
fstream file{"pawellek.dat", ios::out};
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
        "set title\"Pawellek Parameters\"\n"
        "splot 'pawellek.dat' u 1:2:3 w p lt rgb '#ff3333' pt 13\n"
     << "set term qt 2\n"
        "set title \"Pawellek Objectives\"\n"
        "splot 'pawellek.dat' u 4:5:6 w p lt rgb '#ff3333' pt 13\n";
