#include "LambdaCalculation.h"
#include "Debug.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <map>
#include <numeric>
#include <vector>

#define LambdaCalculation_DEBUG 1

// ===== HELPER STRUCTURES =====

struct Probs {
  int alphabet_size;
  std::vector<double> values;

  Probs(const int alphabet_size, bool uniform_init = false)
      : alphabet_size(alphabet_size), values(alphabet_size, 0.0) {
    if (uniform_init) {
      double uniform_prob = 1.0 / static_cast<double>(alphabet_size);
      std::fill(values.begin(), values.end(), uniform_prob);
    }
  }

  double &operator[](int i) { return values[i]; }
  const double &operator[](int i) const { return values[i]; }
};

struct Matrix {
  int row_dim;
  int col_dim;
  std::vector<double> values;

  Matrix(const int row_dim, const int col_dim)
      : row_dim(row_dim), col_dim(col_dim), values(row_dim * col_dim, 0.0) {}

  double &at(int i, int j) { return values[i * col_dim + j]; }
  const double &at(int i, int j) const { return values[i * col_dim + j]; }

  void copy_from(const double **mat) {
    for (int i = 0; i < row_dim; ++i) {
      for (int j = 0; j < col_dim; ++j) {
        at(i, j) = mat[i][j];
      }
    }
  }

  bool validate() const {
    if (row_dim <= 0 || col_dim <= 0)
      return false;
    if (values.size() != static_cast<size_t>(row_dim * col_dim))
      return false;
    return std::none_of(values.begin(), values.end(), [](double v) {
      return std::isnan(v) || std::isinf(v);
    });
  }

  // Fill this matrix with the joint probability distribution p[i] * q[j]
  // Assumes this matrix is already sized to (p.alphabet_size, q.alphabet_size)
  void fill_joint_from_probs(const Probs &p, const Probs &q) {
    for (int i = 0; i < row_dim; ++i) {
      for (int j = 0; j < col_dim; ++j) {
        this->at(i, j) = p[i] * q[j];
      }
    }
  }

  // Function to compute the initial score distribution Ps
  // Given a score matrix S (this), and probability vectors p and q,
  // returns a map from score value s to probability Ps
  std::map<int, double>
  get_score_distribution(const std::vector<double> &p,
                         const std::vector<double> &q) const {
    std::map<int, double> score_dist;
    // Find min and max score values (assume integer scores)
    int min_score = static_cast<int>(std::floor(values[0]));
    int max_score = static_cast<int>(std::ceil(values[0]));
    for (int i = 0; i < row_dim; ++i) {
      for (int j = 0; j < col_dim; ++j) {
        int s = static_cast<int>(std::round(at(i, j)));
        if (s < min_score)
          min_score = s;
        if (s > max_score)
          max_score = s;
      }
    }
    // For each (i, j), accumulate p[i] * q[j] for the corresponding score s
    double total = 0.0;
    for (int i = 0; i < row_dim; ++i) {
      for (int j = 0; j < col_dim; ++j) {
        int s = static_cast<int>(std::round(at(i, j)));
        double prob = p[i] * q[j];
        score_dist[s] += prob;
        total += prob;
      }
    }
    // Normalize so that the sum of probabilities is 1.0
    if (total > 0.0) {
      for (auto &kv : score_dist) {
        kv.second /= total;
      }
    }
    return score_dist;
  }

  double calculate_determinant(void) {
    // Only for square matrices
    if (row_dim != col_dim)
      return 0.0;
    int n = row_dim;
    // Make a copy of the matrix values for row operations
    std::vector<double> mat(values);
    double det = 1.0;
    int sign = 1;

    for (int i = 0; i < n; ++i) {
      // Find pivot
      int pivot = i;
      for (int j = i + 1; j < n; ++j) {
        if (std::abs(mat[j * n + i]) > std::abs(mat[pivot * n + i])) {
          pivot = j;
        }
      }
      if (std::abs(mat[pivot * n + i]) < 1e-15) {
        return 0.0;
      }
      if (pivot != i) {
        // Swap rows
        for (int k = 0; k < n; ++k) {
          std::swap(mat[i * n + k], mat[pivot * n + k]);
        }
        sign *= -1;
      }
      det *= mat[i * n + i];
      // Eliminate below
      for (int j = i + 1; j < n; ++j) {
        double factor = mat[j * n + i] / mat[i * n + i];
        for (int k = i; k < n; ++k) {
          mat[j * n + k] -= factor * mat[i * n + k];
        }
      }
    }
    return det * sign;
  }

