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
gnuplot_pipe xplot{};
xplot << "set title \"Pawellek Parameters\"\n"
         "splot 'pawellek.dat' u 1:2:3 w p lt rgb '#ff3333' pt 13\n";
gnuplot_pipe yplot{};
yplot << "set title \"Pawellek Objectives\"\n"
         "splot 'pawellek.dat' u 4:5:6 w p lt rgb '#ff3333' pt 13\n";

// Wait for the user to stop the program.
// This is needed to keep the gnuplot pipeline open to be able to rotate the
// 3D gnuplot view.
string wait;
getline(cin, wait);