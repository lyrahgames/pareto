// Store the data into a file for plotting.
fstream file{"zdt3.dat", ios::out};
for (size_t i = 0; i < pareto_front.sample_count(); ++i) {
  for (auto y : pareto_front.objectives(i))
    file << setw(20) << y << ' ';
  file << '\n';
}
file << flush;

// Plot the data.
gnuplot::pipe plot{};
plot << "plot 'zdt3.dat' u 1:2 w p lt rgb '#ff3333' pt 13\n";