  void print_debug() const {
    // Debug output now handled by LambdaDebug::log_matrix_info
    // This method is kept for backward compatibility
  }
};

struct ScoreGroup {
  double score;
  double prob_sum;

  ScoreGroup(double s, double p) : score(s), prob_sum(p) {}
};

// ===== DEBUG HELPER =====

class LambdaDebug {
private:
    static const bool enabled = LambdaCalculation_DEBUG;
    
    // Helper to print variadic arguments recursively (C++11/14 compatible)
    template<typename T>
    static void print_args(T&& arg) {
        if (enabled) {
            Debug(Debug::ERROR) << arg;
        }
    }
    
    template<typename T, typename... Args>
    static void print_args(T&& first, Args&&... rest) {
        if (enabled) {
            Debug(Debug::ERROR) << first;
            print_args(rest...);
        }
    }
    
public:
    template<typename... Args>
    static void log(Args&&... args) {
        if (enabled) {
            Debug(Debug::ERROR) << "Lambda: ";
            print_args(args...);
            Debug(Debug::ERROR) << "\n";
        }
    }
    
    static void log_matrix_info(const Matrix& matrix, const std::string& name) {
        if (enabled) {
            log(name, " matrix (first 5x5):");
            for (int i = 0; i < std::min(5, matrix.row_dim); ++i) {
                for (int j = 0; j < std::min(5, matrix.col_dim); ++j) {
                    Debug(Debug::ERROR) << matrix.at(i, j) << "\t";
                }
                Debug(Debug::ERROR) << "\n";
            }
        }
    }
    
    static void log_probabilities(const Probs& probs, const std::string& name, int count = 5) {
        if (enabled) {
            Debug(Debug::ERROR) << "Lambda: " << name << " (first " << count << "): ";
            for (int i = 0; i < std::min(count, probs.alphabet_size); ++i) {
                Debug(Debug::ERROR) << probs[i] << " ";
            }
            Debug(Debug::ERROR) << "\n";
        }
    }
    
    static void log_probability_changes(const std::vector<double>& old_probs, const Probs& new_probs, 
                                       const std::string& name, int count = 5) {
        if (enabled) {
            Debug(Debug::ERROR) << "Lambda: " << name << " changes (first " << count << "): ";
            for (int i = 0; i < std::min(count, new_probs.alphabet_size); ++i) {
                double change = new_probs[i] - old_probs[i];
                Debug(Debug::ERROR) << name << "[" << i << "]:" << old_probs[i] 
                                   << "->" << new_probs[i] << "(Δ=" << change << ") ";
            }
            Debug(Debug::ERROR) << "\n";
        }
    }
    
    static void log_joint_sample(const Matrix& joint_probs, int size = 3) {
        if (enabled) {
            log("Sample joint probabilities (first ", size, "x", size, "):");
            for (int i = 0; i < std::min(size, joint_probs.row_dim); ++i) {
                for (int j = 0; j < std::min(size, joint_probs.col_dim); ++j) {
                    Debug(Debug::ERROR) << joint_probs.at(i, j) << "\t";
                }
                Debug(Debug::ERROR) << "\n";
            }
        }
    }
    
