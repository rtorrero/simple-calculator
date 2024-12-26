// vector.hpp
// author: Rubén Torrero Marijnissen <ruben.torrero101@ulpgc.es>
// creation date: november 26 2024
// Description: This is the header file of class vector

#ifndef VECTOR_HPP
#define VECTOR_HPP

#include "matrix.hpp"
#include <cstring>
#include <string>

using namespace std;

namespace mat_lib {

template <typename T> class vector {
public:
  using element_t = double;

  static_assert(
      is_integral<T>::value || is_floating_point<T>::value ||
          is_same<T, complex<float>>::value ||
          is_same<T, complex<double>>::value ||
          is_same<T, complex<long double>>::value,
      "\n"
      "[ERROR] >>>>>>>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< "
      "[ERROR]\n"
      "[ERROR] >>>> BAD TYPE PROVIDED FOR TEMPLATE mat_lib::matrix! <<<< "
      "[ERROR]\n"
      "[ERROR] >>>> ACCEPTED TYPES:                                 <<<< "
      "[ERROR]\n"
      "[ERROR] >>>>   - All numeric primitive types                 <<<< "
      "[ERROR]\n"
      "[ERROR] >>>>   - std::complex<float>                         <<<< "
      "[ERROR]\n"
      "[ERROR] >>>>   - std::complex<double>                        <<<< "
      "[ERROR]\n"
      "[ERROR] >>>>   - std::complex<long double>                   <<<< "
      "[ERROR]\n"
      "[ERROR] >>>>>>>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< "
      "[ERROR]\n");

  vector() : size__{0}, elements__{nullptr} {}

  vector(size_t size) : size__{size}, elements__{new element_t[size]} {}

  vector(initializer_list<element_t> init)
      : size__{init.size()}, elements__{new element_t[init.size()]} {
    for (size_t i = 0; i < init.size(); i++)
      elements__[i] = init.begin()[i];
  }

  vector(const vector &v) // copy constructor
      : elements__{new element_t[v.size()]} {
    copy_elements__(v);
  }

  vector(vector &&v) // move constructor
      : size__{v.size__}, elements__{v.elements__} {
    v.size__ = 0;
    v.elements__ = nullptr;
  }

  explicit vector(const string& file_name) : size__{0}, elements__{nullptr} {
      ifstream ifs(file_name);
      if (!ifs) {
          ostringstream str_stream;
          str_stream << "cannot open file \"" << file_name << "\"! ("
              << __func__ << "() in " << __FILE__ << ":" << __LINE__ << ")";
          throw logic_error(str_stream.str());
      }

      string content((istreambuf_iterator<char>(ifs)), istreambuf_iterator<char>());

      // Extract dimension and content
      regex vec_pattern(R"(mat_lib::vector\[(\d+)\]\{([\s\S]*)\})");
      smatch matches;

      if (!regex_search(content, matches, vec_pattern)) {
          throw invalid_argument("Invalid vector format");
      }

      size__ = stoul(matches[1]);
      string vector_content = matches[2];

      elements__ = new element_t[size__];

      // Extract numbers from vector content
      regex number_pattern(R"(\(([-+]?[0-9]*\.?[0-9]+),([-+]?[0-9]*\.?[0-9]+)\)|[-+]?[0-9]*\.?[0-9]+)");
      auto numbers_begin = sregex_iterator(vector_content.begin(), vector_content.end(), number_pattern);
      auto numbers_end = sregex_iterator();

      size_t idx = 0;
      for (auto i = numbers_begin; i != numbers_end; ++i) {
          if (idx >= size__) {
              throw invalid_argument("Too many numbers in vector file");
          }
          elements__[idx++] = parse_number<T>(i->str());
      }

      if (idx != size__) {
          throw invalid_argument("Not enough numbers in vector file");
      }
  }

  ~vector() { delete[] elements__; }

  vector &operator=(const vector &v) // copy assigment
  {
    if (size__ != v.size__) {
      delete[] elements__;
      elements__ = ((v.size__) ? new element_t[v.size()] : nullptr);
    }
    copy_elements__(v);
    return *this;
  }

  vector &operator=(vector &&v) // move assigment
  {
    delete[] elements__;

    elements__ = v.elements__;
    size__ = v.size__;

    v.elements__ = nullptr;
    v.size__ = 0;

    return *this;
  }

  size_t size() const { return size__; }

  // access element
  element_t at(size_t i) const {
    if (i >= size__) {
      ostringstream str_stream;
      str_stream << "out_of range exception (" << __func__ << "() in "
                 << __FILE__ << ":" << __LINE__ << ")";
      throw out_of_range(str_stream.str());
    }
    return elements__[i];
  }

  element_t &operator[](size_t i) { return elements__[i]; }
  const element_t &operator[](size_t i) const { return elements__[i]; }

  bool operator==(const vector &v) const {
    if (size__ != v.size__)
      return false;

    for (size_t i = 0; i < size__; i++)
      if (elements__[i] != v.elements__[i])
        return false;

    return true;
  }

  vector &operator+=(const vector &v) {
    if (size__ != v.size__) {
      ostringstream str_stream;
      str_stream << "size mismatch! cannot add vectors (" << __func__
                 << "() in " << __FILE__ << ":" << __LINE__ << ")";
      throw invalid_argument(str_stream.str());
    }

    for (size_t i = 0; i < size__; i++)
      elements__[i] += v.elements__[i];

    return *this;
  }

  vector &operator-=(const vector &v) {
    if (size__ != v.size__) {
      ostringstream str_stream;
      str_stream << "size mismatch! cannot subtract vectors (" << __func__
                 << "() in " << __FILE__ << ":" << __LINE__ << ")";
      throw invalid_argument(str_stream.str());
    }

    for (size_t i = 0; i < size__; i++)
      elements__[i] -= v.elements__[i];

    return *this;
  }

  vector &operator*=(element_t scalar) {
    for (size_t i = 0; i < size__; i++)
      elements__[i] *= scalar;

    return *this;
  }

  vector &operator/=(element_t scalar) {
    if (scalar == 0) {
      ostringstream str_stream;
      str_stream << "divide by zero! bad scalar provided (" << __func__
                 << "() in " << __FILE__ << ":" << __LINE__ << ")";
      throw invalid_argument(str_stream.str());
    }

    for (size_t i = 0; i < size__; i++)
      elements__[i] /= scalar;

    return *this;
  }

  void save_as(const string &file_name) const {
    ofstream ofs(file_name);
    if (!ofs) {
      ostringstream str_stream;
      str_stream << "cannot open file \"" << file_name << "\"! (" << __func__
                 << "() in " << __FILE__ << ":" << __LINE__ << ")";
      throw logic_error(str_stream.str());
    }

    ofs<<(*this)<<endl;
  }

private:
  size_t size__;
  element_t *elements__;

  bool check_initilizer_list__(const initializer_list<element_t> &init) {
    for (size_t i = 0; i < init.size(); i++)
      if (init.begin()[i] != 0)
        return false;
    return true;
  }

  void copy_elements__(const vector &v) {
    size__ = v.size__;
    for (size_t i = 0; i < size__; i++)
      elements__[i] = v.elements__[i];
  }
};

template <typename T>
inline vector<T> operator+(const vector<T> &a, const vector<T> &b) {
  vector c{a};
  return c += b;
}

template <typename T>
inline vector<T> operator-(const vector<T> &a, const vector<T> &b) {
  vector c{a};
  return c -= b;
}

template <typename T>
inline vector<T> operator*(const vector<T> &v,
                           typename vector<T>::element_t scalar) {
  vector c{v};
  return c *= scalar;
}

template <typename T>
inline vector<T> operator*(typename vector<T>::element_t scalar,
                           const vector<T> &v) {
  return v * scalar;
}

template <typename T> inline vector<T> operator-(const vector<T> &v) {
  return v * -1;
}

template <typename T>
vector<T> operator*(const matrix<T> &m, const vector<T> &v) {
  if (m.columns() != v.size()) {
    ostringstream str_stream;
    str_stream << "size mismatch! cannot multiply matrix and vector ("
               << __func__ << "() in " << __FILE__ << ":" << __LINE__ << ")";
    throw invalid_argument(str_stream.str());
  }

  vector<T> result(m.rows());
  for (size_t i = 0; i < m.rows(); i++) {
    const vector<T> row = m.get_row(i);
    result[i] = dot(row, v);
  }

  return result;
}

template <typename T>
vector<T> operator*(const vector<T> &v, const matrix<T> &m) {
  if (v.size() != m.rows()) {
    ostringstream str_stream;
    str_stream << "size mismatch! cannot multiply vector and matrix ("
               << __func__ << "() in " << __FILE__ << ":" << __LINE__ << ")";
    throw invalid_argument(str_stream.str());
  }

  vector<T> result(m.columns());
  for (size_t j = 0; j < m.columns(); j++) {
    const vector<T> column = m.get_column(j);
    result[j] = dot(v, column);
  }

  return result;
}

template <typename T>
typename vector<T>::element_t dot(const vector<T> &a, const vector<T> &b) {
  if (a.size() != b.size()) {
    ostringstream str_stream;
    str_stream << "size mismatch! cannot calculate dot product (" << __func__
               << "() in " << __FILE__ << ":" << __LINE__ << ")";
    throw invalid_argument(str_stream.str());
  }

  typename vector<T>::element_t result = 0;
  for (size_t i = 0; i < a.size(); i++) {
    result += a[i] * b[i];
  }

  return result;
}

template <typename T> ostream &operator<<(ostream &os, const vector<T> &v) {
  os << "mat_lib::vector[" << v.size() << "]{";
  for (size_t i = 0; i < v.size() - 1; i++)
    os << v[i] << ", ";
  if (v.size() > 0)
    os << v[v.size() - 1];
  os << "}";
  return os;
}
} // namespace mat_lib

#endif // VECTOR_HPP