#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

// from
// https://stackoverflow.com/questions/6089231/getting-std-ifstream-to-handle-lf-cr-and-crlf
std::istream& safeGetline(std::istream& is, std::string& t)
{
  t.clear();

  // The characters in the stream are read one-by-one using a std::streambuf.
  // That is faster than reading them one-by-one using the std::istream.
  // Code that uses streambuf this way must be guarded by a sentry object.
  // The sentry object performs various tasks,
  // such as thread synchronization and updating the stream state.

  std::istream::sentry se(is, true);
  std::streambuf* sb = is.rdbuf();

  for (;;) {
    int c = sb->sbumpc();
    switch (c) {
      case '\r':
        if (sb->sgetc() == '\n')
          sb->sbumpc();
      case '\n': return is;
      case std::streambuf::traits_type::eof():
        // Also handle the case when the last line has no line ending
        if (t.empty())
          is.setstate(std::ios::eofbit);
        return is;
      default: t += (char)c;
    }
  }
}

// ======================================================================
// Parsed
// Parses a space-separated list of values, such as a tsv file.
// Assuming there is a single independent variable, linearly interpolates other
// values.

class Parsed
{
private:
  const int nrows, ncols;
  std::vector<double> data;
  const int indep_col;
  double indep_var_step;

  // ----------------------------------------------------------------------
  // get_row
  // Gets index of row containing greatest lower bound of given indep_var_val

  int get_row(double indep_var_val)
  {
    // initial guess; should be precise assuming indep_var is linearly spaced
    int row = std::min((int)(indep_var_val / indep_var_step), nrows - 1);
    while (row >= 0 && indep_var_val < (*this)[row][indep_col])
      row--;
    return row;
  }

public:
  // ----------------------------------------------------------------------
  // ctor

  Parsed(int nrows, int ncols, int indep_col)
    : nrows(nrows), ncols(ncols), data(nrows * ncols), indep_col(indep_col)
  {}

  // ----------------------------------------------------------------------
  // loadData
  // Parses all the data

  void loadData(const std::string file_path, int lines_to_skip)
  {
    std::ifstream file(file_path);

    if (!file.is_open()) {
      std::cout << "Failed to open input file: " << file_path << std::endl;
      exit(EXIT_FAILURE);
    }

    for (int i = 0; i < lines_to_skip; i++)
      file.ignore(512, '\n');

    // iterate over each line
    int row = 0;

    for (std::string line; !safeGetline(file, line).eof();) {
      if (row >= nrows) {
        std ::cout << "Error: too many rows. Expected " << nrows
                   << ", got at least " << row + 1 << std::endl;
        exit(EXIT_FAILURE);
      }

      // iterate over each entry within a line
      std::istringstream iss(line);
      int col = 0;

      for (std::string result; iss >> result;) {
        if (col >= ncols) {
          std ::cout << "Error: too many columns. Expected " << ncols
                     << ", got at least " << col + 1 << " in line \"" << line
                     << "\"" << std::endl;
          exit(EXIT_FAILURE);
        }

        // write entry to data
        (*this)[row][col] = std::stod(result);
        col++;
      }
      if (col != ncols) {
        std ::cout << "Error: not enough columns. Expected " << ncols
                   << ", got " << col << std::endl;
        exit(EXIT_FAILURE);
      }
      row++;
    }
    if (row != nrows) {
      std ::cout << "Error: not enough rows. Expected " << nrows << ", got "
                 << row << std::endl;
      exit(EXIT_FAILURE);
    }
    file.close();

    indep_var_step =
      ((*this)[nrows - 1][indep_col] - (*this)[0][indep_col]) / nrows;
  }

  // ----------------------------------------------------------------------
  // get_interpolated
  // Calculates and returns a linearly interpolated value (specified by col) at
  // given value of independent variable

  double get_interpolated(int col, double indep_var_val)
  {
    assert(indep_var_val >= (*this)[0][indep_col]);
    assert(indep_var_val <= (*this)[nrows - 1][indep_col]);

    int row = get_row(indep_var_val);

    if ((*this)[row][indep_col] == indep_var_val)
      return (*this)[row][col];

    // weights for linear interpolation
    double w1 = (*this)[row + 1][indep_col] - indep_var_val;
    double w2 = indep_var_val - (*this)[row][indep_col];

    return (w1 * (*this)[row][col] + w2 * (*this)[row + 1][col]) / (w1 + w2);
  }

  double* operator[](const int row) { return data.data() + row * ncols; }
};