    static bool validate_probability_distribution(const Probs& probs, const std::string& name) {
        double sum = std::accumulate(probs.values.begin(), probs.values.end(), 0.0);
        bool valid = true;
        
        if (enabled) {
            // Check sum is approximately 1.0
            if (std::abs(sum - 1.0) > 1e-10) {
                log(name, " sum=", sum, " (should be 1.0)");
                valid = false;
            }
            
            // Check all values are non-negative and finite
            for (int i = 0; i < probs.alphabet_size; ++i) {
                if (probs[i] < 0.0) {
                    log(name, "[", i, "]=", probs[i], " (should be >= 0)");
                    valid = false;
                }
                if (std::isnan(probs[i]) || std::isinf(probs[i])) {
                    log(name, "[", i, "] is NaN or Inf");
                    valid = false;
                }
            }
            
            if (valid) {
                log(name, " is valid");
            }
        }
        
        return valid;
    }
};

// ===== MATRIX UTILITIES =====



bool matrix_solvable_for_lambda(const Matrix &mat, double &lambda_upper_bound) {
  LambdaDebug::log("Checking matrix solvability");

  // Check for zero rows/columns
  std::vector<double> row_sums(mat.row_dim, 0.0);
  std::vector<double> col_sums(mat.col_dim, 0.0);

  for (int i = 0; i < mat.row_dim; ++i) {
    for (int j = 0; j < mat.col_dim; ++j) {
      row_sums[i] += mat.at(i, j);
      col_sums[j] += mat.at(i, j);
    }
  }

  // Build row sums string for debugging
  std::string row_sums_str;
  for (int i = 0; i < std::min(5, mat.row_dim); ++i) {
    row_sums_str += std::to_string(row_sums[i]) + " ";
  }
  LambdaDebug::log("Row sums (first 5): ", row_sums_str);
  
  // Build col sums string for debugging
  std::string col_sums_str;
  for (int i = 0; i < std::min(5, mat.col_dim); ++i) {
    col_sums_str += std::to_string(col_sums[i]) + " ";
  }
  LambdaDebug::log("Col sums (first 5): ", col_sums_str);

  if (std::any_of(row_sums.begin(), row_sums.end(),
                  [](double sum) { return std::abs(sum) < 1e-10; }) ||
      std::any_of(col_sums.begin(), col_sums.end(),
                  [](double sum) { return std::abs(sum) < 1e-10; })) {
    LambdaDebug::log("Matrix has zero row/column sums");
    return false;
  }

  // Check that each row and column has both positive and negative values
  std::vector<double> row_maxes(mat.row_dim, 0.0);
  std::vector<double> col_maxes(mat.col_dim, 0.0);

  for (int i = 0; i < mat.row_dim; ++i) {
    bool row_pos = false, row_neg = false;
    for (int j = 0; j < mat.col_dim; ++j) {
      double val = mat.at(i, j);
      if (val > 0.0) {
        row_pos = true;
        row_maxes[i] = std::max(row_maxes[i], val);
        col_maxes[j] = std::max(col_maxes[j], val);
      } else if (val < 0.0) {
        row_neg = true;
      }
    }
    if (!(row_pos && row_neg)) {
      LambdaDebug::log("Row ", i, " lacks both positive and negative values");
      return false;
    }
  }

  // Check columns
  for (int j = 0; j < mat.col_dim; ++j) {
    bool col_pos = false, col_neg = false;
    for (int i = 0; i < mat.row_dim; ++i) {
      double val = mat.at(i, j);
      if (val > 0.0)
        col_pos = true;
      else if (val < 0.0)
        col_neg = true;
    }
    if (!(col_pos && col_neg)) {
      LambdaDebug::log("Column ", j, " lacks both positive and negative values");
      return false;
    }
  }

  lambda_upper_bound =
      std::min(*std::min_element(row_maxes.begin(), row_maxes.end()),
               *std::min_element(col_maxes.begin(), col_maxes.end()));

  LambdaDebug::log("Computed lambda_upper_bound=", lambda_upper_bound);
  LambdaDebug::log("Matrix is solvable");

  return true;
}

// ===== SCORING FUNCTIONS =====

std::vector<ScoreGroup> group_scores_by_value(const Matrix &score_matrix,
                                              const Probs &p, const Probs &q) {
  std::map<double, double> score_to_prob;
  const double tolerance = 1e-10;

  for (int i = 0; i < score_matrix.row_dim; ++i) {
    for (int j = 0; j < score_matrix.col_dim; ++j) {
      double score_val = score_matrix.at(i, j);

      // Find existing score within tolerance or create new one
      bool found = false;
      for (auto &pair : score_to_prob) {
        if (std::abs(pair.first - score_val) < tolerance) {
          pair.second += p[i] * q[j];
          found = true;
          break;
        }
      }

      if (!found) {
        score_to_prob[score_val] += p[i] * q[j];
      }
    }
  }

  std::vector<ScoreGroup> score_groups;
  for (const auto &pair : score_to_prob) {
    score_groups.emplace_back(pair.first, pair.second);
  }

  return score_groups;
}

double restriction_value_grouped(const std::vector<ScoreGroup> &score_groups,
                                 double lambda) {
  double sum = 0.0;
  for (const auto &group : score_groups) {
    sum += group.prob_sum * std::exp(lambda * group.score);
  }
  return sum - 1.0;
}

// New function: operates directly on the matrix and probability vectors
double restriction_value_matrix(const Matrix &score_matrix, const Probs &p,
                                const Probs &q, double lambda) {
  double sum = 0.0;
  for (int i = 0; i < score_matrix.row_dim; ++i) {
    for (int j = 0; j < score_matrix.col_dim; ++j) {
      sum += p[i] * q[j] * std::exp(lambda * score_matrix.at(i, j));
    }
  }
  return sum - 1.0;
}

double restriction_value_derivative_grouped(
    const std::vector<ScoreGroup> &score_groups, double lambda) {
  double sum = 0.0;
  for (const auto &group : score_groups) {
    sum += group.prob_sum * group.score * std::exp(lambda * group.score);
  }
  return sum;
}

// New function: operates directly on the matrix and probability vectors
double restriction_value_derivative_matrix(const Matrix &score_matrix,
                                           const Probs &p, const Probs &q,
                                           double lambda) {
  double sum = 0.0;
  for (int i = 0; i < score_matrix.row_dim; ++i) {
    for (int j = 0; j < score_matrix.col_dim; ++j) {
      sum += p[i] * q[j] * score_matrix.at(i, j) *
             std::exp(lambda * score_matrix.at(i, j));
    }
  }
  return sum;
}

// ===== LAMBDA CALCULATION =====

// Update the joint probability matrix q using marginals p, q, score matrix S, and lambda
void updateJoint(const Matrix &S,
    const Probs &p,
    const Probs &q,
    const double lambda,
    Matrix &joint_q) {
  int n = S.row_dim;
  double total = 0.0;
  
  LambdaDebug::log("updateJoint: lambda=", lambda);
  LambdaDebug::log("Computing joint probabilities with exp(lambda * S[i,j])");

  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < n; ++j) {
      joint_q.at(i, j) = p[i] * q[j] * std::exp(lambda * S.at(i, j));
      total += joint_q.at(i, j);
    }
  }

  LambdaDebug::log("Total before normalization=", total);

  // Normalize
  if (total > 0.0) {
    for (int i = 0; i < n; ++i) {
      for (int j = 0; j < n; ++j) {
        joint_q.at(i, j) /= total;
      }
    }
  } else {
    LambdaDebug::log("WARNING - Total is zero, cannot normalize!");
  }

  LambdaDebug::log_joint_sample(joint_q);
}

// Update marginal probabilities p and q from the joint probability matrix q_mat
void updateMarginals(const Matrix& joint_probs, Probs& p, Probs& q) {
  int n = joint_probs.row_dim;
  
  LambdaDebug::log("Computing marginal probabilities from joint distribution");
  
  // Store old values for comparison
  std::vector<double> old_p = p.values;
  std::vector<double> old_q = q.values;

  // Update p[i] = sum_j q_mat(i, j)
  for (int i = 0; i < n; ++i) {
    p[i] = std::accumulate(
      joint_probs.values.begin() + i * n,
      joint_probs.values.begin() + (i + 1) * n,
      0.0
    );
  }
  
  // Update q[j] = sum_i q_mat(i, j)
  for (int j = 0; j < n; ++j) {
    double sum = 0.0;
    for (int i = 0; i < n; ++i) {
      sum += joint_probs.values[i * n + j];
    }
    q[j] = sum;
  }

  // Validate that probabilities sum to 1.0
  double p_sum = std::accumulate(p.values.begin(), p.values.end(), 0.0);
  double q_sum = std::accumulate(q.values.begin(), q.values.end(), 0.0);
  
  LambdaDebug::log("p_sum=", p_sum, ", q_sum=", q_sum);
  
  if (std::abs(p_sum - 1.0) > 1e-10 || std::abs(q_sum - 1.0) > 1e-10) {
    LambdaDebug::log("WARNING - Marginal probabilities don't sum to 1.0!");
  }
  
  LambdaDebug::log_probability_changes(old_p, p, "p");
  LambdaDebug::log_probability_changes(old_q, q, "q");
}


bool bracket_lambda(Matrix score_matrix, const Probs p, const Probs q,
                    double &lambda_low, double &lambda_high,
                    double lambda_upper_bound) {
  // For a proper scoring matrix, f(0) = 0 and f'(0) < 0
  // So we need to find a small positive lambda where f(lambda) > 0

  double f_zero = restriction_value_matrix(score_matrix, p, q, 0.0);

  LambdaDebug::log("f(0) = ", f_zero);

  // Check that we're close to f(0) = 0 (within tolerance)
  if (std::abs(f_zero) > 1e-10) {
    LambdaDebug::log("f(0) = ", f_zero, " is not close to 0, matrix may not be properly normalized");
    return false;
  }

  // Check the derivative at 0 (should be negative for a proper scoring matrix)
  double f_prime_zero =
      restriction_value_derivative_matrix(score_matrix, p, q, 0.0);

  LambdaDebug::log("f'(0) = ", f_prime_zero);

  if (f_prime_zero >= 0) {
    LambdaDebug::log("f'(0) = ", f_prime_zero, " >= 0, expected negative derivative");
    return false;
  }

  // Start with lambda_low = 0 and find lambda_high where f(lambda_high) > 0
  lambda_low = 0.0;
  lambda_high = 0.01; // Start with a small positive value

  const double increment_factor = 1.5;
  const int max_attempts = 50;

  for (int i = 0; i < max_attempts; ++i) {
    double f_high = restriction_value_matrix(score_matrix, p, q, lambda_high);

    LambdaDebug::log("Trying lambda_high = ", lambda_high, ", f(lambda_high) = ", f_high);

    if (f_high > 0.0) {
      LambdaDebug::log("Successfully bracketed lambda: [", lambda_low, ", ", lambda_high, "]");
      return true;
    }

    lambda_high *= increment_factor;

    // Don't exceed the theoretical upper bound
    if (lambda_high > lambda_upper_bound * 0.95) {
      lambda_high = lambda_upper_bound * 0.95;
      double f_high_final =
          restriction_value_matrix(score_matrix, p, q, lambda_high);

      LambdaDebug::log("At upper bound lambda_high = ", lambda_high, ", f(lambda_high) = ", f_high_final);

      if (f_high_final > 0.0) {
        LambdaDebug::log("Successfully bracketed lambda at upper bound: [", lambda_low, ", ", lambda_high, "]");
        return true;
      } else {
        LambdaDebug::log("Failed to bracket lambda even at upper bound");
        return false;
      }
    }
  }

  return false;
}

double find_lambda_bisection(const Matrix score_matrix, const Probs p,
                             const Probs q, double lambda_low,
                             double lambda_high, double epsilon = 1e-12,
                             int max_iters = 100) {
  LambdaDebug::log("Starting bisection: [", lambda_low, ", ", lambda_high, "]");

  for (int iter = 0; iter < max_iters; ++iter) {
    if (lambda_high - lambda_low < epsilon) {
      LambdaDebug::log("Converged: interval width = ", (lambda_high - lambda_low));
      break;
    }

    double mid = 0.5 * (lambda_low + lambda_high);
    double f_mid = restriction_value_matrix(score_matrix, p, q, mid);

    LambdaDebug::log("iter ", iter + 1, ": mid=", mid, " f(mid)=", f_mid);

    // Update bounds
    if (f_mid > 0.0) {
      lambda_high = mid;
    } else {
      lambda_low = mid;
    }

    if (std::abs(f_mid) < epsilon) {
      LambdaDebug::log("Converged: |f(mid)| = ", std::abs(f_mid));
      return mid;
    }
  }

  return 0.5 * (lambda_low + lambda_high);
}

// Newton-Raphson method for finding lambda
// Uses both function value and derivative for faster convergence
double find_lambda_newton_raphson(const Matrix score_matrix, const Probs p,
                                  const Probs q, double lambda_low,
                                  double lambda_high, double epsilon = 1e-12,
                                  int max_iters = 100) {
  LambdaDebug::log("Starting Newton-Raphson: initial bracket [", lambda_low, ", ", lambda_high, "]");

  // Start with midpoint of the bracket
  double lambda = 0.5 * (lambda_low + lambda_high);
  
  for (int iter = 0; iter < max_iters; ++iter) {
    double f_val = restriction_value_matrix(score_matrix, p, q, lambda);
    double f_prime = restriction_value_derivative_matrix(score_matrix, p, q, lambda);
    
    LambdaDebug::log("Newton-Raphson iter ", iter + 1, ": lambda=", lambda, " f(lambda)=", f_val, " f'(lambda)=", f_prime);

    // Check for convergence
    if (std::abs(f_val) < epsilon) {
      LambdaDebug::log("Newton-Raphson converged: |f(lambda)| = ", std::abs(f_val));
      return lambda;
    }
    
    // Check for zero derivative (would cause division by zero)
    if (std::abs(f_prime) < 1e-15) {
      LambdaDebug::log("Newton-Raphson: derivative too small, falling back to bisection");
      return find_lambda_bisection(score_matrix, p, q, lambda_low, lambda_high, epsilon, max_iters);
    }
    
    // Newton-Raphson step: lambda_new = lambda_old - f(lambda_old) / f'(lambda_old)
    double newton_step = -f_val / f_prime;
    double lambda_new = lambda + newton_step;
    
    LambdaDebug::log("Newton-Raphson: full step would be ", newton_step, " -> lambda=", lambda_new);
    
    // Use adaptive damping to stay within bounds
    double damping_factor = 1.0;
    if (lambda_new < lambda_low || lambda_new > lambda_high) {
      // Calculate damping factor to stay within bounds
      double max_allowed_step = 0.0;
      if (newton_step > 0) {
        max_allowed_step = (lambda_high - lambda) * 0.8;  // Stay 80% within bounds
      } else {
        max_allowed_step = (lambda_low - lambda) * 0.8;   // Stay 80% within bounds
      }
      
      if (std::abs(newton_step) > std::abs(max_allowed_step)) {
        damping_factor = max_allowed_step / newton_step;
      }
      
      LambdaDebug::log("Newton-Raphson: damping factor=", damping_factor);
    }
    
    lambda_new = lambda + damping_factor * newton_step;
    
    LambdaDebug::log("Newton-Raphson: damped step=", (damping_factor * newton_step), " -> lambda=", lambda_new);
    
    // Check for convergence in lambda
    if (std::abs(lambda_new - lambda) < epsilon) {
      LambdaDebug::log("Newton-Raphson converged: lambda change = ", std::abs(lambda_new - lambda));
      return lambda_new;
    }
    
    // Update bracket bounds to maintain the bracket property
    double f_new = restriction_value_matrix(score_matrix, p, q, lambda_new);
    if (f_new > 0.0) {
      lambda_high = lambda_new;
    } else {
      lambda_low = lambda_new;
    }
    
    lambda = lambda_new;
  }

  LambdaDebug::log("Newton-Raphson: Max iterations reached, final lambda=", lambda);

  return lambda;
}

// ===== PUBLIC INTERFACE =====

double calculate_lambda(const double **raw_mat_b, const int alpha_size,
                        std::vector<double> &final_p,
                        std::vector<double> &final_q, double eps,
                        const int min_em_iters, const int max_em_iters) {
  if (alpha_size <= 0) {
    LambdaDebug::log("Invalid alphabet size: ", alpha_size);
    return -1.0;
  }

  LambdaDebug::log("Starting lambda calculation with fixed background probabilities");
  LambdaDebug::log("alphabet_size=", alpha_size, ", eps=", eps);

  Matrix score_matrix(alpha_size, alpha_size);
  score_matrix.copy_from(raw_mat_b);
  
  // Validate and debug the score matrix
  if (!score_matrix.validate()) {
    LambdaDebug::log("Score matrix validation failed!");
    return -1.0;
  }
  
  LambdaDebug::log_matrix_info(score_matrix, "Score");
  
  // Check average score (should be negative for a proper scoring matrix)
  double avg_score = 0.0;
  for (int i = 0; i < alpha_size; ++i) {
    for (int j = 0; j < alpha_size; ++j) {
      avg_score += score_matrix.at(i, j);
    }
  }
  avg_score /= (alpha_size * alpha_size);
  LambdaDebug::log("Average score: ", avg_score);

  // Check if matrix is solvable
  double lambda_upper_bound = 0.0;
  if (!matrix_solvable_for_lambda(score_matrix, lambda_upper_bound)) {
    LambdaDebug::log("Matrix is not solvable for lambda");
    return -1.0;
  }
  if (lambda_upper_bound <= 0.0) {
    LambdaDebug::log("Invalid lambda upper bound: ", lambda_upper_bound);
    return -1.0;
  }

  LambdaDebug::log("Lambda upper bound: ", lambda_upper_bound);

  // Initialize background probabilities as uniform (fixed, not updated)
  Probs p(alpha_size, true);  // uniform initialization
  Probs q(alpha_size, true);  // uniform initialization
  
  LambdaDebug::log("Using uniform background probabilities:");
  LambdaDebug::log_probabilities(p, "p (uniform)");
  LambdaDebug::log_probabilities(q, "q (uniform)");
  
  // Validate initial probabilities
  if (!LambdaDebug::validate_probability_distribution(p, "background p") ||
      !LambdaDebug::validate_probability_distribution(q, "background q")) {
    LambdaDebug::log("Invalid background probabilities");
    return -1.0;
  }

  // Find lambda that satisfies the restriction equation:
  // sum_{i,j} p[i] * q[j] * exp(lambda * S[i,j]) = 1
  
  // First, bracket lambda
  double lambda_low, lambda_high;
  if (!bracket_lambda(score_matrix, p, q, lambda_low, lambda_high, lambda_upper_bound)) {
    LambdaDebug::log("Failed to bracket lambda");
    return -1.0;
  }

  LambdaDebug::log("Lambda bracket: [", lambda_low, ", ", lambda_high, "]");

  // Find lambda using Newton-Raphson method
  double lambda = find_lambda_newton_raphson(score_matrix, p, q, lambda_low, lambda_high, eps);
  
  LambdaDebug::log("Final lambda=", lambda);

  // Validate final result
  double final_restriction = restriction_value_matrix(score_matrix, p, q, lambda);
  LambdaDebug::log("Final restriction value: ", final_restriction);
  
  if (std::abs(final_restriction) > 1e-6) {
    LambdaDebug::log("Failed to satisfy restriction equation, |f(lambda)| = ", std::abs(final_restriction));
    return -1.0;
  }

  if (lambda <= 0.0) {
    LambdaDebug::log("Lambda is non-positive: ", lambda);
    return -1.0;
  }

  // Return the uniform background probabilities
  final_p = p.values;
  final_q = q.values;
  
  LambdaDebug::log("Lambda calculation successful");
  LambdaDebug::log("Final lambda=", lambda);
  LambdaDebug::log("Background probabilities are uniform: 1/", alpha_size, " = ", 1.0/alpha_size);
  
  return lambda;
}
